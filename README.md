# MessageServer - 分布式消息中间件

这是一个高性能、轻量级的分布式消息中间件，支持 **同线程通信**、**跨进程通信 (IPC)** 以及 **跨设备远程通信**。采用 C++ 编写，通过 PIMPL 模式和 C 风格接口确保了跨平台的 ABI 兼容性。

## 🌟 核心特性

- **多维通信支持**：
  - **同线程/进程内**：基于内存队列，极低延迟。
  - **跨进程/跨设备**：基于 TCP 星型拓扑架构，通过消息中心（Broker）进行高效转发。
- **Broker 架构**：引入独立的消息中心，解决 P2P 模式下的端口冲突、重复连接和消息回流循环问题。
- **透明发布订阅**：应用层只需关注 `Topic/MsgId`，无需关心目标节点的物理地址。
- **ABI 兼容性**：公共头文件完全移除 `std::string`, `std::vector` 等标准库容器，适合作为动态库供第三方调用。
- **按需初始化**：网络服务仅在真正需要时延迟启动，节省系统资源。

## 🏗️ 项目结构

- `MessageServer/`: 核心动态库源码
  - `include/`: 公共 API 头文件
  - `src/`: 中间件实现及 `MessageBroker` 源码
- `MessageExample/`: 独立示例项目
  - `libs/`: 预编译的动态库文件
  - `include/`: 本地化的 API 头文件
  - `src/`: 包含线程内通信、跨设备通信的完整示例

## 🚀 快速开始

### 1. 编译中间件与 Broker
```bash
cd MessageServer
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 2. 运行消息中心 (Broker)
在进行跨进程或跨设备通信前，必须先启动 Broker：
```bash
# Windows
.\Debug\MessageBroker.exe
# Linux
./MessageBroker
```
默认监听端口：`10002`

### 3. 编译并运行示例
```bash
cd MessageExample
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

#### 运行线程内通信示例：
```bash
.\Debug\ThreadExample.exe
```

#### 运行跨设备通信示例：
启动节点 A（订阅者/发布者）：
```bash
# 参数：<moduleId> [brokerIp] [brokerPort]
.\Debug\RemoteNodeExample.exe 1001 127.0.0.1 10002
```
启动节点 B（订阅者/发布者）：
```bash
.\Debug\RemoteNodeExample.exe 1002 127.0.0.1 10002
```
*所有节点启动后将自动连接到指定的 Broker，并每隔 3 秒同步一次数据。*

## 🛠️ 开发者指南

### 注册远程节点
```cpp
MiddlewareConfig cfg;
cfg.moduleId = 1;
cfg.enableNetwork = true;
MsgMiddleware::instance().init(&cfg);

// 仅需注册 Broker 地址，无需关心其他节点 ID
MsgMiddleware::instance().registerRemoteNode("127.0.0.1", 10002);
```

### 发布与订阅
```cpp
// 订阅
MsgMiddleware::instance().subscribe(MY_TOPIC_ID, {[](const Message *msg, void *data0, void *data1)
                                               {
                                                   // 处理逻辑
                                               },
                                               0, 0});

// 发布
MsgMiddleware::instance().publish(MY_TOPIC_ID, data, size);
```

## 📄 许可证
[MIT License](LICENSE)
