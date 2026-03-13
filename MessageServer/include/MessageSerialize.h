#ifndef MESSAGESERIALIZE_H
#define MESSAGESERIALIZE_H

#include "Message.h"
#include <cstring>
#include <cstdlib>

class MessageSerialize {
public:
    // 将 Message 转为字节流：返回分配的内存，调用者负责 free
    static uint8_t* serialize(const Message& msg, size_t& outLen) {
        outLen = sizeof(MsgHeader) + msg.header.length;
        uint8_t* buffer = (uint8_t*)std::malloc(outLen);
        if (!buffer) return nullptr;
        
        std::memcpy(buffer, &msg.header, sizeof(MsgHeader));
        if (msg.payload && msg.header.length > 0) {
            std::memcpy(buffer + sizeof(MsgHeader), msg.payload, msg.header.length);
        }
        return buffer;
    }

    // 将字节流转为 Message：内部会为 payload 分配内存，需使用 MessageFree 清理
    static Message deserialize(const uint8_t* data, size_t len) {
        Message msg;
        msg.payload = nullptr;
        msg.header = {0};

        if (len < sizeof(MsgHeader)) return msg; // 数据不完整
        
        std::memcpy(&msg.header, data, sizeof(MsgHeader));
        
        // 校验长度防止缓冲区溢出
        const size_t maxPayload = 1024 * 1024; // 1MB 限制
        if (msg.header.length > maxPayload || msg.header.length > len - sizeof(MsgHeader)) {
            msg.header.length = 0;
            return msg;
        }

        if (msg.header.length > 0) {
            msg.payload = new uint8_t[msg.header.length];
            std::memcpy(msg.payload, data + sizeof(MsgHeader), msg.header.length);
        }
        return msg;
    }
};
#endif // MESSAGESERIALIZE_H
