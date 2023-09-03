#include "ca/ca_algorithm.h"

#include <sys/time.h>
#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <crypto/cryptopp/base64.h>
#include "proto/transaction.pb.h"

// #include "db/db_api.h"
#include "utils/MagicSingleton.h"
#include "utils/time_util.h"
#include "ca_transaction.h"
// #include "ca/ca_blockcache.h"
#include "ca_global.h"
// #include "ca/ca_CCalBlockGas.h"
#include "ca.h"
// #include "ca/ca_sync_block.h"

// #include "ca_block_http_callback.h"
// #include "utils/AccountManager.h"
#include "utils/console.h"
#include "include/ScopeGuard.h"
// #include "ca_blockhelper.h"
#include "utils/EDManager.h"
#include "proto/sdk.pb.h"
// #include "ca_DonHost.hpp"
// #include "ca_vm_interface.h"
// #include "ca_evmone.h"
// #include "utils/ContractUtils.h"
// #include "utils/TFSbenchmark.h"

typedef boost::multiprecision::cpp_bin_float_50 cpp_bin_float;

// static uint64_t GetLocalTimestampUsec()
// {
//     struct timeval tv;
//     gettimeofday(&tv, nullptr);
//     return tv.tv_sec * 1000000 + tv.tv_usec;
// }

// int ca_algorithm::GetAbnormalSignAddrListByPeriod(uint64_t &cur_time, std::vector<std::string> &abnormal_addr_list, std::unordered_map<std::string, uint64_t> & addr_sign_cnt)
// {
//     DBReader db_reader;
//     std::vector<std::string> SignAddrs;
//     uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(cur_time);
//     if (DBStatus::DB_SUCCESS != db_reader.GetSignAddrByPeriod(Period - 1, SignAddrs))
//     {
//         ERRORLOG("GetSignAddrByPeriod error Period:{}", Period - 1);
//         return -1;
//     }
//     uint64_t SignNumber = 0;

//     for(auto addr : SignAddrs)
//     {
//         if (DBStatus::DB_SUCCESS != db_reader.GetSignNumberByPeriod(Period - 1, addr, SignNumber))
//         {
//             ERRORLOG("GetSignNumberByPeriod error Period:{},addr:{}", Period - 1, addr);
//             return -2;
//         }
//         addr_sign_cnt[addr] = SignNumber;
//     }

//     uint64_t quarter_num = addr_sign_cnt.size() * 0.25;
//     uint64_t three_quarter_num = addr_sign_cnt.size() * 0.75;
//     if (quarter_num == three_quarter_num)
//     {
//         return 0;
//     }

//     std::vector<uint64_t> sign_cnt;     // Number of signatures stored
//     for (auto &item : addr_sign_cnt)
//     {
//         sign_cnt.push_back(item.second);
//     }
//     std::sort(sign_cnt.begin(), sign_cnt.end());

//     uint64_t sign_cnt_quarter_num_value = sign_cnt.at(quarter_num);
//     uint64_t sign_cnt_three_quarter_num_value = sign_cnt.at(three_quarter_num);
//     int64_t sign_cnt_lower_limit_value = sign_cnt_quarter_num_value -
//                                           ((sign_cnt_three_quarter_num_value - sign_cnt_quarter_num_value) * 1.5);

//     if(sign_cnt_lower_limit_value >= 0)
//     {
//         for (auto &item : addr_sign_cnt)
//         {
//             if (item.second < sign_cnt_lower_limit_value)
//             {
//                 abnormal_addr_list.push_back(item.first);
//             }
//         }
//     }
//     return 0;  
// }


int64_t ca_algorithm::GetPledgeTimeByAddr(const std::string &addr, global::ca::StakeType stakeType,  const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
    if (stakeType != global::ca::StakeType::kStakeType_Node)
    {
        //ERRORLOG("unknow pledge type");
        return -1;
    }
    // DBReader db_reader;
    // if (nullptr == db_reader_ptr)
    // {
    //     db_reader_ptr = &db_reader;
    // }
    
    // std::vector<std::string> pledge_utxos;
    // auto ret = db_reader.GetStakeAddressUtxo(addr, pledge_utxos);
    // if (DBStatus::DB_NOT_FOUND == ret)
    // {
    //     TRACELOG("DB_NOT_FOUND");
    //     return 0;
    // }
    // else if (DBStatus::DB_SUCCESS != ret)
    // {
    //     ERRORLOG("fail to query addr pledge");
    //     return -2;
    // }

    std::vector<CTransaction> txs;
    CTransaction tx;
    std::string tx_raw;
    std::string tmp_pledge_type;
    //for (auto &pledge_utxo : pledge_utxos)

    for (size_t i = 0; i < (size_t) SDKAckMsg->pledgetx_size(); ++i)
    {
        const SDKPledgeTx item = SDKAckMsg->pledgetx(i);
      
		if(addr == item.address())
        {
            tx.ParseFromString(item.tx());
            if((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
            {
                try
                {
                    nlohmann::json data_json = nlohmann::json::parse(tx.data());
                    global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();

                    if (global::ca::TxType::kTxTypeStake != tx_type)
                    {
                        continue;
                    }

                    nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
                    tmp_pledge_type.clear();
                    tx_info["StakeType"].get_to(tmp_pledge_type);

                    if (tmp_pledge_type != global::ca::kStakeTypeNet)
                    {
                        continue;
                    }
                    txs.push_back(tx);
                }
                catch (...)
                {
                    //ERRORLOG("get pledge trasaction fail");
                    return -5;
                }
            }
        }
        // if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(pledge_utxo, tx_raw))
        // {
        //     ERRORLOG("faile to query trasaction");
        //     return -3;
        // }
        // tx.Clear();
        // if (!tx.ParseFromString(tx_raw))
        // {
        //     ERRORLOG("trasaction parse fail");
        //     return -4;
        // }

        
    }
    std::sort(txs.begin(), txs.end(),
              [](const CTransaction &tx1, const CTransaction &tx2)
              {
                  return tx1.time() < tx2.time();
              });

    uint64_t total_stake_amount = 0;
    uint64_t last_time = 0;

    for (auto &tx : txs)
    {
        for (auto &vout : tx.utxo().vout())
        {
            if (vout.addr() == global::ca::kVirtualStakeAddr)
            {
                total_stake_amount += vout.value();
                last_time = tx.time();
                break;
            }
        }
        if (total_stake_amount >= global::ca::kMinStakeAmt)
        {
            break;
        }
    }

    if (total_stake_amount < global::ca::kMinStakeAmt)
    {
        //TRACELOG("node type pledge amount is {}", total_stake_amount);
        return 0;
    }

    //DEBUGLOG("Pledge Time lastest: {}", last_time);
    return last_time;
}


// std::string ca_algorithm::CalcTransactionHash(CTransaction tx)
// {
//     tx.clear_hash();
//     return getsha256hash(tx.SerializeAsString());
// }

// std::string ca_algorithm::CalcBlockHash(CBlock block)
// {
//     block.clear_hash();
//     block.clear_sign();
//     return getsha256hash(block.SerializeAsString());
// }

// std::string ca_algorithm::CalcBlockMerkle(CBlock cblock)
// {
// 	std::string merkle;
// 	if (cblock.txs_size() == 0)
// 	{
// 		return merkle;
// 	}

// 	std::vector<std::string> vTxHashs;
// 	for (int i = 0; i != cblock.txs_size(); ++i)
// 	{
// 		CTransaction tx = cblock.txs(i);
// 		vTxHashs.push_back(tx.hash());
// 	}

// 	unsigned int j = 0, nSize;
//     for (nSize = cblock.txs_size(); nSize > 1; nSize = (nSize + 1) / 2)
// 	{
//         for (unsigned int i = 0; i < nSize; i += 2)
// 		{
//             unsigned int i2 = MIN(i+1, nSize-1);

// 			std::string data1 = vTxHashs[j + i];
// 			std::string data2 = vTxHashs[j + i2];
// 			data1 = getsha256hash(data1);
// 			data2 = getsha256hash(data2);

// 			vTxHashs.push_back(getsha256hash(data1 + data2));
//         }

//         j += nSize;
//     }

// 	merkle = vTxHashs[vTxHashs.size() - 1];

// 	return merkle;
// }


// int ca_algorithm::MemVerifyTransactionTx(const CTransaction & tx)
// {
//     uint64_t start_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
// 	// Transaction version number must be 0
//     if (tx.version() != 0)
//     {
//         return -1;
//     }

//     // Is the transaction type a normal transaction
//     if (GetTransactionType(tx) != kTransactionType_Tx)
//     {
//         return -2;
//     }

// 	if (tx.time() <= global::ca::kGenesisTime)
// 	{
// 		return -3;
// 	}

// 	// Is the transaction hash length 64
//     if (tx.hash().size() != 64)
//     {
//         return -4;
//     }

//     // Verify whether the transaction hash is consistent
//     if (tx.hash() != ca_algorithm::CalcTransactionHash(tx))
//     {
//         return -5;
//     }

// 	if (tx.identity().size() == 0)
// 	{
// 		return -6;
// 	}
    
// 	if ( !CheckBase58Addr(tx.identity()) )
// 	{
// 		return -7;
// 	}

//     if (tx.utxo().owner_size() == 0)
//     {
//         return -8;
//     }
    
//     // The number of vins must be less than or equal to 100 (it will be adjusted later)
//     if (tx.utxo().vin_size() > 100 || tx.utxo().vin_size() <= 0)
//     {
//         ERRORLOG("The number of vins must be less than or equal to 100!");
//         return -9;
//     }

//     if (tx.utxo().vout_size() < 2)
//     {
//         return -10;
//     }

//     if (tx.utxo().multisign_size() == 0)
//     {
//         return -11;
//     }
    

//     uint64_t sign_gas =  tx.gas();
//     global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();
// 	int needConsensus = tx.consensus();
//     nlohmann::json tx_info;
	
//     if((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
//     {
//         try
//         {
//             nlohmann::json data_json = nlohmann::json::parse(tx.data());
//             tx_info = data_json["TxInfo"].get<nlohmann::json>();
//         }
//         catch (...)
//         {
//             return -12;
//         }

//         if (needConsensus != global::ca::kConsensus)
//         {
//             return -13;
//         }
//     }


//     uint64_t gas = 0;
// 	if(CalculateGas(tx,gas) != 0)
// 	{
//         std::cout << "MemVerifyTransactionTx gas = " << gas << std::endl;
//         ERRORLOG("CalculateGas error gas = {}", gas);
// 		return -14;
// 	}
//     uint64_t thresholdGas = gas * 0.05;
//     if(sign_gas < gas - thresholdGas || sign_gas > gas + thresholdGas)
//     {
//         ERRORLOG("The signature is beyond the floating range of five percent ! sign_gas = {},gas = {}",sign_gas,gas);
//         return -15;    
//     } 

//     // The transaction type can only be one of six, the output quantity  can only be 2
// 	bool is_tx = false;
//     bool is_stake = false;
//     bool is_unstake = false;
//     bool is_invest = false;
//     bool is_disinvest = false;
//     bool is_bonus = false;
// 	bool is_declare = false;
//     bool is_deployContract = false;
//     bool is_callContract = false;
//     uint64_t stake_amount = 0;
//     uint64_t invest_amount = 0; 
//     uint64_t deploy_amount = 0; 

// 	try
// 	{
// 		if (global::ca::TxType::kTxTypeStake == tx_type)
// 		{
// 			is_stake = true;
// 			stake_amount = tx_info["StakeAmount"].get<uint64_t>();
// 			std::string pledge_type = tx_info["StakeType"].get<std::string>();

// 			if (global::ca::kStakeTypeNet != pledge_type)
// 			{
// 				ERRORLOG("Stake type can only be online stake and public network stake!");            
// 				return -16;
// 			}
// 		}
// 		else if (global::ca::TxType::kTxTypeUnstake == tx_type)
// 		{
// 			is_unstake = true;
// 			std::string redeem_utxo_hash = tx_info["UnstakeUtxo"].get<std::string>();
// 			if(redeem_utxo_hash.size() != 64)
// 			{
// 				return -17;
// 			}
// 		}
// 		else if (global::ca::TxType::kTxTypeInvest == tx_type)
// 		{
// 			is_invest = true;
// 			invest_amount = tx_info["InvestAmount"].get<uint64_t>();
// 			std::string invest_node = tx_info["BonusAddr"].get<std::string>();
// 			if (!CheckBase58Addr(invest_node))
// 			{
// 				return -18;
// 			}
// 			std::string invest_type = tx_info["InvestType"].get<std::string>();
// 			if (global::ca::kInvestTypeNormal != invest_type )
// 			{
// 				ERRORLOG("the invest type can only be invest licence and reserve invest licence!");
// 				return -19;
// 			}
// 		}
// 		else if (global::ca::TxType::kTxTypeDisinvest == tx_type)
// 		{
// 			is_disinvest = true;
// 			std::string divest_utxo_hash = tx_info["DisinvestUtxo"].get<std::string>();
// 			if (divest_utxo_hash.size() != 64)
// 			{
// 				return -20;
// 			}
// 		}
// 		else if (global::ca::TxType::kTxTypeBonus == tx_type)
// 		{
// 			is_bonus = true;
// 			uint64_t bonusAddrList;
// 			bonusAddrList = tx_info["BonusAddrList"].get<uint64_t>();

// 			if (bonusAddrList != tx.utxo().vout_size())
// 			{
// 				return -21;
// 			}
// 		}
//         else if (global::ca::TxType::kTxTypeDeclaration == tx_type)
//         {
//             is_declare = true;
// 			std::string multiSignPub = tx_info["MultiSignPub"].get<std::string>();
//             multiSignPub = Base64Decode(multiSignPub);
// 			nlohmann::json signAddrList = tx_info["SignAddrList"].get<nlohmann::json>();
// 			uint64_t threshold = tx_info["SignThreshold"].get<uint64_t>();
            
// 			std::string multiSignAddr = GetBase58Addr(multiSignPub, Base58Ver::kBase58Ver_MultiSign);
// 			if (tx.utxo().vout_size() != 2 && tx.utxo().vout(1).addr() != multiSignAddr)
// 			{
// 				return -22;
// 			}

// 			if(signAddrList.size() < 2 || signAddrList.size() > 100)
// 			{
// 				return -23;
// 			}

//             std::set<std::string> setSignAddr;
// 			for (auto & signAddr : signAddrList)
// 			{
// 				if (!CheckBase58Addr(signAddr, Base58Ver::kBase58Ver_Normal))
// 				{
// 					return -24;
// 				}
//                 setSignAddr.insert(std::string(signAddr));
// 			}

//             if (setSignAddr.size() != signAddrList.size())
//             {
//                 return -25;
//             }

// 			if (threshold > signAddrList.size())
// 			{
// 				return -26;
// 			}
//         }
//         else if (global::ca::TxType::kTxTypeDeployContract == tx_type)
//         {
//             is_deployContract = true;
//             deploy_amount = tx_info["deploy_amount"].get<uint64_t>();
//         }
//         else if (global::ca::TxType::kTxTypeCallContract == tx_type)
//         {
//             is_callContract = true;
//         }
// 		else if (global::ca::TxType::kTxTypeTx == tx_type)
// 		{
// 			is_tx = true;
// 		}               
// 		else
// 		{
// 			ERRORLOG("Unknown tx type!");  
// 			return -27;
// 		}
// 	}
// 	catch(const std::exception& e)
// 	{
// 		std::cerr << e.what() << '\n';
// 		return -28;
// 	}


//     if((is_stake || is_unstake || is_invest || is_disinvest || is_declare) && tx.utxo().vout_size() != 2)
//     {
//         ERRORLOG("The number of vouts must be equal to 2!" );
//         return -29;
//     }

//     std::set<std::string> owner_addrs;
//     {
//         std::vector<std::string> tmp_addrs(tx.utxo().owner().begin(), tx.utxo().owner().end());
//         std::set<std::string>(tmp_addrs.cbegin(), tmp_addrs.cend()).swap(owner_addrs);

// 		if (owner_addrs.size() == 0)
// 		{
// 			return -30;
// 		}

// 		for (auto & addr : owner_addrs)
// 		{
// 			if (!is_tx)
// 			{
// 				if (CheckBase58Addr(addr, Base58Ver::kBase58Ver_MultiSign) == true)
// 				{
// 					return -31;
// 				}
// 			}
// 			else
// 			{
// 				if (!CheckBase58Addr(addr, Base58Ver::kBase58Ver_All))
// 				{
// 					return -32;
// 				}
// 			}
// 		}

//         // The owner size of the following five types of transactions must be 1
//         if (is_stake || is_unstake || is_invest || is_disinvest || is_bonus || is_declare)
//         {
//             if (1 != owner_addrs.size())
//             {
//                 ERRORLOG( "The owner size of the following five types of transactions must be 1!" );
//                 return -33;
//             }
//         }
//         else
//         {
//             // Txowner does not allow duplicate
//             if (tmp_addrs.size() != owner_addrs.size())
//             {
//                 ERRORLOG( "Txowner does not allow duplicate!" );
//                 return -34;
//             }
//         }
//     }

//     {
//         uint64_t multiSignOwners = 0;
//         if (owner_addrs.size() > 1)
//         {
//             for (auto & addr : owner_addrs)
//             {
//                 if (CheckBase58Addr(addr, Base58Ver::kBase58Ver_MultiSign))
//                 {
//                     multiSignOwners++;
//                 }
//             }

//             if (multiSignOwners > 1)
//             {
//                 return -35;
//             }
//         }
//     }
    
//     // bool isMultiSign = owner_addrs.size() == 1 && CheckBase58Addr(*owner_addrs.begin(), Base58Ver::kBase58Ver_MultiSign) && is_tx && tx.utxo().vin_size() == 1; 
//     bool isMultiSign = IsMultiSign(tx);
   
//     std::set<std::string> vin_addrs;
//     std::set<std::string> vin_utxos;
// 	uint64_t total_vin_size = 0;
//     for (auto &vin : tx.utxo().vin())
//     {
// 		if (vin.vinsign().pub().size() == 0)
// 		{
// 			return -36;
// 		}

//         Base58Ver ver = isMultiSign ? Base58Ver::kBase58Ver_MultiSign : Base58Ver::kBase58Ver_Normal;
//         std::string addr = GetBase58Addr(vin.vinsign().pub(), ver);
//         if (!CheckBase58Addr(addr))
//         {
//             ERRORLOG( "Check Base58Addr failed!" );
//             return -37;
//         }
//         vin_addrs.insert(addr);
// 		if (vin.prevout_size() == 0)
// 		{
// 			return -38;
// 		}

// 		for(auto & prevout : vin.prevout())
// 		{
// 			if (prevout.hash().size() != 64)
// 			{
// 				return -39;
// 			}
// 			total_vin_size++;
// 			vin_utxos.insert(prevout.hash() + "_" + std::to_string(prevout.n()));
// 		}
//     }
	
//     // Verify whether txowner and VIN signers are consistent
//     if (!std::equal(owner_addrs.cbegin(), owner_addrs.cend(), vin_addrs.cbegin(), vin_addrs.cend()))
//     {
//         ERRORLOG("Txowner and VIN signers are not consistent!");
//         return -40;
//     }

// 	// Verify the sequence of VIN
//     for (int i = 0; i < tx.utxo().vin_size(); i++)
//     {
//         if (i != tx.utxo().vin(i).sequence())
//         {
//             ERRORLOG(RED "The sequence of VIN is not consistent!" RESET);
//             return -41;
//         }

        
//     }
    
//     // Vin cannot be repeated except for the redeem or divest transaction
// 	if (is_unstake || is_disinvest)
//     {
//         if (vin_utxos.size() != total_vin_size && (vin_utxos.size() != (total_vin_size - 1)))
//         {
// 			// 使用上次解质押的utxo时会出现(vin_utxos.size() != (tx.vin_size() - 1))
//             ERRORLOG("Vin cannot be repeated except for the redeem or divest transaction!");
//             return -42;
//         }
//     }
//     else
//     {
//         if (vin_utxos.size() != total_vin_size)
//         {
//             ERRORLOG( "Vin cannot be repeated except for the redeem or divest transaction!" );
//             return -43;
//         }
//     }

// 	if (tx.utxo().vout_size() == 0 || tx.utxo().vout_size() > 1000)
// 	{
// 		return -44;
// 	}

//     std::set<std::string> vout_addrs;
//     for (int i = 0; i < tx.utxo().vout_size(); i++)
//     {
//         auto &vout = tx.utxo().vout(i);
//         if (is_stake)
//         {
// 			if (i == 0 && (vout.addr() != global::ca::kVirtualStakeAddr || vout.value() != stake_amount))
// 			{
// 				// The pledge amount should be consistent with the output
// 				ERRORLOG(RED "The pledge amount should be consistent with the output!" RESET);
// 				return -45;
// 			}

//             if(i == 0 && vout.value() < 500000000000)
//             {
// 				ERRORLOG(RED "The pledge amount should be consistent with the output!" RESET);
//                 return -46;
//             }
			
//             if (i == 1 && (! CheckBase58Addr(vout.addr(), Base58Ver::kBase58Ver_Normal)))
// 			{
// 				return -47;
// 			}
			
//             if (i >= 2)
// 			{
// 				return -48;
// 			}
//         }
//         else if(is_invest)
//         {
// 			if (i == 0 && (vout.addr() != global::ca::kVirtualInvestAddr || vout.value() != invest_amount))
// 			{
// 				// The invest amount should be consistent with the output
// 				ERRORLOG(RED "The invest amount should be consistent with the output!" RESET);
// 				return -49;
// 			}
			
//             if (i == 1 && (! CheckBase58Addr(vout.addr(), Base58Ver::kBase58Ver_Normal)))
// 			{
// 				return -50;
// 			}
			
//             if(i >= 2)
// 			{
// 				return -51;
// 			}
//         }
// 		else if (is_declare)
// 		{
// 			if (i == 0 && (!CheckBase58Addr(vout.addr(), Base58Ver::kBase58Ver_MultiSign) || vout.value() < 0) )
// 			{
// 				return -52;
// 			}
			
//             if (i == 1 && (!CheckBase58Addr(vout.addr(), Base58Ver::kBase58Ver_Normal) || (*owner_addrs.begin() != vout.addr() ) ) )
// 			{
// 				return -53;
// 			}
//             if (i >= 2)
// 			{
// 				return -54;
// 			}
// 		}
//         else if(is_deployContract)
//         {
//             if (i == 0 && (vout.addr() != global::ca::kVirtualDeployContractAddr || vout.value() != deploy_amount))
// 			{
// 				// The DeployContract amount should be consistent with the output
// 				ERRORLOG(RED "The DeployContract amount should be consistent with the output!" RESET);
// 				return -55;
// 			}
			
//             if (i == 1 && (! CheckBase58Addr(vout.addr(), Base58Ver::kBase58Ver_Normal)))
// 			{
// 				return -56;
// 			}
			
//             if(i >= 2)
// 			{
// 				return -57;
// 			}
//         }
//         else if(is_callContract && i == 0 && (vout.addr() == global::ca::kVirtualDeployContractAddr))
//         {
// 			continue;
//         }
//         else
//         {
//             if (!CheckBase58Addr(vout.addr()))
//             {
//                 ERRORLOG( "Check Base58Addr failed!" );
//                 return -58;
//             }
            
//             // The amount in the output must be Greater than 0
//             if (vout.value() < 0)
//             {
//                 ERRORLOG( "The amount in the output must be Greater than 0!" );
//                 return -59;
//             }
//         }
//         vout_addrs.insert(vout.addr());
//     }

//     // Multi-to-Multi transaction is not allowed
//     if (vin_addrs.size() > 1)
//     {
//         std::vector<std::string> v_diff;
//         std::set_difference(vout_addrs.begin(), vout_addrs.end(),
//                             vin_addrs.begin(),vin_addrs.end(),
//                             std::back_inserter(v_diff));
//         if(v_diff.size() > 1)
//         {
//             ERRORLOG(RED "Multi-to-Multi transaction is not allowed!" RESET);
//             return -60;       
//         }
//     }


//     auto VerifySignLambda = [](const CSign & sign, const std::string & serHash)->int {
        
//         if (sign.sign().size() == 0 || sign.pub().size() == 0)
// 		{
// 			return -61;
// 		}
//         if (serHash.size() == 0)
//         {
//             return -62;
//         }

//         EVP_PKEY* eckey = nullptr;
//         if(GetEDPubKeyByBytes(sign.pub(), eckey) == false)
//         {
//             EVP_PKEY_free(eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -63;
//         }

//         if(ED25519VerifyMessage(serHash, eckey, sign.sign()) == false)
//         {
//             EVP_PKEY_free(eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -64;
//         }
//         EVP_PKEY_free(eckey);
//         return 0;
//     };




//     if( !(tx.utxo().vin_size() == tx.utxo().multisign_size() == tx.utxo().owner_size() == 1 && tx.utxo().vout_size() == 2 && tx.utxo().vout(1).addr() == tx.utxo().owner(0)) )
//     {
//         if( !(is_stake || is_unstake || is_invest || is_disinvest || is_bonus || is_declare) )
//         {
//             // check vin sign 
//             for (auto & v : tx.utxo().vin())
//             {
//                 CTxInput vin = v;
//                 CSign sign = vin.vinsign();
//                 vin.clear_vinsign();
//                 std::string serVinHash = getsha256hash(vin.SerializeAsString());

//                 int verifySignRet = VerifySignLambda(sign, serVinHash);
//                 if (verifySignRet != 0)
//                 {
//                     return -65;
//                 }
//             }
//         }
//     }


// 	// check multiSign 
// 	if (tx.utxo().multisign_size() == 0)
// 	{
// 		return -66;
// 	}

// 	std::set<std::string> multiSignAddr;
// 	CTxUtxo utxo = tx.utxo();
// 	utxo.clear_multisign();
// 	std::string serUtxoHash = getsha256hash(utxo.SerializeAsString());
// 	for(auto & multiSign : tx.utxo().multisign())
// 	{
// 		multiSignAddr.insert(GetBase58Addr(multiSign.pub()));

//         int verifySignRet = VerifySignLambda(multiSign, serUtxoHash);
//         if (verifySignRet != 0)
//         {
//             return -67;
//         }
// 	}

// 	if (isMultiSign)
// 	{
// 		if (GetBase58Addr(tx.utxo().multisign(0).pub(), Base58Ver::kBase58Ver_MultiSign) != *owner_addrs.begin())
// 		{
// 			return -68;
// 		}
// 	}
// 	else
// 	{
// 		if (!std::equal(owner_addrs.cbegin(), owner_addrs.cend(), multiSignAddr.cbegin(), multiSignAddr.cend()))
// 		{
// 			ERRORLOG("Txowner and multi sign signers are not consistent!");
// 			return -69;
// 		}
// 	}

// 	// check tx sign
//     if (tx.verifysign_size() < 0 || tx.verifysign_size() > global::ca::kConsensus)
//     {
//         return -70;
//     }

//     if (tx.verifysign_size() > 0 && GetBase58Addr(tx.verifysign(0).pub()) != tx.identity())
//     {
//         ERRORLOG("tx verify sign size = {} " , tx.verifysign_size());
//         ERRORLOG("addr = {} , tx identity = {} ", GetBase58Addr(tx.verifysign(0).pub()), tx.identity());
//         return -71;
//     }

// 	CTransaction copyTx = tx;
// 	copyTx.clear_hash();
// 	copyTx.clear_verifysign();
// 	std::string serTxHash = getsha256hash(copyTx.SerializeAsString());

// 	for (auto & verifySign : tx.verifysign())
// 	{
// 		if (!CheckBase58Addr(GetBase58Addr(verifySign.pub()), Base58Ver::kBase58Ver_Normal))
// 		{
// 			return -72;
// 		}

//         int verifySignRet = VerifySignLambda(verifySign, serTxHash);
//         if (verifySignRet != 0)
//         {
//             return -73;
//         }
        
// 	}

//     if (tx.info().size() != 0)
//     {
//         return -74;
//     }

//     if (tx.reserve0().size() != 0 || tx.reserve1().size() != 0)
//     {
//         return -75;
//     }

//     // The initiator and receiver of the transaction are not allowed to sign for mining
//     if(!is_bonus)
//     {
//         std::vector<std::string> v_union;
//         std::set_union(vin_addrs.cbegin(), vin_addrs.cend(), vout_addrs.cbegin(), vout_addrs.cend(), std::back_inserter(v_union));
//         std::vector<std::string> v_sign_addr;
//         std::string sign_addr;
//         for (int i = 1; i < tx.verifysign_size(); ++i)
//         {
//             auto &tx_sign_pre_hash = tx.verifysign(i);
// 			if (tx_sign_pre_hash.pub().size() == 0)
// 			{
// 				return -76;
// 			}
//             sign_addr = GetBase58Addr(tx_sign_pre_hash.pub());
//             if (!CheckBase58Addr(sign_addr))
//             {
//                 ERRORLOG(RED "Check Base58Addr failed!" RESET);
//                 return -77;
//             }
//             v_sign_addr.push_back(sign_addr);
//         }
//         std::vector<std::string> v_intersection;
//         std::sort(v_union.begin(), v_union.end());
//         std::sort(v_sign_addr.begin(), v_sign_addr.end());
//         std::set_intersection(v_union.cbegin(), v_union.cend(), v_sign_addr.cbegin(), v_sign_addr.cend(), std::back_inserter(v_intersection));
//         if (!v_intersection.empty())
//         {
//             ERRORLOG(RED "The initiator and receiver of the transaction are not allowed to sign for mining!" RESET);
//             return -78;
//         }
//     }
//     uint64_t end_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//     if(global::ca::TxType::kTxTypeTx == tx_type)
//     {
//         MagicSingleton<TFSBenchmark>::GetInstance()->AddtransactionMemVerifyMap(tx.hash(), end_time - start_time);
//     }
	
// 	return 0;
// }


// int ca_algorithm::MemVerifyTransactionGas(const CTransaction & tx)
// {
// 	// Transaction version number must be 0
//     if (tx.version() != 0)
//     {
//         return -1;
//     }

//     // Is the transaction type a normal transaction
//     if (GetTransactionType(tx) != kTransactionType_Gas)
//     {
//         return -2;
//     }

// 	if (tx.time() <= global::ca::kGenesisTime)
// 	{
// 		return -3;
// 	}

// 	// Is the transaction hash length 64
//     if (tx.hash().size() != 64)
//     {
//         return -4;
//     }

//     // Verify whether the transaction hash is consistent
//     if (tx.hash() != ca_algorithm::CalcTransactionHash(tx))
//     {
//         return -5;
//     }

//     // Extension field be empty
//     if (tx.data().size() != 0)
//     {
//         return -6;
//     }

// 	if (tx.identity().size() == 0)
// 	{
// 		return -7;
// 	}
    
// 	if ( !CheckBase58Addr(tx.identity()) )
// 	{
// 		return -8;
// 	}

// 	const CTxUtxo & utxo = tx.utxo();

// 	if (utxo.owner_size() != 1)
// 	{
// 		return -9;
// 	}

// 	if (utxo.owner(0).size() == 0)
// 	{
// 		return -10;
// 	}

// 	if (!CheckBase58Addr(utxo.owner(0)))
// 	{
// 		return -11;
// 	}

// 	if (utxo.vin_size() != 1)
// 	{
// 		return -12;
// 	}

// 	const CTxInput & vin0 = utxo.vin(0);
// 	if (vin0.prevout_size() != 1)
// 	{
// 		return -13;
// 	}

// 	if (vin0.prevout(0).hash().size() != 64)
// 	{
// 		return -14;
// 	}

// 	if (vin0.vinsign().sign().size() == 0 || vin0.vinsign().pub().size() == 0)
// 	{
// 		return -15;
// 	}

// 	if (vin0.sequence() != 0)
// 	{
// 		return -16;
// 	}

// 	{ // verify vin sign
// 		CTxInput cpVin = vin0;
// 		cpVin.clear_vinsign();

//         EVP_PKEY* eckey = nullptr;
//         if(GetEDPubKeyByBytes(vin0.vinsign().pub(), eckey) == false)
//         {
//             EVP_PKEY_free(eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -17;
//         }

//         if(ED25519VerifyMessage(getsha256hash(cpVin.SerializeAsString()), eckey, vin0.vinsign().sign()) == false)
//         {
//             EVP_PKEY_free(eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -18;
//         }
//         EVP_PKEY_free(eckey);
// 	}

//     if (utxo.vout_size() != global::ca::kConsensus)
//     {
//         return -19;
//     }

// 	// multisign
// 	if (utxo.multisign_size() != 1)
// 	{
// 		return -20;
// 	}

// 	if (utxo.multisign(0).pub().size() == 0 || utxo.multisign(0).sign().size() == 0)
// 	{
// 		return -21;
// 	}

// 	if (GetBase58Addr(vin0.vinsign().pub()) != GetBase58Addr(utxo.multisign(0).pub()))
// 	{
// 		return -22;
// 	}

// 	{ // verify multisign sign
// 		CTxUtxo cpUtxo = utxo;
// 		cpUtxo.clear_multisign();

//         EVP_PKEY* multi_eckey = nullptr;
//         if(GetEDPubKeyByBytes(utxo.multisign(0).pub(), multi_eckey) == false)
//         {
//             EVP_PKEY_free(multi_eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -23;
//         }

//         if(ED25519VerifyMessage(getsha256hash(cpUtxo.SerializeAsString()), multi_eckey, utxo.multisign(0).sign()) == false)
//         {
//             EVP_PKEY_free(multi_eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -24;
//         }
//         EVP_PKEY_free(multi_eckey);
// 	}

// 	// verifySign
// 	if (tx.verifysign_size() != 1)
// 	{
// 		return -25;
// 	}
	
// 	if (GetBase58Addr(tx.verifysign(0).pub()) != GetBase58Addr(utxo.multisign(0).pub()))
// 	{
// 		return -26;
// 	}

// 	{
// 		// verify versign 
// 		CTransaction cpTx = tx;
// 		cpTx.clear_hash();
// 		cpTx.clear_verifysign();

//         EVP_PKEY* verify_eckey = nullptr;
//         if(GetEDPubKeyByBytes(tx.verifysign(0).pub(), verify_eckey) == false)
//         {
//             EVP_PKEY_free(verify_eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -27;
//         }

//         if(ED25519VerifyMessage(getsha256hash(cpTx.SerializeAsString()), verify_eckey, tx.verifysign(0).sign()) == false)
//         {
//             EVP_PKEY_free(verify_eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -28;
//         }
//         EVP_PKEY_free(verify_eckey);
// 	}

// 	return 0;
// }

// int ca_algorithm::MemVerifyTransactionBurn(const CTransaction & tx)
// {
// 	// Transaction version number must be 0
//     if (tx.version() != 0)
//     {
//         return -1;
//     }

//     // Is the transaction type a normal transaction
//     if (GetTransactionType(tx) != kTransactionType_Burn)
//     {
//         return -2;
//     }

// 	if (tx.time() <= global::ca::kGenesisTime)
// 	{
// 		return -3;
// 	}

// 	// Is the transaction hash length 64
//     if (tx.hash().size() != 64)
//     {
//         return -4;
//     }

//     // Verify whether the transaction hash is consistent
//     if (tx.hash() != ca_algorithm::CalcTransactionHash(tx))
//     {
//         return -5;
//     }

//     // Extension field be empty
//     if (tx.data().size() != 0)
//     {
//         return -6;
//     }

// 	if (tx.identity().size() == 0)
// 	{
// 		return -7;
// 	}
    
// 	if ( !CheckBase58Addr(tx.identity()) )
// 	{
// 		return -8;
// 	}

// 	const CTxUtxo & utxo = tx.utxo();
// 	if (utxo.owner_size() != 1)
// 	{
// 		return -9;
// 	}

// 	if (utxo.owner(0).size() == 0)
// 	{
// 		return -10;
// 	}

// 	if (!CheckBase58Addr(utxo.owner(0)))
// 	{
// 		return -11;
// 	}

// 	if (utxo.vin_size() != 1)
// 	{
// 		return -12;
// 	}

// 	const CTxInput & vin0 = utxo.vin(0);
// 	if (vin0.prevout_size() != 1)
// 	{
// 		return -13;
// 	}

// 	if (vin0.prevout(0).hash().size() != 64)
// 	{
// 		return -14;
// 	}

// 	if (vin0.vinsign().sign().size() == 0 || vin0.vinsign().pub().size() == 0)
// 	{
// 		return -15;
// 	}

// 	if (vin0.sequence() != 0)
// 	{
// 		return -16;
// 	}

// 	{ // verify vin sign
// 		CTxInput cpVin = vin0;
// 		cpVin.clear_vinsign();

//         EVP_PKEY* eckey = nullptr;
//         if(GetEDPubKeyByBytes(vin0.vinsign().pub(), eckey) == false)
//         {
//             EVP_PKEY_free(eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -17;
//         }

//         if(ED25519VerifyMessage(getsha256hash(cpVin.SerializeAsString()), eckey, vin0.vinsign().sign()) == false)
//         {
//             EVP_PKEY_free(eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -18;
//         }
//         EVP_PKEY_free(eckey);
// 	}

//     // vout
//     if (tx.utxo().vout_size() != 2)
//     {
//         return -19;
//     }
//     for (int i = 0; i < tx.utxo().vout_size() ; i++)
//     {
//         auto vout = tx.utxo().vout(i);
//         if(i == 0)
//         {
//             if(vout.addr() != global::ca::kVirtualBurnGasAddr)
//             {
//                 return -20;
//             }
//         }
//         else
//         {
//             if(vout.value() != 0)
//             {
//                 return -21;
//             }
//         }
//     }

// 	// multisign
// 	if (utxo.multisign_size() != 1)
// 	{
// 		return -22;
// 	}

// 	if (utxo.multisign(0).pub().size() == 0 || utxo.multisign(0).sign().size() == 0)
// 	{
// 		return -23;
// 	}

// 	if (GetBase58Addr(vin0.vinsign().pub()) != GetBase58Addr(utxo.multisign(0).pub()))
// 	{
// 		return -24;
// 	}

// 	{ // verify multisign sign
// 		CTxUtxo cpUtxo = utxo;
// 		cpUtxo.clear_multisign();

//         EVP_PKEY* multi_eckey = nullptr;
//         if(GetEDPubKeyByBytes(utxo.multisign(0).pub(), multi_eckey) == false)
//         {
//             EVP_PKEY_free(multi_eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -25;
//         }

//         if(ED25519VerifyMessage(getsha256hash(cpUtxo.SerializeAsString()), multi_eckey, utxo.multisign(0).sign()) == false)
//         {
//             EVP_PKEY_free(multi_eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -26;
//         }
//         EVP_PKEY_free(multi_eckey);
// 	}

// 	// verifySign
// 	if (tx.verifysign_size() != 1)
// 	{
// 		return -27;
// 	}
	
// 	if (GetBase58Addr(tx.verifysign(0).pub()) != GetBase58Addr(utxo.multisign(0).pub()))
// 	{
// 		return -28;
// 	}

// 	{
// 		// verify versign 
// 		CTransaction cpTx = tx;
// 		cpTx.clear_hash();
// 		cpTx.clear_verifysign();

//         EVP_PKEY* verify_eckey = nullptr;
//         if(GetEDPubKeyByBytes(tx.verifysign(0).pub(), verify_eckey) == false)
//         {
//             EVP_PKEY_free(verify_eckey);
//             ERRORLOG(RED "Get public key from bytes failed!" RESET);
//             return -29;
//         }

//         if(ED25519VerifyMessage(getsha256hash(cpTx.SerializeAsString()), verify_eckey, tx.verifysign(0).sign()) == false)
//         {
//             EVP_PKEY_free(verify_eckey);
//             ERRORLOG(RED "Public key verify sign failed!" RESET);
//             return -30;
//         }
//         EVP_PKEY_free(verify_eckey);
// 	}

// 	return 0;
// }


// int ca_algorithm::VerifyTransactionTx(const CTransaction &tx, uint64_t tx_height, bool turn_on_missing_block_protocol, bool verify_abnormal)
// {
//     uint64_t start_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
    
//     DBReader db_reader;

//     // Parse parameters
//     bool is_redeem = false;
//     std::string redeem_utxo_hash;
//     bool is_invest = false;
//     std::string invest_node;
//     uint64_t invest_amount = 0;
//     bool is_divest = false;
//     std::string invested_node;
//     std::string divest_utxo_hash; 
// 	bool is_declare = false;
// 	std::string multiSignPub;
//     bool is_deploy_contract = false;
//     bool is_call_contract = false;
//     std::string deployer_addr;
//     std::string deploy_hash;
//     std::string OwnerEvmAddr;
//     std::string input;
//     std::string output;
//     nlohmann::json storage;
//     VmInterface::VmType vm_type;

//     uint64_t sign_fee = tx.gas();
//     uint64_t package_fee = tx.cost();
//     uint32_t need_sign_node_num = tx.consensus();
//     global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();

//     bool is_claim = false;
//     uint64_t claim_amount = 0;

//     bool is_tx = false;

//     if((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
//     {
//         try
//         {
//             nlohmann::json data_json = nlohmann::json::parse(tx.data());
//             nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();

//             if (global::ca::TxType::kTxTypeUnstake == tx_type)
//             {   
//                 is_redeem = true;            
//                 redeem_utxo_hash = tx_info["UnstakeUtxo"].get<std::string>();
//             }
//             else if (global::ca::TxType::kTxTypeInvest == tx_type)
//             {
//                 is_invest = true;
//                 invest_node = tx_info["BonusAddr"].get<std::string>();
//                 invest_amount = tx_info["InvestAmount"].get<uint64_t>();
//             }  
//             else if (global::ca::TxType::kTxTypeDisinvest == tx_type)
//             {
//                 is_divest = true;
//                 divest_utxo_hash = tx_info["DisinvestUtxo"].get<std::string>();
//                 invested_node = tx_info["BonusAddr"].get<std::string>();        
//             }
//             else if(global::ca::TxType::kTxTypeBonus == tx_type)
//             {
//                 is_claim = true;
//                 claim_amount = tx_info["BonusAmount"].get<uint64_t>();  
//             }
//             else if (global::ca::TxType::kTxTypeDeclaration == tx_type)
//             {
//                 is_declare = true;
//                 multiSignPub = tx_info["MultiSignPub"].get<std::string>();
//                 multiSignPub = Base64Decode(multiSignPub);
//             }
//             else if (global::ca::TxType::kTxTypeDeployContract == tx_type)
//             {
//                 is_deploy_contract = true;
//                 if(tx_info.find("OwnerEvmAddr") != tx_info.end())
//                 {
//                     OwnerEvmAddr = tx_info["OwnerEvmAddr"].get<std::string>();
//                 }
                
//                 input = tx_info["Input"].get<std::string>();
//                 output = tx_info["Output"].get<std::string>();
//                 storage = tx_info["Storage"];
//                 vm_type = tx_info["VmType"].get<VmInterface::VmType>();
//             }
//             else if (global::ca::TxType::kTxTypeCallContract == tx_type)
//             {
//                 is_call_contract = true;
//                 if(tx_info.find("OwnerEvmAddr") != tx_info.end())
//                 {
//                     OwnerEvmAddr = tx_info["OwnerEvmAddr"].get<std::string>();
//                 }
//                 deployer_addr = tx_info["DeployerAddr"].get<std::string>();
//                 deploy_hash = tx_info["DeployHash"].get<std::string>();
//                 input = tx_info["Input"].get<std::string>();
//                 output = tx_info["Output"].get<std::string>();
//                 storage = tx_info["Storage"];
//                 //prev_tx_hash = tx_info["PrevHash"].get<std::string>();
//                 vm_type = tx_info["VmType"].get<VmInterface::VmType>();
//             }
//             else if (global::ca::TxType::kTxTypeTx == tx_type)
//             {
//                 is_tx = true;
//             }

//         }
//         catch(...)
//         {
//             ERRORLOG(RED "JSON failed to parse data field!" RESET);
//             return -1;
//         }
//     }

//     auto passCode = DoubleSpendCheck(tx, turn_on_missing_block_protocol);
//     if (passCode != 0)
//     {
//         return passCode - 20000;
//     }

//     // std::set<std::string> owner_addrs(tx.utxo().owner().begin(), tx.utxo().owner().end());
//     // bool isMultiSign = owner_addrs.size() == 1 && CheckBase58Addr(*owner_addrs.begin(), Base58Ver::kBase58Ver_MultiSign) && is_tx && tx.utxo().vin_size() == 1; 
//     bool isMultiSign = IsMultiSign(tx);

//     uint64_t vin_amount = 0;
    
//     int count = 0;
//     for (auto &vin : tx.utxo().vin())
//     {
//         Base58Ver ver = isMultiSign ? Base58Ver::kBase58Ver_MultiSign : Base58Ver::kBase58Ver_Normal;
//         std::string addr = GetBase58Addr(vin.vinsign().pub(), ver);
        
//         for (auto & prevout : vin.prevout())
//         {
//             std::string utxo = prevout.hash();
//             uint32_t index = prevout.n();
//             INFOLOG("index = {}", index);
            
//             // Errors are not allowed for subscripts of pre utxo array
//             std::string tx_raw;
//             if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(utxo, tx_raw))
//             {
//                 MagicSingleton<BlockHelper>::GetInstance()->PushMissUTXO(utxo);
//                 ERRORLOG(RED "Errors are not allowed for subscripts of pre utxo array!" RESET);
//                 return -4;
//             }

//             CTransaction utxo_tx;
//             if (!utxo_tx.ParseFromString(tx_raw))
//             {
//                 ERRORLOG(RED "Protobuf parse failed!" RESET);
//                 return -5;
//             }
//             // if (index >= utxo_tx.utxo().vout_size())
//             // {
//             //     ERRORLOG(RED "Subscript out of bounds!" RESET);
//             //     return -6;
//             // }
           
//             TransactionType transaction_type = GetTransactionType(utxo_tx);
//             global::ca::TxType utxo_tx_type = (global::ca::TxType)utxo_tx.txtype();


//             if (global::ca::TxType::kTxTypeUnstake == utxo_tx_type && 
//                 TransactionType::kTransactionType_Tx == transaction_type)
//             {
//                 for (auto &vout : utxo_tx.utxo().vout())
//                 {
//                     vin_amount += vout.value();
//                 }
//             }
//             else if (global::ca::TxType::kTxTypeDisinvest == utxo_tx_type && 
//                     TransactionType::kTransactionType_Tx == transaction_type)
//             {
//                 for (auto &vout : utxo_tx.utxo().vout())
//                 {
//                     vin_amount += vout.value();
//                 }
//             }
//             else if (global::ca::TxType::kTxTypeBonus == utxo_tx_type && 
//                     TransactionType::kTransactionType_Tx == transaction_type)
//             {
//                 for (auto &vout : utxo_tx.utxo().vout())
//                 {
//                     if(vout.addr() == addr)
//                     {
//                         vin_amount += vout.value();
//                     }
//                 }
//             }
//             else if (global::ca::TxType::kTxTypeCallContract == utxo_tx_type && 
//                     TransactionType::kTransactionType_Tx == transaction_type)
//             {
//                 for (auto &vout : utxo_tx.utxo().vout())
//                 {
//                     if(vout.addr() == addr)
//                     {
//                         vin_amount += vout.value();
//                     }
//                 }
//             }
//             else
//             {   
//                 if(global::ca::TxType::kTxTypeUnstake == tx_type ||
//                     global::ca::TxType::kTxTypeDisinvest == tx_type)
//                 {
//                     if(++count > 1)
//                     {
//                         for (auto &vout : utxo_tx.utxo().vout())
//                         {
//                             if(vout.addr() == addr)
//                             {
//                                 vin_amount += vout.value();
//                             }
//                         } 
//                     }
//                     else
//                     {
//                         for (auto &vout : utxo_tx.utxo().vout())
//                         {
//                             if(vout.addr() == global::ca::kVirtualStakeAddr ||
//                                 vout.addr() == global::ca::kVirtualInvestAddr)
//                             {
//                                 vin_amount += vout.value();
//                             }
//                         } 
//                     }
//                 }
//                 else
//                 {
//                     for (auto &vout : utxo_tx.utxo().vout())
//                     {
//                         if(vout.addr() == addr)
//                         {
//                             vin_amount += vout.value();
//                         }
//                     } 
//                 }
//             }
//         }
//     }
    
//     //vin Accumulated claim balance
//     std::map<std::string, uint64_t> CompanyDividend;
//     uint64_t costo=0;
// 	uint64_t NodeDividend=0;
//     uint64_t VinAmountCopia=vin_amount;
//     uint64_t TotalClaim=0;
//     if(is_claim)
//     {
//         std::string Addr;
//         Addr = GetBase58Addr(tx.utxo().vin(0).vinsign().pub());
//         uint64_t tx_time = tx.time();
//         auto ret = ca_algorithm::CalcBonusValue(tx_time, Addr, CompanyDividend);
//         if(ret < 0)
//         {
//             ERRORLOG(RED "Failed to obtain the amount claimed by the investor ret:({})" RESET, ret);
//             return -7;
//         } 
//         for(auto & Company : CompanyDividend)
//         {
//             costo = Company.second * 0.05 + 0.5;
//             NodeDividend += costo;
//             vin_amount += (Company.second-costo);
//             TotalClaim += (Company.second-costo);
//         }
//         vin_amount += NodeDividend;
//         TotalClaim += NodeDividend;

//         if(TotalClaim != claim_amount) 
//         {
//             return -8;
//         }
//     }

//     // 3. The transaction amount must be consistent (input = output + packaging fee + handling fee)
//     uint64_t vout_amount = 0;
//     for (auto &vout : tx.utxo().vout())
//     {
//         vout_amount += vout.value();
//     }
//     if ((vout_amount + package_fee + sign_fee * (need_sign_node_num - 1)) != vin_amount)
//     {
//         ERRORLOG(RED "Input is not equal to output + packaging fee + handling fee!" RESET);
//         DEBUGLOG("vout_amount = {} , package_fee = {} , sign_fee = {}, vin_amount = {}",vout_amount,package_fee,sign_fee,vin_amount);
//         return -9;
//     }

//     { // 计算前置交易是否包含所用账号
//     //Calculate whether the pre-transaction includes the account number used
//         std::set<std::string> txVinVec;
//         for(auto & vin : tx.utxo().vin())
//         {
//             Base58Ver ver = isMultiSign ? Base58Ver::kBase58Ver_MultiSign : Base58Ver::kBase58Ver_Normal;
//             std::string vinAddr = GetBase58Addr(vin.vinsign().pub(), ver);
//             for (auto & prevHash : vin.prevout())
//             {
//                 std::string prevUtxo = prevHash.hash();
//                 std::string strTxRaw;
//                 if (DBStatus::DB_SUCCESS !=  db_reader.GetTransactionByHash(prevUtxo, strTxRaw))
//                 {
//                     ERRORLOG("get tx failed");
//                     return -10;
//                 }

//                 CTransaction prevTx;
//                 prevTx.ParseFromString(strTxRaw);
//                 if (prevTx.hash().size() == 0)
//                 {
//                     return -11;
//                 }

//                 std::vector<std::string> prevTxOutAddr;
//                 for (auto & txOut : prevTx.utxo().vout())
//                 {
//                     prevTxOutAddr.push_back(txOut.addr());
//                 }

//                 if (std::find(prevTxOutAddr.begin(), prevTxOutAddr.end(), vinAddr) == prevTxOutAddr.end())
//                 {
//                     return -12;
//                 }
//             }
//         }
//     }

//     std::string redeem_utxo_raw;
//     CTransaction redeem_utxo;
//     std::string divest_utxo_raw;
//     CTransaction divest_utxo;

//     if (global::ca::TxType::kTxTypeUnstake == tx_type)
//     {
//         std::set<std::string> setOwner(tx.utxo().owner().begin(), tx.utxo().owner().end());
//         if (setOwner.size() != 1)
//         {
//             return -13;
//         }
//         uint64_t staked_amount = 0;
//         std::string owner = tx.utxo().owner().at(0);
//         int ret = IsQualifiedToUnstake(owner, redeem_utxo_hash, staked_amount);
//         if(ret != 0)
//         {
//             ERRORLOG(RED "Not allowed to invest!" RESET);
//             return ret - 200;
//         }
//         if (tx.utxo().vout(0).addr() != owner)
//         {
//             ERRORLOG(RED "The address of the withdrawal utxo is incorrect!" RESET);
//             return -14;
//         }
//         if (tx.utxo().vout(0).value() != staked_amount)
//         {
//             ERRORLOG(RED "The value of the withdrawal utxo is incorrect!" RESET);
//             return -15;
//         }
//     }
//     if(global::ca::TxType::kTxTypeInvest == tx_type)
//     {
//         if (tx.utxo().owner().size() != 1)
//         {
//             return -16;
//         }
//         std::string owner = tx.utxo().owner().at(0);
//         int ret = CheckInvestQualification(owner, invest_node, invest_amount);
//         if(ret != 0)
//         {
//             ERRORLOG(RED "Not allowed to invest!" RESET);
//             return ret - 300;
//         }
//     }
//     else if (global::ca::TxType::kTxTypeDisinvest == tx_type)
//     {
//         std::set<std::string> setOwner(tx.utxo().owner().begin(), tx.utxo().owner().end());
//         if (setOwner.size() != 1)
//         {
//             return -17;
//         }

//         uint64_t invested_amount = 0;
//         std::string owner = tx.utxo().owner().at(0);
//         int ret = IsQualifiedToDisinvest(owner, invested_node, divest_utxo_hash, invested_amount);
//         if(ret != 0)
//         {
//             ERRORLOG(RED "Not allowed to Dinvest!" RESET);
//             return ret - 400;
//         }
//         if (tx.utxo().vout(0).addr() != owner)
//         {
//             ERRORLOG(RED "The address of the withdrawal utxo is incorrect!" RESET);
//             return -18;
//         }
//         if (tx.utxo().vout(0).value() != invested_amount)
//         {
//             ERRORLOG(RED "The value of the withdrawal utxo is incorrect!" RESET);
//             return -19;
//         }
//     }
// 	else if (global::ca::TxType::kTxTypeDeclaration == tx_type)
// 	{
// 		std::string multiSignAddr = GetBase58Addr(multiSignPub, Base58Ver::kBase58Ver_MultiSign);
        
//         DBReader db_reader;
//         std::vector<std::string> multiSignAddrs;
//         auto db_status = db_reader.GetMutliSignAddress(multiSignAddrs);
//         if (DBStatus::DB_SUCCESS != db_status)
//         {
//             if (DBStatus::DB_NOT_FOUND != db_status)
//             {
//                 return -20;
//             }
//         }

//         if (std::find(multiSignAddrs.begin(), multiSignAddrs.end(), multiSignAddr) != multiSignAddrs.end())
//         {
//             return -21;
//         }
// 	}
//     else if(global::ca::TxType::kTxTypeBonus == tx_type)
//     {
//         std::string Addr;
//         uint64_t MiningBalance=0;
//         Addr = GetBase58Addr(tx.utxo().vin(0).vinsign().pub());
//         uint64_t cur_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//         uint64_t zero_time = MagicSingleton<TimeUtil>::GetInstance()->getMorningTime(cur_time)*1000000;//Convert to subtle
//         uint64_t error_time = cur_time + (10 * 1000000);
//         if(verify_abnormal) //broadcast
//         {
//             //Time to initiate claim > 1:00 a.m. & < my time
//             if(tx.time() > error_time || tx.time() < (zero_time + (uint64_t)60 * 60 * 1000000))
//             {
//                 return -22;
//             }
//         }
//         else //sync
//         {
//             if(tx.time() > error_time)
//             {
//                 return -23;
//             }
//         }

//         std::vector<std::string> utxos;
//         uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//         auto status =  db_reader.GetBonusUtxoByPeriod(Period, utxos);
//         if (status != DBStatus::DB_SUCCESS && status != DBStatus::DB_NOT_FOUND)
//         {
//             return -24;
//         }
        
//         if(status == DBStatus::DB_SUCCESS)
//         {
//             std::string strTx;
//             CTransaction Claimtx;
            
//             for(auto utxo = utxos.rbegin(); utxo != utxos.rend(); utxo++)
//             {
//                 if (db_reader.GetTransactionByHash(*utxo, strTx) != DBStatus::DB_SUCCESS)
//                 {
//                     MagicSingleton<BlockHelper>::GetInstance()->PushMissUTXO(*utxo);
//                     return -25;
//                 }	
//                 if(!Claimtx.ParseFromString(strTx))
//                 {
//                     return -26;
//                 }
//                 std::string ClaimAddr = GetBase58Addr(Claimtx.utxo().vin(0).vinsign().pub());
//                 if(Addr == ClaimAddr)//Application completed
//                 {
//                     return -27;
//                 }
//             }
//         }
//         // The total number of investors must be more than 10 before they can apply for it
//         std::vector<string> addresses;
//         if (db_reader.GetInvestAddrsByBonusAddr(Addr, addresses) != DBStatus::DB_SUCCESS)
//         {
//             return -28;
//         }
//         if(addresses.size() < 1 || addresses.size() > 999)//modify
//         {
//             return -29;
//         }

//         // The total investment can be bonused only if it is more than 5000 yuan
//         uint64_t sum_invest_amount = 0;
//         for(auto& address : addresses)
//         {
//             std::vector<string> utxos;
//             if (db_reader.GetBonusAddrInvestUtxosByBonusAddr(Addr,address,utxos) != DBStatus::DB_SUCCESS)
//             {
//                 return -30;
//             }

//             uint64_t invest_amount = 0;
//             for (const auto& hash : utxos)
//             {
//                 std::string strTx;
//                 if (db_reader.GetTransactionByHash(hash, strTx) != DBStatus::DB_SUCCESS)
//                 {
//                     return -31;
//                 }

//                 CTransaction tx;
//                 if(!tx.ParseFromString(strTx))
//                 {
//                     return -32;
//                 }

//                 for (int i = 0; i < tx.utxo().vout_size(); i++)
//                 {
//                     if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
//                     {
//                         invest_amount += tx.utxo().vout(i).value();
//                         break;
//                     }
//                 }
//             }
//             sum_invest_amount += invest_amount;
//         }
//         if(sum_invest_amount < global::ca::kMinInvestAmt)
//         {
//             return -33;
//         }
//         {
//             std::lock_guard<std::mutex> lock(global::ca::kBonusMutex);
//             if (DBStatus::DB_SUCCESS != db_reader.GetTotalAwardAmount(MiningBalance))
//             {
//                 return -34;
//             }
//         }
        
//         int i=0;
//         costo=0;
//         NodeDividend=0;
//         for(auto &vout : tx.utxo().vout())
//         {
//             if(tx.utxo().vout().size()-1 != i)
//             {
//                 if(CompanyDividend.end() != CompanyDividend.find(vout.addr()))
//                 {
//                     costo=CompanyDividend[vout.addr()] * 0.05 + 0.5;
//                     NodeDividend+=costo;
//                     if(CompanyDividend[vout.addr()] - costo != vout.value())
//                     {
//                         return -35;
//                     }
//                 }
//                 else
//                 {
//                     return -36;
//                 }
//                 ++i;
//             }
//         }
//         uint64_t LastVoutAmount=VinAmountCopia-(package_fee + sign_fee * (need_sign_node_num - 1))+NodeDividend;
//         if(Addr==tx.utxo().vout(i).addr())
//         {
//             if(LastVoutAmount != tx.utxo().vout(i).value())
//             {
//                 return -37;
//             }
//         }
//     }
//     else if (global::ca::TxType::kTxTypeDeployContract == tx_type)
//     {
//         std::string expected_output;
//         nlohmann::json expected_storage;
//         DonHost host;
//         int ret;
//         if(vm_type == VmInterface::VmType::EVM)
//         {
//             ret = Evmone::DeployContract(OwnerEvmAddr, input, expected_output, host);
//             Evmone::getStorage(host, expected_storage);
//         }
//         else
//         {
//             return -500;
//         }

//         if (ret != 0)
//         {
//             ERRORLOG("VM failed to deploy contract!");
//             ret -= 600;
//             return ret;
//         }

//         if(output != expected_output || storage != expected_storage)
//         {
//             return -38;
//         }

//     }
//     else if (global::ca::TxType::kTxTypeCallContract == tx_type)
//     {
//         std::string expected_output;
//         nlohmann::json expected_storage;
//         DonHost host;
//         int ret;
//         if(vm_type == VmInterface::VmType::EVM)
//         {
//             ret = Evmone::CallContract(OwnerEvmAddr, deployer_addr, deploy_hash, input, expected_output, host);
//             Evmone::getStorage(host, expected_storage);
//         }
//         else
//         {
//             return -39;
//         }

//         if (ret != 0)
//         {
//             ERRORLOG("VM failed to call contract!");
//             ret -= 300;
//             return ret;
//         }
//         if(output != expected_output || storage != expected_storage)
//         {
//             return -40;
//         }

//     }

//     std::string award_addr;
//     std::vector<std::string> pledge_addrs;
//     auto status = db_reader.GetStakeAddress(pledge_addrs);
//     if (DBStatus::DB_SUCCESS != status && DBStatus::DB_NOT_FOUND != status)
//     {
//         return -41;
//     }

//     if (global::ca::TxType::kTxTypeTx != tx_type)
//     {
//         std::set<std::string> setOwner(tx.utxo().owner().begin(), tx.utxo().owner().end());
//         if (setOwner.size() != 1)
//         {
//             return -42;
//         }
//     }

//     // Within the height of 50, the pledge transaction or the transaction of the initial account can be initiated arbitrarily
//     if (tx_height <= global::ca::kMinUnstakeHeight && (tx.utxo().owner().at(0) == global::ca::kInitAccountBase58Addr || global::ca::TxType::kTxTypeStake == tx_type || global::ca::TxType::kTxTypeInvest == tx_type))
//     {
//         int i = 0;
//         for (auto &tx_sign_pre_hash : tx.verifysign())
//         {
//             award_addr = GetBase58Addr(tx_sign_pre_hash.pub());
//             if (!CheckBase58Addr(award_addr, Base58Ver::kBase58Ver_Normal))
//             {
//                 return -43;
//             }
//             ++i;
//         }
//     }
//     else
//     {

//         // 当代发时判断第一个流转节点是否足额质押和投资  
//         //Modern development time judges whether the first circulation node is fully pledged and invested

//         bool isNeedAgent = TxHelper::IsNeedAgent(tx);

//         for (int i = (isNeedAgent ? 0 : 1); i < tx.verifysign_size(); ++i)
//         {
//             std::string sign_addr = GetBase58Addr(tx.verifysign(i).pub(), Base58Ver::kBase58Ver_Normal);
//             auto ret = VerifyBonusAddr(sign_addr);
//             if(ret < 0)
//             {
//                 ERRORLOG("VerifyBonusAddr error ret:{} sign_addr:({})", ret, sign_addr);
//                 return -44;
//             }
            
//             int64_t stake_time = ca_algorithm::GetPledgeTimeByAddr(sign_addr, global::ca::StakeType::kStakeType_Node);
//             if (stake_time <= 0)
//             {
//                 return -45;
//             }
//         }
//     }
//     uint64_t end_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//     if(global::ca::TxType::kTxTypeTx == tx_type)
//     {
//         MagicSingleton<TFSBenchmark>::GetInstance()->AddtransactionDBVerifyMap(tx.hash(), end_time - start_time);  
//     }
	
//     return 0;
// }

// int ca_algorithm::GetTxSignAddr(const CTransaction &tx, std::vector<std::string> &tx_sign_addr)
// {
//     if(tx.verifysign().empty())
//     {
//         return -1;
//     }

//     for (auto &tx_sign_pre_hash : tx.verifysign())
//     {
//         std::string addr = GetBase58Addr(tx_sign_pre_hash.pub());
//         if (!CheckBase58Addr(addr))
//         {
//             return -2;
//         }
//         tx_sign_addr.push_back(addr);
//     }

//     return 0;
// }

// int ca_algorithm::GetSignTxSignAddr(const CTransaction &tx, std::vector<std::string> &sign_addrs)
// {
//     if(tx.utxo().vout().empty())
//     {
//         return -1;
//     }

//     for (auto &vout : tx.utxo().vout())
//     {
//         std::string addr = vout.addr();
//         if (!CheckBase58Addr(addr))
//         {
//             return -2;
//         }
//         sign_addrs.push_back(addr);
//     }

//     return 0;
// }

// int ca_algorithm::GetBurnTxAddr(const CTransaction &tx, std::vector<std::string> &burn_addrs)
// {
//     if(tx.utxo().vout().empty())
//     {
//         return -1;
//     }

//     for(int i = 0 ; i < 2 ; i++) //目前发起方只有一个 There is currently only one initiator
//     {
//        const CTxOutput & vout =  tx.utxo().vout(i);
//        if(i == 0)
//        {
//            if(vout.addr() != global::ca::kVirtualBurnGasAddr)
//            {
//                return -2;
//            }
//        }
//        else 
//        {
//            if(vout.addr() != tx.utxo().owner(0))
//            {
//                return -3;
//            }
//        }
//        burn_addrs.push_back(vout.addr());
//     }

//     return 0;
// }

// int ca_algorithm::DoubleSpendCheck(const CTransaction &tx, bool turn_on_missing_block_protocol, std::string* missing_utxo)
// {
//     std::string redeem_utxo_hash;
//     std::string divest_utxo_hash;
//     std::string invested_node;
//     global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();

//     if((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
//     {
//         try
//         {
//             nlohmann::json data_json = nlohmann::json::parse(tx.data());
//             nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//             if (global::ca::TxType::kTxTypeUnstake == tx_type)
//             {             
//                 redeem_utxo_hash = tx_info["UnstakeUtxo"].get<std::string>();
//             }
//             else if (global::ca::TxType::kTxTypeDisinvest == tx_type)
//             {
//                 divest_utxo_hash = tx_info["DisinvestUtxo"].get<std::string>();
//                 invested_node = tx_info["BonusAddr"].get<std::string>();        
//             }

//         }
//         catch(...)
//         {
//             ERRORLOG(RED "JSON failed to parse data field!" RESET);
//             return -1;
//         }
//     }

//     bool isMultiSign = IsMultiSign(tx);
//     DBReader db_reader;
//     for (auto &vin : tx.utxo().vin())
//     {
//         Base58Ver ver = isMultiSign ? Base58Ver::kBase58Ver_MultiSign : Base58Ver::kBase58Ver_Normal;
//         std::string addr = GetBase58Addr(vin.vinsign().pub(), ver);

//         // Verify whether the utxo used exists and is used
//         std::vector<std::string> utxo_hashs;
//         if (DBStatus::DB_SUCCESS != db_reader.GetUtxoHashsByAddress(addr, utxo_hashs))
//         {
//             ERRORLOG(RED "GetUtxoHashsByAddress failed!" RESET);
//             return -2;
//         }
        
//         for (auto & prevout : vin.prevout())
//         {
//             std::string utxo = prevout.hash();
//             uint32_t index = prevout.n();
            

//             if(global::ca::TxType::kTxTypeStake == tx_type)
//             {
//                 std::vector<std::string> stake_utxos;
//                 auto ret = db_reader.GetStakeAddressUtxo(addr,stake_utxos);
//                 if(ret != DBStatus::DB_NOT_FOUND)
//                 {
//                     ERRORLOG("There has been a pledge transaction before!");
//                     return -3;
//                 }

//             }
//             else if ((global::ca::TxType::kTxTypeUnstake == tx_type) && redeem_utxo_hash == utxo && 0 == index)
//             {
//                 if (DBStatus::DB_SUCCESS != db_reader.GetStakeAddressUtxo(addr, utxo_hashs))
//                 {
//                     ERRORLOG(RED "GetStakeAddressUtxo failed!" RESET);
//                     return -4;
//                 } 
//                 if (utxo_hashs.cend() == std::find(utxo_hashs.cbegin(), utxo_hashs.cend(), utxo))
//                 {
//                     if(missing_utxo != nullptr)
//                     {
//                         *missing_utxo = utxo;
//                     }
//                     return -5;
//                 }
//             }
//             else if((global::ca::TxType::kTxTypeDisinvest == tx_type) && divest_utxo_hash == utxo && 0 == index)
//             {
//                 if (DBStatus::DB_SUCCESS != db_reader.GetBonusAddrInvestUtxosByBonusAddr(invested_node, addr, utxo_hashs))
//                 {
//                     ERRORLOG(RED "GetBonusAddrInvestUtxosByBonusAddr failed!" RESET);
//                     return -6;
//                 }
//                 if (utxo_hashs.cend() == std::find(utxo_hashs.cbegin(), utxo_hashs.cend(), utxo))
//                 {
//                     if(missing_utxo != nullptr)
//                     {
//                         *missing_utxo = utxo;
//                     }
//                     return -7;
//                 }                
//             }
//             else
//             {
//                 if (utxo_hashs.cend() == std::find(utxo_hashs.cbegin(), utxo_hashs.cend(), utxo))
//                 {
//                     if(missing_utxo != nullptr)
//                     {
//                         *missing_utxo = utxo;
//                     }
//                     //前块是存在的，但是utxo对应的块不存在
//                     if(turn_on_missing_block_protocol)
//                     {
//                         DEBUGLOG("turn_on_missing_block_protocol");
//                         MagicSingleton<BlockHelper>::GetInstance()->PushMissUTXO(utxo);                        
//                     }
//                     return -8;
//                 }
//             }
//         }
//     }
//     return 0;
// }

// int ca_algorithm::VerifyCacheTranscation(const CTransaction &tx)
// {
//     // 1. Transaction  number must be 0
//     if (tx.version() != 0)
//     {
//         return -1;
//     }

//     // 2. The number of signatures must be equal to 8\
//     the same as the extended field   
//     std::vector<std::string> tx_sign_addr;
//     int res = GetTxSignAddr(tx,tx_sign_addr);
//     if(res != 0)
//     {
//         return res;
//     }
//     if (tx_sign_addr.size() != global::ca::kConsensus)
//     {
//         return -2;
//     }

//     std::vector<std::string> award_addrs;
//     for (int i = 2; i < tx.verifysign_size(); ++i)
//     {
//         auto &pub = tx.verifysign(i).pub();
//         award_addrs.push_back(GetBase58Addr(tx.verifysign(i).pub()));
//     }
//     {
//         std::set<std::string> addrs(award_addrs.cbegin(), award_addrs.cend());
//         if (addrs.size() != (global::ca::kConsensus - 2) || addrs.size() != award_addrs.size())
//         {
//             return -3;
//         }
//     }
    
//     return 0;
// }

// int ca_algorithm::MemVerifyBlock(const CBlock& block)
// {
//     // Block version number must be 0
//     if (block.version() != 0)
//     {
//         return -1;
//     }

//     // The size of the serialized block must be less than 1000000
//     if (block.SerializeAsString().length() > 1000000)
//     {
//         return -2;
//     }

//     // Verify whether the block hash is the same
//     if (block.hash() != CalcBlockHash(block))
//     {
//         return -3;
//     }

//     // Verify that merklelot is the same
//     if (block.merkleroot() != CalcBlockMerkle(block))
//     {
//         return -4;
//     }

//     // The number of transactions in the block must be greater than 0 \
//     and a multiple of 2 (normal transaction, signature transaction)
//     if(block.txs_size() % 3 != 0)
//     {
//        return -5;
//     } 

//     // Transactions in the block must be in groups of 2 \
//     Key is the calculated hash and value is the transaction
//     std::map<std::string ,vector<CTransaction>>  tx_group; 
//     for(auto tx : block.txs())
//     {
//         std::string hash;
//         if (GetTransactionType(tx) == kTransactionType_Tx) 
//         {
//             hash = tx.hash();
//         }
//         else if(GetTransactionType(tx) == kTransactionType_Gas) 
//         {
//             if (tx.utxo().vin_size() == 0)
//             {
//                 return -6;
//             }
//             hash = tx.utxo().vin(0).prevout(0).hash();
//         }
//         else if(GetTransactionType(tx) == kTransactionType_Burn)
//         {
//             if (tx.utxo().vin_size() == 0)
//             {
//                 return -7;
//             }
//             hash = tx.utxo().vin().at(0).prevout(0).hash();
//         }
//         else
//         {
//             return -8;
//         }

//         auto iter = tx_group.find(hash);
//         if(iter == tx_group.end())
//         {
//             tx_group[hash] = std::vector<CTransaction>{};
//         }
//         tx_group[hash].push_back(tx);
//     }

//     // Whether there are two transactions in the same group and whether they are of two types. \
//     Re insert the type sorting into the new array
//     std::vector<CTransaction> tx_sort;
//     auto iter = tx_group.begin();

//     CBlock tmp_block = block;
//     tmp_block.clear_txs();
//     while(iter != tx_group.end())
//     {
//         if(iter->second.size() != 3)
//         {
//             DEBUGLOG("Number of transaction types:{}",iter->second.size());
//             return -9;
//         }

//         CTransaction tx;
// 		CTransaction gasTx;
// 		CTransaction burnTx;
//         for(auto itemTx : iter->second)
//         {
//             TransactionType tx_type = GetTransactionType(itemTx);
//             if(tx_type == kTransactionType_Tx)
//             {
//                 tx = itemTx;
//             }
//             if(tx_type == kTransactionType_Gas)
//             {
//                 gasTx = itemTx;
//             }
//             if(tx_type == kTransactionType_Burn)
//             {
// 				burnTx = itemTx;
//             }
//         }

// 		if (tx.hash().empty() || gasTx.hash().empty() || burnTx.hash().empty())
// 		{
// 			return -10;
// 		}
//         auto ret = MemVerifyTransactionTx(tx);
// 		if (ret != 0)
// 		{
//             ERRORLOG("MemVerifyTransactionTx Error:{},Hash:{}",ret, block.hash());
// 			return -11;
// 		}

// 		if (MemVerifyTransactionGas(gasTx) != 0)
// 		{
//               ERRORLOG( " MemVerifyTransactionGas !!");
// 			return -12;
// 		}

// 		if (MemVerifyTransactionBurn(burnTx) != 0)
// 		{
//             ERRORLOG( " MemVerifyTransactionBurn !!");
// 			return -13;
// 		}

// 		std::vector<std::string> verifySignAddr;
// 		for (auto & sign : tx.verifysign())
// 		{
// 			verifySignAddr.push_back(GetBase58Addr(sign.pub()));
// 		}

// 		std::vector<std::string> voutAddr;
// 		for (auto & vout : gasTx.utxo().vout())
// 		{
// 			voutAddr.push_back(vout.addr());
// 		}

// 		std::vector<std::string> v_diff;
//         std::set_difference(verifySignAddr.begin(), verifySignAddr.end(),
//                             voutAddr.begin(),voutAddr.end(),
//                             std::back_inserter(v_diff));
//         if(v_diff.size() > 1)
//         {
//             return -14;
//         }

//         bool isNeedAgent = TxHelper::IsNeedAgent(tx);

//         uint64_t gas = 0;
//         if(CalculateGas(tx,gas) != 0)
//         {
//             std::cout << "MemVerifyBlock gas = " << gas << std::endl;
//             return -15;
//         }

// 		if (gasTx.utxo().vout(0).value() != (isNeedAgent ? gas : 0))
// 		{
//             ERRORLOG("-><-[-] ERROR isNeddAgent:{},ags:{},vout(0):{}",(isNeedAgent ?"true":"false"),gas,gasTx.utxo().vout(0).value());
            
// 			return -16;
// 		}
// 		for (int index = 1; index != gasTx.utxo().vout_size(); ++index)
// 		{
// 			const CTxOutput & vout = gasTx.utxo().vout(index);
// 			if (vout.value() != gas / 2)
// 			{
//                 ERRORLOG("-><-[-] gas:{}",gas);
// 				return -17;
// 			} 
// 		}

// 		int64_t needBurnValue =  0;
        
// 		needBurnValue += (tx.verifysign_size() - 1) * gas / 2;

// 		int64_t burnValue = 0;
// 		for (auto & vout : burnTx.utxo().vout())
// 		{
// 			if (vout.addr() == global::ca::kVirtualBurnGasAddr)
// 			{
// 				burnValue = vout.value();
// 				break;
// 			}
// 		}

// 		if (needBurnValue != burnValue)
// 		{
//             ERRORLOG("-><-[-] needBurnVale:{},burnValue:{}",needBurnValue,burnValue);
// 			return -18;
// 		}
//         ++iter;
//     }

//     return 0;
// }

// int ca_algorithm::VerifyBlock(const CBlock &block, bool turn_on_missing_block_protocol, bool verify_abnormal)
// {
//     uint64_t start_time_for_benchmark = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();

//     DBReader db_reader;

//     // Verify whether the block exists locally
//     std::string block_raw;
//     auto status = db_reader.GetBlockByBlockHash(block.hash(), block_raw);
//     if (DBStatus::DB_SUCCESS == status)
//     {
//         return 0;
//     }
//     if (DBStatus::DB_NOT_FOUND != status)
//     {
//         return -1;
//     }

//     // Verify whether the front block exists
//     block_raw.clear();
//     if (DBStatus::DB_SUCCESS != db_reader.GetBlockByBlockHash(block.prevhash(), block_raw))
//     {
//         MagicSingleton<BlockHelper>::GetInstance()->SetMissingPrehash();
//         return -2;
//     }
//     CBlock pre_block;
//     if (!pre_block.ParseFromString(block_raw))
//     {
//         MagicSingleton<BlockHelper>::GetInstance()->SetMissingPrehash();
//         return -3;
//     }

//     // The block height must be the height of the preceding block plus one
//     if (block.height() - pre_block.height() != 1)
//     {
//         return -4;
//     }


//     auto ret = MemVerifyBlock(block);
//     if (0 != ret)
//     {
//         ret -= 10000;
//         ERRORLOG(RED "MemVerifyBlock failed! The error code is {}." RESET, ret);
//         return ret;
//     }

//     // Verify whether the block time is greater than the maximum block time before 10 heights
//     uint64_t start_time = 0;
//     uint64_t end_time = GetLocalTimestampUsec() + 10 * 60 * 1000 * 1000;
//     {
//         uint64_t block_height = 0;
//         if (block.height() > 10)
//         {
//             block_height = block.height() - 10;
//         }
//         std::vector<std::string> block_hashs;
//         if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashsByBlockHeight(block_height, block_hashs))
//         {
//             return -5;
//         }
//         std::vector<std::string> blocks;
//         if (DBStatus::DB_SUCCESS != db_reader.GetBlocksByBlockHash(block_hashs, blocks))
//         {
//             return -6;
//         }
//         CBlock block;
//         for (auto &block_raw : blocks)
//         {
//             if (!block.ParseFromString(block_raw))
//             {
//                 return -7;
//             }
//             if (start_time < block.time())
//             {
//                 start_time = block.time();
//             }
//         }
//     }
//     if (block.time() > end_time || block.time() < start_time)
//     {
//         return -8;
//     }

//     // Verify whether the transaction time is greater than the maximum block time before 10 heights
//     for (auto& tx : block.txs())
//     {
//         if (GetTransactionType(tx) != kTransactionType_Tx)
//         {
//             continue;
//         }
//         ret = VerifyTransactionTx(tx, block.height(), turn_on_missing_block_protocol, verify_abnormal);
//         if (0 != ret)
//         {
//             return ret - 20000;
//         }
//     }

//     uint64_t end_time_for_benchmark = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//     MagicSingleton<TFSBenchmark>::GetInstance()->AddBlockVerifyMap(block.hash(), end_time_for_benchmark - start_time_for_benchmark);
//     return 0;
// }

// static bool CalculateHeightSumHash(uint64_t start_height, uint64_t end_height, DBReadWriter &db_writer, std::string& sum_hash)
// {
//     std::map<uint64_t, std::vector<std::string>> sum_hash_data;
//     for(uint64_t height = start_height; height < end_height; ++height)
//     {
//         std::vector<std::string> blockhashes;
//         auto ret = db_writer.GetBlockHashsByBlockHeight(height, blockhashes);
//         if(ret != DBStatus::DB_SUCCESS)
//         {
//             ERRORLOG("calculate sum hash fail");
//             return false;
//         }
//         sum_hash_data[height] = blockhashes;
//     }
//     SyncBlock::SumHeightsHash(sum_hash_data, sum_hash);
//     return true;
// }


// int ca_algorithm::SaveBlock(DBReadWriter &db_writer, const CBlock &block, global::ca::SaveType saveType, global::ca::BlockObtainMean obtainMean)
// {
//     // Determine whether there is a local block
//     std::string block_raw;
//     auto ret = db_writer.GetBlockByBlockHash(block.hash(), block_raw);
//     if (DBStatus::DB_SUCCESS == ret)
//     {
//         INFOLOG("block {} already in cache , skip",block.hash().substr(0, 6));
//         return 0;
//     }
//     else if (DBStatus::DB_NOT_FOUND != ret)
//     {
//         ERRORLOG("DB error!  {}",ret);
//         return -1;
//     }

//     std::string pre_block_raw;
//     if (DBStatus::DB_SUCCESS != db_writer.GetBlockByBlockHash(block.prevhash(), pre_block_raw))
//     {
//         MagicSingleton<BlockHelper>::GetInstance()->SetMissingPrehash();
//         DEBUGLOG("DB error or not found prev block");
//         return -2;
//     }

//     // Update node height
//     uint64_t node_height = 0;
//     if (DBStatus::DB_SUCCESS != db_writer.GetBlockTop(node_height))
//     {
//          ERRORLOG("GetBlockTop error!");
//         return -3;
//     }

//     if (block.height() > node_height)
//     {
//         if (DBStatus::DB_SUCCESS != db_writer.SetBlockTop(block.height()))
//         {
//             ERRORLOG("GetBlockTop");
//             return -4;
//         }
//     }

//     // Add the height corresponding to the block hash
//     if (DBStatus::DB_SUCCESS != db_writer.SetBlockHeightByBlockHash(block.hash(), block.height()))
//     {
//         ERRORLOG("SetBlockHeightByBlockHash error! hash:{}",block.hash());
//         return -5;
//     }

//     // Update block hash on height
//     if (DBStatus::DB_SUCCESS != db_writer.SetBlockHashByBlockHeight(block.height(), block.hash(), false))
//     {
//         ERRORLOG("SetBlockHeightByBlockHash error! hash:{}",block.hash());
//         return -6;
//     }
    
//     // Add block data corresponding to block hash
//     if (DBStatus::DB_SUCCESS != db_writer.SetBlockByBlockHash(block.hash(), block.SerializeAsString()))
//     {
//         ERRORLOG("SetBlockByBlockHash error! hash:{}",block.hash());
//         return -7;
//     }

//     std::set<std::string> block_addr;
//     std::set<std::string> all_addr;
//     for (auto &tx : block.txs())
//     {
//         auto transaction_type = GetTransactionType(tx);
//         block_addr.insert(all_addr.cbegin(), all_addr.cend());
//         all_addr.clear();
//         if (kTransactionType_Tx == transaction_type)
//         {
//             std::string redeem_utxo_hash;
//             std::string divest_utxo_hash;
//             if((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
//             {
//                 try
//                 {
//                     nlohmann::json data_json = nlohmann::json::parse(tx.data());
//                     global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();
//                     // The pledge transaction updates the pledge address and the utxo of the pledge address
//                     if (tx_type == global::ca::TxType::kTxTypeStake)
//                     {
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             std::string addr = GetBase58Addr(vin.vinsign().pub());
//                             if (!CheckBase58Addr(addr))
//                             {
//                                 ERRORLOG("CheckBase58Addr error!",addr);
//                                 return -8;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetStakeAddressUtxo(addr, tx.hash()))
//                             {
//                                 ERRORLOG("SetStakeAddressUtxo error! addr:{} hash:{}",addr,tx.hash());
//                                 return -9;
//                             }
//                             std::vector<std::string> pledge_addrs;
//                             ret = db_writer.GetStakeAddress(pledge_addrs);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 ERRORLOG("GetStakeAddress error! ret:{}",ret);
//                                 return -10;
//                             }
//                             if (pledge_addrs.cend() == std::find(pledge_addrs.cbegin(), pledge_addrs.cend(), addr))
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetStakeAddresses(addr))
//                                 {
//                                     ERRORLOG("SetStakeAddresses addr:{}",addr);
//                                     return -11;
//                                 }
//                             }
//                             break;
//                         }
//                     }
//                     // The reddem transaction, update the pledge address and the utxo of the pledge address
//                     else if (tx_type == global::ca::TxType::kTxTypeUnstake)
//                     {
//                         bool flag = false;
                        
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         redeem_utxo_hash = tx_info["UnstakeUtxo"].get<std::string>();
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             for (auto & prevout : vin.prevout())
//                             {
//                                 if (redeem_utxo_hash != prevout.hash())
//                                 {
//                                     continue;
//                                 }
//                                 flag = true;
//                                 std::string addr = GetBase58Addr(vin.vinsign().pub());
//                                 if (!CheckBase58Addr(addr))
//                                 {
//                                     ERRORLOG("CheckBase58Addr error! addr:{}",addr);
//                                     return -12;
//                                 }
//                                 if (DBStatus::DB_SUCCESS != db_writer.RemoveStakeAddressUtxo(addr, redeem_utxo_hash))
//                                 {
//                                     ERRORLOG("RemoveStakeAddressUtxo error");
//                                     return -13;
//                                 }
//                                 std::vector<std::string> pledge_utxo_hashs;
//                                 ret = db_writer.GetStakeAddressUtxo(addr, pledge_utxo_hashs);
//                                 if (DBStatus::DB_NOT_FOUND == ret || pledge_utxo_hashs.empty())
//                                 {
//                                     if (DBStatus::DB_SUCCESS != db_writer.RemoveStakeAddresses(addr))
//                                     {
//                                         ERRORLOG("RemoveStakeAddresses addr:{}",addr);
//                                         return -14;
//                                     }
//                                 }
//                                 else if (DBStatus::DB_SUCCESS != ret)
//                                 {
//                                     ERRORLOG("GetStakeAddressUtxo ret:{}",ret);
//                                     return -15;
//                                 }
//                                 break;

//                             }
//                         }
//                         if (!flag)
//                         {
//                             ERRORLOG(" TxType unknow");
//                             return -16;
//                         }
//                     }
//                     else if ( global::ca::TxType::kTxTypeDeclaration == tx_type )
//                     {
//                         std::string addr;
//                         for (auto & vout : tx.utxo().vout())
//                         {
//                             if (CheckBase58Addr(vout.addr(), Base58Ver::kBase58Ver_MultiSign))
//                             {
//                                 addr = vout.addr();
//                                 break;
//                             }
//                         }
//                         if(addr.size() == 0)
//                         {
//                             ERRORLOG("addr.size() == 0");
//                             return -17;
//                         }
//                         std::vector<std::string> mutliaddrs;
//                         auto db_status = db_writer.GetMutliSignAddress(mutliaddrs);
//                         if (DBStatus::DB_SUCCESS != db_status)
//                         {
//                             if (DBStatus::DB_NOT_FOUND != db_status)
//                             {
//                                 ERRORLOG("DB error! db_status:{}",db_status);
//                                 return -18;
//                             }
//                         }
//                         if(std::find(mutliaddrs.begin(), mutliaddrs.end(), addr) != mutliaddrs.end())
//                         {
//                             ERRORLOG("can't find addr from mutliaddrs! addr:{} ",addr);
//                             return -19;
//                         }

//                         if (DBStatus::DB_SUCCESS != db_writer.SetMutliSignAddresses(addr) )
//                         {
//                             ERRORLOG(" SetMutliSignAddresses error! addr:{}", addr);
//                             return -20;
//                         }
//                         if (DBStatus::DB_SUCCESS != db_writer.SetMutliSignAddressUtxo(addr,tx.hash()))
//                         {
//                             ERRORLOG("SetMutliSignAddressUtxo addr:{}",addr);
//                             return -21;
//                         }
//                     }
//                     // The invest transaction updates the investment address and the utxo of the investment address
//                     else if (tx_type == global::ca::TxType::kTxTypeInvest)
//                     {
                        

//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         std::string invest_node = tx_info["BonusAddr"].get<std::string>();
//                         uint64_t invest_amount = tx_info["InvestAmount"].get<uint64_t>();
//                         std::vector<std::string> invest_nodes;
//                         ret = db_writer.GetBonusaddr(invest_nodes);
//                         if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                         {
//                             ERRORLOG("GetBonusaddr error ret:{}",ret);
//                             return -22;
//                         }
//                         if (invest_nodes.cend() == std::find(invest_nodes.cbegin(), invest_nodes.cend(), invest_node))
//                         {
//                             if (DBStatus::DB_SUCCESS != db_writer.SetBonusAddr(invest_node))
//                             {
//                                 ERRORLOG("can't find invest_node from invest_nodes");
//                                 return -23;
//                             }
//                         }

//                         uint64_t Totalinvest = 0;
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             std::string addr = GetBase58Addr(vin.vinsign().pub());
//                             if (!CheckBase58Addr(addr))
//                             {
//                                 ERRORLOG("CheckBase58Addr addr:{}",addr);
//                                 return -24;
//                             }                      

//                             if (DBStatus::DB_SUCCESS != db_writer.SetBonusAddrInvestAddrUtxoByBonusAddr(invest_node, addr, tx.hash()))
//                             {
//                                 ERRORLOG("SetBonusAddrInvestAddrUtxoByBonusAddr addr:{} invest_node:{}",addr,invest_node);
//                                 return -25;
//                             }
//                             std::vector<std::string> invest_addrs;
//                             ret = db_writer.GetInvestAddrsByBonusAddr(invest_node, invest_addrs);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 ERRORLOG("GetInvestAddrsByBonusAddr ret:{}",ret);
//                                 return -26;
//                             }
//                             if (invest_addrs.cend() == std::find(invest_addrs.cbegin(), invest_addrs.cend(), addr))
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetInvestAddrByBonusAddr(invest_node, addr))
//                                 {
//                                     ERRORLOG("can't find addr from invest_addrs addr:{}",addr);
//                                     return -27;
//                                 }
//                             }

//                             std::vector<std::string> invest_nodes;
//                             ret = db_writer.GetBonusAddrByInvestAddr(addr, invest_nodes);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 ERRORLOG("GetBonusAddrByInvestAddr addr:{}",addr);
//                                 return -28;
//                             }
//                             if (invest_nodes.cend() == std::find(invest_nodes.cbegin(), invest_nodes.cend(), invest_node))
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetBonusAddrByInvestAddr(addr,invest_node))
//                                 {
//                                     ERRORLOG("can't find invest_node from invest_nodes addr:{} ,invest_node{}", addr,invest_node);
//                                     return -29;
//                                 }
//                             }
//                             break;
//                         }
//                         {
//                             std::lock_guard<std::mutex> lock(global::ca::kInvestMutex);
//                             ret = db_writer.GetTotalInvestAmount(Totalinvest);
//                             if(DBStatus::DB_SUCCESS != ret)
//                             {
//                                 if(DBStatus::DB_NOT_FOUND == ret)
//                                 {
//                                     Totalinvest=0;
//                                 }
//                                 else
//                                 {
//                                     ERRORLOG("------------------>>?????");
//                                     return -30;
//                                 }
//                             }
                            
//                             Totalinvest += invest_amount;
//                             if (DBStatus::DB_SUCCESS != db_writer.SetTotalInvestAmount(Totalinvest))
//                             {
//                                 ERRORLOG("SetTotalInvestAmount error! {}",Totalinvest);
//                                 return -31;
//                             }
//                             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//                             if(DBStatus::DB_SUCCESS != db_writer.SetInvestUtxoByPeriod(Period, tx.hash()))
//                             {
//                                 ERRORLOG("SetInvestUtxoByPeriod Period:{},hash:{}",Period,tx.hash());
//                                 return -32;
//                             }
//                         }
//                     }
//                     // The divest transaction updates the investment address and the utxo of the investment address
//                     else if (tx_type == global::ca::TxType::kTxTypeDisinvest)
//                     {
//                         bool flag = false;
                        

//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         divest_utxo_hash = tx_info["DisinvestUtxo"].get<std::string>();
//                         std::string invested_node = tx_info["BonusAddr"].get<std::string>();

//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             for (auto & prevout : vin.prevout())
//                             {
//                                 if (divest_utxo_hash != prevout.hash())
//                                 {
//                                     continue;
//                                 }
//                                 flag = true;
//                                 std::string addr = GetBase58Addr(vin.vinsign().pub());
//                                 if (!CheckBase58Addr(addr))
//                                 {
//                                     ERRORLOG("CheckBase58Addr error addr:{}",addr);
//                                     return -33;
//                                 }
//                                 if (DBStatus::DB_SUCCESS != db_writer.RemoveBonusAddrInvestAddrUtxoByBonusAddr(invested_node, addr, divest_utxo_hash))
//                                 {
//                                     ERRORLOG("RemoveBonusAddrInvestAddrUtxoByBonusAddr error invested_node:{},addr:{},divest_utxo_hash:{}",invested_node, addr, divest_utxo_hash);
//                                     return -34;
//                                 }
//                                 std::vector<string> utxos;
//                                 ret = db_writer.GetBonusAddrInvestUtxosByBonusAddr(invested_node, addr, utxos);
//                                 if (ret == DBStatus::DB_NOT_FOUND || utxos.empty())
//                                 {
//                                     if (DBStatus::DB_SUCCESS != db_writer.RemoveInvestAddrByBonusAddr(invested_node, addr))
//                                     {
//                                         ERRORLOG("RemoveInvestAddrByBonusAddr invested_node:{},addr:{}",invested_node, addr);
//                                         return -35;
//                                     }
//                                     std::vector<string> invest_addrs;
//                                     if (db_writer.GetInvestAddrsByBonusAddr(invested_node, invest_addrs) == DBStatus::DB_NOT_FOUND || invest_addrs.empty())
//                                     {
//                                         if (DBStatus::DB_SUCCESS != db_writer.RemoveBonusAddr(invested_node))
//                                         {
//                                             ERRORLOG("RemoveBonusAddr error invested_node:{}",invested_node);
//                                             return -36;
//                                         }                                
//                                     }
                                    
//                                     if (DBStatus::DB_SUCCESS != db_writer.RemoveBonusAddrByInvestAddr(addr,invested_node))
//                                     {
//                                         ERRORLOG("RemoveBonusAddrByInvestAddr error! addr:{},invested_node:{}",addr,invested_node);
//                                         return -37;
//                                     }
//                                 }
//                                 else if (DBStatus::DB_SUCCESS != ret)
//                                 {
//                                     ERRORLOG("GetBonusAddrInvestUtxosByBonusAddr error! ret:{}",ret);
//                                     return -38;
//                                 }
//                                 break;
//                             }
                            
//                         }
//                         if (!flag)
//                         {
//                             ERRORLOG("unknow type");
//                             return -40;
//                         }
//                         //////////////////
//                         uint64_t invest_amount = 0;
//                         std::string strTx;
//                         if (db_writer.GetTransactionByHash(divest_utxo_hash, strTx) != DBStatus::DB_SUCCESS)
//                         {
//                             MagicSingleton<BlockHelper>::GetInstance()->PushMissUTXO(divest_utxo_hash);
//                             ERRORLOG("GetTransactionByHash error! divest_utxo_hash:{},strTx:{}",divest_utxo_hash, strTx);
//                             return -41;
//                         }
//                         CTransaction InvestTx;
//                         if(!InvestTx.ParseFromString(strTx))
//                         {
//                             ERRORLOG("InvestTx.ParseFromString(strTx) = {} ",strTx);
//                             return -42;
//                         }

//                         for (int i = 0; i < InvestTx.utxo().vout_size(); i++)
//                         {
//                             if (InvestTx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
//                             {
//                                 invest_amount += InvestTx.utxo().vout(i).value();
//                                 break;
//                             }
//                         }
//                         uint64_t Totalinvest = 0;
//                         std::vector<std::string> utxos;
//                         {
//                             std::lock_guard<std::mutex> lock(global::ca::kInvestMutex);
//                             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(InvestTx.time());
//                             auto ret = db_writer.GetInvestUtxoByPeriod(Period, utxos);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 ERRORLOG("DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret {}",ret);
//                                 return -43;
//                             }
//                             if (utxos.cend() != std::find(utxos.cbegin(), utxos.cend(), divest_utxo_hash))
//                             {
//                                 if(DBStatus::DB_SUCCESS != db_writer.GetTotalInvestAmount(Totalinvest))
//                                 {
//                                     ERRORLOG("GetTotalInvestAmount error! Totalinvest:{}",Totalinvest);
//                                     return -44;
//                                 }
//                                 Totalinvest-=invest_amount;
//                                 if(DBStatus::DB_SUCCESS != db_writer.SetTotalInvestAmount(Totalinvest))
//                                 {
//                                     ERRORLOG("SetTotalInvestAmount error! Totalinvest:{}",Totalinvest);
//                                     return -45;
//                                 }
//                                 uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(InvestTx.time());
//                                 if(DBStatus::DB_SUCCESS != db_writer.RemoveInvestUtxoByPeriod(Period, divest_utxo_hash))
//                                 {
//                                     ERRORLOG("RemoveInvestUtxoByPeriod error! Period:{},divest_utxo_hash:{}",Period, divest_utxo_hash);
//                                     return -46;
//                                 }
//                             }
//                         }
//                     }
//                     // Renewal of claim transaction......
//                     else if(tx_type == global::ca::TxType::kTxTypeBonus)
//                     {
//                         uint64_t claim_amount = 0;
//                         uint64_t claim_Vout_amount = 0;
//                         uint64_t TotalCirculation = 0;
//                         uint64_t MiningBalance=0;
//                         int i=0;
                        
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         tx_info["BonusAmount"].get_to(claim_amount);
                        
//                         {
//                             std::lock_guard<std::mutex> lock(global::ca::kBonusMutex);
//                             if(DBStatus::DB_SUCCESS != db_writer.GetM2(TotalCirculation))
//                             {
//                                 ERRORLOG("GetM2 error! TotalCirculation:{}",TotalCirculation);
//                                 return -47;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.GetTotalAwardAmount(MiningBalance))
//                             {
//                                 ERRORLOG("GetTotalAwardAmount MiningBalance:{}",MiningBalance);
//                                 return -48;
//                             }
//                             TotalCirculation += claim_amount;
//                             MiningBalance-=claim_amount;
//                             if(MiningBalance - claim_amount < 0)
//                             {
//                                 ERRORLOG("MiningBalance - claim_amount < 0 {},{}",MiningBalance,claim_amount);
//                                 return -49;
//                             }
//                             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//                             if(DBStatus::DB_SUCCESS != db_writer.SetBonusUtxoByPeriod(Period,tx.hash()))
//                             {
//                                 ERRORLOG("SetBonusUtxoByPeriod error! Period:{},hash:{}",Period,tx.hash());
//                                 return -50;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetTotalAwardAmount(MiningBalance))
//                             {
//                                 ERRORLOG("SetTotalAwardAmount error! MiningBalance:{}",MiningBalance);
//                                 return -51;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetTotalCirculation(TotalCirculation))
//                             {
//                                 ERRORLOG("SetTotalCirculation TotalCirculation:{}",TotalCirculation);
//                                 return -52;
//                             }
//                         } 
//                     }
//                     else if(tx_type == global::ca::TxType::kTxTypeDeployContract)
//                     {
//                         for (auto &vin : tx.utxo().vin())
//                         {
                            

//                             const std::string deployer_address = GetBase58Addr(vin.vinsign().pub());
//                             const std::string deploy_hash = tx.hash();
//                             std::string ContractAddress = evm_utils::generateEvmAddr(deployer_address + deploy_hash);//Evmone::GenContractAddress(deployer_address, deploy_hash);
//                             std::cout << "ContractAddress: " << ContractAddress << std::endl;
//                             nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                             std::string code = tx_info["Output"].get<std::string>();
//                             if (DBStatus::DB_SUCCESS != db_writer.SetContractDeployUtxoByContractAddr(ContractAddress, deploy_hash))
//                             {
//                                 return -53;
//                             }
//                             for(auto &it : tx_info["PrevHash"].items())
//                             {
//                                 if(it.key() == "") continue;
//                                 std::string currentPreHash;
//                                 if (DBStatus::DB_SUCCESS != db_writer.GetLatestUtxoByContractAddr(it.key(), currentPreHash))
//                                 {
//                                     return -54;
//                                 }

//                                 if(currentPreHash != it.value())
//                                 {
//                                     return -55;
//                                 }

//                                 if (DBStatus::DB_SUCCESS != db_writer.SetLatestUtxoByContractAddr(it.key(), deploy_hash))
//                                 {
//                                     return -56;
//                                 }
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetLatestUtxoByContractAddr(ContractAddress, deploy_hash))
//                             {
//                                 return -57;
//                             }

//                             if (DBStatus::DB_SUCCESS != db_writer.SetDeployUtxoByDeployerAddr(deployer_address, deploy_hash))
//                             {
//                                 return -58;
//                             }
//                             std::vector<std::string> vecDeployerAddrs;
//                             auto ret = db_writer.GetAllDeployerAddr(vecDeployerAddrs);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 return -59;
//                             }
//                             auto iter = std::find(vecDeployerAddrs.begin(), vecDeployerAddrs.end(), deployer_address);
//                             if(iter == vecDeployerAddrs.end())
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetDeployerAddr(deployer_address))
//                                 {
//                                     return -60;
//                                 }
//                             }
//                             nlohmann::json storage = tx_info["Storage"];
//                             if(!storage.is_null())
//                             {
//                                 // nlohmann::json storage = nlohmann::json::parse(Storage_json);
//                                 for (auto it = storage.begin(); it != storage.end(); ++it)
//                                 {
//                                     std::string strKey = it.key();
//                                     if (strKey.substr(strKey.length() - 8 , strKey.length()) == "rootHash" || strKey.empty())
//                                     {
//                                         continue;
//                                     }
//                                     if(strKey[0] == '_')
//                                     {
//                                         strKey = ContractAddress + strKey;
//                                     }
//                                     if (db_writer.SetMptValueByMptKey(strKey, it.value()) != DBStatus::DB_SUCCESS)
//                                     {
//                                         return -61;
//                                     }
//                                 }
//                             }

//                             break;
//                         }
//                     }
//                     else if(tx_type == global::ca::TxType::kTxTypeCallContract)
//                     {
                        

//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         for(auto &it : tx_info["PrevHash"].items())
//                         {
//                             std::string currentPreHash;
//                             if (DBStatus::DB_SUCCESS != db_writer.GetLatestUtxoByContractAddr(it.key(), currentPreHash))
//                             {
//                                 return -62;
//                             }

//                             if(currentPreHash != it.value())
//                             {
//                                 return -63;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetLatestUtxoByContractAddr(it.key(), tx.hash()))
//                             {
//                                 return -64;
//                             }
//                         }
//                         nlohmann::json storage = tx_info["Storage"];
//                         if(!storage.is_null())
//                         {
//                             // nlohmann::json storage = nlohmann::json::parse(Storage_json);
//                             for (auto it = storage.begin(); it != storage.end(); ++it)
//                             {
//                                 std::string strKey = it.key();
//                                 if (strKey.substr(strKey.length() - 8 , strKey.length()) == "rootHash" || strKey.empty())
//                                 {
//                                     continue;
//                                 }
//                                 if (db_writer.SetMptValueByMptKey(strKey, it.value()) != DBStatus::DB_SUCCESS)
//                                 {
//                                     return -65;
//                                 }
//                             }
//                         }

//                     }
//                 }
//                 catch (...)
//                 {
//                     return -66;
//                 }
//             }

//             bool isMultiSign = IsMultiSign(tx);
//             // All transaction updates delete the utxo used, and subtract the balance of utxo used by the transaction address
//             std::vector<std::string> vin_hashs;
//             for (auto &vin : tx.utxo().vin())
//             {
//                 global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();
                
//                 Base58Ver ver = isMultiSign ? Base58Ver::kBase58Ver_MultiSign : Base58Ver::kBase58Ver_Normal;
//                 std::string addr = GetBase58Addr(vin.vinsign().pub(), ver);
//                 all_addr.insert(addr);

//                 for (auto & prevout : vin.prevout())
//                 {
//                     std::string utxo_hash = prevout.hash();
//                     std::string utxo_n = utxo_hash + "_" + std::to_string(prevout.n());
//                     if (tx_type == global::ca::TxType::kTxTypeUnstake && 
//                         redeem_utxo_hash == utxo_hash && 
//                         0 == prevout.n())
//                     {
//                         continue;
//                     }
//                     else if (tx_type == global::ca::TxType::kTxTypeDisinvest && 
//                             divest_utxo_hash == utxo_hash && 
//                             0 == prevout.n())
//                     {
//                         continue;
//                     }                

//                     if (vin_hashs.cend() != std::find(vin_hashs.cbegin(), vin_hashs.cend(), utxo_n))
//                     {
//                         continue;
//                     }
//                     vin_hashs.push_back(utxo_n);

//                     if (DBStatus::DB_SUCCESS != db_writer.RemoveUtxoHashsByAddress(addr, utxo_hash))
//                     {
//                         ERRORLOG("RemoveUtxoHashsByAddress addr:{},utxo_hash:{}",addr,utxo_hash);
//                         return -67;
//                     }


//                     std::string utxo_tx_raw;
//                     if (DBStatus::DB_SUCCESS != db_writer.GetTransactionByHash(utxo_hash, utxo_tx_raw))
//                     {
//                         MagicSingleton<BlockHelper>::GetInstance()->PushMissUTXO(utxo_hash);
//                         ERRORLOG("GetTransactionByHash error! utxo_hash:{},utxo_tx_raw:{}",utxo_hash,utxo_tx_raw);
//                         return -69;
//                     }

//                     CTransaction utxo_tx;
//                     if (!utxo_tx.ParseFromString(utxo_tx_raw))
//                     {
//                         ERRORLOG("ParseFromString utxo_tx_raw:{}",utxo_tx_raw);
//                         return -70;
//                     }

//                     uint64_t amount = 0;
//                     for (int j = 0; j < utxo_tx.utxo().vout_size(); j++)
//                     {
//                         const CTxOutput & txout = utxo_tx.utxo().vout(j);
//                         if (txout.addr() == addr)
//                         {
//                             amount += txout.value();
//                         }
//                     }
//                     int64_t balance = 0;
//                     ret = db_writer.GetBalanceByAddress(addr, balance);
//                     if (DBStatus::DB_SUCCESS != ret)
//                     {
//                         if (DBStatus::DB_NOT_FOUND != ret)
//                         {
//                             ERRORLOG("GetBalanceByAddress error! ret:{}",ret);
//                             return -71;
//                         }
//                         else
//                         {
//                             balance = 0;
//                         }
//                     }
//                     balance -= amount;
//                     if (balance < 0)
//                     {
//                         ERRORLOG("SaveBlock vin height:{} hash:{} addr:{} balance:{}", block.height(), block.hash(), addr, balance);
//                         return -72;
//                     }
//                     if (0 == balance)
//                     {
//                         if (DBStatus::DB_SUCCESS != db_writer.DeleteBalanceByAddress(addr))
//                         {
//                             ERRORLOG("DeleteBalanceByAddress error! addr:{}",addr);
//                             return -73;
//                         }
//                     }
//                     else
//                     {
//                         if (DBStatus::DB_SUCCESS != db_writer.SetBalanceByAddress(addr, balance))
//                         {
//                             ERRORLOG("SetBalanceByAddress error! addr:{},balance:{}",addr,balance);
//                             return -74;
//                         }
//                     }
//                 }

                
//             }
//         }
//         else if (kTransactionType_Gas == transaction_type)
//         {
//             uint64_t SignNumber = 0;
//             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//             std::vector<std::string> SignAddrs;
//             auto ret = db_writer.GetSignAddrByPeriod(Period, SignAddrs);
//             if(DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//             {
//                 ERRORLOG("GetSignAddrByPeriod error! ret:{}",ret);
//                 return -75;
//             }
//             for (auto &vout : tx.utxo().vout())
//             {
//                 if (vout.value() <= 0)
//                 {
//                     continue;
//                 }
//                 auto found = std::find(SignAddrs.begin(), SignAddrs.end(), vout.addr());
//                 if(found == SignAddrs.end())
//                 {
//                     if(DBStatus::DB_SUCCESS != db_writer.SetSignAddrByPeriod(Period, vout.addr()))
//                     {
//                         ERRORLOG("SetSignAddrByPeriod error! Period:{},vout.addr{}",Period,vout.addr());
//                         return -76;
//                     }
//                 }
//                 auto ret = db_writer.GetSignNumberByPeriod(Period, vout.addr(), SignNumber);
//                 if(DBStatus::DB_SUCCESS != ret)
//                 {
//                     if(DBStatus::DB_NOT_FOUND == ret)
//                     {
//                         SignNumber = 0;
//                     }
//                     else
//                     {
//                         ERRORLOG("GetSignNumberByPeriod error! ret:{}",ret);
//                         return -77;
//                     }
//                 }
//                 SignNumber += 1;
//                 if(DBStatus::DB_SUCCESS != db_writer.SetSignNumberByPeriod(Period, vout.addr(), SignNumber))
//                 {
//                     ERRORLOG("SetSignNumberByPeriod error! Period:{},vout.addr:{},SignNumber:{}",Period,vout.addr(),SignNumber);
//                     return -78;
//                 }
//             }
//         }    
//         // Add new utxo to all transactions and increase the balance of utxo used by the transaction address
//         std::multimap<std::string,std::string> utxo_value;
//         for (auto &vout : tx.utxo().vout())
//         {
//             global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();

//             if (vout.addr() != global::ca::kVirtualStakeAddr && vout.addr() != global::ca::kVirtualInvestAddr)
//             {
//                 all_addr.insert(vout.addr());
//             }
//             ret = db_writer.SetUtxoHashsByAddress(vout.addr(), tx.hash());
//             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//             {
//                 ERRORLOG("SetUtxoHashsByAddress error! ret:{}",ret);
//                 return -79;
//             }
            
//             if(tx_type == global::ca::TxType::kTxTypeUnstake || tx_type == global::ca::TxType::kTxTypeDisinvest)
//             {
//                 utxo_value.insert(std::make_pair(tx.hash() + "_" + vout.addr(),std::to_string(vout.value())));
//             }
//             else
//             {
//                 ret = db_writer.SetUtxoValueByUtxoHashs(tx.hash(), vout.addr(), std::to_string(vout.value()));
//                 if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                 {
//                     ERRORLOG("SetUtxoValueByUtxoHashs error! ret:{}",ret);
//                     return -80;
//                 }
//             }



//             int64_t balance = 0;
//             ret = db_writer.GetBalanceByAddress(vout.addr(), balance);
//             if (DBStatus::DB_SUCCESS != ret)
//             {
//                 if (DBStatus::DB_NOT_FOUND != ret)
//                 {
//                     ERRORLOG("DBStatus::DB_SUCCESS != ret");
//                     return -81;
//                 }
//                 else
//                 {
//                     balance = 0;
//                 }
//             }
//             balance += vout.value();
//             if (balance < 0)
//             {
//                 ERRORLOG("SaveBlock vout height:{} hash:{} addr:{} balance:{}", block.height(), block.hash(), vout.addr(), balance);
//                 return -82;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.SetBalanceByAddress(vout.addr(), balance))
//             {
//                 ERRORLOG("SetBalanceByAddress error! vout.addr{},balance:{}",vout.addr(),balance);
//                 return -83;
//             }
//         }

//         std::vector<std::string> utxo_values;
//         if(!utxo_value.empty() || utxo_value.size() != 2)
//         {
//             std::string utxo_hash_balance;
//             for(auto & item : utxo_value)
//             {
//                 auto cnt = utxo_value.count(item.first);
//                 auto iter = utxo_value.find(item.first);

//                 utxo_hash_balance = iter->second + "_" + (iter++)->second;

//                 utxo_values.clear();
//                 StringUtil::SplitString(iter->first, "_", utxo_values);

//                 ret = db_writer.SetUtxoValueByUtxoHashs(utxo_values[0], utxo_values[1], utxo_hash_balance);
//                 if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                 {
//                     ERRORLOG("SetUtxoValueByUtxoHashs error! ret:{}",ret);
//                     return -80;
//                 }
//             }
        
//         }

//         // Add transaction body data corresponding to transaction hash
//         if (DBStatus::DB_SUCCESS != db_writer.SetTransactionByHash(tx.hash(), tx.SerializeAsString()))
//         {
//             ERRORLOG("SetTransactionByHash error! tx.hash:{},tx:{}",tx.SerializeAsString());
//             return -84;
//         }

//         // Add block hash corresponding to transaction hash
//         if (DBStatus::DB_SUCCESS != db_writer.SetBlockHashByTransactionHash(tx.hash(), block.hash()))
//         {
//             ERRORLOG("SetBlockHashByTransactionHash error! tx.hash:{},block.hash:{}",tx.hash(),block.hash());
//             return -85;
//         } 
//     }

//     int result = CalcHeightsSumHash(block.height(), saveType, obtainMean, db_writer);
//     if(result != 0)
//     {
//         return result - 10000;
//     }
    
//     return 0;
// }

// int ca_algorithm::DeleteBlock(DBReadWriter &db_writer, const std::string &block_hash)
// {
//     CBlock block;
//     std::string block_raw;
//     auto ret = db_writer.GetBlockByBlockHash(block_hash, block_raw); //一个块hash

//     if (DBStatus::DB_NOT_FOUND == ret)
//     {
//         return 0;
//     }
//     else if (DBStatus::DB_SUCCESS != ret)
//     {
//         ERRORLOG("GetBlockByBlockHash block_hash:{},block_raw:{}",block_hash,block_raw);
//         return -1;
//     }
//     if (!block.ParseFromString(block_raw))
//     {
//         ERRORLOG("ParseFromString error!");
//         return -2;
//     }

//     // Judge the height of the block and whether to update the node height
//     uint64_t top = 0;
//     if (DBStatus::DB_SUCCESS != db_writer.GetBlockTop(top))
//     {
//         ERRORLOG("GetBlockTop error! top:{}",top);
//         return -3;
//     }
//     if (block.height() == top)
//     {
//         std::vector<std::string> tmp_block_hashs;
//         if (DBStatus::DB_SUCCESS != db_writer.GetBlockHashsByBlockHeight(block.height(), tmp_block_hashs)) //拿到传来这一个块的高度的所有块hash
//         {
//             ERRORLOG("GetBlockHashsByBlockHeight  block.height:{}",block.height());
//             return -4;
//         }
//         if (1 == tmp_block_hashs.size())
//         {
//             if (DBStatus::DB_SUCCESS != db_writer.SetBlockTop(block.height() - 1))
//             {
//                 return -5;
//             }
//         }
// 	}

//     // Delete the height corresponding to the block hash
//     if (DBStatus::DB_SUCCESS != db_writer.DeleteBlockHeightByBlockHash(block.hash()))
//     {
//         ERRORLOG("DeleteBlockHeightByBlockHash block.hash:{}",block.hash());
//         return -6;
//     }

//     // Delete block hash corresponding to height
//     if (DBStatus::DB_SUCCESS != db_writer.RemoveBlockHashByBlockHeight(block.height(), block.hash()))
//     {
//         ERRORLOG("RemoveBlockHashByBlockHeight block.height:{},block.hash:{}",block.height(),block.hash());
//         return -7;
//     }

//     // Delete the block data corresponding to the block hash
//     if (DBStatus::DB_SUCCESS != db_writer.DeleteBlockByBlockHash(block.hash()))
//     {
//         ERRORLOG("DeleteBlockByBlockHash failed block.hash:{}",block.hash());
//         return -8;
//     }

//     std::set<std::string> block_addr; //目前全部移除  开始处理块中的交易
//     std::set<std::string> all_addr;
//     for (auto &tx : block.txs())
//     {
//         auto transaction_type = GetTransactionType(tx);
//         block_addr.insert(all_addr.cbegin(), all_addr.cend());
//         all_addr.clear();
//         if (kTransactionType_Tx == transaction_type)
//         {
//             bool is_redeem = false;
//             std::string redeem_utxo_hash;
//             bool is_divest = false;
//             std::string divest_utxo_hash;
//             if((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
//             {
//                 try
//                 {
//                     nlohmann::json data_json = nlohmann::json::parse(tx.data());
//                     global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();

//                     // The stake transaction updates the pledge address and the utxo of the pledge address
//                     if (global::ca::TxType::kTxTypeStake == tx_type)
//                     {
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             std::string addr = GetBase58Addr(vin.vinsign().pub());
//                             if (!CheckBase58Addr(addr))
//                             {
//                                 ERRORLOG("CheckBase58Addr error! addr:{}",addr);
//                                 return -9;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.RemoveStakeAddressUtxo(addr, tx.hash()))
//                             {
//                                 ERRORLOG("RemoveStakeAddressUtxo error! addr:{},tx.hash:{}",addr,tx.hash());
//                                 return -10;
//                             }
//                             std::vector<std::string> pledge_utxo_hashs;
//                             auto ret = db_writer.GetStakeAddressUtxo(addr, pledge_utxo_hashs);
//                             if (DBStatus::DB_NOT_FOUND == ret || pledge_utxo_hashs.empty())
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.RemoveStakeAddresses(addr))
//                                 {
//                                     ERRORLOG("RemoveStakeAddressUtxo error! ret:{}",ret);
//                                     return -11;
//                                 }
//                             }
//                             break;
//                         }
//                     }
//                     // The redeem transaction, update the pledge address and the utxo of the pledge address
//                     else if (global::ca::TxType::kTxTypeUnstake == tx_type)
//                     {
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         is_redeem = true;
//                         redeem_utxo_hash = tx_info["UnstakeUtxo"].get<std::string>();
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             std::string addr = GetBase58Addr(vin.vinsign().pub());
//                             if (!CheckBase58Addr(addr))
//                             {
//                                 ERRORLOG("CheckBase58Addr error! addr:{}",addr);
//                                 return -12;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetStakeAddressUtxo(addr, redeem_utxo_hash))
//                             {
//                                 ERRORLOG("SetStakeAddressUtxo addr:{},utxo_hash:{}",addr,redeem_utxo_hash);
//                                 return -13;
//                             }
//                             std::vector<std::string> pledge_addrs;
//                             auto ret = db_writer.GetStakeAddress(pledge_addrs);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 ERRORLOG("GetStakeAddress error! ret:{}",ret);
//                                 return -14;
//                             }
//                             if (pledge_addrs.cend() == std::find(pledge_addrs.cbegin(), pledge_addrs.cend(), addr))
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetStakeAddresses(addr))
//                                 {
//                                     ERRORLOG("SetStakeAddresses error! can't find addr from pledge_addrs addr:{}",addr);
//                                     return -15;
//                                 }
//                             }
//                             break;
//                         }
//                     }
//                     // The investment transaction updates the investment address and the utxo of the investment address
//                     else if (global::ca::TxType::kTxTypeInvest == tx_type)
//                     {
//                         uint64_t invest_amount = 0; 
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         std::string invest_node = tx_info["BonusAddr"].get<std::string>();
//                         tx_info["InvestAmount"].get_to(invest_amount);
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             std::string addr = GetBase58Addr(vin.vinsign().pub());

//                             if (DBStatus::DB_SUCCESS != db_writer.RemoveBonusAddrInvestAddrUtxoByBonusAddr(invest_node, addr, tx.hash()))
//                             {
//                                 ERRORLOG("RemoveBonusAddrInvestAddrUtxoByBonusAddr error invest_node:{},addr:{},tx.hash:{}",invest_node, addr, tx.hash());
//                                 return -16;
//                             }
//                             std::vector<string> utxos;
//                             if (db_writer.GetBonusAddrInvestUtxosByBonusAddr(invest_node, addr, utxos) == DBStatus::DB_NOT_FOUND || utxos.empty())
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.RemoveInvestAddrByBonusAddr(invest_node, addr))
//                                 {
//                                     ERRORLOG("RemoveInvestAddrByBonusAddr error! invest_node:{},addr:{}",invest_node,addr);
//                                     return -17;
//                                 }
//                                 std::vector<string> invest_addrs;
//                                 if (db_writer.GetInvestAddrsByBonusAddr(invest_node, invest_addrs) == DBStatus::DB_NOT_FOUND || invest_addrs.empty())
//                                 {
//                                     if (DBStatus::DB_SUCCESS != db_writer.RemoveBonusAddr(invest_node))
//                                     {
//                                         ERRORLOG("RemoveBonusAddr invest_node:{}",invest_node);
//                                         return -18;
//                                     }                                
//                                 }

//                                 if (DBStatus::DB_SUCCESS != db_writer.RemoveBonusAddrByInvestAddr(addr,invest_node))
//                                 {
//                                     ERRORLOG("RemoveBonusAddrByInvestAddr addr:{},invest_node:{}",addr,invest_node);
//                                     return -19;
//                                 }
//                             }
//                             break;
//                         }
//                         uint64_t Totalinvest = 0;
//                         std::vector<std::string> utxos;
//                         {
//                             std::lock_guard<std::mutex> lock(global::ca::kInvestMutex);
//                             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//                             auto ret = db_writer.GetInvestUtxoByPeriod(Period, utxos);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 return -20;
//                             }
//                             if (utxos.cend() != std::find(utxos.cbegin(), utxos.cend(), tx.hash()))
//                             {
//                                 if(DBStatus::DB_SUCCESS != db_writer.GetTotalInvestAmount(Totalinvest))
//                                 {
//                                     ERRORLOG("GetTotalInvestAmount Totalinves:{}",Totalinvest);
//                                     return -21;
//                                 }
//                                 Totalinvest-=invest_amount;
//                                 if(DBStatus::DB_SUCCESS != db_writer.SetTotalInvestAmount(Totalinvest))
//                                 {
//                                     ERRORLOG("SetTotalInvestAmount error! Totalinvest:{}",Totalinvest);
//                                     return -22;
//                                 }
//                                 uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//                                 if(DBStatus::DB_SUCCESS != db_writer.RemoveInvestUtxoByPeriod(Period, tx.hash()))
//                                 {
//                                     ERRORLOG("RemoveInvestUtxoByPeriod error! Period:{},tx.hash:{}",Period,tx.hash());
//                                     return -23;
//                                 }
//                             }
//                         }
                        

//                     }
//                     // The divestment transaction updates the investment address and the utxo of the investment address
//                     else if(global::ca::TxType::kTxTypeDisinvest == tx_type)
//                     {
//                         is_divest = true;
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         divest_utxo_hash = tx_info["DisinvestUtxo"].get<std::string>();
//                         std::string invested_node = tx_info["BonusAddr"].get<std::string>();
//                         std::vector<std::string> invest_nodes;
//                         ret = db_writer.GetBonusaddr(invest_nodes);
//                         if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                         {
//                             return -24;
//                         }
//                         if (invest_nodes.cend() == std::find(invest_nodes.cbegin(), invest_nodes.cend(), invested_node))
//                         {
//                             if (DBStatus::DB_SUCCESS != db_writer.SetBonusAddr(invested_node))
//                             {
//                                 return -25;
//                             }
//                         }

//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             std::string addr = GetBase58Addr(vin.vinsign().pub());
//                             if (!CheckBase58Addr(addr))
//                             {
//                                 return -26;
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.SetBonusAddrInvestAddrUtxoByBonusAddr(invested_node, addr, divest_utxo_hash))
//                             {
//                                 return -27;
//                             }
//                             std::vector<std::string> invest_addrs;
//                             auto ret = db_writer.GetInvestAddrsByBonusAddr(invested_node, invest_addrs);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 return -28;
//                             }
//                             if (invest_addrs.cend() == std::find(invest_addrs.cbegin(), invest_addrs.cend(), addr))
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetInvestAddrByBonusAddr(invested_node, addr))
//                                 {
//                                     return -29;
//                                 }
//                             }

//                             std::vector<std::string> invest_nodes;
//                             ret = db_writer.GetBonusAddrByInvestAddr(addr, invest_nodes);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 return -30;
//                             }
//                             if (invest_nodes.cend() == std::find(invest_nodes.cbegin(), invest_nodes.cend(), invested_node))
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetBonusAddrByInvestAddr(addr, invested_node))
//                                 {
//                                     return -31;
//                                 }
//                             }
//                             break;
//                         }

//                         uint64_t invest_amount = 0;
//                         std::string strTx;
//                         if (db_writer.GetTransactionByHash(divest_utxo_hash, strTx) != DBStatus::DB_SUCCESS)
//                         {
//                             return -32;
//                         }
//                         CTransaction InvestTx;
//                         if(!InvestTx.ParseFromString(strTx))
//                         {
//                             return -33;
//                         }

//                         for (int i = 0; i < InvestTx.utxo().vout_size(); i++)
//                         {
//                             if (InvestTx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
//                             {
//                                 invest_amount += InvestTx.utxo().vout(i).value();
//                                 break;
//                             }
//                         }
//                         uint64_t Totalinvest = 0;
//                         std::vector<std::string> utxos;
//                         {
//                             std::lock_guard<std::mutex> lock(global::ca::kInvestMutex);
//                             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(InvestTx.time());
//                             auto ret = db_writer.GetInvestUtxoByPeriod(Period, utxos);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 return -34;
//                             }
//                             if (utxos.cend() == std::find(utxos.cbegin(), utxos.cend(), divest_utxo_hash))
//                             {
//                                 if(DBStatus::DB_SUCCESS != db_writer.GetTotalInvestAmount(Totalinvest))
//                                 {
//                                     return -35;
//                                 }
//                                 Totalinvest+=invest_amount;
//                                 if(DBStatus::DB_SUCCESS != db_writer.SetTotalInvestAmount(Totalinvest))
//                                 {
//                                     return -36;
//                                 }
//                                 uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(InvestTx.time());
//                                 if(DBStatus::DB_SUCCESS != db_writer.SetInvestUtxoByPeriod(Period, divest_utxo_hash))
//                                 {
//                                     return -37;
//                                 }
//                             }
//                         }
//                     }
//                     else if(global::ca::TxType::kTxTypeBonus == tx_type)
//                     {
//                         uint64_t TotalCirculation = 0;
//                         uint64_t claim_amount=0;
//                         uint64_t MiningBalance=0;
//                         int i = 0;
//                         std::vector<std::string> utxos;
                        
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         tx_info["BonusAmount"].get_to(claim_amount);

//                         {
//                             std::lock_guard<std::mutex> lock(global::ca::kBonusMutex);
//                             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//                             auto ret = db_writer.GetBonusUtxoByPeriod(Period, utxos);
//                             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//                             {
//                                 return -38;
//                             }
//                             if (utxos.cend() != std::find(utxos.cbegin(), utxos.cend(), tx.hash()))
//                             {
//                                 if(DBStatus::DB_SUCCESS != db_writer.GetM2(TotalCirculation))
//                                 {
//                                     return -39;
//                                 }
//                                 TotalCirculation-=claim_amount;
//                                 if(DBStatus::DB_SUCCESS != db_writer.SetTotalCirculation(TotalCirculation))
//                                 {
//                                     return -40;
//                                 }
//                                 uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());
//                                 if(DBStatus::DB_SUCCESS != db_writer.RemoveBonusUtxoByPeriod(Period, tx.hash()))
//                                 {
//                                     return -41;
//                                 }
//                                 if (DBStatus::DB_SUCCESS != db_writer.GetTotalAwardAmount(MiningBalance))
//                                 {
//                                     return -42;
//                                 }
//                                 MiningBalance += claim_amount;
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetTotalAwardAmount(MiningBalance))
//                                 {
//                                     return -43;
//                                 }
//                             }
//                         }
                        
//                     }
//                     else if(global::ca::TxType::kTxTypeDeclaration == tx_type)
//                     {

//                         for(auto &vout : tx.utxo().vout())
//                         {
//                             if(!CheckBase58Addr(vout.addr(),Base58Ver::kBase58Ver_MultiSign))
//                             {
//                                 return -44;
//                             }
//                             if(DBStatus::DB_SUCCESS != db_writer.RemoveMutliSignAddressUtxo(vout.addr(),tx.hash()))
//                             {
//                                 return -45;
//                             }

//                             std::vector<std::string> utxos;
//                             auto ret =  db_writer.GetMutliSignAddressUtxo(vout.addr(),utxos);
//                             if(DBStatus::DB_NOT_FOUND == ret || utxos.empty())
//                             {
//                                 if(DBStatus::DB_SUCCESS != db_writer.RemoveMutliSignAddresses(vout.addr()))
//                                 {
//                                     return -46;
//                                 }
//                             }
//                             break;
//                         }

//                     }
//                     else if(global::ca::TxType::kTxTypeDeployContract == tx_type)
//                     {
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             const std::string deployer_address = GetBase58Addr(vin.vinsign().pub());
//                             const std::string deploy_hash = tx.hash();
//                             std::string ContractAddress = evm_utils::generateEvmAddr(deployer_address + deploy_hash);//Evmone::GenContractAddress(deployer_address, deploy_hash);

//                             nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                             std::string code = tx_info["Output"].get<std::string>();

//                             // if (DBStatus::DB_SUCCESS != db_writer.RemoveContractCodeByContractAddr(ContractAddress))
//                             if (DBStatus::DB_SUCCESS != db_writer.RemoveContractDeployUtxoByContractAddr(ContractAddress))
//                             {
//                                 return -47;
//                             }
//                             for(auto &it : tx_info["PrevHash"].items())
//                             {
//                                 if(it.key() == "") continue;
//                                 std::cout<<"delete_addr: "<< it.key() <<std::endl;
//                                 std::string strPrevTxHash = it.value().get<std::string>();
//                                 std::cout<<"strPrevTxHash: "<< strPrevTxHash <<std::endl;
//                                 if (DBStatus::DB_SUCCESS != db_writer.SetLatestUtxoByContractAddr(it.key(), strPrevTxHash))
//                                 {
//                                     return -48;
//                                 }
//                             }
//                             if (DBStatus::DB_SUCCESS != db_writer.RemoveLatestUtxoByContractAddr(ContractAddress))
//                             {
//                                 return -49;
//                             }

//                             if (DBStatus::DB_SUCCESS != db_writer.RemoveDeployUtxoByDeployerAddr(deployer_address, deploy_hash))
//                             {
//                                 return -50;
//                             }
//                             std::vector<std::string> vecDeployUtxos;
//                             auto ret = db_writer.GetDeployUtxoByDeployerAddr(deployer_address, vecDeployUtxos);
//                             if (DBStatus::DB_NOT_FOUND == ret || vecDeployUtxos.empty())
//                             {
//                                 if (DBStatus::DB_SUCCESS != db_writer.RemoveDeployerAddr(deployer_address))
//                                 {
//                                     return -51;
//                                 }
//                             }
//                             nlohmann::json storage = tx_info["Storage"];
//                             if(!storage.is_null())
//                             {
//                                 // nlohmann::json storage = nlohmann::json::parse(Storage_json);
//                                 for (auto it = storage.begin(); it != storage.end(); ++it)
//                                 {
//                                     std::string strKey = it.key();
//                                     if (strKey.substr(strKey.length() - 8 , strKey.length()) == "rootHash" || strKey.empty())
//                                     {
//                                         continue;
//                                     }
//                                     if(strKey[0] == '_')
//                                     {
//                                         strKey = ContractAddress + strKey;
//                                         std::cout<<"strKey: "<< strKey << std::endl;
//                                     }
//                                     if (db_writer.RemoveMptValueByMptKey(strKey) != DBStatus::DB_SUCCESS)
//                                     {
//                                         return -52;
//                                     }
//                                 }                            
//                             }

//                             break;
//                         }
//                     }
//                     else if(global::ca::TxType::kTxTypeCallContract == tx_type)
//                     {
//                         nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
//                         // std::string deployer_addr = tx_info["DeployerAddr"].get<std::string>();   
//                         // std::string deploy_hash = tx_info["DeployHash"].get<std::string>();
//                         // std::string prev_tx_hash = tx_info["PrevHash"].get<std::string>(); 
//                         // std::string ContractAddress = Evmone::GenContractAddress(deployer_addr, deploy_hash);
//                         // if (DBStatus::DB_SUCCESS != db_writer.SetLatestUtxoByContractAddr(ContractAddress, prev_tx_hash))
//                         // {
//                         //     return -50;
//                         // }
//                         for(auto &it : tx_info["PrevHash"].items())
//                         {
//                             if(it.key() == "") continue;
//                             std::cout<<"delete_addr: "<< it.key() <<std::endl;
//                             std::string strPrevTxHash = it.value().get<std::string>();
//                             std::cout<<"strPrevTxHash: "<< strPrevTxHash <<std::endl;
//                             if (DBStatus::DB_SUCCESS != db_writer.SetLatestUtxoByContractAddr(it.key(), strPrevTxHash))
//                             {
//                                 return -53;
//                             }
//                         }

//                         nlohmann::json storage = tx_info["Storage"];
//                         if(!storage.is_null())
//                         {
//                             // nlohmann::json storage = nlohmann::json::parse(Storage_json);
//                             for (auto it = storage.begin(); it != storage.end(); ++it)
//                             {
//                                 std::string strKey = it.key();
//                                 if (strKey.substr(strKey.length() - 8 , strKey.length()) == "rootHash" || strKey.empty())
//                                 {
//                                     continue;
//                                 }
//                                 if (db_writer.RemoveMptValueByMptKey(strKey) != DBStatus::DB_SUCCESS)
//                                 {
//                                     return -54;
//                                 }
//                             }                        
//                         }

//                     }
//                 }
//                 catch (...)
//                 {
//                     return -55;
//                 }
//             }

//             std::string addr;
//             std::string utxo_hash;
//             CTransaction utxo_tx;
//             std::string utxo_tx_raw;
//             std::vector<std::string> vin_hashs;
//             std::string utxo_n;
//             for (auto &vin : tx.utxo().vin())
//             {
//                 addr = GetBase58Addr(vin.vinsign().pub());
//                 all_addr.insert(addr); //交易vin地址放到set里

//                 for (auto & prevout : vin.prevout())
//                 {
//                     utxo_hash = prevout.hash(); 
//                     utxo_n = utxo_hash + "_" + std::to_string(prevout.n());
//                     if (is_redeem && redeem_utxo_hash == utxo_hash && 0 == prevout.n())
//                     {
//                         continue;
//                     }
//                     else if (is_divest && divest_utxo_hash == utxo_hash && 0 == prevout.n())
//                     {
//                         continue;
//                     }  

//                     if (vin_hashs.cend() != std::find(vin_hashs.cbegin(), vin_hashs.cend(), utxo_n)) //
//                     {
//                         continue;
//                     }
//                     vin_hashs.push_back(utxo_n); //
//                     // All transactions update the utxo used and the balance of utxo used by the transaction address
//                     if (DBStatus::DB_SUCCESS != db_writer.SetUtxoHashsByAddress(addr, utxo_hash))
//                     {
//                         return -56;
//                     }

//                     if (DBStatus::DB_SUCCESS != db_writer.GetTransactionByHash(utxo_hash, utxo_tx_raw))
//                     {
//                         return -58;
//                     }
//                     if (!utxo_tx.ParseFromString(utxo_tx_raw))
//                     {
//                         return -59;
//                     }

//                     uint64_t amount = 0;
//                     for (int j = 0; j < utxo_tx.utxo().vout_size(); j++)
//                     {
//                         const CTxOutput & txout = utxo_tx.utxo().vout(j);
//                         if (txout.addr() == addr)
//                         {
//                             amount += txout.value();
//                         }

                        
//                     }
//                     int64_t balance = 0;
//                     ret = db_writer.GetBalanceByAddress(addr, balance);
//                     if (DBStatus::DB_SUCCESS != ret)
//                     {
//                         if (DBStatus::DB_NOT_FOUND != ret)
//                         {
//                             return -60;
//                         }
//                         else
//                         {
//                             balance = 0;
//                         }
//                     }
//                     balance += amount;
//                     if (balance < 0)
//                     {
//                         ERRORLOG("DeleteBlock vin height:{} hash:{} addr:{} balance:{}", block.height(), block.hash(), addr, balance);
//                         return -61;
//                     }
//                     if (DBStatus::DB_SUCCESS != db_writer.SetBalanceByAddress(addr, balance))
//                     {
//                         return -62;
//                     }
//                 }
//             }
//         }
//         else if (kTransactionType_Gas == transaction_type)
//         {
//             uint64_t SignNumber = 0;
//             uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(tx.time());

//             for (auto &vout : tx.utxo().vout())
//             {
//                 if (vout.value() <= 0)
//                 {
//                     continue;
//                 }

//                 if(DBStatus::DB_SUCCESS != db_writer.GetSignNumberByPeriod(Period, vout.addr(), SignNumber))
//                 {
//                     ERRORLOG("GetSignNumberByPeriod Period:{},vout.addr:{},SignNumber:{}",Period, vout.addr(), SignNumber);
//                     return -63;
//                 }

//                 if(SignNumber > 1)
//                 {
//                     --SignNumber;
//                     if(DBStatus::DB_SUCCESS != db_writer.SetSignNumberByPeriod(Period, vout.addr(), SignNumber))
//                     {
//                         ERRORLOG("SetSignNumberByPeriod error! Period:{},vout.addr:{},SignNumber:{}",Period, vout.addr(), SignNumber);
//                         return -64;
//                     }
//                 }
//                 else
//                 {
//                     if(DBStatus::DB_SUCCESS != db_writer.RemoveSignAddrberByPeriod(Period, vout.addr()))
//                     {
//                         ERRORLOG("RemoveSignAddrberByPeriod error! Period:{},vout.addr:{}",Period,vout.addr());
//                         return -65;
//                     }
//                     if(DBStatus::DB_SUCCESS != db_writer.RemoveSignNumberByPeriod(Period, vout.addr()))
//                     {
//                         ERRORLOG("RemoveSignNumberByPeriod error! Period:{},vout.addr:{}",Period,vout.addr());
//                         return -66;
//                     }
//                 }

//             }
//         }
//         // All transactions delete utxo and the balance of utxo used by the transaction address
//         for (auto &vout : tx.utxo().vout())
//         {
//             all_addr.insert(vout.addr());
//             ret = db_writer.RemoveUtxoHashsByAddress(vout.addr(), tx.hash());
//             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//             {
//                 return -67;
//             }

//             ret = db_writer.RemoveUtxoValueByUtxoHashs(tx.hash(), vout.addr(), std::to_string(vout.value()));
//             if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
//             {
//                 return -68;
//             }

//             int64_t balance = 0;
//             ret = db_writer.GetBalanceByAddress(vout.addr(), balance);
//             if (DBStatus::DB_SUCCESS != ret)
//             {
//                 if (DBStatus::DB_NOT_FOUND != ret)
//                 {
//                     ERRORLOG("GetBalanceByAddress error! ret:{}",ret);
//                     return -69;
//                 }
//                 else
//                 {
//                     balance = 0;
//                 }
//             }
//             balance -= vout.value();
//             if (balance < 0)
//             {
//                 ERRORLOG("DeleteBlock vout height:{} hash:{} addr:{} balance:{}", block.height(), block.hash(), vout.addr(), balance);
//                 return -70;
//             }
//             if (0 == balance)
//             {
//                 if (DBStatus::DB_SUCCESS != db_writer.DeleteBalanceByAddress(vout.addr()))
//                 {
//                     ERRORLOG("DeleteBalanceByAddress vout.addr:{}",vout.addr());
//                     return -71;
//                 }
//             }
//             else
//             {
//                 if (DBStatus::DB_SUCCESS != db_writer.SetBalanceByAddress(vout.addr(), balance))
//                 {
//                     ERRORLOG("SetBalanceByAddress vout.addr:{},balance:{}",vout.addr(),balance);
//                     return -72;
//                 }
//             }
//         }
        
//         // Delete transaction body data corresponding to transaction hash
//         if (DBStatus::DB_SUCCESS != db_writer.DeleteTransactionByHash(tx.hash()))
//         {
//             ERRORLOG("DeleteTransactionByHash hash:{}",tx.hash());
//             return -73;
//         }

//         // Delete the block hash corresponding to the transaction hash
//         if (DBStatus::DB_SUCCESS != db_writer.DeleteBlockHashByTransactionHash(tx.hash()))
//         {
//             ERRORLOG("DeleteBlockHashByTransactionHash error! hash:{}",tx.hash());
//             return -74;
//         }
//         std::string sum_hash;
//         auto block_height = block.height();
//         auto sum_hash_height = GetSumHashCeilingHeight(block_height);
//         if (DBStatus::DB_SUCCESS == db_writer.GetSumHashByHeight(sum_hash_height, sum_hash))
//         {
//             if(DBStatus::DB_SUCCESS == db_writer.GetSumHashByHeight(sum_hash_height + global::ca::sum_hash_range, sum_hash))
//             {
//                 auto start_height = GetSumHashFloorHeight(block_height) + 1;
//                 auto end_height = GetSumHashCeilingHeight(block_height) + 1;
//                 if(!CalculateHeightSumHash(start_height, end_height, db_writer, sum_hash))
//                 {
//                     return -73;
//                 }
//                 if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(sum_hash_height, sum_hash))
//                 {
//                     return -74;
//                 }
//                 INFOLOG("set height sum hash at height {} hash: {}", sum_hash_height, sum_hash);                
//             } 
//             else
//             {
//                 if (DBStatus::DB_SUCCESS != db_writer.RemoveSumHashByHeight(sum_hash_height))
//                 {
//                     return -75;
//                 }
//                 INFOLOG("delete height sum hash at height {}", sum_hash_height);           
//             }

//         }
//     }
//     return 0;
// }

// static int RollBackBlock(const std::multimap<uint64_t, std::string> &hashs)
// {
//     int i = 0;
//     DBReadWriter db_writer;
//     for (auto it = hashs.rbegin(); hashs.rend() != it; ++it)
//     {
//         ++i;
//         std::string delete_hash = it->second;
//         TRACELOG("begin delete block {}", delete_hash);
//         auto ret = ca_algorithm::DeleteBlock(db_writer, delete_hash);
//         if (0 != ret)
//         {
//             ERRORLOG("faill to delete block {}, ret {}", delete_hash, ret);
//             return ret - 100;
//         }
//         else
//         {
//             DEBUGLOG("RollBackBlock delete block: {}",delete_hash);
//             if(MagicSingleton<CBlockCache>::GetInstance()->Remove(it->first, delete_hash) != 0)
//             {
//                 ERRORLOG("RollBackBlock delete block fail!  block hash :{}", delete_hash);
//                 return -1;
//             }
//         }
//         TRACELOG("successfully delete block {}", delete_hash);
//         if(10 == i)
//         {
//             i = 0;
//             if (DBStatus::DB_SUCCESS != db_writer.TransactionCommit())
//             {
//                 return -2;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.ReTransactionInit())
//             {
//                 return -3;
//             }
//         }
//     }
//     if(i > 0)
//     {
//         if (DBStatus::DB_SUCCESS != db_writer.TransactionCommit())
//         {
//             return -4;
//         }
//     }
//     return 0;
// }
// int ca_algorithm::RollBackToHeight(uint64_t height)
// {
//     DBReadWriter db_writer;
//     uint64_t node_height = 0;
//     if (DBStatus::DB_SUCCESS != db_writer.GetBlockTop(node_height))
//     {
//         return -1;
//     }
//     std::multimap<uint64_t, std::string> hashs;
//     std::vector<CBlock>backupblocks;
//     std::vector<std::string> block_hashs;
//     for (uint32_t i = node_height; i > height; --i)
//     {
//         block_hashs.clear();
//         int res = db_writer.GetBlockHashsByBlockHeight(i, block_hashs);
//         if (DBStatus::DB_SUCCESS != res)
//         {
//             DEBUGLOG("query block height {} fail, ret: {}", i, res);
//             return -2;
//         }
//         for (auto &hash : block_hashs)
//         {
//             hashs.insert(std::make_pair(i, hash)); //一个高度内的所有块hash key可以有多个
//         }
//     }

//     for(auto it = hashs.begin(); hashs.end() != it; ++it)
//     {
//         std::string blockStr;
//         if (DBStatus::DB_SUCCESS != db_writer.GetBlockByBlockHash(it->second,blockStr))
//         {
//             return -3;
//         }
//         CBlock block;
//         if (!block.ParseFromString(blockStr))
//         {
//             return -4;
//         }
//         backupblocks.push_back(block);
//     }

//     auto ret = RollBackBlock(hashs);
//     if (0 != ret)
//     {
//         return ret - 1000;
//     }
//     if(ret == 0)
//     {
//         for (auto &block:backupblocks)
//         {
//             if (MagicSingleton<CBlockHttpCallback>::GetInstance()->IsRunning())
//             {
//                 MagicSingleton<CBlockHttpCallback>::GetInstance()->RollbackBlock(block);
//             }
//         }
//     }
    
//     return 0;
// }

// int ca_algorithm::RollBackByHash(const std::string &block_hash)
// {
//     DBReadWriter db_writer;
//     std::multimap<uint64_t, std::string> hashs;
//     std::vector<CBlock> backupblocks;
//     uint64_t height = 0;
//     bool rollback_by_height = false;
//     {
//         std::set<std::string> rollback_block_hashs;
//         std::set<std::string> rollback_trans_hashs;
//         CBlock block;
//         {
//             std::string block_raw;
//             auto ret = db_writer.GetBlockByBlockHash(block_hash, block_raw);
//             if (DBStatus::DB_NOT_FOUND == ret)
//             {
//                 return 0;
//             }
//             else if (DBStatus::DB_SUCCESS != ret)
//             {
//                 return -1;
//             }
//             if (!block.ParseFromString(block_raw))
//             {
//                 return -2;
//             }
//             hashs.insert(std::make_pair(block.height(), block.hash()));
//             rollback_block_hashs.insert(block.hash());
//             for (auto &tx : block.txs())
//             {
//                 rollback_trans_hashs.insert(tx.hash());
//             }
//         }
//         uint64_t node_height = 0;
//         if (DBStatus::DB_SUCCESS != db_writer.GetBlockTop(node_height))
//         {
//             return -3;
//         }
//         std::vector<std::string> block_raws;
//         std::vector<std::string> block_hashs;
//         for (height = block.height(); height < node_height + 1; ++height)
//         {
//             block_hashs.clear();
//             block_raws.clear();
//             if (DBStatus::DB_SUCCESS != db_writer.GetBlockHashsByBlockHeight(height, block_hashs))
//             {
//                 return -4;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.GetBlocksByBlockHash(block_hashs, block_raws))
//             {
//                 return -5;
//             }
//             bool flag = false;
//             for (auto &block_raw : block_raws)
//             {
//                 block.Clear();
//                 if (!block.ParseFromString(block_raw))
//                 {
//                     return -6;
//                 }
//                 if (rollback_block_hashs.end() == std::find(rollback_block_hashs.cbegin(), rollback_block_hashs.cend(), block.prevhash()))
//                 {
//                     for (auto &tx : block.txs())
//                     {
//                         if (GetTransactionType(tx) != kTransactionType_Tx)
//                         {
//                             continue;
//                         }
//                         for (auto &vin : tx.utxo().vin())
//                         {
//                             for (auto & prevout : vin.prevout())
//                             {
//                                 auto utxo_hash = prevout.hash();
//                                 if (rollback_trans_hashs.end() != std::find(rollback_trans_hashs.cbegin(), rollback_trans_hashs.cend(), utxo_hash))
//                                 {
//                                     flag = true;
//                                     break;
//                                 }
//                             }
                            
//                         }
//                         if (flag)
//                         {
//                             break;
//                         }
//                     }
//                 }
//                 else
//                 {
//                     flag = true;
//                 }
//                 if (flag)
//                 {
//                     hashs.insert(std::make_pair(block.height(), block.hash()));
//                     rollback_block_hashs.insert(block.hash());
//                     for (auto &tx : block.txs())
//                     {
//                         rollback_trans_hashs.insert(tx.hash());
//                     }
//                 }
//             }
//             if (flag && block_hashs.size() <= 1)
//             {
//                 height = block.height();
//                 rollback_by_height = true;
//                 break;
//             }
//         }
//     }
//     if (rollback_by_height)
//     {
//         uint64_t node_height = 0;
//         if (DBStatus::DB_SUCCESS != db_writer.GetBlockTop(node_height))
//         {
//             return -7;
//         }
//         std::vector<std::string> block_hashs;

//         for (uint32_t i = node_height; i > height; --i)
//         {
//             block_hashs.clear();
//             if (DBStatus::DB_SUCCESS != db_writer.GetBlockHashsByBlockHeight(i, block_hashs))
//             {
//                 return -8;
//             }
//             for (auto &hash : block_hashs)
//             {
//                 hashs.insert(std::make_pair(i, hash));

//             }
//         }
//     }

//     for(auto it = hashs.begin(); hashs.end() != it; ++it)
//     {
//         std::string blockStr;
//         if (DBStatus::DB_SUCCESS != db_writer.GetBlockByBlockHash(it->second,blockStr))
//         {
//             return -9;
//         }
//         CBlock block;
//         if (!block.ParseFromString(blockStr))
//         {
//             return -10;
//         }
//         backupblocks.push_back(block);
//     }


//     auto ret = RollBackBlock(hashs);

//     if (0 != ret)
//     {
//         return ret - 1000;
//     }
//     if(ret == 0)
//     {
//         for (auto &block :backupblocks)
//         {
//             if (MagicSingleton<CBlockHttpCallback>::GetInstance()->IsRunning())
//             {
//                 MagicSingleton<CBlockHttpCallback>::GetInstance()->RollbackBlock(block);
//             }
//         }
//     }
//     return 0;
// }

// void ca_algorithm::PrintTx(const CTransaction &tx)
// {
//     using namespace std;
//     cout << "========================================================================================================================" << endl;
//     cout << "\ttx.version:" << tx.version() << endl;
//     cout << "\ttx.time:" << tx.time() << endl;

//     std::string hex;
//     for (auto &sign_pre_hash : tx.verifysign())
//     {
//         hex.clear();
//         Bytes2Hex(sign_pre_hash.sign(), hex, true);
//         hex.clear();
//         Bytes2Hex(sign_pre_hash.pub(), hex, true);
//         cout << "\t\tsign_pre_hash.addr:" << GetBase58Addr(sign_pre_hash.pub()) << endl;
//     }

//     for (auto &vin : tx.utxo().vin())
//     {
//         cout << "\t\tvin.sequence:" << vin.sequence() << endl;
//         for (auto & prevout : vin.prevout())
//         {
//             cout << "\t\t\tprevout.hash:" << prevout.hash() << endl;
//             cout << "\t\t\tprevout.n:" << prevout.n() << endl;
//         }
//         auto &scriptSig = vin.vinsign();
//         cout << "\t\t\tscriptSig.addr:" << GetBase58Addr(scriptSig.pub()) << endl;
//     }
//     for (auto &vout : tx.utxo().vout())
//     {
//         cout << "\t\tvout.scriptpubkey:" << vout.addr() << endl;
//         cout << "\t\tvout.value:" << vout.value() << endl;
//     }

//     cout << "\ttx.owner:";
//     for (auto & owner : tx.utxo().owner())
//     {
//         cout << owner << ", ";
//     }
//     cout << endl;
//     cout << "\ttx.n:" << tx.n() << endl;
//     cout << "\ttx.identity:" << tx.identity() << endl;
//     cout << "\ttx.hash:" << tx.hash() << endl;
//     cout << "\ttx.data:" << tx.data() << endl;
//     cout << "\ttx.info:" << tx.info() << endl;
// }

// void ca_algorithm::PrintBlock(const CBlock &block)
// {
//     using namespace std;
//     cout << "version:" << block.version() << endl;
//     cout << "hash:" << block.hash() << endl;
//     cout << "prevhash:" << block.prevhash() << endl;
//     cout << "height:" << block.height() << endl;
//     cout << "merkleroot:" << block.merkleroot() << endl;

//     for (auto &tx : block.txs())
//     {
//         PrintTx(tx);
//     }

//     cout << "data:" << block.data() << endl;
//     cout << "info:" << block.info() << endl;
//     cout << "time:" << block.time() << endl;
//     cout << "========================================================================================================================" << endl;
// }

// // // int ca_algorithm::CalcInflationRate()
// // // {
// // //     for(int i=1;i<=30;i++)
// // // 	{
// // // 		kInflationRate[1][i]=0.07;
// // // 	}
// // // 	for(int i=1;i<=20;i++)
// // // 	{
// // // 		for(int j=1;j<=90;j++)
// // // 		{
// // // 			if(i==1&&j<=30) continue;
// // // 			else if(i==1)
// // // 			{
// // // 				kInflationRate[i][j]=kInflationRate[i][j-1]*0.99;
// // // 			}
// // // 			else
// // // 			{
// // // 				if(kInflationRate[i-1][j]*0.90>=0.018)
// // // 				{
// // // 					kInflationRate[i][j]=kInflationRate[i-1][j]*0.90;
// // // 				}
// // // 				else 
// // // 				{
// // // 					kInflationRate[i][j]=0.018;
// // // 				}
				
// // // 			}
// // // 		}
// // // 	}   
// // // 	return 0;
// // // }

int ca_algorithm::GetInflationRate(const uint64_t &cur_time, const uint64_t &&StakeRate, double &InflationRate)
{
    //此表是由CalcInflationRate算法生成打印 目前函数已被注释 但是保留方便以后查验
    std::vector<std::vector<double>> RateArray = 
    {
        {0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.077,0.0759605,0.074935,0.0739234,0.0729254,0.071941,0.0709697,0.0700117,0.0690665,0.0681341,0.0672143,0.0663069,0.0654118,0.0645287,0.0636576,0.0627982,0.0619504,0.0611141,0.060289,0.0594751,0.0586722,0.0578801,0.0570988,0.0563279,0.0555675,0.0548173,0.0540773,0.0533473,0.0526271,0.0519166,0.0512157,0.0505243,0.0498422,0.0491694,0.0485056,0.0478508,0.0472048,0.0465675,0.0459389,0.0453187,0.0447069,0.0441033,0.0435079,0.0429206,0.0423412,0.0417695,0.0412057,0.0406494,0.0401006,0.0395593,0.0390252,0.0384984,0.0379786,0.0374659,0.0369601,0.0364612},
        {0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.06776,0.0668452,0.0659428,0.0650526,0.0641744,0.063308,0.0624534,0.0616103,0.0607785,0.059958,0.0591486,0.0583501,0.0575623,0.0567853,0.0560187,0.0552624,0.0545164,0.0537804,0.0530544,0.0523381,0.0516316,0.0509345,0.0502469,0.0495686,0.0488994,0.0482393,0.047588,0.0469456,0.0463118,0.0456866,0.0450698,0.0444614,0.0438612,0.043269,0.0426849,0.0421087,0.0415402,0.0409794,0.0404262,0.0398804,0.039342,0.0388109,0.038287,0.0377701,0.0372602,0.0367572,0.036261,0.0357715,0.0352885,0.0348121,0.0343422,0.0338786,0.0334212,0.03297,0.0325249,0.0320858},
        {0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0596288,0.0588238,0.0580297,0.0572463,0.0564735,0.0557111,0.054959,0.054217,0.0534851,0.052763,0.0520507,0.0513481,0.0506549,0.049971,0.0492964,0.0486309,0.0479744,0.0473267,0.0466878,0.0460575,0.0454358,0.0448224,0.0442173,0.0436203,0.0430315,0.0424505,0.0418775,0.0413121,0.0407544,0.0402042,0.0396615,0.039126,0.0385978,0.0380768,0.0375627,0.0370556,0.0365554,0.0360619,0.035575,0.0350948,0.034621,0.0341536,0.0336925,0.0332377,0.032789,0.0323463,0.0319097,0.0314789,0.0310539,0.0306347,0.0302211,0.0298131,0.0294107,0.0290136,0.0286219,0.0282355},
        {0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.0524733,0.051765,0.0510661,0.0503767,0.0496966,0.0490257,0.0483639,0.047711,0.0470669,0.0464315,0.0458047,0.0451863,0.0445763,0.0439745,0.0433808,0.0427952,0.0422175,0.0416475,0.0410853,0.0405306,0.0399835,0.0394437,0.0389112,0.0383859,0.0378677,0.0373565,0.0368522,0.0363547,0.0358639,0.0353797,0.0349021,0.0344309,0.0339661,0.0335076,0.0330552,0.032609,0.0321687,0.0317345,0.031306,0.0308834,0.0304665,0.0300552,0.0296494,0.0292492,0.0288543,0.0284648,0.0280805,0.0277014,0.0273274,0.0269585,0.0265946,0.0262356,0.0258814,0.025532,0.0251873,0.0248473},
        {0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0461765,0.0455532,0.0449382,0.0443315,0.0437331,0.0431427,0.0425602,0.0419857,0.0414189,0.0408597,0.0403081,0.0397639,0.0392271,0.0386976,0.0381751,0.0376598,0.0371514,0.0366498,0.0361551,0.035667,0.0351855,0.0347105,0.0342419,0.0337796,0.0333236,0.0328737,0.0324299,0.0319921,0.0315602,0.0311341,0.0307138,0.0302992,0.0298902,0.0294866,0.0290886,0.0286959,0.0283085,0.0279263,0.0275493,0.0271774,0.0268105,0.0264486,0.0260915,0.0257393,0.0253918,0.025049,0.0247108,0.0243772,0.0240482,0.0237235,0.0234032,0.0230873,0.0227756,0.0224681,0.0221648,0.0218656},
        {0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0406354,0.0400868,0.0395456,0.0390117,0.0384851,0.0379655,0.037453,0.0369474,0.0364486,0.0359565,0.0354711,0.0349923,0.0345199,0.0340539,0.0335941,0.0331406,0.0326932,0.0322518,0.0318164,0.0313869,0.0309632,0.0305452,0.0301328,0.029726,0.0293247,0.0289289,0.0285383,0.0281531,0.027773,0.0273981,0.0270282,0.0266633,0.0263033,0.0259482,0.0255979,0.0252524,0.0249115,0.0245752,0.0242434,0.0239161,0.0235932,0.0232747,0.0229605,0.0226506,0.0223448,0.0220431,0.0217455,0.021452,0.0211624,0.0208767,0.0205948,0.0203168,0.0200425,0.02,0.02,0.02},
        {0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0357591,0.0352764,0.0348001,0.0343303,0.0338669,0.0334097,0.0329586,0.0325137,0.0320748,0.0316418,0.0312146,0.0307932,0.0303775,0.0299674,0.0295628,0.0291637,0.02877,0.0283816,0.0279985,0.0276205,0.0272476,0.0268798,0.0265169,0.0261589,0.0258058,0.0254574,0.0251137,0.0247747,0.0244402,0.0241103,0.0237848,0.0234637,0.0231469,0.0228345,0.0225262,0.0222221,0.0219221,0.0216261,0.0213342,0.0210462,0.0207621,0.0204818,0.0202053,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.031468,0.0310432,0.0306241,0.0302107,0.0298028,0.0294005,0.0290036,0.0286121,0.0282258,0.0278447,0.0274688,0.027098,0.0267322,0.0263713,0.0260153,0.0256641,0.0253176,0.0249758,0.0246387,0.024306,0.0239779,0.0236542,0.0233349,0.0230199,0.0227091,0.0224025,0.0221001,0.0218017,0.0215074,0.0212171,0.0209306,0.0206481,0.0203693,0.0200943,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.0276919,0.027318,0.0269492,0.0265854,0.0262265,0.0258724,0.0255232,0.0251786,0.0248387,0.0245034,0.0241726,0.0238462,0.0235243,0.0232067,0.0228935,0.0225844,0.0222795,0.0219787,0.021682,0.0213893,0.0211006,0.0208157,0.0205347,0.0202575,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0243688,0.0240399,0.0237153,0.0233952,0.0230793,0.0227678,0.0224604,0.0221572,0.0218581,0.021563,0.0212719,0.0209847,0.0207014,0.0204219,0.0201462,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0214446,0.0211551,0.0208695,0.0205877,0.0203098,0.0200356,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
        {0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02},
    };

    time_t now = (time_t)(cur_time / 1000000);
	tm* curr_tm = gmtime(&now); 
	unsigned int CurrentYear = curr_tm -> tm_year + 1900;
    unsigned int Year = CurrentYear - 2021;
	InflationRate = RateArray.at(Year).at(StakeRate);
	return 0;
}

// int ca_algorithm::CalcBonusValue(uint64_t &cur_time, const std::string &bonusAddr, std::map<std::string, uint64_t> & values)
// {
//     std::vector<std::string> abnormal_addr_list;
//     std::unordered_map<std::string, uint64_t> addr_sign_cnt;
//     auto ret = ca_algorithm::GetAbnormalSignAddrListByPeriod(cur_time, abnormal_addr_list, addr_sign_cnt);
//     if(ret < 0) return ret - 100;
    
//     if(addr_sign_cnt.find(bonusAddr) == addr_sign_cnt.end() || std::find(abnormal_addr_list.begin(),abnormal_addr_list.end(), bonusAddr) != abnormal_addr_list.end())
//     {
//         std::cout << RED <<"AbnormalAddr:"<<bonusAddr<< RESET << std::endl;
//         return -1;
//     }
    
// 	std::string strTx;
// 	CTransaction tx;
//     uint64_t total_award;
//     uint64_t TotalCirculationYesterday;
//     uint64_t TotalinvestYesterday;
// 	uint64_t zero_time = MagicSingleton<TimeUtil>::GetInstance()->getMorningTime(cur_time) * 1000000;
//     std::map<std::string, std::pair<uint64_t,uint64_t>> mpInvestAddr2Amount;

//     ret = GetInvestmentAmountAndDuration(bonusAddr, cur_time, zero_time, mpInvestAddr2Amount);
//     if(ret < 0) return ret-=200;

//     ret = GetTotalCirculationYesterday(cur_time, TotalCirculationYesterday);
//     if(ret < 0) return ret-=300;

//     ret = GetTotalInvestmentYesterday(cur_time, TotalinvestYesterday);
//     if(ret < 0) return ret-=400;

// 	if(TotalinvestYesterday == 0) return -4;
    
// 	uint64_t StakeRate = ((double)TotalinvestYesterday / TotalCirculationYesterday + 0.005) * 100;
// 	double InflationRate;
//     if(StakeRate <= 35) StakeRate = 35;
//     else if(StakeRate >= 90) StakeRate = 90;
// 	ret = ca_algorithm::GetInflationRate(cur_time, StakeRate - 1, InflationRate);
// 	if(ret < 0) return ret-=500;

//     std::stringstream ss;
//     ss << std::setprecision(8) << InflationRate;
//     std::string InflationRateStr = ss.str();
//     ss.str(std::string());
//     ss.clear();
//     ss << std::setprecision(2) << (StakeRate/100.0);

//     std::string StakeRateStr = ss.str();
//     cpp_bin_float EarningRate0 = static_cast<cpp_bin_float>(std::to_string(global::ca::kDecimalNum)) * (static_cast<cpp_bin_float>(InflationRateStr) / static_cast<cpp_bin_float>(StakeRateStr));
//     ss.str(std::string());
//     ss.clear();
//     ss << std::setprecision(8) << EarningRate0;
    
//     uint64_t EarningRate1 = std::stoi(ss.str());

//     double EarningRate2 = (double)EarningRate1 / global::ca::kDecimalNum;
//     // 0.077 / 0.35 = 0.22
//     //最大通胀率 / 最小质押率 = 最大收益率
//     if(EarningRate2 > 0.22) return -5;

//     for(auto &it : mpInvestAddr2Amount)
//     {
//         values[it.first] = EarningRate2 * it.second.first / 365;
//     }
//     return 0;
// }

// int ca_algorithm::CalcBonusValue()
// {
//     const std::string bonusAddr = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
//     std::map<std::string, uint64_t> values;
//     std::vector<std::string> abnormal_addr_list;
//     std::unordered_map<std::string, uint64_t> addr_sign_cnt;
//     uint64_t cur_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//     auto ret = ca_algorithm::GetAbnormalSignAddrListByPeriod(cur_time, abnormal_addr_list, addr_sign_cnt);
//     if(ret < 0) return ret - 100;

//     std::cout << "SignInfo:" << std::endl;
//     for(auto it : addr_sign_cnt)
//     {
//         std::cout << it.first << " " << it.second << std::endl;
//     }
//     std::cout << "AbnormalAddr: " << std::endl;
//     for(auto it : abnormal_addr_list)
//     {
//         std::cout << it << std::endl;
//     }

//     if(addr_sign_cnt.find(bonusAddr) == addr_sign_cnt.end() || std::find(abnormal_addr_list.begin(),abnormal_addr_list.end(), bonusAddr) != abnormal_addr_list.end())
//     {
//         std::cout << RED <<"AbnormalAddr:"<<bonusAddr<< RESET << std::endl;
//         return -1;
//     }
    
// 	std::string strTx;
// 	CTransaction tx;
//     uint64_t total_award;
//     uint64_t TotalCirculationYesterday;
//     uint64_t TotalinvestYesterday;
// 	uint64_t zero_time = MagicSingleton<TimeUtil>::GetInstance()->getMorningTime(cur_time) * 1000000;
//     std::map<std::string, std::pair<uint64_t,uint64_t>> mpInvestAddr2Amount;

//     ret = GetInvestmentAmountAndDuration(bonusAddr, cur_time, zero_time, mpInvestAddr2Amount);
//     if(ret < 0) return ret-=200;

//     std::cout << "bonusAddr: " << bonusAddr << std::endl;
//     for(auto &it : mpInvestAddr2Amount)
//     {
//         std::cout << "investor: " << it.first << " amount: " << it.second.first << " duration: " << it.second.second << std::endl;
//     }
//     std::cout<<std::endl;

//     ret = GetTotalCirculationYesterday(cur_time, TotalCirculationYesterday);
//     if(ret < 0) return ret-=300;

//     ret = GetTotalInvestmentYesterday(cur_time, TotalinvestYesterday);
//     if(ret < 0) return ret-=400;

//     std::cout << "TotalCirculationYesterday: " << TotalCirculationYesterday << std::endl;
//     std::cout << "TotalinvestYesterday: " << TotalinvestYesterday << std::endl;

// 	if(TotalinvestYesterday == 0) return -4;
    
// 	uint64_t StakeRate = ((double)TotalinvestYesterday / TotalCirculationYesterday + 0.005) * 100;
// 	double InflationRate;
//     if(StakeRate <= 35) StakeRate = 35;
//     else if(StakeRate >= 90) StakeRate = 90;
// 	ret = ca_algorithm::GetInflationRate(cur_time, StakeRate - 1, InflationRate);
// 	if(ret < 0) return ret-=500;

//     std::cout << "interest rate: " << InflationRate << std::endl;

//     std::stringstream ss;
//     ss << std::setprecision(8) << InflationRate;
//     std::string InflationRateStr = ss.str();
//     ss.str(std::string());
//     ss.clear();
//     ss << std::setprecision(2) << (StakeRate/100.0);
//     std::string StakeRateStr = ss.str();
//     cpp_bin_float EarningRate0 = static_cast<cpp_bin_float>(std::to_string(global::ca::kDecimalNum)) * (static_cast<cpp_bin_float>(InflationRateStr) / static_cast<cpp_bin_float>(StakeRateStr));
//     ss.str(std::string());
//     ss.clear();
//     ss << std::setprecision(8) << EarningRate0;
    
//     uint64_t EarningRate1 = std::stoi(ss.str());

//     double EarningRate2 = (double)EarningRate1 / global::ca::kDecimalNum;

//     std::cout << "EarningRate2 : " << EarningRate2 << std::endl;

//     // 0.077 / 0.35 = 0.22
//     //最大通胀率 / 最小质押率 = 最大收益率
//     if(EarningRate2 > 0.22) return -5;
//     for(auto &it : mpInvestAddr2Amount)
//     {
//         values[it.first] = EarningRate2 * it.second.first / 365;

//         std::cout << "InvestAddr: " <<it.first << std::endl;
//         std::cout << "InvestAmount: " << it.second.first << std::endl; 
//         std::cout << "reward: " << values[it.first] << std::endl;
//     }
//     return 0;
// }

// uint64_t ca_algorithm::GetSumHashCeilingHeight(uint64_t height)
// {
//     if(height == 0)
//     {
//         return global::ca::sum_hash_range;
//     }
//     auto quotient = height / global::ca::sum_hash_range;
//     auto remainder = height % global::ca::sum_hash_range;
//     if(remainder == 0)
//     {
//         return height;
//     }
//     else
//     {
//         return (quotient + 1) * global::ca::sum_hash_range;
//     }
// }

// uint64_t ca_algorithm::GetSumHashFloorHeight(uint64_t height)
// {
//     auto quotient = height / global::ca::sum_hash_range;
//     auto remainder = height % global::ca::sum_hash_range;
//     if(remainder == 0)
//     {
//         return (quotient - 1) * global::ca::sum_hash_range;
//     }
//     else
//     {
//         return quotient * global::ca::sum_hash_range;
//     }
// }

// int ca_algorithm::CalcHeightsSumHash(uint64_t block_height, global::ca::SaveType saveType, global::ca::BlockObtainMean obtainMean, DBReadWriter &db_writer)
// {
//     if(block_height <= global::ca::sum_hash_range)
//     {
//         return 0;
//     }
//     //如果是从零同步且是普通同步
//     if(global::ca::SaveType::SyncFromZero == saveType && global::ca::BlockObtainMean::Normal == obtainMean)
//     {
//         //如果 块高度 % 100 == 1 并且数据库里没有 块高度-1 的和哈希
//         std::string sumHash;
//         auto ret = db_writer.GetSumHashByHeight(block_height - 1, sumHash);
//         if(block_height % global::ca::sum_hash_range == 1 && DBStatus::DB_SUCCESS != ret)
//         {
//             //计算和哈希并存入数据库
//             auto start_height = block_height - global::ca::sum_hash_range;
//             auto end_height = block_height;
//             if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//             {
//                 return -1;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(block_height - 1, sumHash))
//             {
//                 return -2;
//             }
//             INFOLOG("set height sum hash at height {} hash: {}", block_height - 1, sumHash);
//         }
//     }
//     //如果是从零同步且是缺钱哈希
//     else if(global::ca::SaveType::SyncFromZero == saveType && global::ca::BlockObtainMean::ByPreHash == obtainMean)
//     {
//         //如果块 高度 % 100 == 0 
//         if(block_height % global::ca::sum_hash_range == 0)
//         {
//             std::string sumHash;
//             auto start_height = block_height + 1 - global::ca::sum_hash_range;
//             auto end_height = block_height + 1;
//             if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//             {
//                 return -3;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(block_height, sumHash))
//             {
//                 return -4;
//             }
//             INFOLOG("set height sum hash at height {} hash: {}", block_height, sumHash);
//         }
//     }
//     //如果是从零同步且是缺uxto
//     else if(global::ca::SaveType::SyncFromZero == saveType && global::ca::BlockObtainMean::ByUtxo == obtainMean)
//     {
//         //如果数据库里有这个和哈希
//         std::string sumHash;
//         if(db_writer.GetSumHashByHeight(GetSumHashCeilingHeight(block_height), sumHash) == DBStatus::DB_SUCCESS)
//         {
//             auto start_height = GetSumHashFloorHeight(block_height) + 1;
//             auto end_height = GetSumHashCeilingHeight(block_height) + 1;
//             if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//             {
//                 return -5;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(block_height, sumHash))
//             {
//                 return -6;
//             }
//             INFOLOG("set height sum hash at height {} hash: {}", block_height, sumHash);
//         }
//     }
//     //如果是同步且是普通同步 或者 如果是同步且缺钱哈希       
//     else if((global::ca::SaveType::SyncNormal == saveType && global::ca::BlockObtainMean::Normal == obtainMean)
//             || (global::ca::SaveType::SyncNormal == saveType && global::ca::BlockObtainMean::ByPreHash == obtainMean))
//     {
//         //如果块高度大于等于当前节点高度
//         uint64_t node_height = 0;
//         if (DBStatus::DB_SUCCESS != db_writer.GetBlockTop(node_height))
//         {
//             ERRORLOG("GetBlockTop error!");
//             return -7;
//         }
//         if(block_height >= node_height)
//         {
//             //如果 块高度 % 100 == 1 并且数据库里没有 块高度-1-100 的和哈希
//             std::string sumHash;
//             if(block_height % global::ca::sum_hash_range == 1 && DBStatus::DB_SUCCESS != db_writer.GetSumHashByHeight(block_height - 1 - global::ca::sum_hash_range, sumHash))
//             {
//                 //计算块高度-1-100和哈希并存入数据库
//                 auto start_height = block_height - 2 * global::ca::sum_hash_range;
//                 auto end_height = block_height - 1 - global::ca::sum_hash_range + 1;
//                 if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//                 {
//                     return -8;
//                 }
//                 if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(end_height - 1, sumHash))
//                 {
//                     return -9;
//                 }
//                 INFOLOG("set height sum hash at height {} hash: {}", end_height - 1, sumHash);
//             }
//         }
//         else
//         {
//             //如果所在区见有何哈希,计算块所在区见的哈希和并更新
//             std::string sumHash;
//             if(db_writer.GetSumHashByHeight(GetSumHashCeilingHeight(block_height), sumHash) == DBStatus::DB_SUCCESS)
//             {
//                 auto start_height = GetSumHashFloorHeight(block_height) + 1;
//                 auto end_height = GetSumHashCeilingHeight(block_height) + 1;
//                 if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//                 {
//                     return -10;
//                 }
//                 if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(block_height, sumHash))
//                 {
//                     return -11;
//                 }
//                 INFOLOG("set height sum hash at height {} hash: {}", block_height, sumHash);
//             } 
//         }
//     }
//     //如果是同步且缺utxo
//     else if(global::ca::SaveType::SyncNormal == saveType && global::ca::BlockObtainMean::ByUtxo == obtainMean)
//     {
//         //如果所在区见有何哈希//如果当前高度向下取整 - 100大于等于块高度,计算块所在区见的哈希和并更新
//         std::string sumHash;
//         if(db_writer.GetSumHashByHeight(GetSumHashCeilingHeight(block_height), sumHash) == DBStatus::DB_SUCCESS)
//         {
//             auto start_height = GetSumHashFloorHeight(block_height) + 1;
//             auto end_height = GetSumHashCeilingHeight(block_height) + 1;
//             if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//             {
//                 return -12;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(block_height, sumHash))
//             {
//                 return -13;
//             }
//             INFOLOG("set height sum hash at height {} hash: {}", block_height, sumHash);
//         } 
//     }
//     //如果是广播见块      
//     else if(global::ca::SaveType::Broadcast == saveType)
//     {
//         //如果 块高度 % 100 == 1 并且数据库里没有 块高度-1-100 的和哈希
//         std::string sumHash;
//         if(block_height % 100 == 1 && DBStatus::DB_SUCCESS != db_writer.GetSumHashByHeight(block_height - 1 - global::ca::sum_hash_range, sumHash))
//         {
//             //计算块高度-1-100和哈希并存入数据库
//             auto start_height = block_height - 2 * global::ca::sum_hash_range;
//             auto end_height = block_height - 1 - global::ca::sum_hash_range + 1;
//             if(!CalculateHeightSumHash(start_height, end_height, db_writer, sumHash))
//             {
//                 return -14;
//             }
//             if (DBStatus::DB_SUCCESS != db_writer.SetSumHashByHeight(end_height - 1, sumHash))
//             {
//                 return -15;
//             }
//             INFOLOG("set height sum hash at height {} hash: {}", end_height - 1, sumHash);
//         }
//     }

//     return 0;
// }





int ca_algorithm::CalcBonusValue(uint64_t &cur_time, const std::string &bonusAddr, std::map<std::string, uint64_t> & values,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
    std::vector<std::string> abnormal_addr_list;
    std::unordered_map<std::string, uint64_t> addr_sign_cnt;
    for(int i = 0;i<SDKAckMsg->abnormaladdr_cnt_size();++i)
    {
        abnormal_addr_list.emplace_back(SDKAckMsg->abnormaladdr_cnt(i).address());
        addr_sign_cnt.insert(std::pair<std::string, uint64_t>(SDKAckMsg->abnormaladdr_cnt(i).address(),SDKAckMsg->abnormaladdr_cnt(i).count()));
    }   

    
    // auto ret = ca_algorithm::GetAbnormalSignAddrListByPeriod(cur_time, abnormal_addr_list, addr_sign_cnt);
    // if(ret < 0) return ret - 100;
    
    if(addr_sign_cnt.find(bonusAddr) == addr_sign_cnt.end() || std::find(abnormal_addr_list.begin(),abnormal_addr_list.end(), bonusAddr) != abnormal_addr_list.end())
    {
        std::cout << RED <<"AbnormalAddr:"<<bonusAddr<< RESET << std::endl;
        return -1;
    }
    
	std::string strTx;
	CTransaction tx;
    uint64_t total_award;
    uint64_t TotalCirculationYesterday;
    uint64_t TotalinvestYesterday;
	uint64_t zero_time = MagicSingleton<TimeUtil>::GetInstance()->getMorningTime(cur_time) * 1000000;
    std::map<std::string, std::pair<uint64_t,uint64_t>> mpInvestAddr2Amount;

   auto  ret = GetInvestmentAmountAndDuration(bonusAddr, cur_time, zero_time, mpInvestAddr2Amount,SDKAckMsg);
    if(ret < 0) return ret-=200;

    ret = GetTotalCirculationYesterday(cur_time, TotalCirculationYesterday,SDKAckMsg);
    if(ret < 0) return ret-=300;

    ret = GetTotalInvestmentYesterday(cur_time, TotalinvestYesterday,SDKAckMsg);
    if(ret < 0) return ret-=400;

	if(TotalinvestYesterday == 0) return -4;
    
	uint64_t StakeRate = ((double)TotalinvestYesterday / TotalCirculationYesterday + 0.005) * 100;
	double InflationRate;
    if(StakeRate <= 35) StakeRate = 35;
    else if(StakeRate >= 90) StakeRate = 90;
	ret = ca_algorithm::GetInflationRate(cur_time, StakeRate - 1, InflationRate);
	if(ret < 0) return ret-=500;

    std::stringstream ss;
    ss << std::setprecision(8) << InflationRate;
    std::string InflationRateStr = ss.str();
    ss.str(std::string());
    ss.clear();
    ss << std::setprecision(2) << (StakeRate/100.0);

    std::string StakeRateStr = ss.str();
    cpp_bin_float EarningRate0 = static_cast<cpp_bin_float>(std::to_string(global::ca::kDecimalNum)) * (static_cast<cpp_bin_float>(InflationRateStr) / static_cast<cpp_bin_float>(StakeRateStr));
    ss.str(std::string());
    ss.clear();
    ss << std::setprecision(8) << EarningRate0;
    
    uint64_t EarningRate1 = std::stoi(ss.str());

    double EarningRate2 = (double)EarningRate1 / global::ca::kDecimalNum;
    // 0.077 / 0.35 = 0.22
    //最大通胀率 / 最小质押率 = 最大收益率
    if(EarningRate2 > 0.22) return -5;

    for(auto &it : mpInvestAddr2Amount)
    {
        values[it.first] = EarningRate2 * it.second.first / 365;
    }
    return 0;
}