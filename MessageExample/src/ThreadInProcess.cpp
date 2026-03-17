#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include "MsgMiddlewareInit.h"

using namespace std;

// 消息类型定义
const int MSG_TYPE_TASK = 1001;

void threadHandler(const Message* msg, void* data0, void* data1) {
    if (!msg) return;
    string payload((char*)msg->payload, msg->header.length);
    cout << "[Thread-Sub] Received task in thread " << this_thread::get_id() 
         << " | Content: " << payload << endl;
}

class TestA
{
public:
    void subMsg(const int id) {
        MsgMiddleware::instance().subscribe(id, {&TestA::handlerMsg, this, 0});
    }
    void printfMsg(const string& msg) {
        cout << "printfMsg:" << msg << endl;
    }
    static void handlerMsg(const Message* msg, void* data0, void* data1) {
        if (!msg) return;
        TestA *pthis = static_cast<TestA *>(data0);
        if (pthis != nullptr) {
            string payload((char*)msg->payload, msg->header.length);
            pthis->printfMsg(payload);
        }
    }
};

class TestB
{
public:
    void publishMsg(int id, const char* data) {
        MsgMiddleware::instance().publish(id, data, strlen(data));
    }
};

int main() {
    cout << "--- Starting In-Process Thread Communication Example ---" << endl;

    MiddlewareConfig cfg;
    cfg.moduleId = 10;
    memset(cfg.processName, 0, sizeof(cfg.processName));
    strncpy(cfg.processName, "InProcessApp", sizeof(cfg.processName) - 1);
    cfg.enableNetwork = false;

    MsgMiddleware::instance().init(&cfg);

    string data = "Hello from producer thread!";
    TestA objA;
    TestB objB;
    objA.subMsg(MSG_TYPE_TASK);
    objB.publishMsg(MSG_TYPE_TASK, data.c_str());
    while(true) {
        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}
