CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = ./ssh-sniper/ssh-sniper
SRC = ./ssh-sniper/main.cpp

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "[+] Compiling $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	@echo "[+] Build complete."

clean:
	@echo "[-] Cleaning up..."
	rm -f $(TARGET)

install: $(TARGET)
	@echo "[+] Installing $(TARGET) to /usr/local/bin..."
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)

uninstall:
	@echo "[-] Removing $(TARGET) from /usr/local/bin..."
	rm -f /usr/local/bin/$(TARGET)
