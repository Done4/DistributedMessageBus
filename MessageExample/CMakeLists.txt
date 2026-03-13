cmake_minimum_required(VERSION 3.5)
project(MessageExample CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含本地中间件头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/MsgMiddleware)

# 查找本地动态库
if(WIN32)
    set(MIDDLEWARE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/libs/MsgMiddleware.lib)
    set(DLL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/libs/MsgMiddleware.dll)
else()
    set(MIDDLEWARE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/libs/libMsgMiddleware.so)
endif()

# 示例 1: 线程内通信
add_executable(ThreadExample src/ThreadInProcess.cpp)
target_link_libraries(ThreadExample ${MIDDLEWARE_LIB})

# 示例 2: 跨设备/跨进程通信
add_executable(RemoteNodeExample src/RemoteNode.cpp)
target_link_libraries(RemoteNodeExample ${MIDDLEWARE_LIB})

# 跨平台自动拷贝动态库到输出目录
if(WIN32)
    add_custom_command(TARGET ThreadExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:ThreadExample>)
    add_custom_command(TARGET RemoteNodeExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:RemoteNodeExample>)
else()
    # Linux 下设置 RPATH 以便找到 .so 文件
    set_target_properties(ThreadExample PROPERTIES INSTALL_RPATH ".")
    set_target_properties(RemoteNodeExample PROPERTIES INSTALL_RPATH ".")
    
    add_custom_command(TARGET ThreadExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MIDDLEWARE_LIB}"
        $<TARGET_FILE_DIR:ThreadExample>)
    add_custom_command(TARGET RemoteNodeExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MIDDLEWARE_LIB}"
        $<TARGET_FILE_DIR:RemoteNodeExample>)
endif()
