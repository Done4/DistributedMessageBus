#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include "MsgMiddlewareInit.h"

using namespace std;

// 定义一个全局统一的消息主题 (Topic/MsgId)
const int TOPIC_DATA_SYNC = 1001;

class TestRemote {
public:
    static void remoteHandler(const Message* msg, void* data0, void* data1) {
        if (!msg) return;
        cout << "[Subscriber] Topic: " << msg->header.type
             << " | From Module: " << msg->header.sourceModuleId
                << endl;
        TestRemote *pthis = static_cast<TestRemote *>(data0);
        if (pthis != nullptr) {
            string payload((char*)msg->payload, msg->header.length);
            pthis->printfRemoteMsg(payload);
        }
    }
private:
    void printfRemoteMsg(const string& msg) {
        cout << "printfRemoteMsg:" << msg << endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <myModuleId> [brokerIp] [brokerPort]" << endl;
        cout << "Example: " << argv[0] << " 1001 127.0.0.1 10002" << endl;
        return 0;
    }

    int myModuleId = atoi(argv[1]);
    const char* brokerIp = (argc >= 3) ? argv[2] : "127.0.0.1";
    int brokerPort = (argc >= 4) ? atoi(argv[3]) : 10002;
    
    MiddlewareConfig cfg;
    cfg.moduleId = myModuleId;
    memset(cfg.processName, 0, sizeof(cfg.processName));
    string procName = "Node_" + to_string(myModuleId);
    strncpy(cfg.processName, procName.c_str(), sizeof(cfg.processName) - 1);
    cfg.enableNetwork = true; 

    MsgMiddleware::instance().init(&cfg);

    TestRemote remoteNode;
    // 订阅同步主题
    MsgMiddleware::instance().subscribe(TOPIC_DATA_SYNC, {&TestRemote::remoteHandler, &remoteNode, 0});

    // 只有在提供了参数或使用默认值时注册 Broker
    cout << "[Node] Connecting to Broker at " << brokerIp << ":" << brokerPort << endl;
    MsgMiddleware::instance().registerRemoteNode(brokerIp, brokerPort);

    // 循环发送测试消息
    string data = "Hello from Module " + to_string(myModuleId);
    cout << "Node is running. Topic: " << TOPIC_DATA_SYNC << endl;
    
    while(true) {
        this_thread::sleep_for(chrono::seconds(3));
        cout << "[Publisher] Publishing to topic: " << TOPIC_DATA_SYNC << endl;
        MsgMiddleware::instance().publish(TOPIC_DATA_SYNC, data.c_str(), data.length());
    }
    return 0;
}
