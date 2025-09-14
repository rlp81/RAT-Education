// C++ program to show the example of server application in
// socket programming
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <ctime>
#include <random>
#include <map>
#include <vector>

using namespace std;

class Client {
private:
    int bufferSize = 512;
    char rbuffer[512] = { 0 };
    int sResult;
    int reResult;
public:
    string cliname;
    int clientSocket;

    int create(string name) {
        cliname = name;
        reResult = recv(clientSocket, rbuffer, bufferSize, 0);
        string resp = rbuffer;
        if (resp == "?connect") {
            return 0;
        }
        return 1;
    }

    int sendClient(string cmd) {
        cout << "Sending command..";
        const char* buffer = cmd.c_str();
        sResult = send(clientSocket, buffer, bufferSize, 0);
        reResult = recv(clientSocket, rbuffer, bufferSize, 0);
        string resp = rbuffer;
        if (resp[0] != '?') {
            cout << "Recieved output: " << resp << endl;
        }
        return 0;

    }
};

class Server {
private:

public:
    int serverSocket;
    bool quit = false;
    map<string, Client> clients;


    void startup() {
        // creating socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        cout << "Server initializing..\n";
        // specifying the address
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(5555);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        cout << "Server started.\n";
        // binding socket.
        bind(serverSocket, (struct sockaddr*)&serverAddress,
             sizeof(serverAddress));
        cout << "Listening for connections..\n";
        thread listeningThread(&Server::listener, this);
        listeningThread.detach();
        listClients();
    }

    int listener() {
        cout << "\nWaiting for clients..\n";
        while (quit == false) {
            listen(serverSocket, 5);

            // accepting connection request
            int clientSocket
                = accept(serverSocket, nullptr, nullptr);
            Client client;
            client.clientSocket = clientSocket;
            srand(time(0));
            string cliname = "client-" + to_string(rand());
            cout << "Client connected: " << cliname << endl;
            clients[cliname] = move(client);
            thread clientThread(&Client::create, &client, cliname);
            clientThread.detach();

        }
        return 0;
    }

    int listClients() {
        int connected = clients.size();
        vector<string> cList;
        int num = 0;
        int selected;
        while (connected <= 0) {
            if (clients.size() > 0) {
                connected = clients.size();
            }
        }
        cout << "Connected clients: " << connected << endl;
        for (map<string, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            num += 1;
            string client = it->first;
            cList.push_back(client);
            cout << num << ". " << client << endl;
        }

        cout << "Type the client number or 0 to broadcast: ";
        cin >> selected;
        selected -= 1;

        if (selected <= cList.size() && selected >= -1) {
            if (selected == -1) {} else {
                cout << "Selected Client: " << cList[selected] << endl;
                runCommand(clients[cList[selected]]);
                listClients();
            }
        } else {
            cout << "Invalid selection.\n";
            listClients();
        }
    }

    int runCommand(Client client) {
        string command;
        cout << "\nInput command: ";
        cin >> command;
        if (command != "?back") {
            int resp = client.sendClient(command);
            return 0;
        }
        return 1;
    }
};



int main()
{

    Server server;
    server.startup();

    return 0;
}