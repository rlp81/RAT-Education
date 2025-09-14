#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <thread>
#include <chrono>

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5555"

class socketClient {
private:
    bool quit = false;
    int iResult;
    int iSendResult;
    string aliveString = "?true";
public:
    bool active = false;
    SOCKET ClientSocket = INVALID_SOCKET;
    string clientName = "";

    int create(string name) {
        char buffer[DEFAULT_BUFLEN];
        clientName = name;
        iResult = recv(ClientSocket, &buffer[0], DEFAULT_BUFLEN, 0);
        string cppstring = buffer;
        cout << "Init: " << cppstring << endl;
        if (iResult > 0) {
            if (cppstring == "?connect") {
                active = true;
            }
        }
        else {
            if (iResult != 0 && iResult != -1) {
                printf("\nkeepalive: recv failed with error: %d\n", WSAGetLastError());
                active = false;
            }
        }
        //keepAlive();
        return 0;
        
    }

    int keepAlive() {
        cout << "Starting keep alive..\n";
        while (active == true) {
            string rcv;
            char buffer[DEFAULT_BUFLEN];
            iResult = recv(ClientSocket, &buffer[0], DEFAULT_BUFLEN, 0);
            string cppstring = buffer;
            if (iResult > 0) {
                if (cppstring[0] == '?') {
                    cout << "txt: " << cppstring << endl;
                    if (cppstring == "?false") {
                        cout << "Stopped\n";
                        active = false;
                    }
                }
            }
            else {
                if (iResult != 0 && iResult != -1) {
                    printf("\nkeepalive: recv failed with error: %d\n", WSAGetLastError());
                    active = false;
                }
            }
            iSendResult = send(ClientSocket, aliveString.c_str(), DEFAULT_BUFLEN, 0);
            if (iSendResult == SOCKET_ERROR) {
                if (WSAGetLastError() == 10054) {
                    cout << "dead?\n";
                    active = false;
                    return 1;
                }
                else {
                    printf("\nkeepalive: send failed with error: %d\n", WSAGetLastError());\
                    active = false;
                }
            }
            else {
                cout << "Sent alive\n";
                this_thread::sleep_for(chrono::seconds(5));
            }
        }
        cout << "Closing server..\n";
        closesocket(ClientSocket);
        WSACleanup();
        return 1;


        // shutdown the connection since we're done
        /*iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // cleanup
        closesocket(ClientSocket);
        WSACleanup();*/

        return 0;
    }
    int sendClient(string cmd) {
        const char* buffer = cmd.c_str();
        char rbuffer[DEFAULT_BUFLEN];
        cout << "Sending command..\n";
        iSendResult = send(ClientSocket, buffer, DEFAULT_BUFLEN, 0);
        if (iSendResult == SOCKET_ERROR) {
            if (WSAGetLastError() == 10054) {
                quit = true;
                cout << "\nClient disconnected\n";
                closesocket(ClientSocket);
                WSACleanup();
                return 2;
            }
            else {
                printf("\nsend failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        cout << "Recieving response..\n";
        iResult = recv(ClientSocket, &rbuffer[0], DEFAULT_BUFLEN, 0);
        if (iResult > 0) {
            string cppstring = rbuffer;
            if (cppstring[0] != '?') {
                cout << "\nOutput recieved:\n" << cppstring << endl;
            }
        }
        else {
            if (iResult != 0) {
                printf("\nrecv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                //quit = true;
                return 1;
            }
        }
        return 0;
    }
};

class socketHost {
private:
    WSADATA wsaData;
    int iResult;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

public:
    SOCKET ListenSocket = INVALID_SOCKET;
    bool quit = false;
    map<string, socketClient> currentSockets;
    map<string, thread> currentThreads;

    int startUp() {
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed with error: %d\n", iResult);
            return 1;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return 1;
        }
        runSocket();
    }

    int runSocket() {
        // Create a SOCKET for the server to listen for client connections.
        cout << "Server initilizing..\n";
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        cout << "Server initilized.\n";
        if (iResult == SOCKET_ERROR) {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        freeaddrinfo(result);
        thread serverListener(&socketHost::listener, this);
        serverListener.detach();
        listClients();
    }

    int listener() {
        do {
            iResult = listen(ListenSocket, SOMAXCONN);
            if (iResult == SOCKET_ERROR) {
                printf("listen failed with error: %d\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
            }
            socketClient clientSocket;


            SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);

            if (ClientSocket == INVALID_SOCKET) {
                if (WSAGetLastError() != 10004) {
                    printf("accept failed with error: %d\n", WSAGetLastError());
                    closesocket(ListenSocket);
                    WSACleanup();
                    return 1;
                }
            }
            srand(time(0));
            string cliname = "client-" + to_string(rand() % 124);
            clientSocket.clientName = cliname;
            clientSocket.ClientSocket = move(ClientSocket);
            currentSockets[cliname] = move(clientSocket);
            thread c(&socketClient::create, &clientSocket, cliname);
            c.detach();
            cout << "\nConnection made: " << cliname << c.get_id() << endl;
            //currentThreads[cliname] = move(c);
        } while (quit != true);
    }

    void listClients() {
        int connected = currentSockets.size();
        vector<string> clients;
        int num = 0;
        int selected;
        cout << "\nWaiting for clients..\n";
        while (connected <= 0) {
            if (currentSockets.size() > 0) {
                connected = currentSockets.size();
            }
        }
        cout << "\nConnected Clients: " << connected << endl;
        for (map<string, socketClient>::iterator it = currentSockets.begin(); it != currentSockets.end(); ++it) {
            num += 1;
            string client = it->first;
            clients.push_back(client);
            cout << num << ". " << client << endl;
        }
        
        cout << "Type the client number or 0 to broadcast: ";
        cin >> selected;
        selected -= 1;
        if (selected <= clients.size() && selected >= 0) {
            string csel = clients[selected];
            cout << "Selected client: " << csel << endl;
            socketClient sock = currentSockets[csel];
            int resp = requestCommand(sock);
            if (resp != 0) {
                currentSockets.erase(csel);
            }
            listClients();
        }
        else if (selected == -1) {
            cout << "broadcast\n";
        }
        else {
            cout << "Client " << selected << " does not exist.\n";
            listClients();
        }
    }

    int requestCommand(socketClient sock) {
        string command;
        cout << "Input command: ";
        getline(cin, command);
        cin >> command;
        int resp = sock.sendClient(command);
        return resp;
    }

};

int main() {
    socketHost host;
    host.startUp();

    return 0;
}