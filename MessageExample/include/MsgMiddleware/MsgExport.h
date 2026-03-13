#ifndef MSG_EXPORT_H
#define MSG_EXPORT_H

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MSGMIDDLEWARE_EXPORTS
        #define MSG_API __declspec(dllexport)
    #else
        #define MSG_API __declspec(dllimport)
    #endif
#else
    #if __GNUC__ >= 4
        #define MSG_API __attribute__ ((visibility ("default")))
    #else
        #define MSG_API
    #endif
#endif

#endif // MSG_EXPORT_H
