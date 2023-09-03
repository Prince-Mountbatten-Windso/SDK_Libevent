
#include <cmath>
#include "ca_txhelper.h"
//#include "include/logging.h"
#include "utils/MagicSingleton.h"
#include "utils/string_util.h"
#include "utils/time_util.h"
#include "utils/json.hpp"
#include "ca_global.h"
#include "ca_transaction.h"
#include "utils/console.h"
#include "../utils/EDManager.h"
#include "../utils/tmplog.h"
#include "../net/ip_port.h"
#include "../net/Net.h"
#include "../net/connect.h"
#include "net/net_api.h"


using namespace std;

const uint32_t TxHelper::kMaxVinSize = 100;


int TxHelper::Check(const std::vector<std::string>& fromAddr,uint64_t height)
{
	// Fromaddr cannot be empty
	if(fromAddr.empty())
	{
		//ERRORLOG("Fromaddr is empty!");		
		return -1;
	}

	// Fromaddr cannot have duplicate elements
	std::vector<std::string> tempfromAddr = fromAddr;
	std::sort(tempfromAddr.begin(),tempfromAddr.end());
	auto iter = std::unique(tempfromAddr.begin(),tempfromAddr.end());
	tempfromAddr.erase(iter,tempfromAddr.end());
	if(tempfromAddr.size() != fromAddr.size())
	{
		//ERRORLOG("Fromaddr have duplicate elements!");		
		return -2;
	}

	// Fromaddr cannot be a non base58 address
	for(auto& from : fromAddr)
	{
		if (!CheckBase58Addr(from))
		{
			//ERRORLOG("Fromaddr is a non base58 address!");
			return -3;
		}
	}
	// Fromaddr cannot be suspended
	if (height == 0)
	{
		//ERRORLOG("height is zero!");
		return -6;
	}
	return 0;
}


int TxHelper::FindUtxo(const std::vector<std::string>& fromAddr,
						const uint64_t need_utxo_amount,
						uint64_t& total,
						std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare>& setOutUtxos,
						const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	// Count all utxo
	std::vector<TxHelper::Utxo> Utxos;
	for (const auto& addr : fromAddr)
	{
		for (size_t i = 0; i < (size_t) SDKAckMsg->utxos_size(); ++i)
		{
			const SDKUtxo item = SDKAckMsg->utxos(i);
			if(addr == item.address())
			{
				TxHelper::Utxo utxo;
				utxo.hash = item.hash();
				utxo.addr = item.address();
				utxo.value = item.value();
				utxo.n = item.n();
				Utxos.push_back(utxo);
			}
		}
	}
	//从大到小排序
	std::sort(Utxos.begin(), Utxos.end(),[](const TxHelper::Utxo & u1, const TxHelper::Utxo & u2){
		return u1.value > u2.value;
	});

	total = 0;
	if(setOutUtxos.size() < need_utxo_amount)
	{
		// Fill other positions with non-0
		auto it = Utxos.begin();
		while (it != Utxos.end())
		{
			if (setOutUtxos.size() == need_utxo_amount)
			{
				break;
			}
			total += it->value;

			setOutUtxos.insert(*it);
			++it;
		}
	}
	return 0;
}



int TxHelper::CreateTxTransaction(const std::vector<std::string>& fromAddr,
									const std::map<std::string, int64_t> & toAddr,
									uint64_t height,
									CTransaction& outTx,
									TxHelper::vrfAgentType & type,
									Vrf & info,
									const std::shared_ptr<GetSDKAck> &SDKAckMsg,
									std::pair<std::string, uint64_t> &node_ip_port)
{
	// Check parameters
	
	int ret = Check(fromAddr, height);
	if (ret != 0)
	{
		//ERRORLOG(RED "Check parameters failed! The error code is {}." RESET, ret);
		ret -= 100;
		return ret;
	}

	if(toAddr.empty())
	{
		//ERRORLOG("to addr is empty");
		return -1;
	}	

	for (auto& addr : toAddr)
	{
		if (!CheckBase58Addr(addr.first))
		{
			//ERRORLOG(RED "To address is not base58 address!" RESET);
			return -2;
		}

		for (auto& from : fromAddr)
		{
			if (addr.first == from)
			{
				//ERRORLOG(RED "From address and to address is equal!" RESET);
				return -3;
			}
		}
		
		if (addr.second <= 0)
		{
			//ERRORLOG(RED "Value is zero!" RESET);
			return -4;
		}
	}

	
	uint64_t amount = 0;//Transaction fee
	for (auto& i : toAddr)
	{
		amount += i.second;    
	}
	uint64_t expend = amount;

	// Find utxo
	uint64_t total = 0;
	std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare> setOutUtxos;
	ret = FindUtxo(fromAddr, TxHelper::kMaxVinSize, total, setOutUtxos,SDKAckMsg);
	if (ret != 0)
	{
		//ERRORLOG(RED "FindUtxo failed! The error code is {}." RESET, ret);
		ret -= 200;
		return ret;
	}
	if (setOutUtxos.empty())
	{
		//ERRORLOG(RED "Utxo is empty!" RESET);
		return -5;
	}

	outTx.Clear();

	CTxUtxo * txUtxo = outTx.mutable_utxo();
	
	// Fill Vin
	std::set<std::string> setTxowners;
	for (auto & utxo : setOutUtxos)
	{
		setTxowners.insert(utxo.addr);
	}

	if (setTxowners.empty())
	{
		//ERRORLOG(RED "Tx owner is empty!" RESET);
		return -6;
	}

	uint32_t n = 0;
	for (auto & owner : setTxowners)
	{
		txUtxo->add_owner(owner);
		CTxInput * vin = txUtxo->add_vin();
		for (auto & utxo : setOutUtxos)
		{
			if (owner == utxo.addr)
			{
				CTxPrevOutput * prevOutput = vin->add_prevout();
				prevOutput->set_hash(utxo.hash);
				prevOutput->set_n(utxo.n);
			}
		}
		vin->set_sequence(n++);

		std::string serVinHash = getsha256hash(vin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(owner, serVinHash, signature, pub) != 0)
		{
			//ERRORLOG("sign fail");
			return -7;
		}

		CSign * vinSign = vin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}

	outTx.set_data("");
	outTx.set_type(global::ca::kTxSign);

	uint64_t gas = 0;
	std::map<std::string, int64_t> targetAddrs = toAddr;
	targetAddrs.insert(make_pair(*fromAddr.rbegin(), total - expend));
	targetAddrs.insert(make_pair(global::ca::kVirtualBurnGasAddr,gas));
	if(GenerateGas(outTx, targetAddrs, gas) != 0)
	{
		//ERRORLOG(" gas = 0 !");
		return -8;
	}


	auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getgasptr();
    if(call_back == nullptr)
    {
        return -9;
    }
	call_back(gas);
	// Calculate total expenditure
	//uint64_t gasTotal = (global::ca::kConsensus - 1) * gas;
	//uint64_t cost = 0;//Packing fee
	auto current_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();

	GetTxStartIdentity(fromAddr,height,current_time,type,SDKAckMsg);
	//DEBUGLOG("GetTxStartIdentity current_time = {} type = {}",current_time ,type);
	// if(type == TxHelper::vrfAgentType_unknow)
	// {
	// 	ERRORLOG(" +++++++vrfAgentType_unknow +++++");
	// 	return -300;
	// }

	// if (type == TxHelper::vrfAgentType_local || type == TxHelper::vrfAgentType_vrf)
	// {
	// 	cost = gas;
	// }
	
	//expend += gasTotal + cost;
	
	expend += gas;


	//判断utxo的是否够花费的
	if(total < expend)
	{
		//ERRORLOG("The total cost = {} is less than the cost = {}", total, expend);
		return -10;
	}

	//填充vout
	for(auto & to : toAddr)
	{
		CTxOutput * vout = txUtxo->add_vout();
		vout->set_addr(to.first);
		vout->set_value(to.second);
	}
	CTxOutput * voutFromAddr = txUtxo->add_vout();
	voutFromAddr->set_addr(*fromAddr.rbegin());
	voutFromAddr->set_value(total - expend);

	CTxOutput * vout_burn = txUtxo->add_vout();
	vout_burn->set_addr(global::ca::kVirtualBurnGasAddr);
	vout_burn->set_value(gas);

	std::string serUtxoHash = getsha256hash(txUtxo->SerializeAsString());
	for (auto & owner : setTxowners)
	{		
		if (TxHelper::AddMutilSign(owner, outTx) != 0)
		{
			//ERRORLOG("addd mutil sign fail");
			return -11;
		}
	}
	
	// outTx.set_gas(gas);
	// outTx.set_cost(cost);
	outTx.set_time(current_time);
	outTx.set_version(0);
	outTx.set_consensus(global::ca::kConsensus);
	outTx.set_txtype((uint32_t)global::ca::TxType::kTxTypeTx);


	//判断是否代发 默认或本地代发
	//Determine whether dropshipping is default or local dropshipping
	
	if(type == TxHelper::vrfAgentType_local)
	{
		std::vector<SDKNodeInfo> nodelist;
		for(int i = 0;i<SDKAckMsg->nodeinfo_size();++i)
		{
			SDKNodeInfo node = SDKAckMsg->nodeinfo(i);
			nodelist.emplace_back(node);
		}
		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<size_t> dist(0, nodelist.size() - 1);
   		size_t random = dist(engine);
		std::string base58 = nodelist.at(random).base58addr();
		//outTx.set_identity("13h8j1x9LsQzW1Y4D2FDrkgqB7avvr7jhM"); 
		outTx.set_identity(base58); 
		std::string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		//string ip = "192.168.1.63";
		std::cout<<"ip = "<<ip <<std::endl;
		std::cout<<"type = "<<type <<std::endl;
		std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		std::cout<<"public port = "<<nodelist.at(random).public_port()<<std::endl;
		//MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		MagicSingleton<net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		node_ip_port = std::make_pair(ip,nodelist.at(random).listen_port());
		sleep(2);
	}
	else
	{
		//选择代发人 Select dropshippers
		std::string allUtxos;
		for(auto & utxo:setOutUtxos)
		{
			allUtxos+=utxo.hash;
		}
		//根据所有的utxohash选择候选打包人
		//Candidate packers are selected based on all utxohashes
		allUtxos += std::to_string(current_time);
		
		std::string id;
    	int ret = GetBlockPackager(id,allUtxos, info,SDKAckMsg,node_ip_port);
    	if(ret != 0)
		{
        	return ret;
    	}
		outTx.set_identity(id);
		std::cout<<"id = "<<id <<std::endl;
	}

	//DEBUGLOG("GetTxStartIdentity tx time = {}, package = {}", outTx.time(), outTx.identity());
	
	std::string txHash = getsha256hash(outTx.SerializeAsString());
	outTx.set_hash(txHash);
	return 0;
}


int TxHelper::CreateStakeTransaction(const std::string& fromAddr,
										uint64_t stake_amount,
										uint64_t height,
										TxHelper::PledgeType pledgeType,
										CTransaction & outTx,
										std::vector<TxHelper::Utxo> & outVin
										,TxHelper::vrfAgentType &type ,
										 Vrf & info_,
                                        const std::shared_ptr<GetSDKAck> &SDKAckMsg,
										std::pair<std::string, uint64_t> &node_ip_port)
{
	//Check parameters
	std::vector<std::string> vecfromAddr;
	vecfromAddr.push_back(fromAddr);
	int ret = Check(vecfromAddr, height);
	if(ret != 0)
	{
		//ERRORLOG(RED "Check parameters failed! The error code is {}." RESET, ret);
		ret -= 100;
		return ret;
	}

	
	if (!CheckBase58Addr(fromAddr, Base58Ver::kBase58Ver_Normal)) 
	{
		//ERRORLOG(RED "From address invlaid!" RESET);
		return -1;
	}
	

	if(stake_amount == 0 )
	{
		//ERRORLOG(RED "Stake amount is zero !" RESET);
		return -2;		
	}

	if(stake_amount < 500000000000)
	{
		std::cout << "The pledge amount must be greater than 5000 !" << std::endl;
		return -3;
	}

	std::string strStakeType;
	if (pledgeType == TxHelper::PledgeType::kPledgeType_Node)
	{
		strStakeType = global::ca::kStakeTypeNet;
	}
	else
	{
		//ERRORLOG(RED "Unknown stake type!" RESET);
		return -4;
	}

	uint64_t expend = stake_amount;

	// Find utxo
	uint64_t total = 0;
	std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare> setOutUtxos;
	
	ret = FindUtxo(vecfromAddr, TxHelper::kMaxVinSize, total, setOutUtxos,SDKAckMsg);
	if (ret != 0)
	{
		//ERRORLOG(RED "FindUtxo failed! The error code is {}." RESET, ret);
		ret -= 200;
		return ret;
	}

	if (setOutUtxos.empty())
	{
		//ERRORLOG(RED "Utxo is empty!" RESET);
		return -6;
	}

	outTx.Clear();

	CTxUtxo * txUtxo = outTx.mutable_utxo();
	
	// Fill Vin
	std::set<string> setTxowners;
	for (auto & utxo : setOutUtxos)
	{
		setTxowners.insert(utxo.addr);
	}
	
	if (setTxowners.size() != 1)
	{
		//ERRORLOG(RED "Tx owner is invalid!" RESET);
		return -7;
	}

	for (auto & owner : setTxowners)
	{
		txUtxo->add_owner(owner);
		uint32_t n = 0;
		CTxInput * vin = txUtxo->add_vin();
		for (auto & utxo : setOutUtxos)
		{
			if (owner == utxo.addr)
			{
				CTxPrevOutput * prevOutput = vin->add_prevout();
				prevOutput->set_hash(utxo.hash);
				prevOutput->set_n(utxo.n);
			}
		}
		vin->set_sequence(n++);

		std::string serVinHash = getsha256hash(vin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(owner, serVinHash, signature, pub) != 0)
		{
			return -8;
		}

		CSign * vinSign = vin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}

	nlohmann::json txInfo;
	txInfo["StakeType"] = strStakeType;
	txInfo["StakeAmount"] = stake_amount;

	nlohmann::json data;
	data["TxInfo"] = txInfo;
	outTx.set_data(data.dump());
	outTx.set_type(global::ca::kTxSign);	

	uint64_t gas = 0;
	// Calculate total expenditure
	std::map<std::string, int64_t> toAddr;
	toAddr.insert(std::make_pair(global::ca::kVirtualStakeAddr, stake_amount));
	toAddr.insert(std::make_pair(fromAddr, total - expend));
	toAddr.insert(std::make_pair(global::ca::kVirtualBurnGasAddr, gas));
	
	
	if(GenerateGas(outTx, toAddr, gas) != 0)
	{
		//ERRORLOG(" gas = 0 !");
		return -9;
	}
	auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getgasptr();
    if(call_back == nullptr)
    {
        return -10;
    }
	call_back(gas);
	// uint64_t gasTotal = (global::ca::kConsensus - 1) * gas;

	// uint64_t cost = 0;//Packing fee

	auto current_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	
	GetTxStartIdentity(vecfromAddr,height,current_time,type,SDKAckMsg);
	// if(type == TxHelper::vrfAgentType_unknow)
	// {//此时说明 50高度以外30秒以内  当前节点没有满足质押和投资的节点  这时可以发起质押操作
	// 	type = TxHelper::vrfAgentType_local;
	// }

	// if (type == TxHelper::vrfAgentType_local || type == TxHelper::vrfAgentType_vrf)
	// {
	// 	cost = gas;
	// }

	//expend += cost + gasTotal;


	expend += gas;
	//判断utxo的是否够花费的
	if(total < expend)
	{
		//ERRORLOG("The total cost = {} is less than the cost = {}", total, expend);
		return -11;
	}

	CTxOutput * vout = txUtxo->add_vout(); //vout[0]
	vout->set_addr(global::ca::kVirtualStakeAddr);
	vout->set_value(stake_amount);

	CTxOutput * voutFromAddr = txUtxo->add_vout();//vout[1]
	voutFromAddr->set_addr(fromAddr);
	voutFromAddr->set_value(total - expend);

	CTxOutput * vout_burn = txUtxo->add_vout();  //vout[2]
	vout_burn->set_addr(global::ca::kVirtualBurnGasAddr);
	vout_burn->set_value(gas);

	std::string serUtxoHash = getsha256hash(txUtxo->SerializeAsString());
	for (auto & owner : setTxowners)
	{	
		if (TxHelper::AddMutilSign(owner, outTx) != 0)
		{
			return -12;
		}
	}

	// outTx.set_gas(gas);
	// outTx.set_cost(cost);
	outTx.set_version(0);
	outTx.set_time(current_time);
	outTx.set_consensus(global::ca::kConsensus);
	outTx.set_txtype((uint32_t)global::ca::TxType::kTxTypeStake);

	//判断是否代发 默认或本地代发
	//Determine whether dropshipping is default or local dropshipping
	if(type == TxHelper::vrfAgentType_local)
	{

		std::vector<SDKNodeInfo> nodelist;
		for(int i = 0;i<SDKAckMsg->nodeinfo_size();++i)
		{
			SDKNodeInfo node = SDKAckMsg->nodeinfo(i);
			nodelist.emplace_back(node);
		}
		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<size_t> dist(0, nodelist.size() - 1);
   		size_t random = dist(engine);
		std::string base58 = nodelist.at(random).base58addr();
		//outTx.set_identity("13h8j1x9LsQzW1Y4D2FDrkgqB7avvr7jhM"); 
		//string ip = "192.168.1.63";
		outTx.set_identity(base58);	
		std::string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		std::cout<<"type = "<<type <<std::endl;
		std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		//MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		MagicSingleton<net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		node_ip_port = std::make_pair(ip,nodelist.at(random).listen_port());
		sleep(2);
	}
	else
	{
		//选择代发人
		//Select dropshippers
		std::string allUtxos;
		for(auto & utxo:setOutUtxos)
		{
			allUtxos+=utxo.hash;
		}
		allUtxos += std::to_string(current_time);
		
		std::string id;
		
		int ret = GetBlockPackager(id,allUtxos, info_,SDKAckMsg,node_ip_port);
    	if(ret!=0)
		{
        	return ret;
    	}
		outTx.set_identity(id);
	}

	std::string txHash = getsha256hash(outTx.SerializeAsString());
	outTx.set_hash(txHash);
	return 0;
}


int TxHelper::CreatUnstakeTransaction(const std::string& fromAddr,
										const std::string& utxo_hash,
										uint64_t height,
										CTransaction& outTx,
										std::vector<TxHelper::Utxo> & outVin
										,TxHelper::vrfAgentType &type ,Vrf & info_,
										const std::shared_ptr<GetSDKAck> &SDKAckMsg,
										std::pair<std::string, uint64_t> &node_ip_port)
{
	//Check parameters
	std::vector<std::string> vecfromAddr;
	vecfromAddr.push_back(fromAddr);
	int ret = Check(vecfromAddr, height);
	if(ret != 0)
	{
		//ERRORLOG(RED "Check parameters failed! The error code is {}." RESET, ret);
		ret -= 100;
		return ret;
	}

	if (CheckBase58Addr(fromAddr, Base58Ver::kBase58Ver_MultiSign) == true)
	{
		//ERRORLOG(RED "FromAddr is not normal base58 addr." RESET);
		return -1;
	}


	uint64_t stake_amount = 0;
	ret = IsQualifiedToUnstake(fromAddr, utxo_hash, stake_amount,SDKAckMsg);
	if(ret != 0)
	{
		//ERRORLOG(RED "FromAddr is not qualified to unstake! The error code is {}." RESET, ret);
		ret -= 200;
		return ret;
	}	

	// Find utxo
	uint64_t total = 0;
	std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare> setOutUtxos;
	// The number of utxos to be searched here needs to be reduced by 1 \
	because a VIN to be redeem is from the pledged utxo, so just look for 99
	
	ret = FindUtxo(vecfromAddr, TxHelper::kMaxVinSize - 1, total, setOutUtxos,SDKAckMsg); 
	if (ret != 0)
	{
		//ERRORLOG(RED "FindUtxo failed! The error code is {}." RESET, ret);
		ret -= 300;
		return ret;
	}

	if (setOutUtxos.empty())
	{
		//ERRORLOG(RED "Utxo is empty!" RESET);
		return -2;
	}

	outTx.Clear();

	CTxUtxo * txUtxo = outTx.mutable_utxo();
	
	// Fill Vin
	std::set<string> setTxowners;
	for (auto & utxo : setOutUtxos)
	{
		setTxowners.insert(utxo.addr);
	}
	if (setTxowners.empty())
	{
		//ERRORLOG(RED "Tx owner is empty!" RESET);
		return -3;
	}

	{
		// Fill vin
		txUtxo->add_owner(vecfromAddr.at(0));
		CTxInput* txin = txUtxo->add_vin();
		txin->set_sequence(0);
		CTxPrevOutput* prevout = txin->add_prevout();
		prevout->set_hash(utxo_hash);
		prevout->set_n(1);

		std::string serVinHash = getsha256hash(txin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(fromAddr, serVinHash, signature, pub) != 0)
		{
			return -4;
		}

		CSign * vinSign = txin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}


	for (auto & owner : setTxowners)
	{
		txUtxo->add_owner(owner);
		uint32_t n = 1;
		CTxInput * vin = txUtxo->add_vin();
		for (auto & utxo : setOutUtxos)
		{
			if (owner == utxo.addr)
			{
				CTxPrevOutput * prevOutput = vin->add_prevout();
				prevOutput->set_hash(utxo.hash);
				prevOutput->set_n(utxo.n);
			}
		}
		vin->set_sequence(n++);

		std::string serVinHash = getsha256hash(vin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(owner, serVinHash, signature, pub) != 0)
		{
			return -5;
		}

		CSign * vinSign = vin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}


	nlohmann::json txInfo;
	txInfo["UnstakeUtxo"] = utxo_hash;

	nlohmann::json data;
	data["TxInfo"] = txInfo;
	outTx.set_data(data.dump());
	outTx.set_type(global::ca::kTxSign);	
	outTx.set_version(0);
	

	uint64_t gas = 0;
	//填充数量 只参与计算  不影响其他
	std::map<std::string, int64_t> toAddr;
	toAddr.insert(std::make_pair(global::ca::kVirtualStakeAddr, stake_amount));
	toAddr.insert(std::make_pair(fromAddr, total));
	toAddr.insert(std::make_pair(global::ca::kVirtualBurnGasAddr, gas));


	if(GenerateGas(outTx, toAddr, gas) != 0)
	{
		//ERRORLOG(" gas = 0 !");
		return -6;
	}
	auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getgasptr();
    if(call_back == nullptr)
    {
        return -7;
    }
	call_back(gas);
	// Calculate total expenditure
	// uint64_t gasTtoal = (global::ca::kConsensus - 1) * gas;

	// uint64_t cost = 0;//Packing fee

	auto current_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	
	GetTxStartIdentity(vecfromAddr,height,current_time,type,SDKAckMsg);
	if(type == TxHelper::vrfAgentType_unknow)
	{//此时说明 50高度以外30秒以内  当前节点没有满足质押并且投资的节点  这时可以发起解质押操作
		type = TxHelper::vrfAgentType_local;
	}

	// if (type == TxHelper::vrfAgentType_local || type == TxHelper::vrfAgentType_vrf)
	// {
	// 	cost = gas;
	// }

	//uint64_t expend = gasTtoal + cost;

	uint64_t expend = gas;
	//判断有没有足够的钱
	if(total < expend)
	{
		//ERRORLOG("The total cost = {} is less than the cost = {}", total, expend);
		return -8;
	}


	// Fill vout
	CTxOutput* txoutToAddr = txUtxo->add_vout();
	txoutToAddr->set_addr(fromAddr);      	// Release the pledge to my account number
	txoutToAddr->set_value(stake_amount);

	txoutToAddr = txUtxo->add_vout();
	txoutToAddr->set_addr(fromAddr);  		// Give myself the rest
	txoutToAddr->set_value(total - expend);


	CTxOutput * vout_burn = txUtxo->add_vout();
	vout_burn->set_addr(global::ca::kVirtualBurnGasAddr);
	vout_burn->set_value(gas);

	std::string serUtxoHash = getsha256hash(txUtxo->SerializeAsString());
	for (auto & owner : setTxowners)
	{	
		if (TxHelper::AddMutilSign(owner, outTx) != 0)
		{
			return -9;
		}
	}

	 outTx.set_time(current_time);
	// outTx.set_cost(cost);
	//outTx.set_gas(gas);
	outTx.set_version(0);
	outTx.set_consensus(global::ca::kConsensus);
	outTx.set_txtype((uint32_t)global::ca::TxType::kTxTypeUnstake);

	//判断是否代发 默认或本地代发
	//Determine whether dropshipping is default or local dropshipping
	if(type == TxHelper::vrfAgentType_defalut || type == TxHelper::vrfAgentType_local)
	{
		std::vector<SDKNodeInfo> nodelist;
		for(int i = 0;i<SDKAckMsg->nodeinfo_size();++i)
		{
			SDKNodeInfo node = SDKAckMsg->nodeinfo(i);
			nodelist.emplace_back(node);
		}
		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<size_t> dist(0, nodelist.size() - 1);
   		size_t random = dist(engine);
		std::string base58 = nodelist.at(random).base58addr();
		outTx.set_identity(base58); 
		
		std::string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		// std::cout<<"type = "<<type <<std::endl;
		// std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		// std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		// std::cout<<"public port = "<<nodelist.at(random).public_port()<<std::endl;
		// MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		// node_ip_port = std::make_pair(IpPort::ipsz(nodelist.at(random).public_ip()),nodelist.at(random).listen_port());



		// outTx.set_identity("13h8j1x9LsQzW1Y4D2FDrkgqB7avvr7jhM"); 
		// std::string ip = "192.168.1.63";
		//outTx.set_identity(base58);	
		//string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		std::cout<<"type = "<<type <<std::endl;
		std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		//MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		MagicSingleton<net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		node_ip_port = std::make_pair(ip,nodelist.at(random).listen_port());
		sleep(2);
	}
	else
	{
		
		//选择代发人
		//Select dropshippers
		std::string allUtxos = utxo_hash;
		for(auto & utxo:setOutUtxos){
			allUtxos+=utxo.hash;
		}
		allUtxos += std::to_string(current_time);
		
		std::string id;
    	int ret= GetBlockPackager(id,allUtxos,info_,SDKAckMsg,node_ip_port);
    	if(ret!=0)
		{
        	return ret;
    	}
		outTx.set_identity(id);
	}
	std::string txHash = getsha256hash(outTx.SerializeAsString());
	outTx.set_hash(txHash);
	return 0;
}

int TxHelper::CreateInvestTransaction(const std::string& fromAddr,
										const std::string& toAddr,
										uint64_t invest_amount,
										uint64_t height,
										TxHelper::InvestType investType,
										CTransaction & outTx,
										std::vector<TxHelper::Utxo> & outVin
										,TxHelper::vrfAgentType &type ,Vrf & info_,
										const std::shared_ptr<GetSDKAck> &SDKAckMsg,
										std::pair<std::string, uint64_t> &node_ip_port)
{
	//Check parameters
	std::vector<std::string> vecfromAddr;
	vecfromAddr.push_back(fromAddr);
	int ret = Check(vecfromAddr, height);
	if(ret != 0)
	{
		//ERRORLOG(RED "Check parameters failed! The error code is {}." RESET, ret);
		ret -= 100;
		return ret;
	}

	// Neither fromaddr nor toaddr can be a virtual account
	if (CheckBase58Addr(fromAddr, Base58Ver::kBase58Ver_MultiSign) == true)
	{
		//ERRORLOG(RED "FromAddr is not normal base58 addr." RESET);
		return -1;
	}

	if (CheckBase58Addr(toAddr, Base58Ver::kBase58Ver_MultiSign) == true)
	{
		//ERRORLOG(RED "To address is not base58 address!" RESET);
		return -2;
	}

	
	if(invest_amount < 35 * global::ca::kDecimalNum || invest_amount > 65000 * global::ca::kDecimalNum)
	{
		//ERRORLOG("Investment amount exceeds the limit!");
		cout<<"Investment amount exceeds the limit"<<endl;
		return -3;
	}

	// if(invest_amount < 500 * global::ca::kDecimalNum)
	// {
	// 	//ERRORLOG("Invest less 500!");
	// 	cout<<"Invest less 500!"<<endl;
	// 	return -3;
	// }
	// uint64_t stake_count=0;
	// SearchStake(fromAddr,stake_count, global::ca::StakeType::kStakeType_Node,SDKAckMsg);
	// if(stake_count+ invest_amount> 100000 * global::ca::kDecimalNum )
	// {
	// 	//ERRORLOG("Invest add Stake more than 100000!");
	// 	cout<<"Invest add Stake more than 100000!"<<endl;
	// 	return -4;
	// }


	ret = CheckInvestQualification(fromAddr, toAddr, invest_amount,SDKAckMsg);
	if(ret != 0)
	{
		//ERRORLOG(RED "FromAddr is not qualified to invest! The error code is {}." RESET, ret);
		ret -= 200;
		return ret;
	}	
	std::string strinvestType;
	if (investType ==  TxHelper::InvestType::kInvestType_NetLicence)
	{
		strinvestType = global::ca::kInvestTypeNormal;
	}
	else
	{
		//ERRORLOG(RED "Unknown invest type!" RESET);
		return -3;
	}
	
	
	// Find utxo
	uint64_t total = 0;
	uint64_t expend = invest_amount;

	std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare> setOutUtxos;
	ret = FindUtxo(vecfromAddr, TxHelper::kMaxVinSize, total, setOutUtxos,SDKAckMsg);
	if (ret != 0)
	{
		//ERRORLOG(RED "FindUtxo failed! The error code is {}." RESET, ret);
		ret -= 300;
		return ret;
	}
	if (setOutUtxos.empty())
	{
		//ERRORLOG(RED "Utxo is empty!" RESET);
		return -4;
	}

	outTx.Clear();

	CTxUtxo * txUtxo = outTx.mutable_utxo();
	
	// Fill Vin
	std::set<string> setTxowners;
	for (auto & utxo : setOutUtxos)
	{
		setTxowners.insert(utxo.addr);
	}
	if (setTxowners.empty())
	{
		//ERRORLOG(RED "Tx owner is empty!" RESET);
		return -5;
	}

	for (auto & owner : setTxowners)
	{
		txUtxo->add_owner(owner);
		uint32_t n = 0;
		CTxInput * vin = txUtxo->add_vin();
		for (auto & utxo : setOutUtxos)
		{
			if (owner == utxo.addr)
			{
				CTxPrevOutput * prevOutput = vin->add_prevout();
				prevOutput->set_hash(utxo.hash);
				prevOutput->set_n(utxo.n);
			}
		}
		vin->set_sequence(n++);

		std::string serVinHash = getsha256hash(vin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(owner, serVinHash, signature, pub) != 0)
		{
			return -6;
		}

		CSign * vinSign = vin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}

	nlohmann::json txInfo;
	txInfo["InvestType"] = strinvestType;
	txInfo["BonusAddr"] = toAddr;
	txInfo["InvestAmount"] = invest_amount;

	nlohmann::json data;
	data["TxInfo"] = txInfo;
	outTx.set_data(data.dump());
	outTx.set_type(global::ca::kTxSign);


	uint64_t gas = 0;
	// Calculate total expenditure
	std::map<std::string, int64_t> toAddrs;
	toAddrs.insert(std::make_pair(global::ca::kVirtualStakeAddr, invest_amount));
	toAddrs.insert(std::make_pair(fromAddr, total - expend));
	toAddrs.insert(std::make_pair(global::ca::kVirtualBurnGasAddr, gas));

	if(GenerateGas(outTx, toAddrs, gas) != 0)
	{
		std::cout << "GenerateGas gas = " << gas << std::endl;
		//ERRORLOG(" gas = 0 !");
		return -7;
	}
	auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getgasptr();
    if(call_back == nullptr)
    {
        return -8;
    }
	call_back(gas);
	// Calculate total expenditure
	//uint64_t gasTotal = (global::ca::kConsensus - 1) * gas;

	//uint64_t cost = 0;//Packing fee
	auto current_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();

	GetTxStartIdentity(vecfromAddr,height,current_time,type,SDKAckMsg);
	// if(type == TxHelper::vrfAgentType_unknow)
	// {//此时说明 50高度以外30秒以内  当前节点没有满足质押并且投资的节点  这时可以发起投资操作
	// 	type = TxHelper::vrfAgentType_local;
	// }
	// if (type == TxHelper::vrfAgentType_local || type == TxHelper::vrfAgentType_vrf)
	// {
	// 	cost = gas;
	// }

	expend +=  gas;

	if(total < expend)
	{
		//ERRORLOG("The total cost = {} is less than the cost = {}", total, expend);
		return -9;
	}

	CTxOutput * vout = txUtxo->add_vout(); //vout[0]
	vout->set_addr(global::ca::kVirtualInvestAddr);
	vout->set_value(invest_amount);

	CTxOutput * voutFromAddr = txUtxo->add_vout();//vout[1]
	voutFromAddr->set_addr(fromAddr);
	voutFromAddr->set_value(total - expend);

	CTxOutput * vout_burn = txUtxo->add_vout();
	vout_burn->set_addr(global::ca::kVirtualBurnGasAddr);
	vout_burn->set_value(gas);

	std::string serUtxoHash = getsha256hash(txUtxo->SerializeAsString());
	for (auto & owner : setTxowners)
	{	
		if (TxHelper::AddMutilSign(owner, outTx) != 0)
		{
			return -10;
		}
	}
	
	// outTx.set_gas(gas);
	// outTx.set_cost(cost);
	outTx.set_version(0);
	outTx.set_time(current_time);
	outTx.set_consensus(global::ca::kConsensus);
	outTx.set_txtype((uint32_t)global::ca::TxType::kTxTypeInvest);

	//判断是否代发 默认或本地代发
	//Determine whether dropshipping is default or local dropshipping
	if( type == TxHelper::vrfAgentType_local)
	{
		std::vector<SDKNodeInfo> nodelist;
		for(int i = 0;i<SDKAckMsg->nodeinfo_size();++i)
		{
			SDKNodeInfo node = SDKAckMsg->nodeinfo(i);
			nodelist.emplace_back(node);
		}
		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<size_t> dist(0, nodelist.size() - 1);
   		size_t random = dist(engine);


		std::string base58 = nodelist.at(random).base58addr();
		outTx.set_identity(base58); 
		string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		// std::cout<<"type = "<<type <<std::endl;
		// std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		// std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		// std::cout<<"public port = "<<nodelist.at(random).public_port()<<std::endl;
		// MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		// node_ip_port = std::make_pair(IpPort::ipsz(nodelist.at(random).public_ip()),nodelist.at(random).listen_port());



		// outTx.set_identity("13h8j1x9LsQzW1Y4D2FDrkgqB7avvr7jhM"); 
		// string ip = "192.168.1.63";
		//outTx.set_identity(base58);	
		//string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		std::cout<<"type = "<<type <<std::endl;
		std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		//MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		MagicSingleton<net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		node_ip_port = std::make_pair(ip,nodelist.at(random).listen_port());
		sleep(2);
	}
	else
	{
		
		//选择代发人
		//Select dropshippers
		std::string allUtxos;
		for(auto & utxo:setOutUtxos)
		{
			allUtxos+=utxo.hash;
		}
		allUtxos += std::to_string(current_time);
		
		std::string id;
    	int ret= GetBlockPackager(id,allUtxos,info_,SDKAckMsg,node_ip_port);
    	if(ret!=0)
		{
        	return ret;
    	}
		outTx.set_identity(id);
	}

	std::string txHash = getsha256hash(outTx.SerializeAsString());
	outTx.set_hash(txHash);
	return 0;
}



int TxHelper::CreateDisinvestTransaction(const std::string& fromAddr,
										const std::string& toAddr,
										const std::string& utxo_hash,
										uint64_t height,
										CTransaction& outTx,
										std::vector<TxHelper::Utxo> & outVin
										,TxHelper::vrfAgentType &type ,Vrf & info_,
										const std::shared_ptr<GetSDKAck> &SDKAckMsg,
										std::pair<std::string, uint64_t> &node_ip_port)
{
	//Check parameters
	std::vector<std::string> vecfromAddr;
	vecfromAddr.push_back(fromAddr);
	int ret = Check(vecfromAddr, height);
	if(ret != 0)
	{
		//ERRORLOG(RED "Check parameters failed! The error code is {}." RESET, ret);
		ret -= 100;
		return ret;
	}

	if (CheckBase58Addr(fromAddr, Base58Ver::kBase58Ver_MultiSign) == true)
	{
		//ERRORLOG(RED "FromAddr is not normal base58 addr." RESET);
		return -1;
	}

	if (CheckBase58Addr(toAddr, Base58Ver::kBase58Ver_MultiSign) == true)
	{
		//ERRORLOG(RED "To address is not base58 address!" RESET);
		return -2;
	}

	uint64_t invested_amount = 0;
	if(IsQualifiedToDisinvest(fromAddr, toAddr, utxo_hash, invested_amount,SDKAckMsg) != 0)
	{
		//ERRORLOG(RED "FromAddr is not qualified to divest!." RESET);
		return -3;
	}

	// Find utxo
	uint64_t total = 0;
	std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare> setOutUtxos;
	// The utxo quantity sought here needs to be reduced by 1
	ret = FindUtxo(vecfromAddr, TxHelper::kMaxVinSize - 1, total, setOutUtxos,SDKAckMsg); 
	if (ret != 0)
	{
		//ERRORLOG(RED "FindUtxo failed! The error code is {}." RESET, ret);
		ret -= 300;
		return ret;
	}

	if (setOutUtxos.empty())
	{
		//ERRORLOG(RED "Utxo is empty!" RESET);
		return -4;
	}

	outTx.Clear();

	CTxUtxo * txUtxo = outTx.mutable_utxo();

	// Fill Vin
	std::set<string> setTxowners;
	for (auto & utxo : setOutUtxos)
	{
		setTxowners.insert(utxo.addr);
	}
	if (setTxowners.empty())
	{
		//ERRORLOG(RED "Tx owner is empty!" RESET);
		return -5;
	}

	{
		// Fill vin
		txUtxo->add_owner(vecfromAddr.at(0));
		CTxInput* txin = txUtxo->add_vin();
		txin->set_sequence(0);
		CTxPrevOutput* prevout = txin->add_prevout();
		prevout->set_hash(utxo_hash);
		prevout->set_n(1);

		std::string serVinHash = getsha256hash(txin->SerializeAsString());
		std::string signature;
		std::string pub;
		ret = TxHelper::Sign(fromAddr, serVinHash, signature, pub);
		if (ret != 0)
		{
			//ERRORLOG("invest utxo_hash Sign error:{}", ret);
			return -6;
		}

		CSign * vinSign = txin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}


	for (auto & owner : setTxowners)
	{
		txUtxo->add_owner(owner);
		uint32_t n = 1;
		CTxInput * vin = txUtxo->add_vin();
		for (auto & utxo : setOutUtxos)
		{
			if (owner == utxo.addr)
			{
				CTxPrevOutput * prevOutput = vin->add_prevout();
				prevOutput->set_hash(utxo.hash);
				prevOutput->set_n(utxo.n);
			}
		}
		vin->set_sequence(n++);

		std::string serVinHash = getsha256hash(vin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(owner, serVinHash, signature, pub) != 0)
		{
			return -7;
		}

		CSign * vinSign = vin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}

	nlohmann::json txInfo;
	txInfo["BonusAddr"] = toAddr;
	txInfo["DisinvestUtxo"] = utxo_hash;

	nlohmann::json data;
	data["TxInfo"] = txInfo;
	outTx.set_data(data.dump());
	outTx.set_type(global::ca::kTxSign);	


	uint64_t gas = 0;
	// Calculate total expenditure
	std::map<std::string, int64_t> targetAddrs;
	targetAddrs.insert(std::make_pair(global::ca::kVirtualStakeAddr, invested_amount));
	targetAddrs.insert(std::make_pair(fromAddr, total ));
	targetAddrs.insert(std::make_pair(global::ca::kVirtualBurnGasAddr, gas ));



	
	if(GenerateGas(outTx, targetAddrs, gas) != 0)
	{
		//ERRORLOG(" gas = 0 !");
		return -8;
	}

	// uint64_t gasTotal = (global::ca::kConsensus - 1) * gas;

	// uint64_t cost = 0;//Packing fee

	auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getgasptr();
    if(call_back == nullptr)
    {
        return -9;
    }
	call_back(gas);
	auto current_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	GetTxStartIdentity(vecfromAddr,height,current_time,type,SDKAckMsg);
	// if(type == TxHelper::vrfAgentType_unknow)
	// {//此时说明 50高度以外30秒以内  当前节点没有满足质押并且投资的节点  这时可以发起解投资操作
	// 	type = TxHelper::vrfAgentType_local;
	// }

	// if (type == TxHelper::vrfAgentType_local || type == TxHelper::vrfAgentType_vrf)
	// {
	// 	cost = gas;
	// }

	uint64_t expend = gas;
	
	if(total < expend)
	{
		//ERRORLOG("The total cost = {} is less than the cost = {}", total, expend);
		return -10;
	}	

	//Fill vout
	CTxOutput* txoutToAddr = txUtxo->add_vout();
	txoutToAddr->set_addr(fromAddr);      // Give my account the money I withdraw
	txoutToAddr->set_value(invested_amount);

	txoutToAddr = txUtxo->add_vout();
	txoutToAddr->set_addr(fromAddr);  	  // Give myself the rest
	txoutToAddr->set_value(total - expend);


	CTxOutput * vout_burn = txUtxo->add_vout();
	vout_burn->set_addr(global::ca::kVirtualBurnGasAddr);
	vout_burn->set_value(gas);

	std::string serUtxoHash = getsha256hash(txUtxo->SerializeAsString());
	for (auto & owner : setTxowners)
	{	
		if (TxHelper::AddMutilSign(owner, outTx) != 0)
		{
			return -11;
		}
	}

	outTx.set_time(current_time);
	outTx.set_version(0);
	// outTx.set_gas(gas);
	// outTx.set_cost(cost);
	outTx.set_consensus(global::ca::kConsensus);
	outTx.set_txtype((uint32_t)global::ca::TxType::kTxTypeDisinvest);

	//判断是否代发 默认或本地代发
	//Determine whether dropshipping is default or local dropshipping
	if(type == TxHelper::vrfAgentType_defalut || type == TxHelper::vrfAgentType_local)
	{
		std::vector<SDKNodeInfo> nodelist;
		for(int i = 0;i<SDKAckMsg->nodeinfo_size();++i)
		{
			SDKNodeInfo node = SDKAckMsg->nodeinfo(i);
			nodelist.emplace_back(node);
		}
		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<size_t> dist(0, nodelist.size() - 1);
   		size_t random = dist(engine);


		std::string base58 = nodelist.at(random).base58addr();
		outTx.set_identity(base58); 
		string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		// std::cout<<"type = "<<type <<std::endl;
		// std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		// std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		// std::cout<<"public port = "<<nodelist.at(random).public_port()<<std::endl;
		// MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		// node_ip_port = std::make_pair(IpPort::ipsz(nodelist.at(random).public_ip()),nodelist.at(random).listen_port());
		



		// outTx.set_identity("13h8j1x9LsQzW1Y4D2FDrkgqB7avvr7jhM"); 
		// string ip = "192.168.1.63";
		//outTx.set_identity(base58);	
		//string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		std::cout<<"type = "<<type <<std::endl;
		std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		//MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		MagicSingleton<net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		node_ip_port = std::make_pair(ip,nodelist.at(random).listen_port());
		sleep(2);
	}
	else
	{
		
		//选择代发人
		//Select dropshippers
		std::string allUtxos = utxo_hash;
		for(auto & utxo:setOutUtxos)
		{
			allUtxos+=utxo.hash;
		}
		allUtxos += std::to_string(current_time);
		
		std::string id;
    	int ret= GetBlockPackager(id,allUtxos,info_,SDKAckMsg,node_ip_port);
    	if(ret!=0)
		{
        	return ret;
    	}
		outTx.set_identity(id);		
	}

	std::string txHash = getsha256hash(outTx.SerializeAsString());
	outTx.set_hash(txHash);
	return 0;
}




int TxHelper::CreateBonusTransaction(const std::string& Addr,
										uint64_t height,
										CTransaction& outTx,
										std::vector<TxHelper::Utxo> & outVin,
										TxHelper::vrfAgentType &type,
										Vrf & info_,
										const std::shared_ptr<GetSDKAck> &SDKAckMsg,
										uint64_t cur_time,
										std::pair<std::string, uint64_t> &node_ip_port)
{
	std::vector<std::string> vecfromAddr;
	vecfromAddr.push_back(Addr);
	int ret = Check(vecfromAddr, height);
	if(ret != 0)
	{
		//ERRORLOG("Check parameters failed");
		ret -= 100;
		return ret;
	}

	if (CheckBase58Addr(Addr, Base58Ver::kBase58Ver_MultiSign) == true)
	{
		//ERRORLOG(RED "Default is not normal base58 addr." RESET);
		return -1;
	}

	// CBlockDataApi data_reader;
	// DBReader db_reader; 
	std::vector<std::string> utxos;
	//uint64_t cur_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	//uint64_t Period = MagicSingleton<TimeUtil>::GetInstance()->getPeriod(cur_time);
	uint64_t zero_time = MagicSingleton<TimeUtil>::GetInstance()->getMorningTime(cur_time)*1000000;//Convert to subtle
//    auto status = db_reader.GetBonusUtxoByPeriod(Period, utxos);
// 	if (status != DBStatus::DB_SUCCESS && status != DBStatus::DB_NOT_FOUND)
// 	{
// 		ERRORLOG("TxHelper CreatUnstakeTransaction: Get all pledge address failed");
// 		return -2;
// 	}
	
	if(cur_time < ( zero_time + 60 * 60 * 1000000ul ))
	{
		std::cout << RED << "Claim after 1 a.m!" << RESET << std::endl;
		return -3;
	}

	// Application completed
	// if(status == DBStatus::DB_SUCCESS)
	// {
	// 	std::string strTx;
	// 	CTransaction Claimtx;
		
	// 	for(auto utxo = utxos.rbegin(); utxo != utxos.rend(); utxo++)
	// 	{
	// 		if (db_reader.GetTransactionByHash(*utxo, strTx) != DBStatus::DB_SUCCESS)
	// 		{
	// 			MagicSingleton<BlockHelper>::GetInstance()->PushMissUTXO(*utxo);
	// 			return -4;
	// 		}	
	// 		if(!Claimtx.ParseFromString(strTx))
	// 		{
	// 			return -5;
	// 		}
	// 		std::string ClaimAddr = GetBase58Addr(Claimtx.utxo().vin(0).vinsign().pub());
	// 		if(Addr == ClaimAddr)
	// 		{
	// 			std::cout << RED << "Application completed!" << RESET << std::endl;
	// 			return -6;
	// 		}
	// 	}
	// }
	
	// CTransaction tx;
	// for(int i = 0;i<SDKAckMsg->claimtx_size();++i)
	// {
	// 	if(SDKAckMsg->claimtx(i).address() == Addr)
	// 	{
	// 		tx = SDKAckMsg->claimtx(i).tx();
	// 		std::string ClaimAddr = GetBase58Addr(tx.utxo().vin(0).vinsign().pub());
	// 		if(Addr == ClaimAddr)
	// 		{
	// 			std::cout << RED << "Application completed!" << RESET << std::endl;
	// 			return -6;
	// 		}
	// 	}
	// }

	CTransaction tx;
	for(int i = 0;i <SDKAckMsg->claimtx_size();++i)
	{
		Claimtx * claimtx = SDKAckMsg->mutable_claimtx(i);
    	tx.ParseFromString(claimtx->tx());
		
		std::string ClaimAddr = GetBase58Addr(tx.utxo().vin(0).vinsign().pub());
		if(Addr == ClaimAddr)
		{
			std::cout << RED << "Application completed!" << RESET << std::endl;
			return -6;
		}
	}
	

	
	
	
	//The total number of investors must be more than 10 before they can apply for it
	ret = VerifyBonusAddr(Addr,SDKAckMsg);
	if(ret < 0)
	{
		return -7;
	}

	std::map<std::string, uint64_t> CompanyDividend;

    ret=ca_algorithm::CalcBonusValue(cur_time, Addr, CompanyDividend,SDKAckMsg);
	if(ret < 0)
	{
		//ERRORLOG("Failed to obtain the amount claimed by the investor ret:({})",ret);
		ret-=300;
		return ret;
	}

	uint64_t expend = 0;
	uint64_t total = 0;
	std::multiset<TxHelper::Utxo, TxHelper::UtxoCompare> setOutUtxos;
	ret = FindUtxo(vecfromAddr, TxHelper::kMaxVinSize - 1, total, setOutUtxos,SDKAckMsg);
	if (ret != 0)
	{
		//ERRORLOG("TxHelper CreatUnstakeTransaction: FindUtxo failed");
		ret -= 200;
		return ret;
	}
	if (setOutUtxos.empty())
	{
		//ERRORLOG("TxHelper CreatUnstakeTransaction: utxo is zero");
		return -8;
	}

	outTx.Clear();

	CTxUtxo * txUtxo = outTx.mutable_utxo();


	// Fill Vin
	std::set<string> setTxowners;
	for (auto & utxo : setOutUtxos)
	{
		setTxowners.insert(utxo.addr);
	}
	if (setTxowners.empty())
	{
		//ERRORLOG(RED "Tx owner is empty!" RESET);
		return -9;
	}

	for (auto & owner : setTxowners)
	{
		txUtxo->add_owner(owner);
		uint32_t n = 0;
		CTxInput * vin = txUtxo->add_vin();
		for (auto & utxo : setOutUtxos)
		{
			if (owner == utxo.addr)
			{
				CTxPrevOutput * prevOutput = vin->add_prevout();
				prevOutput->set_hash(utxo.hash);
				prevOutput->set_n(utxo.n);
			}
		}
		vin->set_sequence(n++);

		std::string serVinHash = getsha256hash(vin->SerializeAsString());
		std::string signature;
		std::string pub;
		if (TxHelper::Sign(owner, serVinHash, signature, pub) != 0)
		{
			return -10;
		}

		CSign * vinSign = vin->mutable_vinsign();
		vinSign->set_sign(signature);
		vinSign->set_pub(pub);
	}

	//Fill data

	uint64_t tempCosto=0;
	uint64_t tempNodeDividend=0;
	uint64_t tempTotalClaim=0;
	for(auto Company : CompanyDividend)
	{
		tempCosto=Company.second*0.05+0.5;
		tempNodeDividend+=tempCosto;
		std::string addr = Company.first;
		uint64_t award = Company.second - tempCosto;
		tempTotalClaim+=award;		
	}
	tempTotalClaim += tempNodeDividend;

	nlohmann::json txInfo;
	txInfo["BonusAmount"] = tempTotalClaim;
	txInfo["BonusAddrList"] = CompanyDividend.size() + 1;

	nlohmann::json data;
	data["TxInfo"] = txInfo;
	outTx.set_data(data.dump());
	outTx.set_type(global::ca::kTxSign);

	//calculation gas
	uint64_t gas = 0;
	std::map<std::string, int64_t> toAddrs;
	for(const auto & item : CompanyDividend)
	{
		toAddrs.insert(make_pair(item.first, item.second));
	}
	toAddrs.insert(std::make_pair(global::ca::kVirtualStakeAddr, total - expend));
	toAddrs.insert(std::make_pair(global::ca::kVirtualBurnGasAddr, gas));



	if(GenerateGas(outTx, toAddrs, gas) != 0)
	{
		//ERRORLOG(" gas = 0 !");
		return -11;
	}
	auto call_back =  MagicSingleton<Recver>::GetInstance()->phone_getgasptr();
    if(call_back == nullptr)
    {
        return -12;
    }
	call_back(gas);
	// uint64_t gasTotal = (global::ca::kConsensus - 1) * gas;
	// uint64_t cost = 0;

	auto current_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
	GetTxStartIdentity(vecfromAddr,height,current_time,type,SDKAckMsg);
	if(type == TxHelper::vrfAgentType_unknow)
	{
		//ERRORLOG(" +++++++vrfAgentType_unknow +++++");
		return -300;
	}

	// if (type == TxHelper::vrfAgentType_local || type == TxHelper::vrfAgentType_vrf)
	// {
	// 	cost = gas;
	// }


	expend += gas;

	if(total < expend)
	{
		//ERRORLOG("The total cost = {} is less than the cost = {}", total, expend);
		return -13;
	}

	outTx.set_time(current_time);
	outTx.set_version(0);
	// outTx.set_cost(cost);
	// outTx.set_gas(gas);
	outTx.set_consensus(global::ca::kConsensus);
	outTx.set_txtype((uint32_t)global::ca::TxType::kTxTypeBonus);

	//填充vout

	uint64_t costo=0;
	uint64_t NodeDividend=0;
	uint64_t TotalClaim=0;
	std::cout << YELLOW << "Claim Addr : Claim Amount" << RESET << std::endl;
	for(auto Company : CompanyDividend)
	{
		costo=Company.second*0.05+0.5;
		NodeDividend+=costo;
		std::string addr = Company.first;
		uint64_t award = Company.second - costo;
		TotalClaim+=award;
		CTxOutput* txoutToAddr = txUtxo->add_vout();	
		txoutToAddr->set_addr(addr); 
		txoutToAddr->set_value(award);		
		std::cout << Company.first << ":" << Company.second << std::endl;		
	}

	CTxOutput* txoutToAddr = txUtxo->add_vout();
	txoutToAddr->set_addr(Addr);
	txoutToAddr->set_value(total - expend + NodeDividend);

	CTxOutput * vout_burn = txUtxo->add_vout();
	vout_burn->set_addr(global::ca::kVirtualBurnGasAddr);
	vout_burn->set_value(gas);


	std::cout << Addr << ":" << NodeDividend << std::endl;
	TotalClaim+=NodeDividend;
	if(TotalClaim == 0)
	{
		//ERRORLOG("The claim amount is 0");
		return -14;
	}

	// uint64_t MiningBalance=0;
	// {
	// 	std::lock_guard<std::mutex> lock(global::ca::kBonusMutex);
	// 	MiningBalance =  SDKAckMsg->totalawardamount();
	// 	// if (DBStatus::DB_SUCCESS != db_reader.GetTotalAwardAmount(MiningBalance))
	// 	// {
	// 	// 	return -14;
	// 	// }
	// }

	// if(MiningBalance-TotalClaim < 0) 
	// {
	// 	return -15;
	// }

	std::string serUtxoHash = getsha256hash(txUtxo->SerializeAsString());
	for (auto & owner : setTxowners)
	{	
		if (TxHelper::AddMutilSign(owner, outTx) != 0)
		{
			return -15;
		}
	}

	//判断是否代发 默认或本地代发
	//Determine whether dropshipping is default or local dropshipping
	if(type == TxHelper::vrfAgentType_defalut || type == TxHelper::vrfAgentType_local)
	{
		std::vector<SDKNodeInfo> nodelist;
		for(int i = 0;i<SDKAckMsg->nodeinfo_size();++i)
		{
			SDKNodeInfo node = SDKAckMsg->nodeinfo(i);
			nodelist.emplace_back(node);
		}
		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<size_t> dist(0, nodelist.size() - 1);
   		size_t random = dist(engine);

		std::string base58 = nodelist.at(random).base58addr();
		outTx.set_identity(base58); 
		string ip = IpPort::ipsz(nodelist.at(random).public_ip());
		std::cout<<"type = "<<type <<std::endl;
		std::cout<<"base58addr = "<<nodelist.at(random).base58addr() <<std::endl;
		std::cout<<"public ip = "<<IpPort::ipsz(nodelist.at(random).public_ip())<<std::endl;
		std::cout<<"public port = "<<nodelist.at(random).public_port()<<std::endl;
		//MagicSingleton<Net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		MagicSingleton<net>::GetInstance()->connect(ip, nodelist.at(random).listen_port());
		node_ip_port = std::make_pair(ip,nodelist.at(random).listen_port());
	}
	else
	{
		
		//选择代发人
		//Select dropshippers
		std::string allUtxos;
		for(auto & utxo:setOutUtxos)
		{
			allUtxos+=utxo.hash;
		}
		allUtxos += std::to_string(current_time);
		
		std::string id;
    	int ret= GetBlockPackager(id,allUtxos,info_,SDKAckMsg,node_ip_port);
    	if(ret!=0)
		{
        	return ret;
    	}
		outTx.set_identity(id);
	}

	std::string txHash = getsha256hash(outTx.SerializeAsString());
	outTx.set_hash(txHash);

	return 0;
}




int TxHelper::AddMutilSign(const std::string & addr, CTransaction &tx)
{
	if (!CheckBase58Addr(addr))
	{
		return -1;
	}

	CTxUtxo * txUtxo = tx.mutable_utxo();
	CTxUtxo copyTxUtxo = *txUtxo;
	copyTxUtxo.clear_multisign();

	std::string serTxUtxo = getsha256hash(copyTxUtxo.SerializeAsString());
	std::string signature;
	std::string pub;
	if(TxHelper::Sign(addr, serTxUtxo, signature, pub) != 0)
	{
		return -2;
	}

	CSign * multiSign = txUtxo->add_multisign();
	multiSign->set_sign(signature);
	multiSign->set_pub(pub);

	return 0;
}

int TxHelper::AddVerifySign(const std::string & addr, CTransaction &tx)
{
	if (!CheckBase58Addr(addr))
	{
		//ERRORLOG("illegal address {}", addr);
		return -1;
	}

	CTransaction copyTx = tx;

	copyTx.clear_hash();
	copyTx.clear_verifysign();

	std::string serTx = copyTx.SerializeAsString();
	if(serTx.empty())
	{
		//ERRORLOG("fail to serialize trasaction");
		return -2;
	}

	std::string message = getsha256hash(serTx);

	std::string signature;
	std::string pub;
	if (TxHelper::Sign(addr, message, signature, pub) != 0)
	{
		//ERRORLOG("fail to sign message");
		return -3;
	}

	//DEBUGLOG("-------------------add verify sign addr = {} --------------------------",addr);

	CSign * verifySign = tx.add_verifysign();
	verifySign->set_sign(signature);
	verifySign->set_pub(pub);
	

	return 0;
}




int TxHelper::Sign(const std::string & addr, 
					const std::string & message, 
                    std::string & signature, 
					std::string & pub)
{
	if (addr.empty() || message.empty())
	{
		return -1;
	}

	ED account;
	EVP_PKEY_free(account.pkey);
	if(MagicSingleton<EDManager>::GetInstance()->FindAccount(addr ,account) != 0)
	{
		//ERRORLOG("account {} doesn't exist", addr);
		return -2;
	}

	if(!account.Sign(message,signature))
	{
		return -3;
	}

	pub = account.pubStr;
	return 0;
}

bool TxHelper::IsNeedAgent(const std::vector<std::string> & fromAddr)
{
	bool isNeedAgent = true;
	for(auto& owner : fromAddr)
	{
		// If the transaction owner cannot be found in all accounts of the node, it indicates that it is issued on behalf
		if (owner == MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr())
		{
			isNeedAgent = false;
		}
	}

	return isNeedAgent;

}

bool TxHelper::IsNeedAgent(const CTransaction &tx)
{
	if(std::find(tx.utxo().owner().begin(), tx.utxo().owner().end(),tx.identity()) == tx.utxo().owner().end())
	{
		return true;
	}
	
	return false;
}

bool TxHelper:: checkTxTimeOut(const uint64_t & txTime, const uint64_t & timeout,const uint64_t & pre_height,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	if(txTime <= 0)
	{
		//ERRORLOG("tx time = {} ", txTime);
		return false;
	}
    // DBReader db_reader;

    // std::vector<std::string> block_hashes;
    // if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashesByBlockHeight(pre_height, pre_height, block_hashes))
    // {
    //     ERRORLOG("can't GetBlockHashesByBlockHeight");
    //     return false;
    // }

    std::vector<CBlock> blocks;

	for (size_t i = 0; i < (size_t) SDKAckMsg->blocks_size(); ++i)
	{
		//const SDKBlockItem blockitem = SDKAckMsg->blocks(i);
		CBlock block;
		block.ParseFromString(SDKAckMsg->blocks(i));
		cout<<"block hash = "<<block.hash()<<endl;
		blocks.emplace_back(block);
	}
	

    // for (auto &hash : block_hashes)
    // {
    //     std::string blockStr;
    //     if(DBStatus::DB_SUCCESS != db_reader.GetBlockByBlockHash(hash, blockStr))
	// 	{
	// 		ERRORLOG("GetBlockByBlockHash error block hash = {} ", hash);
	// 		return false;
	// 	}

    //     CBlock block;
    //     if(!block.ParseFromString(blockStr))
	// 	{
	// 		ERRORLOG("block parse from string fail = {} ", blockStr);
	// 		return false;
	// 	}
    //     blocks.push_back(block);
    // }

	std::sort(blocks.begin(), blocks.end(), [](const CBlock& x, const CBlock& y){ return x.time() < y.time(); });
	CBlock result_block = blocks[blocks.size() - 1];

	if(result_block.time() <= 0)
	{
		//ERRORLOG("block time = {}  ", result_block.time());
		return false;
	}

	uint64_t result_time = abs(int64_t(txTime - result_block.time()));
    if (result_time > timeout * 1000000)
    {
		//DEBUGLOG("vrf Issuing transaction More than 30 seconds time = {}, tx time= {}, top = {} ", result_time, txTime, pre_height);
        return true;
    }
    return false;
}

TxHelper::vrfAgentType TxHelper::GetVrfAgentType(const CTransaction &tx, uint64_t &pre_height,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
	std::vector<std::string> owners(tx.utxo().owner().begin(), tx.utxo().owner().end());

	//如果在30s之内并且没找到那就是vrf代发 If it is within 30s and you do not find it, it is VRF dropshipping
	if(!TxHelper::checkTxTimeOut(tx.time(),global::ca::TxTimeoutMin, pre_height,SDKAckMsg))//块不大于30s The block is not larger than 30s
	{	
		if(std::find(owners.begin(), owners.end(), tx.identity()) == owners.end())
		{
			return TxHelper::vrfAgentType::vrfAgentType_vrf;
		}
		return TxHelper::vrfAgentType::vrfAgentType_defalut;
	}
	else
	{
		//超过30秒 发起节点和identity不是一个人就是 本地代发
		if(std::find(owners.begin(), owners.end(), tx.identity()) == owners.end())
		{
			return TxHelper::vrfAgentType::vrfAgentType_local;
		}
		// else //超过30秒 发起节点和identity是一个人就是 本地发
		// {
		// 	return TxHelper::vrfAgentType::vrfAgentType_defalut;
		// }
	}
	return TxHelper::vrfAgentType::vrfAgentType_unknow;
}

void TxHelper::GetTxStartIdentity(const std::vector<std::string> &fromaddr,const uint64_t &height,const uint64_t &current_time,TxHelper::vrfAgentType &type,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
{
		//判断时间是否在30秒
		/*1.在30秒内
			  找打包者(打包者找到自己的话就 交易失败)  用vrf发交易 
		2.在30秒外
			判断发起节点是否满足质押和投资 如果满足走 本地发起广播
										如果不满足  寻找默认账号 判断默认账号满足质押和投资 从默认账号发起交易  代发广播
										发起账号和其他账号都不满足质押和投资就交易失败  没有发起交易的资格
		*/

	//发起方的前一个高度
	uint64_t pre_height = height -1;
	
	if(checkTxTimeOut(current_time,global::ca::TxTimeoutMin, pre_height,SDKAckMsg) == true)
	{//30秒以外

		//50高度以外满足条件发能发
		//GetInitiatorType(fromaddr,type,SDKAckMsg);
		type = vrfAgentType_local;
		return;
	}
	else
	{//30秒以内
		type = vrfAgentType_vrf;
		return;
	}
	//type = vrfAgentType_unknow;
}

// void TxHelper::GetInitiatorType(const std::vector<std::string> &fromaddr, TxHelper::vrfAgentType &type,const std::shared_ptr<GetSDKAck> &SDKAckMsg)
// {
// 	for(auto &addr : fromaddr)
// 	{
// 		//验证投资和质押
// 		int ret = VerifyBonusAddr(addr,SDKAckMsg);

// 		int64_t stake_time = ca_algorithm::GetPledgeTimeByAddr(addr, global::ca::StakeType::kStakeType_Node,SDKAckMsg);
// 		if (stake_time > 0 && ret == 0)
// 		{
// 			//发起节点 投资并且质押
// 			type = vrfAgentType_local;
// 		}
// 		//type = vrfAgentType_unknow;
// 		return;
// 	}
// }
