#include "utils/MagicSingleton.h"
#include "message_queue.h"
#include "message_process.h"
#include "dispatcher.h"

MessageProcess::MessageProcess()
{
    continue_running_ = true;
}


MessageProcess::~MessageProcess()
{
    std::cout<<"MessageProcess::~MessageProcess()"<<std::endl;
    ThreadStop();
}


void MessageProcess::ThreadStart()
{
    continue_running_ = true;
    for (uint16_t i = 0; i < 4; i++)
    {
        threads_work_list_.push_back(std::thread([this](){
            while(true)
            {
                MsgData msg;
                if(MagicSingleton<MessageQueue>::GetInstance()->GetMsgFromReadQueue(msg))
                {
                    MagicSingleton<ProtobufDispatcher>::GetInstance()->Handle(msg);
                }
                else
                {
                    if(!continue_running_)
                    {
                        break;
                    }
                }
            }
        }));
        threads_work_list_[i].detach();
    }
}



void MessageProcess::ThreadStop()
{
    MagicSingleton<MessageQueue>::GetInstance()->ExitWait();
    continue_running_ = false;
}










// void MessageProcess::Write_ThreadTasks()
// {
//     while(continue_running_)
//     {
//         MsgData msg;
//         if(MagicSingleton<MessageQueue>::GetInstance()->GetMsgFromWriteQueue(msg))
//         {
//             //if(it->second->IsConnected())   
//            // if(msg.need_pack)
//             {
//                 std:: cout<<"read <---if(msg.need_pack)->WriteMsg(msg)"<<std::endl;
//                 std::string data;
//                 uint32_t len = htole32(msg.len);
//                 data.append((char*)&len, sizeof(len));
//                 data.append(msg.data);
//                 uint32_t checksum = htole32(msg.checksum);
//                 data.append((char*)&checksum, sizeof(checksum));
//                 data.append((char*)&msg.flag, sizeof(msg.flag));
//                 data.append((char*)&msg.end_flag, sizeof(msg.end_flag));
//                 {
//                     std::lock_guard<std::mutex> lck(write_mutex_);
//                     write_data_ += data;
//                 }
//             }
                
//         }
//     }
// }

// bool MessageProcess::Write_ThreadStart()
// {
//     continue_running_ = true;
//     write_thread_ = std::thread(std::bind(&Connection::ThreadTasks, this));
//     write_thread_.detach();
//     return true;
// }

// void MessageProcess::Write_ThreadStop()
// {
//     MagicSingleton<MessageQueue>::GetInstance()->ExitWait();
//     continue_running_ = false;
// }