#ifndef UENC_NET_NETAPI_H_
#define UENC_NET_NETAPI_H_

#include "message_queue.h"
#include "../utils/MagicSingleton.h"
#include "Net.h"
#include "connect.h"







void SendMessage(const std::string &msg_data, const std::string &msg_type,std::string &outdata);



class  Recver
{
public:
    Recver(){}
    ~Recver()
    {
        read_data_.erase();

    }

  

    bool ReadData(std::shared_ptr<net> netobj);
	//bool ReadData(Net::nodeBase *node,const char *recvdata, uint64_t size);
    
    //void set_accountpath(const std::string &path);
    void set_accountpath(const char *path);
    std::string get_accountpath();
    void set_configpath(const std::string &path);
    std::string get_configpath();

    //typedef std::function<int (const std::string &, int,int)> txdata_callback; 
    typedef std::function<int (const char *, int,int)> txdata_callback;
    
    typedef std::function<void (int)> txgas_callback;

    void phone_setptr(txdata_callback   callback);
    std::function<int (const char *, int,int)> phone_getptr();


    void phone_setgasptr(txgas_callback   gascallback);
    std::function<void (int)> phone_getgasptr();

    std::mutex  read_mutex_;
    std::string read_data_;
private:
    std::string  account_path = "./cert/";
    std::string  config_path  = "./config.json";
    txdata_callback phone_ptr = nullptr;
    txgas_callback  phone_gasptr = nullptr;
};



#endif
