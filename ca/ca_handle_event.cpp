#include "ca_handle_event.h"
#include "../net/net_api.h"
#include "utils/MagicSingleton.h"
#include "net/net_api.h"
#include "utils/base58.h"
#include "utils/hexcode.h"
#include "utils/string_util.h"
#include <string>
#include "proto/common.pb.h"
#include  "proto/interface.pb.h"
#include "utils/EDManager.h"
#include "../net/ip_port.h"
#include "ca_txhelper.h"
#include "common/global.h"
#include <iostream>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <random>
#include <iomanip>
#include <boost/functional/hash.hpp>
#include "utils/util.h"
#include "ca/ca_global.h"
#include <shared_mutex>
#include <mutex>
#include <memory>
#include <string>
#include "ca_txhelper.h"
//#include "include/logging.h"
#include "utils/string_util.h"
#include "utils/time_util.h"
#include "utils/json.hpp"
#include "ca/ca_global.h"
#include "../ca/ca_transaction.h"
#include "utils/console.h"
#include "utils/EDManager.h"
#include "utils/tmplog.h"
#include "transaction.pb.h"
#include "proto/ca_protomsg.pb.h"
#include "proto/sdk.pb.h"
#include "proto/transaction.pb.h"
#include "net/connect.h"


using namespace std;
static   std::vector<std::string> from_Addr;
static   std::string str_ToAddr;
static   std::string str_Amt;
static   std::string str_utxo;
static   uint64_t cur_time;
static   uint32_t nContractType;


void set_Fromaddr(const std::vector<std::string> &fromaddr)
{
    from_Addr.assign(fromaddr.begin(),fromaddr.end());
}

void set_toaddr(const std::string &toaddr)
{
    str_ToAddr = toaddr;
}

void set_amount(const std::string &amount)
{
    str_Amt = amount;
}

void set_utxo(const std::string &utxo)
{
    str_utxo = utxo;
}

void set_curtime(const uint64_t &time)
{
    cur_time = time;
}


int HandleTransactionTxMsgAck(const std::shared_ptr<TxMsgAck> &ack, const MsgData &from)
{
    std::cout<<"TxMsgAck---->message = "<<ack->message()<<std::endl;
    std::cout<<"TxMsgAck---->code"<<ack->code()<<std::endl;
    std::cout<<"from.ip = "<<from.ip <<std::endl;
    std::cout<<"from.port = "<<from.port <<std::endl;
   
    auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getptr();
    if(call_back == nullptr)
    {
        return -1;
    }

    
    CTransaction Tx;
    Tx.ParseFromString(ack->tx());
    int type = Tx.txtype();
    call_back(ack->message().data(),ack->code(),type);
    MagicSingleton<net>::GetInstance()->close();
   // MagicSingleton<Net>::GetInstance()->disconnect(from.ip);
    return 0;
}


int HandleSdkTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from)
{
    if(ack->type() == 1)
    {
        std::cout<<""<<std::endl;

        MagicSingleton<net>::GetInstance()->close();
        //MagicSingleton<Net>::GetInstance()->disconnect(from.ip);
        std::cout<<"*********************"<<std::endl;
        std::cout<<"str_fromAddr ="<<from_Addr.at(0)<<std::endl;
        std::cout<<"str_ToAddr ="<<str_ToAddr<<std::endl;
        std::cout<<"str_Amt ="<<str_Amt<<std::endl;

        uint64_t amount = (std::stod(str_Amt) + global::ca::kFixDoubleMinPrecision) * global::ca::kDecimalNum;
        std::map<std::string, int64_t> toAddrAmount;
        toAddrAmount[str_ToAddr] = amount;
    
        uint64_t top = ack->height();
        CTransaction outTx;
        TxHelper::vrfAgentType isNeedAgent_flag;

        Vrf info_;
        std::pair<std::string, uint64_t> node_ip_port;
        int ret = TxHelper::CreateTxTransaction(from_Addr, toAddrAmount, top + 1,  outTx,isNeedAgent_flag,info_,ack,node_ip_port);
        if (ret != 0)
        {
            //ERRORLOG("CreateTxTransaction error!! ret:{}", ret);
            return ret;
        }
        
        std::string newbase58 = outTx.identity();
        TxMsgReq txMsg;
        txMsg.set_version(global::kVersion);
        TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
        txMsgInfo->set_type(0);
        
        txMsgInfo->set_tx(outTx.SerializeAsString());
        txMsgInfo->set_height(top);

        if(isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf)
        {
            Vrf * new_info=txMsg.mutable_vrfinfo();
            new_info -> CopyFrom(info_);
        }
 
        std::string outdata;
        SendMessage(txMsg.SerializeAsString(), txMsg.GetDescriptor()->name(),outdata);
        // if(MagicSingleton<Net>::GetInstance()->isConnected(node_ip_port.first))
        // {
        //     std::cout<<"in in --->node_ip_port.first = "<<node_ip_port.first<<std::endl;
         //    MagicSingleton<Net>::GetInstance()->sendData(node_ip_port.first,outdata.data(),outdata.size());
        // }
        auto iter = MagicSingleton<net>::GetInstance();
        int result = iter->send(outdata.data(),outdata.size());
        if(result < 0)
        {
            return -1;
        }
        sleep(1);
        MagicSingleton<Recver>::GetInstance()->ReadData(iter);
       // MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
        std::cout<<"out out ---->node_ip_port.first = "<<node_ip_port.first<<std::endl;
    }
    else if(ack->type() == 2)
    {
         TxHelper::PledgeType pledgeType = TxHelper::PledgeType::kPledgeType_Node;

        uint64_t stake_amount = std::stod(str_Amt) * global::ca::kDecimalNum;
        uint64_t top = ack->height();

        CTransaction outTx;
        std::vector<TxHelper::Utxo> outVin;
        TxHelper::vrfAgentType isNeedAgent_flag;
        Vrf info_;
        std::pair<std::string, uint64_t> node_ip_port;
        int ret = TxHelper::CreateStakeTransaction(from_Addr.at(0), stake_amount, top + 1,  pledgeType, outTx, outVin,isNeedAgent_flag,info_,ack,node_ip_port);
        if (ret != 0)
        {
            //ERRORLOG("CreateTxTransaction error!! ret:{}", ret);
            return ret;
        }
    
        TxMsgReq txMsg;
        txMsg.set_version(global::kVersion);
        std::string newbase58 = outTx.identity();
        TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
        txMsgInfo->set_type(0);
        txMsgInfo->set_tx(outTx.SerializeAsString());
        txMsgInfo->set_height(top);

        if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf)
        {
            Vrf * new_info=txMsg.mutable_vrfinfo();
            new_info->CopyFrom(info_);
        }

        std::string outdata;
        SendMessage(txMsg.SerializeAsString(), txMsg.GetDescriptor()->name(),outdata);
        auto iter = MagicSingleton<net>::GetInstance();
        int result = iter->send(outdata.data(),outdata.size());
        if(result < 0)
        {
            return -1;
        }
        sleep(1);
        MagicSingleton<Recver>::GetInstance()->ReadData(iter);
        // if(MagicSingleton<Net>::GetInstance()->isConnected(node_ip_port.first))
        // {
        //     std::cout<<"in in --->node_ip_port.first = "<<node_ip_port.first<<std::endl;
        //     MagicSingleton<Net>::GetInstance()->sendData(node_ip_port.first,outdata.data(),outdata.size());
        // }
    }
    else if(ack->type() == 3)
    {
        uint64_t top = ack->height();
        CTransaction outTx;
        std::vector<TxHelper::Utxo> outVin;
        TxHelper::vrfAgentType isNeedAgent_flag;
        Vrf info_;
        std::pair<std::string, uint64_t> node_ip_port;
        int ret = TxHelper::CreatUnstakeTransaction(from_Addr.at(0), str_utxo, top + 1, outTx, outVin,isNeedAgent_flag,info_,ack,node_ip_port);
         if (ret != 0)
        {
            return ret;
        }

        
        TxMsgReq txMsg;
        txMsg.set_version(global::kVersion);
        std::string newbase58 = outTx.identity();
        TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
        txMsgInfo->set_type(0);
        txMsgInfo->set_tx(outTx.SerializeAsString());
        txMsgInfo->set_height(top);

        if(isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf)
        {
            Vrf * new_info=txMsg.mutable_vrfinfo();
            new_info->CopyFrom(info_);

        }
        std::string outdata;
        SendMessage(txMsg.SerializeAsString(), txMsg.GetDescriptor()->name(),outdata);
        // if(MagicSingleton<Net>::GetInstance()->isConnected(node_ip_port.first))
        // {
        //     std::cout<<"in in --->node_ip_port.first = "<<node_ip_port.first<<std::endl;
        //     MagicSingleton<Net>::GetInstance()->sendData(node_ip_port.first,outdata.data(),outdata.size());
        // }
       // MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
        auto iter = MagicSingleton<net>::GetInstance();
        int result = iter->send(outdata.data(),outdata.size());
        if(result < 0)
        {
            return -1;
        }
        sleep(1);
        MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    }
    else if(ack->type() == 4)
    {
        TxHelper::InvestType investType = TxHelper::InvestType::kInvestType_NetLicence;
        uint64_t invest_amount = std::stod(str_Amt) * global::ca::kDecimalNum;

        uint64_t top = ack->height();
        CTransaction outTx;
        std::vector<TxHelper::Utxo> outVin;
        TxHelper::vrfAgentType isNeedAgent_flag;
        Vrf info_;
        std::pair<std::string, uint64_t> node_ip_port;
        int ret = TxHelper::CreateInvestTransaction(from_Addr.at(0), str_ToAddr, invest_amount, top + 1,  investType, outTx, outVin,isNeedAgent_flag,info_,ack,node_ip_port);
        if (ret != 0)
        {
            //ERRORLOG("Failed to create investment transaction! The error code is:{}", ret);
            return ret;
        }

        TxMsgReq txMsg;
        txMsg.set_version(global::kVersion);
        std::string newbase58 = outTx.identity();
        TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
        txMsgInfo->set_type(0);
        txMsgInfo->set_tx(outTx.SerializeAsString());
        txMsgInfo->set_height(top);

        if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf)
        {
            Vrf * new_info=txMsg.mutable_vrfinfo();
            new_info->CopyFrom(info_);

        }
        std::string outdata;
        SendMessage(txMsg.SerializeAsString(), txMsg.GetDescriptor()->name(),outdata);
        // if(MagicSingleton<Net>::GetInstance()->isConnected(node_ip_port.first))
        // {
        //     std::cout<<"in in --->node_ip_port.first = "<<node_ip_port.first<<std::endl;
        //     MagicSingleton<Net>::GetInstance()->sendData(node_ip_port.first,outdata.data(),outdata.size());
        // }
        //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
        auto iter = MagicSingleton<net>::GetInstance();
        int result = iter->send(outdata.data(),outdata.size());
        if(result < 0)
        {
            return -1;
        }
        sleep(1);
        MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    }
    else if(ack->type() == 5)
    {
        uint64_t top = ack->height();
        CTransaction outTx;
        std::vector<TxHelper::Utxo> outVin;
        TxHelper::vrfAgentType isNeedAgent_flag;
        Vrf info_;
        std::pair<std::string, uint64_t> node_ip_port;
        int ret = TxHelper::CreateDisinvestTransaction(from_Addr.at(0),str_ToAddr, str_utxo, top + 1, outTx, outVin,isNeedAgent_flag,info_,ack,node_ip_port);
        if (ret != 0)
        {
            //ERRORLOG("Create divest transaction error!:{}", ret);
            return ret;
        }
        
        TxMsgReq txMsg;
        txMsg.set_version(global::kVersion);
        std::string newbase58 = outTx.identity();
        TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
        txMsgInfo->set_type(0);
        txMsgInfo->set_tx(outTx.SerializeAsString());
        txMsgInfo->set_height(top);

        if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf)
        {
            Vrf * new_info=txMsg.mutable_vrfinfo();
            new_info->CopyFrom(info_);
        }
        std::string outdata;
        SendMessage(txMsg.SerializeAsString(), txMsg.GetDescriptor()->name(),outdata);
        // if(MagicSingleton<Net>::GetInstance()->isConnected(node_ip_port.first))
        // {
        //     std::cout<<"in in --->node_ip_port.first = "<<node_ip_port.first<<std::endl;
        //     MagicSingleton<Net>::GetInstance()->sendData(node_ip_port.first,outdata.data(),outdata.size());
        // }
        //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
        auto iter = MagicSingleton<net>::GetInstance();
        int result = iter->send(outdata.data(),outdata.size());
        if(result < 0)
        {
            return -1;
        }
        sleep(1);
        MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    }
    else if(ack->type() == 6)
    {
        uint64_t top = ack->height();
        CTransaction outTx;
        std::vector<TxHelper::Utxo> outVin;
        TxHelper::vrfAgentType isNeedAgent_flag;
        Vrf info_;
        std::pair<std::string, uint64_t> node_ip_port;
        int ret = TxHelper::CreateBonusTransaction(from_Addr.at(0), top + 1,  outTx, outVin,isNeedAgent_flag,info_,ack,cur_time,node_ip_port);
        if (ret != 0)
        {
            //ERRORLOG("Failed to create bonus transaction! The error code is:{}", ret);
            return ret;
        }
        

        TxMsgReq txMsg;
        txMsg.set_version(global::kVersion);
        std::string newbase58 = outTx.identity();
        TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
        txMsgInfo->set_type(0);
        txMsgInfo->set_tx(outTx.SerializeAsString());
        txMsgInfo->set_height(top);

        if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf)
        {
            Vrf * new_info=txMsg.mutable_vrfinfo();
            new_info->CopyFrom(info_);

        }
        std::string outdata;
        SendMessage(txMsg.SerializeAsString(), txMsg.GetDescriptor()->name(),outdata);
        // if(MagicSingleton<Net>::GetInstance()->isConnected(node_ip_port.first))
        // {
        //     std::cout<<"in in --->node_ip_port.first = "<<node_ip_port.first<<std::endl;
        //     MagicSingleton<Net>::GetInstance()->sendData(node_ip_port.first,outdata.data(),outdata.size());
        // }
        //MagicSingleton<net>::GetInstance()->send(outdata.data(),outdata.size());
        auto iter = MagicSingleton<net>::GetInstance();
        int result = iter->send(outdata.data(),outdata.size());
        if(result < 0)
        {
            return -1;
        }
        sleep(1);
        MagicSingleton<Recver>::GetInstance()->ReadData(iter);
    }
    return 0;
}


int  HandleGetUtxo(const std::shared_ptr<GetBalanceAck> &ack, const MsgData &from)
{
    std::string version = global::kVersion;
    std::string base58 = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();

    uint64_t balance = ack->balance();
    
    uint64_t blockHeight = ack->height();
   
    ca_console infoColor(kConsoleColor_Green, kConsoleColor_Black, true);
    double b = balance / double(100000000);
    cout << infoColor.color();
    cout << "*********************************************************************************" << endl;
    cout << "Version: " << version << endl;
    cout << "Base58: " << base58 << endl;
    cout << "Balance: " << setiosflags(ios::fixed) << setprecision(8) << b << endl;
    cout << "Block top: " << blockHeight << endl;
    cout << "*********************************************************************************" << endl;
    cout << infoColor.reset();
    std::cout<<"from.ip = "<<from.ip <<std::endl;
    std::cout<<"from.port = "<<from.port <<std::endl;
    //MagicSingleton<Net>::GetInstance()->disconnect(from.ip);
    MagicSingleton<net>::GetInstance()->close();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward to Ebpc
uint64_t pack_ip_port(uint32_t ip, uint16_t port)
{
	uint64_t ret = port;
	ret = ret << 32 | ip;
	return ret;
}

std::pair<uint32_t, uint16_t> unpack_ip_port(uint64_t ip_and_port)
{
	uint64_t tmp = ip_and_port;
	uint32_t ip = tmp << 32 >> 32;
	uint16_t port = ip_and_port >> 32;
	return std::pair<uint32_t, uint16_t>(ip, port);
}

