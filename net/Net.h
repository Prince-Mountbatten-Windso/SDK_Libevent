#ifndef __NET_API_
#define __NET_API_

#include <map>
#include <string>
#include <vector>
// #include <event2/bufferevent.h>
#include <functional>
#include <arpa/inet.h>
// #include <event2/buffer.h>
// #include <event2/util.h>
// #include <event2/listener.h>
// #include <event2/event.h>
// #include <event2/thread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <utility>
#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>

// class Net
// {
// public:
//     struct nodeBase
//     {
//         std::string ip;
//         int port;
//         struct bufferevent *buffer = nullptr;
//         Net *man = nullptr;
//     };

//     Net();
//     ~Net();

//     void start();

//     bool connect(const std::string &ip, int port);

//     void sendData(const std::string &ip, const char *dp, uint64_t size);

//     void disconnect(const std::string &ip);

//    // bool ReadData(nodeBase *node, const std::string &recvdata, uint64_t size);
   

//     void exitloop();

//     bool isConnected(const std::string &ip);
   

//     typedef std::function<void(nodeBase *, const char *, int)> data_callback;
//     std::function<void(nodeBase *, const std::string &)> onConnectCallback = nullptr;
//     std::function<void(nodeBase *, const std::string &)> onDisconnectCallback;
//     data_callback ReadCallback = nullptr;

// private:
//     friend nodeBase;
//     static void event_sigint(evutil_socket_t fd, short events, void *args);
//     static void read_callback(struct bufferevent *bev, void *arg);
//     static void write_callback(struct bufferevent *bev, void *arg);
//     static void event_callback(struct bufferevent *bev, short events, void *arg);
//     struct event_base *base = nullptr;
//     struct event *signal_event_SIGINIT = nullptr;

//     std::map<std::string, nodeBase *> all_buffer;
//     std::thread _th;
//     std::mutex net_mutex;
//     std::condition_variable conf;
// };

#endif