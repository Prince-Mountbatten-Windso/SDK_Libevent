#ifndef _TIMEUTIL_H_
#define _TIMEUTIL_H_

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "VxNtpHelper.h"


class TimeUtil
{
    
public:
    TimeUtil();
    ~TimeUtil();
    
    //获取ntp服务器时间戳,单位：微秒     1s = 1000000微秒 Obtain the NTP server timestamp in microseconds 1s = 1,000,000 microseconds
    //param:
    //  is_sync 是否把从ntp获取的时间同步到本地 is_sync Whether to synchronize the time obtained from NTP to the local computer
    x_uint64_t getNtpTimestamp(bool is_sync = false);

    //获取ntp服务器时间戳,从配置文件读取ntp服务器，然后请求 单位：微秒  Get the NTP server timestamp, read the NTP server from the configuration file, and request the unit: microseconds
    x_uint64_t getNtpTimestampConf();

    //获取本地时间戳，单位：微秒    Gets the local timestamp in microseconds 
    x_uint64_t getUTCTimestamp();

    //获取时间戳，先从ntp获取，若不成功再去本地获取 Obtain the timestamp, first obtain it from NTP, and then go to the local level if it is unsuccessful
    x_uint64_t getTimestamp();

    //设置本地时间 Set the local time
    //param:
    //  timestamp,单位：微秒  timestamp, in microseconds
    bool setLocalTime(x_uint64_t timestamp);

    //测试ntp服务器获得时间的延时  Test the delay of the NTP server acquisition time
    void testNtpDelay();

    // 格式化时间戳为YYYY-MM-DD HH-MM-SS The formatted timestamp is YYYY-MM-DD HH-MM-SS
    std::string formatTimestamp(x_uint64_t timestamp);

    // 格式化UTC时间戳为YYYY-MM-DD HH-MM-SS Format the UTC timestamp as YYYY-MM-DD HH-MM-SS
    std::string formatUTCTimestamp(x_uint64_t timestamp);

	uint64_t getMorningTime(time_t t);

    uint64_t getPeriod(uint64_t TxTime);

    std::string GetDate(int d=0);
};


#endif