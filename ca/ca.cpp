// #include <fstream>
// #include <sstream>
#include <string>
#include <random>
#include <map>
#include <array>
#include <fcntl.h>
#include <thread>
#include <shared_mutex>
#include <iomanip>
#include "ca.h"
#include "unistd.h"
//#include "include/logging.h"
#include "ca_handle_event.h"
#include "utils/qrcode.h"
#include "utils/string_util.h"
#include "utils/util.h"
#include "utils/time_util.h"
#include "utils/base64.h"
#include "utils/base64_2.h"
#include "utils/bip39.h"
#include "utils/MagicSingleton.h"
#include "utils/hexcode.h"
#include "utils/console.h"
#include "ca_txhelper.h"
#include "ca_transaction.h"
#include "ca_global.h"
#include "ca_txhelper.h"
#include "proto/sdk.pb.h"
#include "common/global.h"
//#include "common/config.h"
#include "ca_protomsg.pb.h"
#include "utils/EDManager.h"
#include "../proto/sdk.pb.h"
#include "net/Net.h"
#include "net/net_api.h"
#include "net/connect.h"
#include "net/message_process.h"
#include "net/dispatcher.h"


extern "C" {

static   std::string ip;
static   uint64_t port;

void InitStart()
{
    MagicSingleton<MessageProcess>::GetInstance()->ThreadStart();
	MagicSingleton<ProtobufDispatcher>::GetInstance()->registerAll();
}



bool InitAccount(const char *path,int path_len)
{
    std::string input_path(path,path_len);
    std::cout<<"input_path = "<<input_path<<std::endl;
    if(!input_path.empty())
	{
		MagicSingleton<Recver>::GetInstance()->set_accountpath(input_path.data());
	}
	MagicSingleton<EDManager>::GetInstance();
	return true;
}

void set_ip_port(const std::string &node_ip,uint32_t &node_port)
{
    ip = node_ip;
    port = node_port;
}


//bool Require_config_random_node(const std::string &ip ,uint32_t &port)
bool Require_config_random_node(const char *ip ,uint32_t ip_len,uint32_t port)
{
    std::string input_ip(ip,ip_len);
    std::cout<<"before connect ip = "<< input_ip<<std::endl;
    std::cout<<"before connect port = "<< port<<std::endl;
    if(!MagicSingleton<net>::GetInstance()->connect(input_ip, port))
    {
        return false;
    }
    std::cout<<"afeter connect ip = "<< input_ip<<std::endl;
    std::cout<<"afeter connect port = "<< port<<std::endl;
    set_ip_port(ip,port);
    return true;
}

// bool Require_config_random_node()
// {
//     // auto config_server_list = MagicSingleton<Config>::GetInstance()->GetServer();
//     // uint32_t port = MagicSingleton<Config>::GetInstance()->GetServerPort();
//     std::vector<std::string> server;
//     server.push_back("192.168.1.67");
//     uint32_t port = 41516 ;
    
//    // server.assign(config_server_list.begin(),config_server_list.end());

//     std::random_device device;
// 	std::mt19937 engine(device());
// 	std::uniform_int_distribution<size_t> dist(0, server.size() - 1);
//     size_t random = dist(engine);
//     std::string ip =  server.at(random);
   

//     if(!MagicSingleton<net>::GetInstance()->connect(ip, port))
//     {
//         return false;
//     }
//     // if(!MagicSingleton<Net>::GetInstance()->connect(ip, port))
//     // {
//     //     return false;
//     // }
//     std::cout<<"ip = "<< ip<<std::endl;
//     std::cout<<"port = "<< port<<std::endl;
//     set_ip_port(ip,port);
//     return true;
// }

//bool handle_transaction(const std::string &fromAddr,const std::string &strToAddr,const std::string &strAmt)
bool handle_transaction(const char *FromAddr,int fromlen,
                        const  char *ToAddr, int tolen,
                        const char *Amt,int amtlen)
{   
    std::string fromAddr(FromAddr,fromlen);
    std::string strToAddr(ToAddr,tolen);
    std::string strAmt(Amt,amtlen);
    std::vector<std::string> vecfromAddr;
    vecfromAddr.emplace_back(fromAddr);
    set_Fromaddr(vecfromAddr);
    set_toaddr(strToAddr);
    set_amount(strAmt);

    GetSDKReq  sdkReq;
    sdkReq.set_version(global::kVersion);
    sdkReq.set_type(1);
    
    for(const auto &addr :vecfromAddr)
    {
        sdkReq.add_address(addr);
    }   

    std::string outdata;
    SendMessage(sdkReq.SerializeAsString(), sdkReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;

//    int ret =  MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
//    if(ret > 0)
//    {
//         return true;
//    }
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    //     return true;
    // }

    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    //usleep(3000000);
    sleep(1);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}

//void handle_stake(const std::string &fromAddr,const std::string &strStakeFee)
bool handle_stake(const char *FromAddr,int fromlen,
                   const char * StakeFee,int amtlen)
{
    std::string fromAddr(FromAddr,fromlen);
    std::string strStakeFee(StakeFee,amtlen);
  
    std::vector<std::string> vecfromAddr;
    vecfromAddr.emplace_back(fromAddr);
    set_Fromaddr(vecfromAddr);
    set_amount(strStakeFee);

    GetSDKReq  sdkReq;
    sdkReq.set_version(global::kVersion);
    sdkReq.set_type(2);
    for(const auto &addr :vecfromAddr)
    {
        sdkReq.add_address(addr);
    }   
    std::string outdata;
    SendMessage(sdkReq.SerializeAsString(), sdkReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    // }
    //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    sleep(1);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}


//void handle_unstake(const std::string &fromAddr,const std::string &strUtxoHash)
bool handle_unstake(const char *FromAddr,int fromlen,
                    const char *UtxoHash,int hashlen)
{  
    std::string fromAddr(FromAddr,fromlen);
    std::string strUtxoHash(UtxoHash,hashlen);

    std::vector<std::string> vecfromAddr;
    vecfromAddr.emplace_back(fromAddr);
    set_Fromaddr(vecfromAddr);
    set_utxo(strUtxoHash);
   
    GetSDKReq  sdkReq;
    sdkReq.set_version(global::kVersion);
    sdkReq.set_type(3);
    
    for(auto &addr :vecfromAddr)
    {
        sdkReq.add_address(addr);
    }  
    std::string outdata;
    SendMessage(sdkReq.SerializeAsString(), sdkReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    // } 
    //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    sleep(1);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}


//void handle_invest(const std::string &fromAddr,const std::string &strToAddr,const std::string &strInvestFee)
bool handle_invest(const char *FromAddr,int fromlen,
                   const  char *ToAddr, int tolen,
                   const char *Amt,int amtlen)
{
    std::string fromAddr(FromAddr,fromlen);
    std::string strToAddr(ToAddr,tolen);
    std::string strInvestFee(Amt,amtlen);


    std::vector<std::string> vecfromAddr;
    vecfromAddr.emplace_back(fromAddr);
    set_Fromaddr(vecfromAddr);
    set_toaddr(strToAddr);
    set_amount(strInvestFee);

    GetSDKReq  sdkReq;
    sdkReq.set_version(global::kVersion);
    sdkReq.set_type(4);
   
    for(const auto &addr :vecfromAddr)
    {
        sdkReq.add_address(addr);
    }   
    std::string outdata;
    SendMessage(sdkReq.SerializeAsString(), sdkReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    // }
   // MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    sleep(1);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}


//void handle_disinvest(const std::string &fromAddr,const std::string &strToAddr,const std::string &strUtxoHash)
bool handle_disinvest(const char *FromAddr,int fromlen,
                      const char *ToAddr, int tolen,
                      const char * UtxoHash,int hashlen)
{
    std::string fromAddr(FromAddr,fromlen);
    std::string strToAddr(ToAddr,tolen);
    std::string strUtxoHash(UtxoHash,hashlen);


    std::vector<std::string> vecfromAddr;
    vecfromAddr.emplace_back(fromAddr);
	set_Fromaddr(vecfromAddr);
    set_toaddr(strToAddr);
    set_utxo(strUtxoHash);
   
  
    GetSDKReq  sdkReq;
    sdkReq.set_version(global::kVersion);
    sdkReq.set_type(5);
   
    for(const auto &addr :vecfromAddr)
    {
        sdkReq.add_address(addr);
    }   
    sdkReq.set_toaddr(strToAddr);
    std::string outdata;
    SendMessage(sdkReq.SerializeAsString(), sdkReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    // }
    //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    sleep(1);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}


//void handle_bonus(const std::string &fromAddr)
bool handle_bonus(const char *FromAddr,int fromlen)
{
    std::string fromAddr(FromAddr,fromlen);
    std::vector<std::string> vecfromAddr;
    vecfromAddr.emplace_back(fromAddr); 
    set_Fromaddr(vecfromAddr);	
    
    GetSDKReq  sdkReq;
    sdkReq.set_version(global::kVersion);
    sdkReq.set_type(6);
  
    for(const auto &addr :vecfromAddr)
    {
        sdkReq.add_address(addr);
    }   
    uint64_t cur_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
    sdkReq.set_time(cur_time);
    set_curtime(cur_time);
    std::string outdata;
    SendMessage(sdkReq.SerializeAsString(), sdkReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    // }
    //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    sleep(1);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}


bool require_balance_height()
{
    // std::string ip = "192.168.1.67";
    // uint32_t port;
    // Require_config_random_node(ip,port);
    std::string version = global::kVersion;
    std::string defaultbase58 = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
    GetBalanceReq BalanceReq;
    BalanceReq.set_version(global::kVersion);
    BalanceReq.set_address(defaultbase58);
    std::string outdata;
    SendMessage(BalanceReq.SerializeAsString(), BalanceReq.GetDescriptor()->name(),outdata);
    std::cout<<"outdata = "<<outdata<<std::endl;
    // if(MagicSingleton<Net>::GetInstance()->isConnected(ip))
    // {
    //     MagicSingleton<Net>::GetInstance()->sendData(ip,outdata.data(),outdata.size());
    //     return true;
    // }
    auto iter = MagicSingleton<net>::GetInstance();
    int ret = iter->send(outdata.data(),outdata.size());
    if(ret < 0)
    {
        return false;
    }
    usleep(30000);
    
    MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    return true;
}



void gen_key()
{
    std::cout << "Please enter the number of accounts to be generated: ";
    int num = 0;
    std::cin >> num;
    if (num <= 0)
    {
        return;
    }

    std::cout << "please input Normal addr or MultiSign addr" << std::endl;
    std::cout << "0. Normal addr" << std::endl;
    std::cout << "1. MultiSign addr" << std::endl;

    int iVer = 0;
    std::cin >> iVer;

    Base58Ver ver;
    if (iVer == 0)
    {
        ver = Base58Ver::kBase58Ver_Normal;
    }
    else if (iVer == 1)
    {
        ver = Base58Ver::kBase58Ver_MultiSign;
    }
    else
    {
        std::cout << "error input" << std::endl;
        return;
    }

    for (int i = 0; i != num; ++i)
    {
        ED acc(ver);
        MagicSingleton<EDManager>::GetInstance()->AddAccount(acc);
        MagicSingleton<EDManager>::GetInstance()->SavePrivateKeyToFile(acc.base58Addr);
    }
}


void handle_AccountManger()
{
    MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();

    std::cout << std::endl
              << std::endl;
    while (true)
    {
        std::cout << "0.Exit" << std::endl;
        std::cout << "1. Set Defalut Account" << std::endl;
        std::cout << "2. Add Account" << std::endl;
        std::cout << "3. Remove " << std::endl;
        std::cout << "4. Import PrivateKey" << std::endl;
        std::cout << "5. Export PrivateKey" << std::endl;

        std::string strKey;
        std::cout << "Please input your choice: " << std::endl;
        std::cin >> strKey;
        std::regex pattern("^[0-6]$");
        if (!std::regex_match(strKey, pattern))
        {
            std::cout << "Invalid input." << std::endl;
            continue;
        }
        int key = std::stoi(strKey);
        switch (key)
        {
        case 0:
            return;
        case 1:
            handle_SetdefaultAccount();
            break;
        case 2:
            gen_key();
            break;
        case 3:
        {
            std::string addr;
            std::cout << "Please enter the address you want to remove :" << std::endl;
            std::cin >> addr;

            if (MagicSingleton<EDManager>::GetInstance()->DeleteAccount(addr) != 0)
            {
                std::cout << "failed!" << std::endl;
            }
            break;
        }
        case 4:
        {
            std::string pri_key;
            std::cout << "Please input private key :" << std::endl;
            std::cin >> pri_key;

            if (MagicSingleton<EDManager>::GetInstance()->ImportPrivateKeyHex(pri_key) != 0)
            {
                std::cout << "Save PrivateKey failed!" << std::endl;
            }
            break;
        }
        case 5:
            handle_export_private_key();
            break;
        default:
            std::cout << "Invalid input." << std::endl;
            continue;
        }
    }
}

void handle_SetdefaultAccount()
{
    std::string addr;
    std::cout << "Please enter the address you want to set :" << std::endl;
    std::cin >> addr;
    if (addr[0] == '3')
    {
        std::cout << "The Default account cannot be MultiSign Addr" << std::endl;
        return;
    }

    ED oldAccount;
    EVP_PKEY_free(oldAccount.pkey);
    if (MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(oldAccount) != 0)
    {
        //ERRORLOG("not found DefaultKeyBs58Addr  in the _accountList");
        return;
    }

    if (MagicSingleton<EDManager>::GetInstance()->SetDefaultAccount(addr) != 0)
    {
        //ERRORLOG("Set DefaultKeyBs58Addr failed!");
        return;
    }

    ED newAccount;
    EVP_PKEY_free(newAccount.pkey);
    if (MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(newAccount) != 0)
    {
        //ERRORLOG("not found DefaultKeyBs58Addr  in the _accountList");
        return;
    }

    if (!CheckBase58Addr(oldAccount.base58Addr, Base58Ver::kBase58Ver_Normal) ||
        !CheckBase58Addr(newAccount.base58Addr, Base58Ver::kBase58Ver_Normal))
    {
        return;
    }

    // update base 58 addr
//     NodeBase58AddrChangedReq req;
//     req.set_version(global::kVersion);

//     NodeSign *oldSign = req.mutable_oldsign();
//     oldSign->set_pub(oldAccount.pubStr);
//     std::string oldSignature;
//     if (!oldAccount.Sign(getsha256hash(newAccount.base58Addr), oldSignature))
//     {
//         return;
//     }
//     oldSign->set_sign(oldSignature);

//     NodeSign *newSign = req.mutable_newsign();
//     newSign->set_pub(newAccount.pubStr);
//     std::string newSignature;
//     if (!newAccount.Sign(getsha256hash(oldAccount.base58Addr), newSignature))
//     {
//         return;
//     }
//     newSign->set_sign(newSignature);

//     // auto selfNode = MagicSingleton<PeerNode>::GetInstance()->get_self_node();
//    // MagicSingleton<PeerNode>::GetInstance()->set_self_id(newAccount.base58Addr);
//     //MagicSingleton<PeerNode>::GetInstance()->set_self_identity(newAccount.pubStr);
//     std::vector<Node> publicNodes = MagicSingleton<PeerNode>::GetInstance()->get_nodelist();
//     for (auto &node : publicNodes)
//     {
//         net_com::send_message(node, req, net_com::Compress::kCompress_False, net_com::Encrypt::kEncrypt_False, net_com::Priority::kPriority_High_2);
//     }
//     std::cout << "Set Default account success" << std::endl;
}




//void Set_defaultAccount(const std::string &addr)
void Set_defaultAccount(const char* base58 ,int base58_len)
{
    std::string addr(base58,base58_len);
    if (addr[0] == '3')
    {
        std::cout << "The Default account cannot be MultiSign Addr" << std::endl;
        return;
    }

    ED oldAccount;
    EVP_PKEY_free(oldAccount.pkey);
    if (MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(oldAccount) != 0)
    {
        //ERRORLOG("not found DefaultKeyBs58Addr  in the _accountList");
        return;
    }

    if (MagicSingleton<EDManager>::GetInstance()->SetDefaultAccount(addr) != 0)
    {
        //ERRORLOG("Set DefaultKeyBs58Addr failed!");
        return;
    }

    ED newAccount;
    EVP_PKEY_free(newAccount.pkey);
    if (MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(newAccount) != 0)
    {
        //ERRORLOG("not found DefaultKeyBs58Addr  in the _accountList");
        return;
    }

    if (!CheckBase58Addr(oldAccount.base58Addr, Base58Ver::kBase58Ver_Normal) ||
        !CheckBase58Addr(newAccount.base58Addr, Base58Ver::kBase58Ver_Normal))
    {
        return;
    }
}

void Add_Account(int num,int iVer)
{
    Base58Ver ver;
    if (iVer == 0)
    {
        ver = Base58Ver::kBase58Ver_Normal;
    }
    else if (iVer == 1)
    {
        ver = Base58Ver::kBase58Ver_MultiSign;
    }
    else
    {
        std::cout << "error input" << std::endl;
        return;
    }

    for (int i = 0; i != num; ++i)
    {
        ED acc(ver);
        MagicSingleton<EDManager>::GetInstance()->AddAccount(acc);
        MagicSingleton<EDManager>::GetInstance()->SavePrivateKeyToFile(acc.base58Addr);
    }

}


bool Delete_Account(const char* base58 ,int base58_len)
//bool Delete_Account(const std::string &addr)
{
    std::string addr(base58,base58_len);
    if (MagicSingleton<EDManager>::GetInstance()->DeleteAccount(addr) != 0)
    {
        return false;
    }
    return true;
}

//bool  Import_Account(const std::string &pri_key)
bool Import_Account(const char* pri_key ,int pri_key_len)
{
    std::string strpri_key(pri_key,pri_key_len);
    if (MagicSingleton<EDManager>::GetInstance()->ImportPrivateKeyHex(strpri_key) != 0)
    {
        return false;
    }
    return true;
}

//void Export_private_key(const std::string &addr,std::string &Mnemonic,std::string &PriHex,std::string &PubHex)
void Export_private_key(const char * base58,int base58_len,
                        char *mnemonic, 
                        char *out_private_key, int *out_private_len,
                        char *out_public_key,int *out_public_key_len)
{
    std::string addr(base58,base58_len);
    ED account;
    EVP_PKEY_free(account.pkey);
    MagicSingleton<EDManager>::GetInstance()->FindAccount(addr, account);

    char out_data[1024] = {0};
    int data_len = sizeof(out_data);
    mnemonic_from_data((const uint8_t *)account.priStr.c_str(), account.priStr.size(), out_data, data_len);
    memcpy(mnemonic, out_data, strlen(out_data));

    std::string Mnemonic = std::string(out_data,strlen(out_data));
    std::cout << "Mnemonic: " << out_data << std::endl;
    std::cout << "Mnemonic: " << Mnemonic << std::endl;

    std::string PriHex = Str2Hex(account.priStr);
    memcpy(out_private_key, PriHex.data(), PriHex.size());
	*out_private_len = PriHex.size();

    std::string PubHex = Str2Hex(account.pubStr);
    memcpy(out_public_key, PubHex.data(), PubHex.size());
	*out_public_key_len = PubHex.size();
   
    std::cout << "Private key: " << PriHex << std::endl;
    std::cout << "Private key: " << PubHex << std::endl;
    return;
}

        

void set_phoneptr(txdata_callback back_ptr)
{
    MagicSingleton<Recver>::GetInstance()->phone_setptr(back_ptr);
}
        
      
void set_phonegasptr(txgas_callback gasback_ptr)
{
   MagicSingleton<Recver>::GetInstance()->phone_setgasptr(gasback_ptr);
}
        

std::string readFileIntoString(std::string filename)
{
	std::ifstream ifile(filename);
	std::ostringstream buf;
	char ch;
	while(buf&&ifile.get(ch))
    {
        buf.put(ch);
    }
	return buf.str();
}



void handle_export_private_key()
{
    std::cout << std::endl
              << std::endl;
    // 1 private key, 2 annotation, 3 QR code
    std::string fileName("account_private_key.txt");
    std::ofstream file;
    file.open(fileName);
    std::string addr;
    std::cout << "please input the addr you want to export" << std::endl;
    std::cin >> addr;

    ED account;
    EVP_PKEY_free(account.pkey);
    MagicSingleton<EDManager>::GetInstance()->FindAccount(addr, account);

    file << "Please use Courier New font to view" << std::endl
         << std::endl;
    // for (auto& item : g_AccountInfo.AccountList)
    // {
    file << "Base58 addr: " << addr << std::endl;
    std::cout << "Base58 addr: " << addr << std::endl;

    char out_data[1024] = {0};
    int data_len = sizeof(out_data);
    mnemonic_from_data((const uint8_t *)account.priStr.c_str(), account.priStr.size(), out_data, data_len);
    file << "Mnemonic: " << out_data << std::endl;
    std::cout << "Mnemonic: " << out_data << std::endl;

    std::string strPriHex = Str2Hex(account.priStr);
    file << "Private key: " << strPriHex << std::endl;
    std::cout << "Private key: " << strPriHex << std::endl;

    file << "QRCode:";
    std::cout << "QRCode:";

    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(5)];
    qrcode_initText(&qrcode, qrcodeData, 5, ECC_MEDIUM, strPriHex.c_str());

    file << std::endl
         << std::endl;
    std::cout << std::endl
              << std::endl;

    for (uint8_t y = 0; y < qrcode.size; y++)
    {
        file << "        ";
        std::cout << "        ";
        for (uint8_t x = 0; x < qrcode.size; x++)
        {
            file << (qrcode_getModule(&qrcode, x, y) ? "\u2588\u2588" : "  ");
            std::cout << (qrcode_getModule(&qrcode, x, y) ? "\u2588\u2588" : "  ");
        }

        file << std::endl;
        std::cout << std::endl;
    }

    file << std::endl
         << std::endl
         << std::endl
         << std::endl
         << std::endl
         << std::endl;
    std::cout << std::endl
              << std::endl
              << std::endl
              << std::endl
              << std::endl
              << std::endl;
    // }

    ca_console redColor(kConsoleColor_Red, kConsoleColor_Black, true);
    std::cout << redColor.color() << "You can also view above in file:" << fileName << " of current directory." << redColor.reset() << std::endl;
    return;
}











/**
 * @description: Registering Callbacks
 * @param {*}
 * @return {*}
 */
void RegisterCallback()
{
    // net_register_callback<FastSyncGetHashReq>(HandleFastSyncGetHashReq);
    // net_register_callback<FastSyncGetHashAck>(HandleFastSyncGetHashAck);
    // net_register_callback<FastSyncGetBlockReq>(HandleFastSyncGetBlockReq);
    // net_register_callback<FastSyncGetBlockAck>(HandleFastSyncGetBlockAck);

    // net_register_callback<SyncGetSumHashReq>(HandleSyncGetSumHashReq);
    // net_register_callback<SyncGetSumHashAck>(HandleSyncGetSumHashAck);
    // net_register_callback<SyncGetHeightHashReq>(HandleSyncGetHeightHashReq);
    // net_register_callback<SyncGetHeightHashAck>(HandleSyncGetHeightHashAck);
    // net_register_callback<SyncGetBlockReq>(HandleSyncGetBlockReq);
    // net_register_callback<SyncGetBlockAck>(HandleSyncGetBlockAck);

    // net_register_callback<SyncFromZeroGetSumHashReq>(HandleFromZeroSyncGetSumHashReq);
    // net_register_callback<SyncFromZeroGetSumHashAck>(HandleFromZeroSyncGetSumHashAck);
    // net_register_callback<SyncFromZeroGetBlockReq>(HandleFromZeroSyncGetBlockReq);
    // net_register_callback<SyncFromZeroGetBlockAck>(HandleFromZeroSyncGetBlockAck);

    // net_register_callback<GetBlockByUtxoReq>(HandleBlockByUtxoReq);
    // net_register_callback<GetBlockByUtxoAck>(HandleBlockByUtxoAck);

    // net_register_callback<GetBlockByHashReq>(HandleBlockByHashReq);
    // net_register_callback<GetBlockByHashAck>(HandleBlockByHashAck);

    // // PCEnd correlation
    // net_register_callback<TxMsgReq>(HandleTx); // PCEnd transaction flow
    // net_register_callback<TxMsgAck>(HandleDoHandleTxAck);
    // net_register_callback<BlockMsg>(HandleBlock);                                         // PCEnd transaction flow
    // net_register_callback<ConfirmTransactionIsSuccessReq>(HandleConfirmTransactionIsSuccessReq);
    // net_register_callback<ConfirmTransactionIsSuccessAck>(HandleConfirmAck);
    // net_register_callback<FailureBroadcastMsg>(HandleFailureBroadcast);

    // net_register_callback<BuildBlockBroadcastMsgAck>(HandleAddBlockAck);

    // net_register_callback<TxPendingBroadcastMsg>(HandleTxPendingBroadcastMsg);   // Transaction pending broadcast
    // net_register_callback<BuildBlockBroadcastMsg>(HandleBuildBlockBroadcastMsg); // Building block broadcasting

    // // Mobile related
    // net_register_callback<CreateTxMsgReq>(HandleCreateTxInfoReq);                   // The first step of one-to-one transaction (old version) of main network account of mobile terminal
    // net_register_callback<TxMsgReq>(HandlePreTxRaw);                                // The second step of one-to-one transaction (old version) of the main network account of the mobile terminal
    // net_register_callback<CreateMultiTxMsgReq>(HandleCreateMultiTxReq);             // The first step of launching multiple transactions in the main network account of the mobile terminal
    // net_register_callback<MultiTxMsgReq>(HandleMultiTxReq);                         // The second step of launching multiple transactions in the main network account of the mobile terminal

    // net_register_callback<CreateStakeTxMsgReq>(HandleCreateStakeTxMsgReq);        // The first step of initiating pledge for the main network account of mobile terminal
    // net_register_callback<StakeTxMsgReq>(HandleStakeTxMsgReq);                    // The second step of launching pledge for the main network account of mobile terminal
    // net_register_callback<CreateUnstakeTxMsgReq>(HandleCreateUnstakeTxMsgReq);        // The first step of cancelling the pledge initiated by the main network account of the mobile terminal
    // net_register_callback<UnstakeTxMsgReq>(HandleUnstakeTxMsgReq);                    // The second step of releasing the pledge is initiated by the main network account of the mobile terminal
    // net_register_callback<CreateInvestTxMsgReq>(HandleCreateInvestTxMsgReq);
    // net_register_callback<InvestTxMsgReq>(HandleInvestTxMsgReq);
    // net_register_callback<CreateDivestTxMsgReq>(HandleCreateDivestTxMsgReq);
    // net_register_callback<DivestTxMsgReq>(HandleDivestTxMsgReq);

    // // Confirm whether the transaction is successful
    // net_register_callback<ConfirmTransactionReq>(HandleConfirmTransactionReq);
    // net_register_callback<ConfirmTransactionAck>(HandleConfirmTransactionAck);

    // net_register_callback<MultiSignTxReq>(HandleMultiSignTxReq);

    // net_register_chain_height_callback();
}



int checkNtpTime()
{
    // Ntp check
    int64_t getNtpTime = MagicSingleton<TimeUtil>::GetInstance()->getNtpTimestamp();
    int64_t getLocTime = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();

    int64_t tmpTime = abs(getNtpTime - getLocTime);

    std::cout << "UTC Time: " << MagicSingleton<TimeUtil>::GetInstance()->formatUTCTimestamp(getLocTime) << std::endl;
    std::cout << "Ntp Time: " << MagicSingleton<TimeUtil>::GetInstance()->formatUTCTimestamp(getNtpTime) << std::endl;

    if (tmpTime <= 1000000)
    {
        //DEBUGLOG("ntp timestamp check success");
        return 0;
    }
    else
    {
        //DEBUGLOG("ntp timestamp check fail");
        std::cout << "time check fail" << std::endl;
        return -1;
    }
}

}







