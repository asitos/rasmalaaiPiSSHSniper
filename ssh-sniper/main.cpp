#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <cstdlib>
using namespace std;

struct AttackTracker {
  int attempts = 0;
  chrono::steady_clock::time_point first_attempt;
};

void block_ip(const string& ip) {
  cout << "[!] Banning malicious IP: " << ip << endl;
  string command = "sudo iptables -A INPUT -s " + ip + " -j DROP";
  system(command.c_str());
}

int main() {
  string log_path = "/var/log/auth.log";
  ifstream log_file(log_path);

  if (!log_file.is_open()) {
    cerr << "[-] Error: Cannot open " << log_path << ". Did you run as sudo?" << endl;
    return 1;
  }

  cout << "[+] SSH Sniper Active. Watching auth logs..." << endl;

  log_file.seekg(0, ios::end);

  unordered_map<string, AttackTracker> bad_ips;
  string line;

  while (true) {
    while (getline(log_file, line)) {
      if (line.find("Failed password for") != string::npos) {
        size_t ip_pos = line.find("from ");
        if (ip_pos != string::npos) {
          string remaining = line.substr(ip_pos + 5);
          size_t space_pos = remaining.find(" ");
          string ip = remaining.substr(0, space_pos);

          auto now = chrono::steady_clock::now();

          if (bad_ips.find(ip) == bad_ips.end()) {
            bad_ips[ip] = {1, now};
          } else {
            bad_ips[ip].attempts++;
          }


          auto duration = chrono::duration_cast<chrono::seconds>(now - bad_ips[ip].first_attempt).count();
          cout << "[*] Failed attempt " << bad_ips[ip].attempts << " from " << ip << " (Window: " << duration << "s)" << endl;

          if (bad_ips[ip].attempts >= 3 && duration <= 30) {
            block_ip(ip);
            bad_ips.erase(ip);
          } else if (duration > 30){
            bad_ips[ip] = {1, now};
          }
        }
      }
    }

    log_file.clear();
    this_thread::sleep_for(chrono::milliseconds(200));
  }

  return 0;
}
