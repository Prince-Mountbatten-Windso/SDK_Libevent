#ifndef _TXHELPER_H_
#define _TXHELPER_H_


#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <random>
#include <boost/functional/hash.hpp>

#include "ca_algorithm.h"
#include "../proto/transaction.pb.h"
#include "../proto/ca_protomsg.pb.h"
#include "../proto/sdk.pb.h"


class VmInterface;
class TxHelper
{
public:

    struct Utxo
    {
        std::uint64_t value;
        std::string addr;
        std::string hash;
        std::uint32_t n;
    };

    class UtxoCompare
    {
    public:
        bool operator()(const Utxo& utxo1, const Utxo& utxo2) const
        {
            return utxo1.value < utxo2.value;
        }
    };

    typedef enum emPledgeType {
        kPledgeType_Unknown = -1,		// unknown
        kPledgeType_Node = 0,			// Node stake
    } PledgeType;

     typedef enum emInvestType {
        kInvestType_Unknown = -1,		// unknown
        kInvestType_NetLicence = 0,	    //NetLicence
    } InvestType;

    enum vrfAgentType
    {
        vrfAgentType_defalut = 0, 
        vrfAgentType_vrf ,
        vrfAgentType_local ,
        vrfAgentType_unknow,
    };

    static const uint32_t kMaxVinSize;

    TxHelper() = default;
    ~TxHelper() = default;

    static int Check(const std::vector<std::string>& fromAddr,
					uint64_t height);

    static int FindUtxo(const std::vector<std::string>& fromAddr,
						const uint64_t need_utxo_amount,
						uint64_t& total,
						std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare>& setOutUtxos,
                        const std::shared_ptr<GetSDKAck> &SDKAckMsg);

    static int CreateTxTransaction(const std::vector<std::string>& fromAddr,
									const std::map<std::string, int64_t> & toAddr,
									uint64_t height,
									CTransaction& outTx,
                                    TxHelper::vrfAgentType &type ,
                                    Vrf & info_,
                                    const std::shared_ptr<GetSDKAck> &SDKAckMsg,
                                    std::pair<std::string, uint64_t> &node_ip_port);

    static int CreateStakeTransaction(const std::string & vecfromAddr,
                                        uint64_t stake_amount,
                                        uint64_t height,
                                        TxHelper::PledgeType pledgeType,
                                        CTransaction &outTx,
                                        std::vector<TxHelper::Utxo> & outVin
                                        ,TxHelper::vrfAgentType &type ,
                                        Vrf & info_,
                                        const std::shared_ptr<GetSDKAck> &SDKAckMsg,
                                        std::pair<std::string, uint64_t> &node_ip_port);

    static int CreatUnstakeTransaction(const std::string& vecfromAddr,
                                        const std::string& utxo_hash,
                                        uint64_t height,
                                        CTransaction &outTx, 
                                        std::vector<TxHelper::Utxo> & outVin
                                        ,TxHelper::vrfAgentType &type,
                                        Vrf & info_,
                                        const std::shared_ptr<GetSDKAck> &SDKAckMsg,
                                        std::pair<std::string, uint64_t> &node_ip_port);

    static int CreateInvestTransaction(const std::string& vecfromAddr,
										const std::string& toAddr,
										uint64_t invest_amount,
										uint64_t height,
                                        TxHelper::InvestType investType,
										CTransaction & outTx,
										std::vector<TxHelper::Utxo> & outVin
                                        ,TxHelper::vrfAgentType &type,
                                        Vrf & info_,
                                        const std::shared_ptr<GetSDKAck> &SDKAckMsg,
                                        std::pair<std::string, uint64_t> &node_ip_port);

    static int CreateDisinvestTransaction(const std::string& vecfromAddr,
										const std::string& toAddr,
										const std::string& utxo_hash,
										uint64_t height,
										CTransaction& outTx,
										std::vector<TxHelper::Utxo> & outVin
                                        ,TxHelper::vrfAgentType &type,
                                        Vrf & info_,
                                        const std::shared_ptr<GetSDKAck> &SDKAckMsg,
                                        std::pair<std::string, uint64_t> &node_ip_port);

    static int CreateDeclareTransaction(const std::string & fromaddr, //发起方 Initiator
                                        const std::string & toAddr, // 接收方 Recipient
                                        uint64_t amount, 
                                        const std::string & multiSignPub, // 多签地址公钥 Multi-Sig address public key
                                        const std::vector<std::string> & signAddrList, //记录同盟节点 Record the federation node
                                        uint64_t signThreshold, // 多签共识数 The number of consensus numbers for multiple signs
                                        uint64_t height,
                                        CTransaction& outTx
                                        ,TxHelper::vrfAgentType &type ,
                                        Vrf & info_,
                                        std::pair<std::string, uint64_t> &node_ip_port);

    static int CreateBonusTransaction(const std::string& vecfromAddr,
										uint64_t height,
										CTransaction& outTx,
										std::vector<TxHelper::Utxo> & outVin,
                                        TxHelper::vrfAgentType &type,
                                        Vrf & info_,
                                        const std::shared_ptr<GetSDKAck> &SDKAckMsg,
                                        uint64_t cur_time,
                                        std::pair<std::string, uint64_t> &node_ip_port);

    static int SignTransaction(const std::vector<TxHelper::Utxo> & outVin,
                               CTransaction &tx,
                               std::string &serTx,
                               std::string &encodeStrHash);


    // static int CreateDeployContractTransaction(VmInterface& vm,
	// 									const std::string & fromAddr,
    //                                     uint64_t deploy_amount,
    //                                     uint64_t height,
    //                                     CTransaction &outTx,
    //                                     const std::string& OwnerEvmAddr,
    //                                     TxHelper::vrfAgentType &type,
    //                                     Vrf & info_,
    //                                     const std::shared_ptr<GetSDKAck> &SDKAckMsg,
    //                                     std::pair<uint64_t, uint64_t> node_ip_port);

    // static int CreateCallContractTransaction(VmInterface& vm,
	// 									const std::string & fromAddr,
    //                                     const std::string & toAddr,
    //                                     const std::string & txHash,											
    //                                     const std::string & strInput,
    //                                     uint64_t call_amount,
    //                                     uint64_t height,
    //                                     CTransaction & outTx,
    //                                     const std::string& OwnerEvmAddr,
    //                                     TxHelper::vrfAgentType &type,
    //                                     Vrf & info_); 


    static int AddMutilSign(const std::string & addr, CTransaction &tx);

    static int AddVerifySign(const std::string & addr, CTransaction &tx);

    static int Sign(const std::string & addr, 
					const std::string & message, 
                    std::string & signature, 
					std::string & pub);

    static bool IsNeedAgent(const std::vector<std::string> & fromAddr);
    static bool IsNeedAgent(const CTransaction &tx);
    /// @brief 判断txTime 与当前高度最新的块的时间差距是否在timeout之内 Whether the time gap with the current highly recent block is within timeout
    /// @param txTime 
    /// @param timeout 
    /// @return 
    static bool checkTxTimeOut(const uint64_t & txTime, const uint64_t & timeout,const uint64_t & pre_height,const std::shared_ptr<GetSDKAck> &SDKAckMsg);

    static TxHelper::vrfAgentType GetVrfAgentType(const CTransaction &tx, uint64_t &pre_height,const std::shared_ptr<GetSDKAck> &SDKAckMsg);

    static void GetTxStartIdentity(const std::vector<std::string> &fromaddr,const uint64_t &height,const uint64_t &current_time,TxHelper::vrfAgentType &type,const std::shared_ptr<GetSDKAck> &SDKAckMsg);
    //static void GetInitiatorType(const std::vector<std::string> &fromaddr, TxHelper::vrfAgentType &type,const std::shared_ptr<GetSDKAck> &SDKAckMsg);


    
};


#endif



