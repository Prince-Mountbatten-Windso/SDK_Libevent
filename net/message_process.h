#ifndef UENC_NET_MESSAGE_PROCESS_H
#define UENC_NET_MESSAGE_PROCESS_H

#include <thread>
#include <vector>
class MessageProcess
{
public:
    MessageProcess();
    ~MessageProcess();
    void ThreadStart();
    void ThreadStop();


    // void Write_ThreadTasks()
    // void Write_ThreadStart()
    // void Write_ThreadStop();
private:

    bool continue_running_;
    std::vector<std::thread> threads_work_list_;
};

#endif 