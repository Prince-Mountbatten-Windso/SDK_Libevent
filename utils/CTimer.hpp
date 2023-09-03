#ifndef CTimer_hpp
#define CTimer_hpp

#include <stdio.h>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <condition_variable>

class CTimer
{
public:
    CTimer(const std::string sTimerName = "");   //构造定时器，附带名称 Constructs the timer, with a name
    ~CTimer();
    
    /**
     开始运行定时器 Start running the timer

     @param msTime 延迟运行(单位ms) Delayed operation (in ms)
     @param task 任务函数接口 Task function interface
     @param bLoop 是否循环(默认执行1次) Whether to loop (1 time by default)
     @param async 是否异步(默认异步) Async (default async)
     @return true:已准备执行，否则失败 Ready for execution, otherwise it fails
     */
    bool Start(unsigned int msTime, std::function<void()> task, bool bLoop = false, bool async = true);
    
    /**
     取消定时器，同步定时器无法取消(若任务代码已执行则取消无效)
     Cancel the timer, the synchronization timer cannot be canceled (cancellation is invalid if the task code has been executed)
     */
    void Cancel();
    
    /**
     同步执行一次 Synchronously performed once
     #这个接口感觉作用不大，暂时现实在这里 This interface feels like it doesn't work much, and the reality is here for the time being

     @param msTime 延迟时间(ms) Delay time (ms)
     @param fun 函数接口或lambda代码块 Function interface or lambda code block
     @param args 参数 parameter
     @return true:已准备执行，否则失败 Ready for execution, otherwise it fails
     */
    template<typename callable, typename... arguments>
    bool SyncOnce(int msTime, callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...)); //绑定任务函数或lambda成function
        return Start(msTime, task, false, false);
    }
    
    /**
     异步执行一次任务 Executes a task asynchronously
     
     @param msTime 延迟及间隔时间 Delay and interval
     @param fun 函数接口或lambda代码块 Function interface or lambda code block
     @param args 参数
     @return true:已准备执行，否则失败 Ready for execution, otherwise it fails
     */
    template<typename callable, typename... arguments>
    bool AsyncOnce(int msTime, callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...));
        
        return Start(msTime, task, false);
    }
    
    /**
     异步执行一次任务(默认延迟1毫秒后执行) Execute a task asynchronously (execute after 1 millisecond delay by default)
     
     @param fun 函数接口或lambda代码块 Function interface or lambda code block
     @param args 参数
     @return true:已准备执行，否则失败 Ready for execution, otherwise it fails
     */
    template<typename callable, typename... arguments>
    bool AsyncOnce(callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...));
        
        return Start(1, task, false);
    }
    
    
    /**
     异步循环执行任务 Executes tasks in an asynchronous loop

     @param msTime 延迟及间隔时间 Delay and interval
     @param fun 函数接口或lambda代码块 Function interface or lambda code block
     @param args 参数
     @return true:已准备执行，否则失败 Ready for execution, otherwise it fails
     */
    template<typename callable, typename... arguments>
    bool AsyncLoop(int msTime, callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...));
        
        return Start(msTime, task, true);
    }
    
    
private:
    void DeleteThread();    //删除任务线程 Delete a task thread

public:
    int m_nCount = 0;   //循环次数 Number of cycles
    
private:
    std::string m_sName;   //定时器名称 Timer name
    
    std::atomic_bool m_bExpired;       //装载的任务是否已经过期 Whether the loaded task has expired
    std::atomic_bool m_bTryExpired;    //装备让已装载的任务过期(标记) Equipment expires loaded tasks (markers)
    std::atomic_bool m_bLoop;          //是否循环 Whether to loop or not
    
    std::thread *m_Thread = nullptr;
    std::mutex m_ThreadLock;
    std::condition_variable_any m_ThreadCon;
};

#endif /* CTimer_hpp */

