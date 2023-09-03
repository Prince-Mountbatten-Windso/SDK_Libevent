#include "Net.h"
#include "debug.h"
#include <signal.h>

using namespace std;

// void Net::event_sigint(evutil_socket_t fd, short events, void *args)
// {
//     Net *net = (Net *)args;
//     struct timeval delay = {2, 0};
//     fprintf(stdout, "\nexit at two seconds\n");
//     event_base_loopexit(net->base, &delay);
// }

// void Net::event_callback(struct bufferevent* bev, short events, void* arg) 
// {
//     nodeBase *node=(nodeBase *) arg;
//     if (events & BEV_EVENT_CONNECTED) 
//     {
//         node->man->all_buffer[node->ip]=node;
//         infoL("Connected:" << node->ip);
//         if(node->man->onConnectCallback)
//         {
//             node->man->onConnectCallback(node,"BEV_EVENT_CONNECTED");
//         }
//     }
//     else if (events & BEV_EVENT_ERROR) 
//     {
//         errorL("ERROR:" << node->ip);
//         auto nIter=node->man->all_buffer.find(node->ip);
//         if(nIter!=node->man->all_buffer.end())
//         {
//             node->man->all_buffer.erase(node->ip);
//         }
//         if(node->man->onDisconnectCallback)
//         {
//             node->man->onDisconnectCallback(node,"BEV_EVENT_ERROR");
//         }
//     }
//     else if (events & BEV_EVENT_EOF) 
//     {
//         infoL("closed:" << node->ip);
//         auto nIter=node->man->all_buffer.find(node->ip);
//         if(nIter!=node->man->all_buffer.end())
//         {
//             node->man->all_buffer.erase(node->ip);
//         }
//          if(node->man->onDisconnectCallback)
//          {
//             node->man->onDisconnectCallback(node,"BEV_EVENT_EOF");
//         }
//     }
// }


// Net::Net()
// {

    
// }


// Net::~Net()
// {
//     if (nullptr != base)
//     {
//         event_base_free(base);
//     }
//     base = nullptr;

//     std::cout<<"::~Net()<----->::~Net()"<<std::endl;
//     nodeBase * node_ = nullptr;
//     for(auto &item :all_buffer)
//     {
//         node_ = item.second;
//         if (nullptr != node_->buffer)
//         {
//            // bufferevent_free(node_->buffer);
//             node_->buffer = nullptr;
//             node_->man = nullptr;
//             free(node_);
//         }
//     }   
// }


//  void Net::exitloop()
//  {
//     //ReadCallback = nullptr;
//     event_base_loopexit(base, NULL);
//     std::cout<<"Net::exitloop()<----->::Net::exitloop()"<<std::endl;
    
//  }

// void Net::start()
// {
//     _th = std::thread([=]
//                       {
//     // evthread_use_pthreads();
//     // base=event_base_new();
//     event_config *config = event_config_new();
//     // event_config_avoid_method(config,"epoll");
//     // event_config_avoid_method(config,"poll");

//     event_config_require_features(config,EV_FEATURE_ET);

//     base = event_base_new_with_config(config);
//     event_config_free(config);

//     if (!base) {
//         fprintf(stderr, "cant not create base");
//     }

//     signal_event_SIGINIT = evsignal_new(base, SIGINT, event_sigint, (void*)this);

//     if (!signal_event_SIGINIT || event_add(signal_event_SIGINIT, NULL) < 0) {
//         fprintf(stderr, "Could not create /add a signal event!\n");
//     } 
   
//     conf.notify_all();
//     event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
//     });

//     std::unique_lock<std::mutex> glock(net_mutex);

// 	conf.wait(glock);

//     _th.detach();
// }

// bool Net::connect(const std::string &ip, int port)
// {
//    struct sockaddr_in addr;
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(port);
//     inet_aton(ip.c_str(), (struct in_addr*)&addr.sin_addr);

//     nodeBase *node = new nodeBase;
//     node->ip = ip;
//     node->man = this;
//     node->port = port;
   
//     int ret = 0;
//     node->buffer = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
//     if (nullptr ==  node->buffer)
//     {
//         fprintf(stderr, "bufferevent_socket_connect => [%s]\n", strerror(errno));
//         return false;
//     }
//     ret = bufferevent_socket_connect(node->buffer, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
//     if (ret == -1) 
//     {
//         fprintf(stderr, "bufferevent_socket_connect => [%s]\n", strerror(errno));
//         return false;
//     }
//     bufferevent_setcb(node->buffer, &Net::read_callback, &Net::write_callback, &Net::event_callback, (void*)node);
//     ret = bufferevent_enable(node->buffer, EV_READ |EV_WRITE);
//     if (ret == -1) 
//     {
//         fprintf(stderr, "bufferevent_enable => [%s]", strerror(errno));
//         return false;
//     }
    
//     return true;
// }

// void Net::sendData(const std::string & ip,const char * dp,uint64_t size)
// {
//     auto nIter=all_buffer.find(ip);
//     nodeBase * node_=nullptr;
//     if(nIter!=all_buffer.end())
//     {
//         node_=nIter->second;
//         bufferevent_write(node_->buffer, dp, size);
//         return ;
//     }
//     errorL("can't find" << ip);
// }

// void Net::disconnect(const std::string & ip)
// {
//     auto nIter=all_buffer.find(ip);
//     nodeBase * node_=nullptr;
//     if(nIter!=all_buffer.end())
//     {
//         node_=nIter->second;
//         bufferevent_free(node_->buffer);
//         return ;
//     }
//     errorL("can't find" << ip);
// }

// void Net::read_callback(struct bufferevent* bev, void* arg)
// {
//     char buffer_cache[10240] = {0};
//     nodeBase * net = static_cast<nodeBase*>(arg);
//     int ret = bufferevent_read(bev, buffer_cache, sizeof(buffer_cache));
//     if(net->man->ReadCallback)
//     {
//         net->man->ReadCallback(net,buffer_cache,ret);
//     } 
// }

// void Net::write_callback(struct bufferevent* bev, void* arg)
// {
//     printf("writing....\n");
// }



// bool Net::isConnected(const std::string &ip)
// {
//     auto nIter = all_buffer.find(ip);
//     nodeBase *node_ = nullptr;
//     if (nIter != all_buffer.end())
//     {
//         return true;
//     }
//     return false;
// }



