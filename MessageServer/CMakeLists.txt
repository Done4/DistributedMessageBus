cmake_minimum_required(VERSION 3.5)
project(MsgMiddleware CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 收集源文件
set(SOURCES
    src/MsgMiddleware.cpp
    src/NetworkServiceManager.cpp
)

# 生成动态库
add_library(MsgMiddleware SHARED ${SOURCES})

# 定义导出宏，用于 Windows DLL 导出
target_compile_definitions(MsgMiddleware PRIVATE MSGMIDDLEWARE_EXPORTS)

if(WIN32)
    # Windows 下动态库名称不需要 lib 前缀
    set_target_properties(MsgMiddleware PROPERTIES OUTPUT_NAME "MsgMiddleware")
else()
    set_target_properties(MsgMiddleware PROPERTIES OUTPUT_NAME "MsgMiddleware")
    target_link_libraries(MsgMiddleware PRIVATE pthread)
endif()

# 添加测试程序
add_executable(MsgTest main.cpp)
target_link_libraries(MsgTest MsgMiddleware)

# 添加 Broker 程序
add_executable(MessageBroker src/MessageBroker.cpp)
if(WIN32)
    target_link_libraries(MessageBroker PRIVATE ws2_32.lib)
else()
    target_link_libraries(MessageBroker PRIVATE pthread)
endif()
