#include <iostream>
#include <cstring>
#include "MsgMiddlewareInit.h"
#include <chrono>
#include <thread>

using namespace std;

// 简单的消息处理回调
void myHandler(const Message* msg, void* data0, void* data1) {
    if (!msg) return;
    cout << "[Node] Received message type: " << msg->header.type
         << " from module: " << msg->header.sourceModuleId 
         << " payload size: " << msg->header.length << endl;
    if (1001 == msg->header.type && msg->payload) {
        string m((char*)msg->payload, msg->header.length);
        cout << m << endl;
    }
}

int main()
{
    std::cout << "Message Server starting..." << endl;

    MiddlewareConfig cfg;
    cfg.moduleId = 123;
    memset(cfg.processName, 0, sizeof(cfg.processName));
    strncpy(cfg.processName, "ExampleNode", sizeof(cfg.processName) - 1);
    cfg.enableNetwork = true; // 开启网络功能

    // 初始化中间件（此时不会启动网络服务，实现按需初始化）
    MsgMiddleware::instance().init(&cfg);

    // 订阅消息
    MsgMiddleware::instance().subscribe(1001, {myHandler, 0, 0});

    // 注册远程节点（跨设备通信关键）
    MsgMiddleware::instance().registerRemoteNode("127.0.0.1", 10002);

    cout << "Node initialized. Publish a message to trigger network service..." << endl;
    
    // 发布消息（第一次发布会触发 NetworkServiceManager 的按需初始化）
    string data = "Hello MsgMiddleware";
    MsgMiddleware::instance().publish(1001, data.c_str(), data.length(), true);

    // 保持运行
    while(true) {
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}
