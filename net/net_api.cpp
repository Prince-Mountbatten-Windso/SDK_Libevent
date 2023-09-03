#include "net_api.h"
#include "ca_handle_event.h"
//#include "include/logging.h"
#include "utils/compress.h"
#include "utils/util.h"
#include "../proto/common.pb.h"
#include "dispatcher.h"
#include "../net/message_queue.h"
#include <unistd.h>
#include "common/global.h"
#include  "proto/interface.pb.h"




void SendMessage(const std::string &msg_data, const std::string &msg_type,std::string &outdata)
{
    CommonMsg msg;
    msg.set_type(msg_type);
    msg.set_version(global::kNetVersion);
    msg.set_data(msg_data);
    
    std::string data = msg.SerializeAsString();
    uint32_t len = data.size() + 3 * sizeof(int);
    
    // int len = data.size() + sizeof(int) + sizeof(int);
    uint32_t checksum = Util::adler32((const uint8_t *)data.c_str(), data.size());
    uint32_t flag = 0;
    uint32_t end_flag = 7777777;


    std::string packagedata;
    // uint32_t len = htole32(msg.len);
    std::cout<<"sizeof(len) = "<<sizeof(len)<<std::endl;
   // std::cout<<"data.data() = "<<data.data()<<std::endl;
    std::cout<<"data.size() = "<<data.size()<<std::endl;
    packagedata.append((char*)&len, sizeof(len));
    packagedata.append(data.data(),data.size());
    //uint32_t checksum = htole32(msg.checksum);
    //std::cout<<"sizeof(checksum) = "<<sizeof(checksum)<<std::endl;
    packagedata.append((char*)&checksum, sizeof(checksum));
    std::cout<<"sizeof(flag) = "<<sizeof(flag)<<std::endl;
    packagedata.append((char*)&flag, sizeof(flag));
   // std::cout<<"sizeof(end_flag) = "<<sizeof(end_flag)<<std::endl;
    packagedata.append((char*)&end_flag, sizeof(end_flag));
    {
        //std::lock_guard<std::mutex> lck(write_mutex_);
        outdata.clear();
        outdata = packagedata;
      //  std::cout<<"packagedata = "<<packagedata<<std::endl;
      std::cout<<"packagedata  size()= "<<packagedata.size()<<std::endl;
    }
    return ;
}




//bool Recver::ReadData(Net::nodeBase *node,const char *recvdata, uint64_t size)
bool Recver::ReadData(std::shared_ptr<net> netobj)
{
    std::lock_guard<std::mutex> lck(read_mutex_);
    char buffer_cache[1024] = {0};
    uint64_t size = sizeof(buffer_cache);
    std::cout<<"MagicSingleton<net>::GetInstance()->read ---> start"<<std::endl;
    std::cout<<"MagicSingleton<net>::GetInstance()->read ---> end"<<std::endl;
   
    int ret=0;

    while (true)
    {
      ret = MagicSingleton<net>::GetInstance()->read(buffer_cache,size);
      if(ret == 0 )
      {
        break;
        infoL("error read"  << strerror(errno));
      }
        std::string data(buffer_cache,ret);
        read_data_ += data;
    }

    std::vector<MsgData> msgs;
    MsgData msg;
  
    if(read_data_.empty())
    {
        return false;
    }
    size_t read_data_size = read_data_.size();
    size_t curr_msg_len = 0;
    msg.Clear();
    memcpy(&curr_msg_len, read_data_.data(), sizeof(MsgData::len));
    //字节序转换
    // curr_msg_len = le32toh(curr_msg_len);
    curr_msg_len = curr_msg_len;
    std::cout<<"curr_msg_len = "<<curr_msg_len<<std::endl;
    std::cout<<"read_data_size = "<<read_data_size<<std::endl;
    // if (curr_msg_len + sizeof(MsgData::len) > read_data_size)
    // {
    //     std::cout<<"1111111---->Recver::ReadData"<<std::endl;
        
    // }
    msg.ip = netobj->getIp();
    std::cout<<"msg.ip = "<<msg.ip<<std::endl;
    msg.port = netobj->getPort();
    std::cout<<"msg.port = "<<msg.port<<std::endl;
    msg.fd = netobj->getfd();
    std::cout<<" msg.fd= "<< msg.fd<<std::endl;
    //msg.sa_family = AF_INET;
    msg.len = curr_msg_len;
    int pos = sizeof(MsgData::len);
    msg.data = std::string(read_data_.begin() + pos, read_data_.begin() + (curr_msg_len - sizeof(MsgData::flag) - sizeof(MsgData::end_flag)));
    pos = pos + curr_msg_len - sizeof(MsgData::checksum) - sizeof(MsgData::flag) - sizeof(MsgData::end_flag);
    memcpy(&msg.checksum, read_data_.data() + pos, 4);
    //msg.checksum = le32toh(msg.checksum);
    msg.checksum = msg.checksum;
    pos = pos + sizeof(MsgData::checksum);
    memcpy(&msg.flag, read_data_.data() + pos, 4);
    pos = pos + sizeof(MsgData::flag);
    memcpy(&msg.end_flag, read_data_.data() + pos, 4);
    pos = pos + sizeof(MsgData::flag);
    msgs.push_back(msg);
    read_data_.erase(0, sizeof(MsgData::len) + curr_msg_len);
  
    
    if(!msgs.empty())
    {
        int ret = MagicSingleton<MessageQueue>::GetInstance()->AddReadData(msgs);
        
    }
    return true;
}



void Recver::set_accountpath(const char *path)
{
    account_path = path;
    std::cout<<"set account_path= "<<account_path<<std::endl;
}


std::string Recver::get_accountpath()
{
    return account_path;
}

void Recver::set_configpath(const std::string &path)
{
    config_path = path;
    std::cout<<"set global::config_path= "<<config_path<<std::endl;
}

std::string Recver::get_configpath()
{
    return config_path;
}


void Recver::phone_setptr(txdata_callback   callback)
{
    phone_ptr = callback;
}


std::function<int (const char *, int,int)>  Recver::phone_getptr()
{
   return   phone_ptr;
}


void Recver::phone_setgasptr(txgas_callback   gascallback)
{
    phone_gasptr = gascallback;
}

std::function<void (int)> Recver::phone_getgasptr()
{
    return phone_gasptr;
}