#ifndef MSGMIDDLEWAREINIT_H
#define MSGMIDDLEWAREINIT_H

#include "MsgMiddleware.h"

#define MSG_MIDDLEWARE_INIT(cfg) MsgMiddleware::instance().init(cfg)
#define MSG_SUBSCRIBE(type, handler) MsgMiddleware::instance().subscribe(type, handler)
#define MSG_PUBLISH(type, data, size, broadcast, needReply, reply) \
MsgMiddleware::instance().publish(type, data, size, broadcast, needReply, reply)

#endif // MSGMIDDLEWAREINIT_H
