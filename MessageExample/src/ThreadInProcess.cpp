#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include "MsgMiddlewareInit.h"

using namespace std;

// 消息类型定义
const int MSG_TYPE_TASK = 1001;

void threadHandler(const Message* msg, Message* reply) {
    if (!msg) return;
    string payload((char*)msg->payload, msg->header.length);
    cout << "[Thread-Sub] Received task in thread " << this_thread::get_id() 
         << " | Content: " << payload << endl;
}

int main() {
    cout << "--- Starting In-Process Thread Communication Example ---" << endl;

    MiddlewareConfig cfg;
    cfg.moduleId = 10;
    memset(cfg.processName, 0, sizeof(cfg.processName));
    strncpy(cfg.processName, "InProcessApp", sizeof(cfg.processName) - 1);
    cfg.enableNetwork = false;

    MsgMiddleware::instance().init(&cfg);

    MsgMiddleware::instance().subscribe(MSG_TYPE_TASK, threadHandler);
    string data = "Hello from producer thread!";
    MsgMiddleware::instance().publish(MSG_TYPE_TASK, data.c_str(), data.length());
    while(true) {
        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}
