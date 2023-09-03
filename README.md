# SDK_Libevent
利用libevent给第三方安卓、ios提供的第三方库
1.利用lievent进行网络结构，实际就是一个典型的reactor模型
总共有6个核心接口普通交易、质押交易、解质押交易、投资交易、解投资交易、申领交易。
另外还有2个保留的智能合约接口。
以交易为例的协议和组包结构介绍：

#1。交易结构请求体
message GetSDKReq
{
    string   version = 1; // 版本 version
    repeated string address = 2; // 账户发起地址 Account address
    string toaddr = 3 ; // 账户接收地址
    uint64 time               = 4;
    uint64  type = 5;  //为何种交易类型
}
当接服务端接受到GetSDKReq的时候，进行协议解析返回对应的交易结构的数据。

对应的网络协议：
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



/**********通用消息体********/
/**********Generic message body********/
message CommonMsg 
{
  string version         = 1; //发送的版本
  string type           = 2; //发送的数据类型
  int32 encrypt         = 3; //是否加密
  int32 compress        = 4; //是否压缩
  bytes data            = 5; //对应的数据包
  bytes pub    = 6;  //公钥
  bytes sign = 7;  //签名
  bytes key = 8;  //预留
}
核心思想是手机端通过短链接，利用protobuf协议发送到服务端进行数据包的请求，
服务端收到请求之后解析是返回对应的交易结构的数据包，手机端接受到之后利用protobuf的映射机制
自动匹配到对应的回调函数，进行交易体的组包，组包结束之后再进行广播交易由uenc网络结构里边的
节点进行对交易结构签名和验签。
如果本笔交易到达共识数之后交易会进行上链。手机端会查看到交易成功与否。