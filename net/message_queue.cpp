
#include <iostream>
#include "message_queue.h"

MessageQueue::MessageQueue()
{
    continue_wait_ = true;
}
MessageQueue::~MessageQueue()
{
    ExitWait();
}

bool MessageQueue::GetMsgFromReadQueue(MsgData &msg)
{
    msg.Clear();
    std::unique_lock<std::mutex> read_locker(read_mutex_);
    while(read_queue_.empty())
    {
        if(!continue_wait_)
        {
            return false;
        }
        read_cond_.wait(read_locker);
    }
    msg = read_queue_.front();
    read_queue_.pop();
    read_locker.unlock();
    return true;
}

bool MessageQueue::AddReadData(const MsgData &msg)
{

    std:: cout<<"read <---111111111111->MessageQueue::AddReadData(const std::vector<MsgData> &msgs)"<<std::endl;
    {
        std::lock_guard<std::mutex> lck(read_mutex_);
        read_queue_.push(msg);
    }
    if(!read_queue_.empty())
    {
        read_cond_.notify_all();
    }
    return true;
}

bool MessageQueue::AddReadData(const std::vector<MsgData> &msgs)
{
    std:: cout<<"read <---->MessageQueue::AddReadData(const std::vector<MsgData> &msgs)"<<std::endl;
    {
        std::lock_guard<std::mutex> lck(read_mutex_);
        for(auto it = msgs.cbegin(); it != msgs.cend(); it++)
        {
            read_queue_.push(*it);
        }
    }
    if(!read_queue_.empty())
    {
        read_cond_.notify_all();
    }
    return true;
}





bool MessageQueue::GetMsgFromWriteQueue(MsgData &msg)
{
    msg.Clear();
    std::unique_lock<std::mutex> write_locker(write_mutex_);
    while(write_queue_.empty())
    {
        if(!continue_wait_)
        {
            return false;
        }
        std::cout<<"write_cond_.wait(write_locker)---front"<<std::endl;
        write_cond_.wait(write_locker);
    }
    std::cout<<"write_cond_.wait(write_locker)----en'dend"<<std::endl;
    msg = write_queue_.front();
    write_queue_.pop();
    write_locker.unlock();
    std::cout<<"write_locker.unlock()"<<std::endl;
    return true;
}

bool MessageQueue::AddWriteData(const MsgData &msgs)
{
    {
        std::lock_guard<std::mutex> lck(write_mutex_);
        write_queue_.push(msgs);
    }
    if(!write_queue_.empty())
    {
       std::cout<<"write_cond_.notify_all()"<<std::endl;
       write_cond_.notify_all();
    }
    return true;
}

bool MessageQueue::AddWriteData(const std::vector<MsgData> &msgs)
{
    {
        std::lock_guard<std::mutex> lck(write_mutex_);
        for(auto it = msgs.cbegin(); it != msgs.cend(); it++)
        {
            write_queue_.push(*it);
        }
    }
    if(!write_queue_.empty())
    {
       write_cond_.notify_all();
    }
    return true;
}


void MessageQueue::ExitWait()
{
    continue_wait_ = false;
    read_cond_.notify_all();
    write_cond_.notify_all();
}