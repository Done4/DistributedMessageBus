#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <cstring>
#include "Message.h"
#include "MessageSerialize.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

using namespace std;

struct Client {
    int socket;
    int moduleId;
};

vector<Client> clients;
mutex clientsMutex;

void broadcastMessage(const Message& msg, int senderSocket) {
    size_t outLen = 0;
    uint8_t* buffer = MessageSerialize::serialize(msg, outLen);
    if (!buffer) return;

    lock_guard<mutex> lock(clientsMutex);
    for (auto& client : clients) {
        // 转发给除了发送者之外的所有客户端
        if (client.socket != senderSocket) {
            send(client.socket, (const char*)buffer, (int)outLen, 0);
        }
    }
    free(buffer);
}

void handleClient(int clientSocket) {
    char buffer[4096];
    int n = 0;
    int myModuleId = -1;

    while ((n = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        Message msg = MessageSerialize::deserialize((uint8_t*)buffer, n);
        
        if (msg.header.type == -1) { // 握手消息
            myModuleId = msg.header.sourceModuleId;
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back({clientSocket, myModuleId});
            cout << "[Broker] New client registered: Module " << myModuleId << endl;
        } else {
            cout << "[Broker] Routing message type " << msg.header.type 
                 << " from Module " << msg.header.sourceModuleId << endl;
            broadcastMessage(msg, clientSocket);
        }
        MessageFree(&msg);
    }

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif

    lock_guard<mutex> lock(clientsMutex);
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it->socket == clientSocket) {
            cout << "[Broker] Client Module " << it->moduleId << " disconnected." << endl;
            clients.erase(it);
            break;
        }
    }
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    int port = 10002;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    listen(serverFd, 10);
    cout << "[Broker] Message Center started on port " << port << endl;

    while (true) {
        int clientFd = accept(serverFd, nullptr, nullptr);
        if (clientFd < 0) continue;
        thread(handleClient, clientFd).detach();
    }

    return 0;
}
