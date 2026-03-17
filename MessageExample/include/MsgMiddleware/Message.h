#ifndef MESSAGE_API_H
#define MESSAGE_API_H

#include <cstdint>
#include <cstddef>
#include "MsgExport.h"

struct MSG_API MsgHeader {
    int sourceModuleId;  // 来源模块ID
    int type;            // 消息类型
    // bool broadcast;      // 是否广播
    // bool needReply;      // 是否需要回复
    uint32_t length;     // payload长度
};

struct MSG_API Message {
    MsgHeader header;
    uint8_t* payload;
};

inline void MessageFree(Message* msg) {
    if (msg && msg->payload) {
        delete[] msg->payload;
        msg->payload = nullptr;
    }
}

typedef void (*MsgHandler)(const Message* msg, void* data0, void* data1);

struct MsgNotice {
    MsgHandler handle;
    void* data0;
    void* data1;
};

#endif // MESSAGE_API_H
