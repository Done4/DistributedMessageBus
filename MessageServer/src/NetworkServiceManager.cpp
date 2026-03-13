#include "NetworkServiceManager.h"
#include "MsgMiddleware.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

NetworkServiceManager::NetworkServiceManager(int moduleId, MsgMiddleware* middleware)
    : moduleId_(moduleId), middleware_(middleware) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
}

NetworkServiceManager::~NetworkServiceManager() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void NetworkServiceManager::start() {
    serverThread_ = std::thread([this](){ this->serverLoop(); });
    serverThread_.detach();
}

void NetworkServiceManager::addPeer(const char* ip, int port) {
    if (!ip) return;
    std::lock_guard<std::mutex> lock(connMutex_);
    peerAddresses_.push_back({std::string(ip), port});
    
    // Broker 模式下，我们通常只连接一个中心节点
    if (moduleConnections_.empty()) {
        connectToNode(ip, port);
    }
}

void NetworkServiceManager::sendMessage(const Message& msg, bool broadcast) {
    std::lock_guard<std::mutex> lock(connMutex_);
    
    // 如果没有活跃连接，则尝试连接到所有已知的 Peer (Broker)
    if (moduleConnections_.empty()) {
        for (const auto& peer : peerAddresses_) {
            connectToNode(peer.first.c_str(), peer.second);
        }
    }

    // 发送给所有已建立的连接（在 Broker 模式下通常只有一个连接）
    for (auto& kv : moduleConnections_) {
        sendToSocket(kv.second, msg);
    }
}

void NetworkServiceManager::sendReply(int targetModuleId, const Message& reply) {
    // 在 Broker 模式下，回复也是发送给 Broker，由 Broker 路由给目标
    sendMessage(reply, false);
}

void NetworkServiceManager::sendToSocket(int sock, const Message& msg) {
    size_t outLen = 0;
    uint8_t* buffer = MessageSerialize::serialize(msg, outLen);
    if (!buffer) return;

#ifdef _WIN32
    int n = ::send(sock, (const char*)buffer, (int)outLen, 0);
#else
    int n = ::send(sock, (const char*)buffer, outLen, 0);
#endif
    std::free(buffer);
    if (n < 0) {
        std::cerr << "[Network] Send error, closing socket" << std::endl;
        // 标记连接已断开，以便下次重连
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        // 这里简化处理，实际应从 moduleConnections_ 中移除
    }
}

int NetworkServiceManager::connectToNode(const char* ip, int port) {
    if (!ip) return -1;

    // 关键修复：如果已有连接，直接返回，不再重复连接
    if (!moduleConnections_.empty()) {
        return moduleConnections_.begin()->second;
    }

    int sock = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return -1;
    }

    std::cout << "[Network] Connected to Message Broker at " << ip << ":" << port << std::endl;
    
    // 关键修复：立即将连接加入连接池，使用 0 作为 Broker 的占位 ID
    const int BROKER_PLACEHOLDER_ID = 0;
    moduleConnections_[BROKER_PLACEHOLDER_ID] = sock;

    handleNewConnection(sock);
    return sock;
}

void NetworkServiceManager::serverLoop() {
    // std::cout << "[Network] Client mode enabled, node will not listen on any port." << std::endl;
}

void NetworkServiceManager::handleNewConnection(int sock) {
    // 发送握手消息
    Message handshakeMsg;
    handshakeMsg.header.sourceModuleId = moduleId_;
    handshakeMsg.header.type = -1; // 内部握手类型
    handshakeMsg.header.length = 0;
    handshakeMsg.payload = nullptr;
    sendToSocket(sock, handshakeMsg);

    std::thread([this, sock](){
        char buffer[4096];
        int n = 0;
        while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            Message msg = MessageSerialize::deserialize((uint8_t*)buffer, n);
            if (middleware_) {
                middleware_->dispatchMessage(&msg, true);
            }
            MessageFree(&msg);
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto it = moduleConnections_.begin(); it != moduleConnections_.end(); ++it) {
            if (it->second == sock) {
                std::cout << "[Network] Disconnected from peer (Module " << it->first << ")" << std::endl;
                moduleConnections_.erase(it);
                break;
            }
        }
    }).detach();
}
