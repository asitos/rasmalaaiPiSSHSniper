# rasmalaaiPiSSHSniper

a lightweight, zero-dependency intrusion prevention system (ips) written in native c++ for headless linux arm nodes. it intercepts ssh brute-force vectors in real-time by streaming systemd journal logs and dynamically manipulating the linux kernel's packet filtering tables.

---

## installation & how to run

no external dependencies needed. just compile and run it.

```bash
# clone this repo
git clone https://github.com/asitos/rasmalaaiPiSSHSniper.git
cd ./rasmalaaiPiSSHSniper

make

chmod +x ssh-sniper/ssh-sniper

./ssh-sniper/ssh-sniper

# clean up build artifacts
make clean
```

## directory structure

```text
├──  Makefile
├──  README.md
└──  ssh-sniper
    ├──  main.cpp
    └──  ssh-sniper
```

## technical architecture

unlike traditional python-based tools or shell scripts that poll log files at regular intervals, `rasmalaaiPiSSHSniper` leverages low-level linux systems apis for maximum performance and a negligible cpu footprint.

### 1. event-driven logging via posix pipes (`popen`)
instead of parsing a flat `/var/log/auth.log` file using an active polling loop (which wastes cpu cycles and introduces file-io bottlenecks), this engine utilizes a posix pipe (`popen`) to hook directly into the `systemd-journald` binary stream:
* **zero-cpu idle state:** the execution blocks at the os level inside `fgets` until the kernel flushes a new log frame. 
* **targeted filtering:** the engine initiates `journalctl` with flags restricted to the ssh daemon service (`-u ssh`), parsing only relevant auth frames.

### 2. kernel-space packet filtering (`iptables`)
when an ip crosses the threat threshold, the program executes a system call to hook directly into the linux packet-filtering framework:
* **driver-level dropping:** malicious packets are dropped at the interface level (`-j drop`) before they can traverse higher up the tcp/ip stack to consume user-space authentication daemon memory.

---

## state & detection logic

```text
               +-----------------------------+
               |  systemd-journald event     |
               +--------------+--------------+
                              |
                     [popen stdout pipe]
                              |
               +--------------v--------------+
               |  regex / pattern matching   |
               +--------------+--------------+
                              |
                     [failed password?]
                    /                 \
                  yes                  no  --> [ignore]
                  /
   +-------------v-------------+
   |  extract ip address       |
   +-------------+-------------+
                 |
   +-------------v-------------+
   |  check/update hash map    |  <-- tracks ip & window time
   +-------------+-------------+
                 |
         [attempts >= 3?]
        /                \
      yes                 no  --> [update counter & sleep]
      /
+----v--------------------+
| execute `iptables drop` |  <-- dynamic firewall rule append
+-------------------------+
```
