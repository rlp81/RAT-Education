# RAT-Education

**RAT-Education** is a simple Remote Administration Tool (RAT) written in C++ for educational purposes.  
It demonstrates the core concepts behind remote access trojans (RATs) – allowing a server to control and communicate with remote clients over a TCP connection.

> **Warning:**  
> This project is intended **solely for educational use**.  
> Do not use it for unauthorized access or control of any devices.  
> Always have explicit permission before running or connecting to any clients.

---

## Features

- **TCP Server**
  - Available for both **Windows** and **Linux**
  - Listens for connections on a configurable port (`5555` by default)
  - Handles multiple clients using threads
  - Interactive command prompt for sending commands to clients
  - "Broadcast" mode for sending commands to all clients
  - Periodic keep-alive messages to monitor client status

- **Client Handler**
  - Waits for a `?connect` handshake from new clients
  - Supports sending and receiving commands and responses
  - Detects client disconnects and cleans up resources

---

## Getting Started

### Prerequisites

- Windows or Linux OS
- C++ compiler (Visual Studio for Windows, g++/clang for Linux)
- Basic knowledge of C++ and networking

---

### Build Instructions

#### Windows Server

1. **Clone the repository**
   ```sh
   git clone https://github.com/rlp81/RAT-Education.git
   ```
2. **Open the project in Visual Studio or your preferred IDE**
3. **Ensure you link with `Ws2_32.lib`** (required for Winsock)
4. **Build the project**

#### Linux Server

1. **Clone the repository**
   ```sh
   git clone https://github.com/rlp81/RAT-Education.git
   ```
2. **Navigate to the server directory**
   ```sh
   cd RAT-Education/server
   ```
3. **Compile the Linux server**
   ```sh
   g++ -std=c++11 linux.cpp -o linux_server -lpthread
   ```
   - If you encounter errors related to missing headers or libraries, ensure you have build-essential installed:
     ```sh
     sudo apt-get install build-essential
     ```
4. **Run the Linux server**
   ```sh
   ./linux_server
   ```

---

## Usage

- When the server is running, connected clients will be listed in the console.
- You can select an individual client by number, or type `0` to broadcast a command to all clients.
- The server sends periodic keep-alive messages to ensure clients remain connected.

---

## Connecting Clients

- Clients should connect to the server's IP and port.
- Upon connecting, the client must send a `?connect` message to begin communication.

*Note: This repository does not include a client implementation. You may create your own client for learning purposes.*

---

## Code Overview

- **server/windows.cpp**  
  Implements the Windows TCP server and client management.
- **server/linux.cpp**  
  Implements the Linux TCP server and client management.
- Both use C++ standard library features (`thread`, `map`, `vector`, etc.)  
  Handle socket operations and command processing.

---

## Educational Notes

- This tool is a learning resource.  
- It does **not** include authentication, encryption, or advanced error handling.
- **Do not deploy** or use for malicious activity.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

## Disclaimer

This project is for educational purposes only.  
Do not use it for unauthorized access or control of computers.  
Always ensure you have permission to connect to and manage any client devices.

Written by GitHub Copilot
