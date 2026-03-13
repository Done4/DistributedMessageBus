#ifndef NETWORKSERVICEMANAGER_H
#define NETWORKSERVICEMANAGER_H

#include "Message.h"
#include "MessageSerialize.h"
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <condition_variable>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

class MsgMiddleware; // forward declaration to avoid circular include

class NetworkServiceManager {
public:
    NetworkServiceManager(int moduleId, MsgMiddleware* middleware);
    ~NetworkServiceManager();

    void start();

    // Add a peer for auto-connection and discovery
    void addPeer(const char* ip, int port);

    // send message (unicast or broadcast)
    void sendMessage(const Message& msg, bool broadcast);

    // send reply to a specific module
    void sendReply(int targetModuleId, const Message& reply);

private:
    void sendToSocket(int sock, const Message& msg);
    void serverLoop();
    int connectToNode(const char* ip, int port);
    void handleNewConnection(int sock);

private:
    int moduleId_;
    MsgMiddleware* middleware_;
    std::thread serverThread_;
    std::mutex connMutex_;
    std::unordered_map<int, int> moduleConnections_; // moduleId -> TCP socket
    std::vector<std::pair<std::string, int>> peerAddresses_; // List of (IP, Port) for auto-discovery
};

#endif // NETWORKSERVICEMANAGER_H
