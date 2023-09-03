#ifndef UENC_NET_MESSAGE_QUEUE_H
#define UENC_NET_MESSAGE_QUEUE_H

#include <stdint.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
// #include <event.h>

struct MsgData
{
    std::string ip;
    uint16_t port;
    uint32_t fd;
    // sa_family_t sa_family;
    bool need_pack;
    std::string data;
    uint32_t len;
    uint32_t checksum;
    uint32_t flag;
    uint32_t end_flag = 7777777;
    MsgData()
    {
        Clear();
    }
    void Clear()
    {
        //ip = 0;
        std::string().swap(ip);
        port = 0;
        fd = -1;
        //sa_family = AF_INET;
        need_pack = true;
        std::string().swap(data);
        len = 0;
        checksum = 0;
        flag = 0;
        end_flag = 7777777;
    }
};




class MessageQueue
{
public:
    MessageQueue();
    ~MessageQueue();


    bool GetMsgFromReadQueue(MsgData &msg);
    bool AddReadData(const MsgData &msg);
    bool AddReadData(const std::vector<MsgData> &msgs);


	bool GetMsgFromWriteQueue(MsgData &msg);
    bool AddWriteData(const MsgData &msg);
    bool AddWriteData(const std::vector<MsgData> &msgs);



    void ExitWait();
private:
    bool continue_wait_;

    std::mutex read_mutex_;
	std::condition_variable read_cond_;
    std::queue<MsgData> read_queue_;

    std::mutex write_mutex_;
	std::condition_variable write_cond_;
    std::queue<MsgData> write_queue_;
};
#endif 