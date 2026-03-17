#include "MsgMiddleware.h"
#include "NetworkServiceManager.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <cstring>

struct MsgMiddlewareImpl {
    std::unordered_map<int, std::vector<MsgNotice>> subscribers;
    std::queue<Message> messageQueue;
    std::mutex mutex;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    std::unique_ptr<NetworkServiceManager> networkService;
};

MsgMiddleware &MsgMiddleware::instance()
{
    static MsgMiddleware inst;
    return inst;
}

MsgMiddleware::MsgMiddleware() : impl_(new MsgMiddlewareImpl()) {}

MsgMiddleware::~MsgMiddleware()
{
    // 清理消息队列中未处理的 payload
    while (!impl_->messageQueue.empty()) {
        Message msg = impl_->messageQueue.front();
        MessageFree(&msg);
        impl_->messageQueue.pop();
    }
    delete impl_;
}

bool MsgMiddleware::init(const MiddlewareConfig* cfg)
{
    if (!cfg) return false;
    config_ = *cfg;
    startInternalThreads();
    registerModule(cfg->moduleId);
    return true;
}

void MsgMiddleware::ensureNetworkService()
{
    if (!config_.enableNetwork) return;
    
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->networkService)
    {
        // impl_->networkService = std::make_unique<NetworkServiceManager>(config_.moduleId, this); //C++14
        impl_->networkService = std::unique_ptr<NetworkServiceManager>(new NetworkServiceManager(config_.moduleId, this));
        std::cout << "[Middleware] Network service initialized on-demand for Module " << config_.moduleId << std::endl;
    }
}

void MsgMiddleware::registerRemoteNode(const char* ip, int port)
{
    if (!ip) return;
    ensureNetworkService();
    if (impl_->networkService)
    {
        impl_->networkService->addPeer(ip, port);
    }
}

void MsgMiddleware::subscribe(int type, MsgNotice handler)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->subscribers[type].push_back(handler);
}

void MsgMiddleware::publish(int type, const void *data, size_t size,
                            bool broadcast, bool needReply, Message *reply)
{
    Message msg;
    msg.header.sourceModuleId = config_.moduleId;
    msg.header.type = type;
    // msg.header.broadcast = broadcast;
    // msg.header.needReply = needReply;
    msg.header.length = static_cast<uint32_t>(size);
    msg.payload = nullptr;

    if (size > 0 && data)
    {
        msg.payload = new uint8_t[size];
        std::memcpy(msg.payload, data, size);
    }

    // 1. 本地分发：放入本地队列
    {
        std::lock_guard<std::mutex> lock(impl_->queueMutex);
        impl_->messageQueue.push(msg);
    }
    impl_->queueCv.notify_one();

    // 2. 网络分发：发送到 Broker
    if (config_.enableNetwork)
    {
        ensureNetworkService();
        if (impl_->networkService)
        {
            // 注意：sendMessage 内部不应再处理本地分发
            impl_->networkService->sendMessage(msg, broadcast);
        }
    }
}

void MsgMiddleware::dispatchMessage(const Message* msg, bool fromNetwork)
{
    if (!msg) return;
    
    // 核心修复：如果消息来自网络且来源是自己，则丢弃，防止回流循环
    if (fromNetwork && msg->header.sourceModuleId == config_.moduleId) {
        return;
    }

    std::vector<MsgNotice> handlers;
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        auto it = impl_->subscribers.find(msg->header.type);
        if (it != impl_->subscribers.end())
            handlers = it->second;
    }
    
    for (auto &h : handlers)
    {
        h.handle(msg, h.data0, h.data1);
        // Message localReply;
        // localReply.header = {0};
        // localReply.payload = nullptr;
        
        // h(msg, msg->header.needReply ? &localReply : nullptr);
        
        // if (msg->header.needReply && localReply.payload && localReply.header.length > 0)
        // {
        //     // 回复逻辑：如果原消息来自网络，回复也发往网络
        //     if (impl_->networkService)
        //         impl_->networkService->sendReply(msg->header.sourceModuleId, localReply);
        // }
        
        // MessageFree(&localReply);
    }
}

void MsgMiddleware::startInternalThreads()
{
    std::thread([this]()
                { this->internalLoop(); })
        .detach();
}

void MsgMiddleware::internalLoop()
{
    while (true)
    {
        Message msg;
        {
            std::unique_lock<std::mutex> lock(impl_->queueMutex);
            impl_->queueCv.wait(lock, [this]()
                          { return !impl_->messageQueue.empty(); });
            msg = impl_->messageQueue.front();
            impl_->messageQueue.pop();
        }
        
        // 本地队列消息，fromNetwork 为 false
        dispatchMessage(&msg, false);
        
        // 分发完成后清理 payload
        MessageFree(&msg);
    }
}

void MsgMiddleware::registerModule(int moduleId)
{
    printf("Module %d registered: %s\n", moduleId, config_.processName);
}
