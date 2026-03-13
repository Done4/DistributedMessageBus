#ifndef MSGMIDDLEWARE_H
#define MSGMIDDLEWARE_H

#include "Message.h"
#include "MsgExport.h"
#include <cstddef>

struct MsgMiddlewareImpl;

struct MSG_API MiddlewareConfig {
    int moduleId;
    char processName[64];    // 替换 std::string 为固定大小数组
    bool enableNetwork;      // 是否开启网络
};

class MSG_API MsgMiddleware {
public:
    static MsgMiddleware& instance();

    bool init(const MiddlewareConfig* cfg);
    void subscribe(int type, MsgHandler handler);
    void publish(int type, const void* data, size_t size,
                 bool broadcast=false, bool needReply=false, Message* reply=nullptr);

    // 注册远程节点
    void registerRemoteNode(const char* ip, int port);

    // 由网络服务调用，增加 fromNetwork 标识
    void dispatchMessage(const Message* msg, bool fromNetwork = false);

private:
    MsgMiddleware();
    ~MsgMiddleware();
    
    // 禁止拷贝
    MsgMiddleware(const MsgMiddleware&) = delete;
    MsgMiddleware& operator=(const MsgMiddleware&) = delete;

    void startInternalThreads();
    void internalLoop();
    void registerModule(int moduleId);
    void ensureNetworkService();

private:
    MiddlewareConfig config_;
    MsgMiddlewareImpl* impl_;
};

#endif // MSGMIDDLEWARE_H
