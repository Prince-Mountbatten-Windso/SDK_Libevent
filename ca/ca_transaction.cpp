#include "ca_transaction.h"

#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <iostream>
#include <set>
#include <algorithm>
#include <shared_mutex>
#include <mutex>
#include "proto/ca_protomsg.pb.h"
#include "utils/time_util.h"
#include "utils/base64.h"
#include "utils/string_util.h"
#include "utils/MagicSingleton.h"
#include "utils/util2.h"
#include "utils/hexcode.h"
//#include "include/logging.h"
#include "common/global.h"
#include "ca.h"
#include "ca_global.h"
#include "utils/console.h"
#include "utils/base64_2.h"
#include "ca/ca_algorithm.h"
#include "ca/ca_txhelper.h"
#include "utils/time_util.h"
#include "utils/ReturnAckCode.h"
#include "include/ScopeGuard.h"
#include "utils/EDManager.h"
#include "utils/Cycliclist.hpp"
#include "utils/VRF.hpp"
#include "utils/tmplog.h"
#include "../net/ip_port.h"
#include "../net/Net.h"
#include "../net/connect.h"

using namespace std;

// #include "utils/TFSbenchmark.h"

// int GetBalanceByUtxo(const std::string &address, uint64_t &balance)
// {
// 	if (address.size() == 0)
// 	{
// 		return -1;
// 	}

// 	// CBlockDataApi data_reader;
// 	DBReader db_reader;
// 	std::vector<std::string> addr_utxo_hashs;
// 	db_reader.GetUtxoHashsByAddress(address, addr_utxo_hashs);

// 	uint64_t total = 0;
// 	std::string txRaw;
// 	CTransaction tx;
// 	for (auto utxo_hash : addr_utxo_hashs)
// 	{
// 		if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(utxo_hash, txRaw))
// 		{
// 			return -2;
// 		}
// 		if (!tx.ParseFromString(txRaw))
// 		{
// 			return -3;
// 		}
// 		for (auto &vout : tx.utxo().vout())
// 		{
// 			if (vout.addr() == address)
// 			{
// 				total += vout.value();
// 			}
// 		}
// 	}
// 	balance = total;
// 	return 0;
// }

void setVrf(Vrf &dest, const std::string &proof, const std::string &pub, const std::string &data)
{
	CSign *sign = dest.mutable_vrfsign();
	sign->set_pub(pub);
	sign->set_sign(proof);
	dest.set_data(data);
}

int getVrfdata(const Vrf &vrf, std::string &hash, int &range , double &percentage)
{
	try
	{
		auto json = nlohmann::json::parse(vrf.data());
		hash = json["hash"];
		range = json["range"];
		percentage = json["percentage"];
	}
	catch (...)
	{
		//ERRORLOG("getVrfdata json parse fail !");
		return -1;
	}

	return 0;
}

int getVrfdata(const Vrf &vrf, std::string &hash, int &range)
{
	try
	{
		auto json = nlohmann::json::parse(vrf.data());
		hash = json["hash"];
		range = json["range"];
	}
	catch (...)
	{
		//ERRORLOG("getVrfdata json parse fail !");
		return -1;
	}

	return 0;
}

TransactionType GetTransactionType(const CTransaction &tx)
{
	if (tx.type() == global::ca::kGenesisSign)
	{
		return kTransactionType_Genesis;
	}
	if (tx.type() == global::ca::kTxSign)
	{
		return kTransactionType_Tx;
	}
	if (tx.type() == global::ca::kGasSign)
	{
		return kTransactionType_Gas;
	}
	else if (tx.type() == global::ca::kBurnSign)
	{
		return kTransactionType_Burn;
	}

	return kTransactionType_Unknown;
}

// bool checkTop(int top)
// {
// 	// CBlockDataApi data_reader;
// 	uint64_t mytop = 0;
// 	DBReader db_reader;
// 	db_reader.GetBlockTop(mytop);

// 	if (top < (int)mytop - 4)
// 	{
// 		ERRORLOG("checkTop fail other top:{} my top:{}", top, (int)mytop);
// 		return false;
// 	}
// 	else if (top > (int)mytop + 1)
// 	{
// 		ERRORLOG("checkTop fail other top:{} my top:{}", top, (int)mytop);
// 		return false;
// 	}
// 	else
// 	{
// 		return true;
// 	}
// }

bool ContainSelfVerifySign(const CTransaction &tx)
{
	bool isContainSelfVerifySign = false;

	if (tx.verifysign_size() == 0)
	{
		return isContainSelfVerifySign;
	}

	std::string defaultBase58Addr = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
	int index = defaultBase58Addr != tx.identity() ? 0 : 1;

	for (; index != tx.verifysign_size(); index++)
	{
		const CSign &sign = tx.verifysign(index);
		if (defaultBase58Addr == GetBase58Addr(sign.pub()))
		{
			isContainSelfVerifySign = true;
			break;
		}
	}
	return isContainSelfVerifySign;
}


int GetBlockPackager(std::string &packager, const std::string &hash_utxo, Vrf &info,const std::shared_ptr<GetSDKAck> &SDKAckMsg,std::pair<std::string, uint64_t> &node_ip_port)
{
	// DBReader db_reader;
	// uint64_t top;
	// if (DBStatus::DB_SUCCESS != db_reader.GetBlockTop(top))
	// {
	// 	return -1;
	// }

	std::vector<std::string> hashes;

	//50高度以内取当前高度  50高度以外取当前高度-10 高度
	// uint64_t block_height = top;
	// if (top >= 50)
	// {
	// 	block_height = top - 10;
	// }

	// if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashsByBlockHeight(block_height, hashes))
	// {
	// 	return -2;
	// }
	CBlock block;
	std::vector<CBlock> blocks;
	for (size_t i = 0; i < (size_t) SDKAckMsg->blocks_size(); ++i)
	{
		//const SDKBlockItem blockitem = SDKAckMsg->blocks(i);
		block.ParseFromString(SDKAckMsg->blocks(i));
		blocks.emplace_back(block);
	}

	// for (auto &hash : hashes)
	// {
	// 	std::string blockStr;
	// 	db_reader.GetBlockByBlockHash(hash, blockStr);
	// 	CBlock block;
	// 	block.ParseFromString(blockStr);
	// 	blocks.push_back(block);
	// }


	std::sort(blocks.begin(), blocks.end(), [](const CBlock &x, const CBlock &y)
			  { return x.time() < y.time(); });

	CBlock RandomBlock = blocks[0];
	std::string output, proof;
	ED defaultAccount;
	EVP_PKEY_free(defaultAccount.pkey);
	if (MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(defaultAccount) != 0)
	{
		return -3;
	}
	int ret = MagicSingleton<VRF>::GetInstance()->CreateVRF(defaultAccount.pkey, hash_utxo, output, proof);
	if (ret != 0)
	{
		std::cout << "error create:" << ret << std::endl;
		return -4;
	}


	//取块的签名数组中的3，4，5个  在这个3个地址中用vrf随机找一个地址作为打包节点
	std::vector<std::string> BlockSignInfo;
	for (int j = 2; j < 5; ++j)
	{
		BlockSignInfo.push_back(GetBase58Addr(RandomBlock.sign(j).pub()));
	}

	if (BlockSignInfo.size() < 3)
	{
		return -5;
	}

	uint32_t rand_num = MagicSingleton<VRF>::GetInstance()->GetRandNum(output, 3);
	packager = BlockSignInfo[rand_num]; //打包者

	for(int k = 0;k<SDKAckMsg->nodeinfo_size();++k)
	{
		SDKNodeInfo node = SDKAckMsg->nodeinfo(k);
		if(node.base58addr() == packager)
		{
			std::string ip =  IpPort::ipsz(SDKAckMsg->nodeinfo(k).public_ip());
			std::cout<<"11111public ip = "<<IpPort::ipsz(SDKAckMsg->nodeinfo(k).public_ip()) <<std::endl;
			std::cout<<"111111111public port = "<<SDKAckMsg->nodeinfo(k).public_port()<<std::endl;
			std::cout<<"111111111public port = "<<SDKAckMsg->nodeinfo(k).listen_port()<<std::endl;
			//MagicSingleton<Net>::GetInstance()->connect(ip, SDKAckMsg->nodeinfo(k).listen_port());
			MagicSingleton<net>::GetInstance()->connect(ip, SDKAckMsg->nodeinfo(k).listen_port());
			node_ip_port = std::make_pair(ip,SDKAckMsg->nodeinfo(k).listen_port());
			sleep(2);
		}
	}
	std::string defaultbase58 = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
	if (packager == MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr())
	{
		//ERRORLOG("Packager = {} cannot be the transaction initiator", packager);
		std::cout << "Packager cannot be the transaction initiator " << std::endl;
		return -6;
	}

	
	std::cout << "block rand_num: " << rand_num << std::endl;
	std::cout << "packager: " << packager << std::endl;
	nlohmann::json data_string;
	data_string["hash"] = RandomBlock.hash();
	data_string["range"] = 0;
	data_string["percentage"] = 0;
	setVrf(info, proof, defaultAccount.pubStr, data_string.dump());
	std::cout << "**********VRF Generated the number end**********************" << std::endl;

	return 0;
}



int IsQualifiedToUnstake(const std::string &fromAddr,
						 const std::string &utxo_hash,
						 uint64_t &staked_amount,
						 const std::shared_ptr<GetSDKAck> &SDKAckMsg )
{
	// Query whether the account number has stake assets
	// DBReader db_reader;
	// // CBlockDataApi data_reader;
	std::vector<std::string> addresses;
	// if (db_reader.GetStakeAddress(addresses) != DBStatus::DB_SUCCESS)
	// {
	// 	ERRORLOG(RED "Get all stake address failed!" RESET);
	// 	return -1;
	// }
		
	for(int i = 0;i < SDKAckMsg->pledgeaddr_size();++i)
	{
		addresses.emplace_back(SDKAckMsg->pledgeaddr(i));
	}

	if (std::find(addresses.begin(), addresses.end(), fromAddr) == addresses.end())
	{
		//ERRORLOG(RED "The account number has not staked assets!" RESET);
		return -2;
	}

	// Query whether the utxo to be de stake is in the staked utxo
	std::vector<std::string> utxos;
	for(int j = 0;j < SDKAckMsg->pledgeutxo_size();++j)
	{
		utxos.emplace_back(SDKAckMsg->pledgeutxo(j));
	}
	// if (db_reader.GetStakeAddressUtxo(fromAddr, utxos) != DBStatus::DB_SUCCESS)
	// {
	// 	ERRORLOG(RED "Get stake utxo from address failed!" RESET);
	// 	return -3;
	// }

	if (std::find(utxos.begin(), utxos.end(), utxo_hash) == utxos.end())
	{
		//ERRORLOG(RED "The utxo to be de staked is not in the staked utxo!" RESET);
		return -4;
	}


	CTransaction StakeTx;
	for(const auto &item:SDKAckMsg->pledgetx())
	{
		if(fromAddr == item.address())
		{
			StakeTx.ParseFromString(item.tx());
			//StakeTx = item.tx();
		}
	}

	// Check whether the stake exceeds 30 days
	if (IsMoreThan30DaysForUnstake(utxo_hash,StakeTx) != true)
	{
		//ERRORLOG(RED "The staked utxo is not more than 30 days" RESET);
		return -5;
	}

	

	// std::string strStakeTx;
	// if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(utxo_hash, strStakeTx))
	// {
	// 	ERRORLOG(RED "Stake tx not found!" RESET);
	// 	return -6;
	// }

	// CTransaction StakeTx;
	// if (!StakeTx.ParseFromString(strStakeTx))
	// {
	// 	ERRORLOG(RED "Failed to parse transaction body!" RESET);
	// 	return -7;
	// }


	for (int i = 0; i < StakeTx.utxo().vout_size(); i++)
	{
		if (StakeTx.utxo().vout(i).addr() == global::ca::kVirtualStakeAddr)
		{
			staked_amount = StakeTx.utxo().vout(i).value();
			break;
		}
	}

	if (staked_amount == 0)
	{
		//ERRORLOG(RED "Stake value is zero!" RESET);
		return -8;
	}

	return 0;
}

int CheckInvestQualification(const std::string &fromAddr,
							 const std::string &toAddr,
							 uint64_t invest_amount,
							 const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	// Each investor can only invest once
	//DBReader db_reader;
	// CBlockDataApi data_reader;
	std::vector<string> nodes;

	for(int i = 0;i<SDKAckMsg->bonusaddr_size();++i)
	{
		nodes.emplace_back(SDKAckMsg->bonusaddr(i));
	}
	if(!nodes.empty())
	{
		//ERRORLOG(RED "The investor have already invested in a node!" RESET);
		return -1;
	}

	// auto status = db_reader.GetBonusAddrByInvestAddr(fromAddr, nodes);
	// if (status == DBStatus::DB_SUCCESS && !nodes.empty())
	// {
	// 	ERRORLOG(RED "The investor have already invested in a node!" RESET);
	// 	return -1;
	// }

	// Each investor shall not invest less than 99 yuan
	if (invest_amount < (uint64_t)99 * global::ca::kDecimalNum)
	{
		//ERRORLOG(RED "The investment amount is less than 99" RESET);
		return -2;
	}

	// The node to be invested must have spent 999 to access the Internet
	int64_t stake_time = ca_algorithm::GetPledgeTimeByAddr(toAddr, global::ca::StakeType::kStakeType_Node,SDKAckMsg);
	if (stake_time <= 0)
	{
		//ERRORLOG(RED "The account to be invested has not spent 500 to access the Internet!" RESET);
		return -3;
	}

	// The node to be invested can only be invested by 999 people at most
	std::vector<string> addresses;
	for(int j = 0;j<SDKAckMsg->investedaddr_size(); ++j)
	{
		addresses.emplace_back(SDKAckMsg->investedaddr(j));
	}
	// status = db_reader.GetInvestAddrsByBonusAddr(toAddr, addresses);
	// if (status != DBStatus::DB_SUCCESS && status != DBStatus::DB_NOT_FOUND)
	// {
	// 	ERRORLOG(RED "Get invest addrs by node failed!" RESET);
	// 	return -4;
	// }
	if (addresses.size() + 1 > 999)
	{
		//ERRORLOG(RED "The account number to be invested have been invested by 999 people!" RESET);
		return -5;
	}

	// The node to be invested can only be be invested 100000 TFS at most
	uint64_t sum_invest_amount = 0;
	for (auto &address : addresses)
	{
		CTransaction tx;
	
		for(const auto &item :SDKAckMsg->bonustx())
		{
			if(item.address() == address)
			{
				tx.ParseFromString(item.tx()); 
				for (auto &vout : tx.utxo().vout())
				{
					if (vout.addr() == global::ca::kVirtualInvestAddr)
					{
						sum_invest_amount += vout.value();
						break;
					}
				}
			}
		}
			
		// std::vector<string> utxos;
		// if (db_reader.GetBonusAddrInvestUtxosByBonusAddr(toAddr, address, utxos) != DBStatus::DB_SUCCESS)
		// {
		// 	ERRORLOG("GetBonusAddrInvestUtxosByBonusAddr failed!");
		// 	return -6;
		// }

		// for (const auto &utxo : utxos)
		// {
		// 	std::string strTx;
		// 	if (db_reader.GetTransactionByHash(utxo, strTx) != DBStatus::DB_SUCCESS)
		// 	{
		// 		ERRORLOG("GetTransactionByHash failed!");
		// 		return -7;
		// 	}

		// 	CTransaction tx;
		// 	if (!tx.ParseFromString(strTx))
		// 	{
		// 		ERRORLOG("Failed to parse transaction body!");
		// 		return -8;
		// 	}
		// 	for (auto &vout : tx.utxo().vout())
		// 	{
		// 		if (vout.addr() == global::ca::kVirtualInvestAddr)
		// 		{
		// 			sum_invest_amount += vout.value();
		// 			break;
		// 		}
		// 	}
		// }
	}


	if (sum_invest_amount + invest_amount > 100000ull * global::ca::kDecimalNum)
	{
		//ERRORLOG(RED "The total amount invested in a single node will be more than 100000!" RESET);
		return -9;
	}
	return 0;
}


int IsQualifiedToDisinvest(const std::string &fromAddr,
						   const std::string &toAddr,
						   const std::string &utxo_hash,
						   uint64_t &invested_amount,
						   const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	// Query whether the account has invested assets to node
	//DBReader db_reader;
	// CBlockDataApi data_reader;
	std::vector<string> nodes;

	for(int i = 0;i<SDKAckMsg->bonusaddr_size(); ++i)
	{
		nodes.emplace_back(SDKAckMsg->bonusaddr(i));
	}

	// if (db_reader.GetBonusAddrByInvestAddr(fromAddr, nodes) != DBStatus::DB_SUCCESS)
	// {
	// 	ERRORLOG("GetBonusAddrByInvestAddr failed!");
	// 	return -1;
	// }
	if (std::find(nodes.begin(), nodes.end(), toAddr) == nodes.end())
	{
		//ERRORLOG(RED "The account has not invested assets to node!" RESET);
		return -2;
	}

	// Query whether the utxo to divest is in the utxos that have been invested
	std::vector<std::string> utxos;
	for(int j = 0;j<SDKAckMsg->bonusaddrinvestutxos_size(); ++j)
	{
		utxos.emplace_back(SDKAckMsg->bonusaddrinvestutxos(j));
	}

	// if (db_reader.GetBonusAddrInvestUtxosByBonusAddr(toAddr, fromAddr, utxos) != DBStatus::DB_SUCCESS)
	// {
	// 	ERRORLOG("GetBonusAddrInvestUtxosByBonusAddr failed!");
	// 	return -3;
	// }

	if (std::find(utxos.begin(), utxos.end(), utxo_hash) == utxos.end())
	{
		//ERRORLOG(RED "The utxo to divest is not in the utxos that have been invested!" RESET);
		return -4;
	}

	CTransaction InvestedTx;
	for(int k = 0;k<SDKAckMsg->bonustx_size();++k)
	{
		if(fromAddr == SDKAckMsg->bonustx(k).address())
		{
			InvestedTx.ParseFromString(SDKAckMsg->bonustx(k).tx()); 
		}
	}
	// for(const auto &item:SDKAckMsg->bonustx_size())
	// {
	// 	if(fromAddr == item.address())
	// 	{
	// 		InvestedTx = item.tx();
	// 	}
	// }

	// Query whether the investment exceeds one day
	if (IsMoreThan1DayForDivest(utxo_hash,InvestedTx) != true)
	{
		//ERRORLOG(RED "The invested utxo is not more than 1 day!" RESET);
		return -5;
	}

	// The amount to be divested must be greater than 0
	// std::string strInvestTx;
	// if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(utxo_hash, strInvestTx))
	// {
	// 	ERRORLOG("Invest tx not found!");
	// 	return -6;
	// }
	
	// if (!InvestedTx.ParseFromString(strInvestTx))
	// {
	// 	ERRORLOG("Failed to parse transaction body!");
	// 	return -7;
	// }

	nlohmann::json data_json = nlohmann::json::parse(InvestedTx.data());
	nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
	std::string invested_addr = tx_info["BonusAddr"].get<std::string>();
	if (toAddr != invested_addr)
	{
		//ERRORLOG(RED "The node to be divested is not invested!" RESET);
		return -8;
	}

	for (int i = 0; i < InvestedTx.utxo().vout_size(); i++)
	{
		if (InvestedTx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
		{
			invested_amount = InvestedTx.utxo().vout(i).value();
			break;
		}
	}

	if (invested_amount == 0)
	{
		//ERRORLOG(RED "The invested value is zero!" RESET);
		return -9;
	}
	return 0;
}



// Check time of the unstake, unstake time must be more than 30 days, add 20201208   LiuMingLiang
bool IsMoreThan30DaysForUnstake(const std::string &utxo,const CTransaction &StakeTx)
{
	// DBReader db_reader;

	// std::string strTransaction;
	// DBStatus status = db_reader.GetTransactionByHash(utxo, strTransaction);
	// if (status != DBStatus::DB_SUCCESS)
	// {
	// 	return false;
	// }

	// CTransaction utxoStake;
	// utxoStake.ParseFromString(strTransaction);
	uint64_t nowTime = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	uint64_t DAYS30 = (uint64_t)1000000 * 1;
	if (global::kBuildType == global::BuildType::kBuildType_Dev)
	{
		DAYS30 = (uint64_t)1000000 * 60;
	}

	return (nowTime - StakeTx.time()) >= DAYS30;
}

// Check time of the redeem, redeem time must be more than 30 days, add 20201208   LiuMingLiang
bool IsMoreThan1DayForDivest(const std::string &utxo,const CTransaction &InvestedTx)
{
// 	DBReader db_reader;

// 	std::string strTransaction;
// 	DBStatus status = db_reader.GetTransactionByHash(utxo, strTransaction);
// 	if (status != DBStatus::DB_SUCCESS)
// 	{
// 		return -1;
// 	}
// 	CTransaction utxoStake;
// 	utxoStake.ParseFromString(strTransaction);
	uint64_t nowTime = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	uint64_t DAY = (uint64_t)1000000 * 1;
	if (global::kBuildType == global::BuildType::kBuildType_Dev)
	{
		DAY = (uint64_t)1000000 * 60;
	}
	return (nowTime - InvestedTx.time()) >= DAY;
}

int VerifyBonusAddr(const std::string &BonusAddr,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	uint64_t invest_amount = 0;
	for (size_t i = 0; i < (size_t) SDKAckMsg->bonusamount_size(); ++i)
    {
        const SDKBonusamout item = SDKAckMsg->bonusamount(i);
       
		if(BonusAddr == item.address())
        {
           invest_amount +=  item.invest_amount();
        }
	}
	// auto ret = MagicSingleton<BounsAddrCache>::GetInstance()->get_amount(BonusAddr, invest_amount);
	// if (ret < 0)
	// {
	// 	ERRORLOG("invest BonusAddr: {}, ret:{}", BonusAddr, ret);
	// 	return -99;
	// }
	// DEBUGLOG("invest BonusAddr: {}, invest total: {}", BonusAddr, invest_amount);
	return invest_amount >= global::ca::kMinInvestAmt ? 0 : -99;
}





int GetInvestmentAmountAndDuration(const std::string &bonusAddr, const uint64_t &cur_time, const uint64_t &zero_time, std::map<std::string, std::pair<uint64_t, uint64_t>> &mpInvestAddr2Amount,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	//DBReadWriter db_writer;
	// CBlockDataApi data_reader;
	std::string strTx;
	CTransaction tx;
	std::vector<string> addresses;

	time_t t = cur_time;
	t = t / 1000000;
	struct tm *tm = gmtime(&t);
	tm->tm_hour = 23;
	tm->tm_min = 59;
	tm->tm_sec = 59;
	uint64_t end_time = mktime(tm);
	end_time *= 1000000;

	uint64_t invest_amount = 0;
	uint64_t invest_amountDay = 0;

	// for(int k = 0;k<SDKAckMsg->bonustx_size();++k)
	// {
	// 	if(bonusAddr == SDKAckMsg->bonustx(k).address())
	// 	{
	// 		tx = SDKAckMsg->bonustx(k).tx();
	// 		if (tx.time() >= zero_time && tx.time() <= end_time)
	// 		{
	// 			for (int i = 0; i < tx.utxo().vout_size(); i++)
	// 			{
	// 				if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
	// 				{
	// 					invest_amountDay += tx.utxo().vout(i).value();
	// 					invest_amount += tx.utxo().vout(i).value();
	// 					break;
	// 				}
	// 			}
	// 		}
	// 		else
	// 		{
	// 			for (int i = 0; i < tx.utxo().vout_size(); i++)
	// 			{
	// 				if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
	// 				{
	// 					invest_amount += tx.utxo().vout(i).value();
	// 					break;
	// 				}
	// 			}
	// 			break;
	// 		}
	// 	}
		
	// 	invest_amount = (invest_amount - invest_amountDay);
	// 	if (invest_amount == 0)
	// 	{
	// 		continue;
	// 	}
	// 	mpInvestAddr2Amount[bonusAddr].first = invest_amount;
	// }

	// if (db_writer.GetInvestAddrsByBonusAddr(bonusAddr, addresses) != DBStatus::DB_SUCCESS)
	// {
	// 	return -1;
	// }


	for(int k = 0;k < SDKAckMsg->claiminvestedaddr_size();++k)
	{
		addresses.emplace_back(SDKAckMsg->claiminvestedaddr(k));
	}

	for (auto &address : addresses)
	{
		for(int k = 0;k<SDKAckMsg->claimbonustx_size();++k)
		{
			if(address == SDKAckMsg->claimbonustx(k).address())
			{
				tx.ParseFromString(SDKAckMsg->claimbonustx(k).tx()); 
				if (tx.time() >= zero_time && tx.time() <= end_time)
				{
					for (int i = 0; i < tx.utxo().vout_size(); i++)
					{
						if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
						{
							invest_amountDay += tx.utxo().vout(i).value();
							invest_amount += tx.utxo().vout(i).value();
							break;
						}
					}
				}
				else
				{
					for (int i = 0; i < tx.utxo().vout_size(); i++)
					{
						if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
						{
							invest_amount += tx.utxo().vout(i).value();
							break;
						}
					}
					break;
				}
			}
		}



		// std::vector<std::string> utxos;
		// if (db_writer.GetBonusAddrInvestUtxosByBonusAddr(bonusAddr, address, utxos) != DBStatus::DB_SUCCESS)
		// {
		// 	return -2;
		// }

		// invest_amount = 0;
		// invest_amountDay = 0;
		// for (const auto &hash : utxos)
		// {
		// 	tx.Clear();
		// 	if (db_writer.GetTransactionByHash(hash, strTx) != DBStatus::DB_SUCCESS)
		// 	{
		// 		return -3;
		// 	}
		// 	if (!tx.ParseFromString(strTx))
		// 	{
		// 		return -4;
		// 	}
		// 	if (tx.time() >= zero_time && tx.time() <= end_time)
		// 	{
		// 		for (int i = 0; i < tx.utxo().vout_size(); i++)
		// 		{
		// 			if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
		// 			{
		// 				invest_amountDay += tx.utxo().vout(i).value();
		// 				invest_amount += tx.utxo().vout(i).value();
		// 				break;
		// 			}
		// 		}
		// 	}
		// 	else
		// 	{
		// 		for (int i = 0; i < tx.utxo().vout_size(); i++)
		// 		{
		// 			if (tx.utxo().vout(i).addr() == global::ca::kVirtualInvestAddr)
		// 			{
		// 				invest_amount += tx.utxo().vout(i).value();
		// 				break;
		// 			}
		// 		}
		// 		break;
		// 	}
		// }
		invest_amount = (invest_amount - invest_amountDay);
		if (invest_amount == 0)
		{
			continue;
		}
		mpInvestAddr2Amount[address].first = invest_amount;
	}
	if (mpInvestAddr2Amount.empty())
	{
		return -9;
	}
	return 0;
}





int GetTotalCirculationYesterday(const uint64_t &cur_time, uint64_t &TotalCirculation,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	//DBReadWriter db_writer;
	// CBlockDataApi data_reader;
	std::vector<std::string> utxos;
	std::string strTx;
	CTransaction tx;
	{
		std::lock_guard<std::mutex> lock(global::ca::kBonusMutex);
		TotalCirculation = SDKAckMsg->m2();
		// if (DBStatus::DB_SUCCESS != db_writer.GetM2(TotalCirculation))
		// {
		// 	return -1;
		// }
		uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(cur_time);
		// auto ret = db_writer.GetBonusUtxoByPeriod(Period, utxos);
		// if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
		// {
		// 	return -2;
		// }
	}
	uint64_t Claim_Vout_amount = 0;
	uint64_t TotalClaimDay = 0;
	// for (auto utxo = utxos.rbegin(); utxo != utxos.rend(); utxo++)
	// {
	// 	if (db_writer.GetTransactionByHash(*utxo, strTx) != DBStatus::DB_SUCCESS)
	// 	{
	// 		return -3;
	// 	}
	// 	if (!tx.ParseFromString(strTx))
	// 	{
	// 		return -4;
	// 	}
	// 	uint64_t claim_amount = 0;
	// 	if ((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
	// 	{
	// 		nlohmann::json data_json = nlohmann::json::parse(tx.data());
	// 		nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
	// 		tx_info["BonusAmount"].get_to(claim_amount);
	// 		TotalClaimDay += claim_amount;
	// 	}
	// }



	CTransaction Claimtx;
	for(int i = 0;i<SDKAckMsg->claimtx_size();++i)
	{
		//if(SDKAckMsg->claimtx(i).address() == Addr)
		{
			tx.ParseFromString(SDKAckMsg->claimtx(i).tx()); 
			uint64_t claim_amount = 0;
			if ((global::ca::TxType)tx.txtype() != global::ca::TxType::kTxTypeTx)
			{
				nlohmann::json data_json = nlohmann::json::parse(tx.data());
				nlohmann::json tx_info = data_json["TxInfo"].get<nlohmann::json>();
				tx_info["BonusAmount"].get_to(claim_amount);
				TotalClaimDay += claim_amount;
			}
		}
	}


	if (global::kBuildType == global::BuildType::kBuildType_Dev)
	{
		// std::cout<<"TotalCirculation: "<<TotalCirculation<<std::endl;
	}
	TotalCirculation -= TotalClaimDay;
	return 0;
}

int GetTotalInvestmentYesterday(const uint64_t &cur_time, uint64_t &Totalinvest,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	//DBReadWriter db_writer;
	// CBlockDataApi data_reader;
	std::vector<std::string> utxos;
	std::string strTx;
	CTransaction tx;
	{
		std::lock_guard<std::mutex> lock(global::ca::kInvestMutex);
		Totalinvest = SDKAckMsg->totalinvest();
		//auto ret = db_writer.GetTotalInvestAmount(Totalinvest);
		// if (DBStatus::DB_SUCCESS != ret)
		// {
		// 	if (DBStatus::DB_NOT_FOUND != ret)
		// 	{
		// 		return -1;
		// 	}
		// 	else
		// 	{
		// 		Totalinvest = 0;
		// 	}
		// }
		uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(cur_time);
		// ret = db_writer.GetInvestUtxoByPeriod(Period, utxos);
		// if (DBStatus::DB_SUCCESS != ret && DBStatus::DB_NOT_FOUND != ret)
		// {
		// 	return -2;
		// }
	}
	uint64_t Invest_Vout_amount = 0;
	uint64_t TotalInvestmentDay = 0;
	// for (auto utxo = utxos.rbegin(); utxo != utxos.rend(); utxo++)
	// {
	// 	Invest_Vout_amount = 0;
	// 	if (db_writer.GetTransactionByHash(*utxo, strTx) != DBStatus::DB_SUCCESS)
	// 	{
	// 		return -3;
	// 	}
	// 	if (!tx.ParseFromString(strTx))
	// 	{
	// 		return -4;
	// 	}
	// 	for (auto &vout : tx.utxo().vout())
	// 	{
	// 		if (vout.addr() == global::ca::kVirtualInvestAddr)
	// 		{
	// 			Invest_Vout_amount += vout.value();
	// 			break;
	// 		}
	// 	}
	// 	TotalInvestmentDay += Invest_Vout_amount;
	// }

	for(int i = 0;i<SDKAckMsg->claimbonustx_size();++i)
	{
		tx.ParseFromString(SDKAckMsg->claimbonustx(i).tx()); 
		for (auto &vout : tx.utxo().vout())
		{
			if (vout.addr() == global::ca::kVirtualInvestAddr)
			{
				Invest_Vout_amount += vout.value();
				break;
			}
		}
		TotalInvestmentDay += Invest_Vout_amount;
	}

	if (global::kBuildType == global::BuildType::kBuildType_Dev)
	{
		// std::cout<<"Totalinvest: "<<Totalinvest<<std::endl;
	}
	Totalinvest -= TotalInvestmentDay;
	return 0;
}




std::map<int32_t, std::string> GetMultiSignTxReqCode()
{
	std::map<int32_t, std::string> errInfo = {
		std::make_pair(0, ""),
		std::make_pair(-1, ""),
		std::make_pair(-2, ""),
		std::make_pair(-3, ""),
		std::make_pair(-4, ""),
		std::make_pair(-5, ""),
		std::make_pair(-6, ""),
	};

	return errInfo;
}



bool IsMultiSign(const CTransaction &tx)
{
	global::ca::TxType tx_type = (global::ca::TxType)tx.txtype();

	return tx.utxo().owner_size() == 1 &&
		   (CheckBase58Addr(tx.utxo().owner(0), Base58Ver::kBase58Ver_MultiSign) &&
			(tx.utxo().vin_size() == 1) &&
			global::ca::TxType::kTxTypeTx == tx_type);
}




int CalculateGas(const CTransaction &tx, uint64_t &gas)
{

	TransactionType tx_type = GetTransactionType(tx);
	if (tx_type == kTransactionType_Genesis || tx_type == kTransactionType_Tx)
	{

		uint64_t utxo_size = 0;
		const CTxUtxo &utxo = tx.utxo();

		utxo_size += utxo.owner_size() * 34;

		for (auto &vin : utxo.vin())
		{
			utxo_size += vin.prevout().size() * 64;
		}
		utxo_size += utxo.vout_size() * 34;

		gas += utxo_size;
		gas += tx.type().size() + tx.data().size() + tx.info().size();
		gas += tx.reserve0().size() + tx.reserve1().size();
	}

	gas *= 2;

	if (gas == 0)
	{
		//ERRORLOG(" gas = 0 !");
		return -1;
	}

	return 0;
}

//创建交易时用的接口
int GenerateGas(const CTransaction &tx, const std::map<std::string, int64_t> &toAddr, uint64_t &gas)
{

	// TransactionType tx_type = GetTransactionType(tx);
	// if (tx_type == kTransactionType_Genesis || tx_type == kTransactionType_Tx)
	// {

	// 	uint64_t utxo_size = 0;
	// 	const CTxUtxo &utxo = tx.utxo();

	// 	utxo_size += utxo.owner_size() * 34;

	// 	for (auto &vin : utxo.vin())
	// 	{
	// 		utxo_size += vin.prevout().size() * 64;
	// 	}

	// 	utxo_size += toAddr.size() * 34;

	// 	gas += utxo_size;
	// 	gas += tx.type().size() + tx.data().size() + tx.info().size();
	// 	gas += tx.reserve0().size() + tx.reserve1().size();
	// }

	// gas *= 2;

	// if (gas == 0)
	// {
	// 	//ERRORLOG(" gas = 0 !");
	// 	return -1;
	// }

	// return 0;




	uint64_t UtxoSize = 0;
	TransactionType tx_type = GetTransactionType(tx);
	if (tx_type == kTransactionType_Genesis || tx_type == kTransactionType_Tx)
	{

		uint64_t utxo_size = 0;
		const CTxUtxo &utxo = tx.utxo();

		utxo_size += utxo.owner_size() * 34;

		for (auto &vin : utxo.vin())
		{
			utxo_size += vin.prevout().size() * 64;
			UtxoSize  += vin.prevout().size();
		}

		utxo_size += utxo.vout_size() * 34;

		gas += utxo_size;
		gas += tx.type().size() + tx.data().size() + tx.info().size();
		gas += tx.reserve0().size() + tx.reserve1().size();
	}

    
	gas *= UtxoSize * 100;

	if (gas == 0)
	{
		ERRORLOG(" gas = 0 !");
		return -1;
	}

	return 0;

	// uint64_t UtxoSize = 0;
	// TransactionType tx_type = GetTransactionType(tx);
	// if (tx_type == kTransactionType_Genesis || tx_type == kTransactionType_Tx)
	// {

	// 	uint64_t utxo_size = 0;
	// 	const CTxUtxo &utxo = tx.utxo();

	// 	utxo_size += utxo.owner_size() * 34;

	// 	for (auto &vin : utxo.vin())
	// 	{
	// 		utxo_size += vin.prevout().size() * 64;
	// 		UtxoSize  += vin.prevout().size();
	// 	}

	// 	utxo_size += toAddr.size() * 34;

	// 	gas += utxo_size;
	// 	gas += tx.type().size() + tx.data().size() + tx.info().size();
	// 	gas += tx.reserve0().size() + tx.reserve1().size();

	// }

	// gas *= UtxoSize;

	// if (gas == 0)
	// {
	// 	//ERRORLOG(" gas = 0 !");
	// 	std::cout<<"gas = 0 !"<<std::endl;
	// 	return -1;
	// }

	return 0;
}




int SearchStake(const std::string &address, uint64_t &stakeamount, global::ca::StakeType stakeType,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	// DBReader db_reader;
	// // CBlockDataApi data_reader;
	// std::vector<string> utxos;
	// auto status = db_reader.GetStakeAddressUtxo(address, utxos);
	// if (DBStatus::DB_SUCCESS != status)
	// {
	// 	ERRORLOG("GetStakeAddressUtxo fail db_status:{}", status);
	// 	return -1;
	// }

	std::vector<string> rawtx;
	for(int i = 0;i<SDKAckMsg->pledgetx_size();++i)
	{
		rawtx.emplace_back(SDKAckMsg->pledgetx(i).tx());
	}

	uint64_t total = 0;
	for (auto &item : rawtx)
	{
		// std::string strTxRaw;
		// if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(item, strTxRaw))
		// {
		// 	continue;
		// }
		CTransaction utxoTx;
		utxoTx.ParseFromString(item);
		// if (DBStatus::DB_SUCCESS != db_reader.GetTransactionByHash(item, utxoTx))
		//{
		//	continue;
		// }

		nlohmann::json data = nlohmann::json::parse(utxoTx.data());
		nlohmann::json txInfo = data["TxInfo"].get<nlohmann::json>();
		std::string txStakeTypeNet = txInfo["StakeType"].get<std::string>();

		if (stakeType == global::ca::StakeType::kStakeType_Node && txStakeTypeNet != global::ca::kStakeTypeNet)
		{
			continue;
		}

		for (int i = 0; i < utxoTx.utxo().vout_size(); i++)
		{
			CTxOutput txout = utxoTx.utxo().vout(i);
			if (txout.addr() == global::ca::kVirtualStakeAddr)
			{
				total += txout.value();
			}
		}
	}
	stakeamount = total;
	return 0;
}