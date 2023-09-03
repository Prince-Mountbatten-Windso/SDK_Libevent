#include "ca_AdvancedMenu.h"
#include <sys/time.h>
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include <regex>
#include <iomanip>
#include "net/net_api.h"
//#include "include/logging.h"
#include "include/ScopeGuard.h"
#include "utils/time_util.h"
#include "utils/qrcode.h"
#include "common/global_data.h"
#include "utils/MagicSingleton.h"
#include "ca/ca_global.h"
#include "utils/hexcode.h"
#include "utils/bip39.h"
#include "utils/console.h"
#include "utils/EDManager.h"
// #include "utils/ContractUtils.h"
#include "utils/Cycliclist.hpp"
#include "ca.h"

void menu_advanced()
{
    while (true)
    {
        std::cout << std::endl;
        std::cout << "1.Ca" << std::endl;
        std::cout << "2.Net" << std::endl;
        std::cout << "0.Exit" << std::endl;

        std::string strKey;
        std::cout << "please input your choice:";
        std::cin >> strKey;

        std::regex pattern("[0-2]");
        if (!std::regex_match(strKey, pattern))
        {
            std::cout << "Input invalid." << std::endl;
            return;
        }
        int key = std::stoi(strKey);
        switch (key)
        {
        case 0:
            return;
        case 1:
            menu_ca();
            break;
        // case 2:
        //     menu_net();
        //     break;
        default:
            std::cout << "Invalid input." << std::endl;
            continue;
        }
        sleep(1);
    }
}

void menu_ca()
{
    while (true)
    {
        std::cout << std::endl;
        std::cout << "1.Account Manager" << std::endl;
        // std::cout << "2.Print CA Info" << std::endl;
        std::cout << "3.Save To DB" << std::endl;
        // std::cout << "4.Cal Gas." << std::endl;
        // std::cout << "5.Create tx." << std::endl;
        // std::cout << "6.Create coinbase tx." << std::endl;
        std::cout << "7.View block info." << std::endl;
        std::cout << "8.Rollback block." << std::endl;
        // std::cout << "9.Print online time." << std::endl;
        // std::cout << "10.Print hash of first 100 blocks in descending order." << std::endl;
        std::cout << "11.Test." << std::endl;
        // std::cout << "12.Get node info." << std::endl;
        // std::cout << "13.Sync block form other node." << std::endl;
        std::cout << "14.Print pending transaction in cache." << std::endl;
        std::cout << "15.Print failure transaction in cache." << std::endl;
        std::cout << "16.Clear pending transaction in cache." << std::endl;
        // std::cout << "17.Get hash of specified height interval for all Public Nodes." << std::endl;
        std::cout << "18.See which addr have been invested." << std::endl;
        std::cout << "19.Calculate pledge yield." << std::endl;
        std::cout << "20.Obtain block information through transaction hash" << std::endl;
        std::cout << "21.Get all transaction hashes in the height" << std::endl;
        std::cout << "22.Check the address and amount of the investee account" << std::endl;
        std::cout << "23. Stake List" << std::endl;
        std::cout << "0.Exit." << std::endl;

        std::cout << "AddrList : " << std::endl;
        MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();

        std::string strKey;
        std::cout << "please input your choice:";
        std::cin >> strKey;

        std::regex pattern("^[0-9]|([1][0-9])|([2][0-4])$");
        if (!std::regex_match(strKey, pattern))
        {
            std::cout << "Input invalid." << std::endl;
            return;
        }
        int key = std::stoi(strKey);
        switch (key)
        {
        case 0:
            return;
        case 1:
            handle_AccountManger();
            break;
//         case 2:
//             testEDFunction();
//             break;
//         case 3:
//             //                MagicSingleton<CBlockCache>::GetInstance()->FlushBlockManually();
//             TestED25519Time();
//             break;
//         case 4:
//             break;
//         // case 5:
//         //     handle_transaction();
//         //     break;
//         // case 6:
//         //     break;
//         case 7:
//             menu_blockinfo();
//             break;
//         case 8:
//             rollback();
//             break;
//             // case 9:
//             //     break;
//             // case 10:
//             //     print_all_block();
//             break;
//         case 11:
//             menu_test();
//             break;
//         // case 12:
//         //     menu_node();
//         //     break;
//         // case 13:
//         //     break;
//         case 14:
//             // MagicSingleton<TxVinCache>::GetInstance()->Print();
//             MagicSingleton<TranMonitor>::GetInstance()->Print();
//             break;
//         case 15:
//             // MagicSingleton<TxFailureCache>::GetInstance()->Print();
//             {
//                 std::cout << "Print to screen[0] or file[1] ";
//                 int nType = 0;
//                 std::cin >> nType;
//                 if (nType == 0)
//                 {
//                     MagicSingleton<TranMonitor>::GetInstance()->PrintFailureList(std::cout);
//                 }
//                 else if (nType == 1)
//                 {
//                     std::string fileName = "print_failureList_.txt";
//                     std::ofstream filestream;
//                     filestream.open(fileName);
//                     if (!filestream)
//                     {
//                         std::cout << "Open file failed!" << std::endl;
//                         return;
//                     }
//                     MagicSingleton<TranMonitor>::GetInstance()->PrintFailureList(filestream);
//                 }
//             }

//             break;
//         case 16:
//             // MagicSingleton<TxVinCache>::GetInstance()->Clear();
//             MagicSingleton<TranMonitor>::GetInstance()->Clear();
//             break;
//         // case 17:
//         //     CompPublicNodeHash();
//         //     break;
//         case 18:
//         {
//             auto ret = GetBounsAddrInfo();
//             std::cout << "ret :" << ret << std::endl;
//             break;
//         }
//         case 19:
//         {
//             auto ret = ca_algorithm::CalcBonusValue();
//             std::cout << "ret :" << ret << std::endl;
//             break;
//         }
//         case 20:
//             get_blockinfo_by_txhash();
//             break;
//         case 21:
//             get_tx_hash_by_height();
//             break;
//         case 22:
//             get_investedNodeBlance();
//             break;
//         case 23:
//             GetStakeList();
//             break;
//         case 24:
//         {
//             std::cout << "Print to screen[0] or file[1] ";
//             int Type = 0;
//             std::cin >> Type;
//             if (Type == 0)
//             {
//                 MagicSingleton<TranMonitor>::GetInstance()->PrintTxStatus(std::cout);
//             }
//             else if (Type == 1)
//             {
//                 std::string fileName = "PrintTxStatus_list_.txt";
//                 std::ofstream filestream;
//                 filestream.open(fileName);
//                 if (!filestream)
//                 {
//                     std::cout << "Open file failed!" << std::endl;
//                     return;
//                 }
//                 MagicSingleton<TranMonitor>::GetInstance()->PrintTxStatus(filestream);
//             }
//         }
        break;
        default:
            std::cout << "Invalid input." << std::endl;
            continue;
        }
    }
 }

// void gen_key()
// {
//     std::cout << "Please enter the number of accounts to be generated: ";
//     int num = 0;
//     std::cin >> num;
//     if (num <= 0)
//     {
//         return;
//     }

//     std::cout << "please input Normal addr or MultiSign addr" << std::endl;
//     std::cout << "0. Normal addr" << std::endl;
//     std::cout << "1. MultiSign addr" << std::endl;

//     int iVer = 0;
//     std::cin >> iVer;

//     Base58Ver ver;
//     if (iVer == 0)
//     {
//         ver = Base58Ver::kBase58Ver_Normal;
//     }
//     else if (iVer == 1)
//     {
//         ver = Base58Ver::kBase58Ver_MultiSign;
//     }
//     else
//     {
//         std::cout << "error input" << std::endl;
//         return;
//     }

//     for (int i = 0; i != num; ++i)
//     {
//         ED acc(ver);
//         MagicSingleton<EDManager>::GetInstance()->AddAccount(acc);
//         MagicSingleton<EDManager>::GetInstance()->SavePrivateKeyToFile(acc.base58Addr);
//     }
// }

// void in_private_key()
// {
//     std::string data;
//     cout << "please input private key:";
//     cin >> data;
//     // int ret = g_AccountInfo.ImportPrivateKeyHex(data);
//     int ret = MagicSingleton<AccountManager>::GetInstance()->ImportPrivateKeyHex(data);
//     if (ret = 0)
//         std::cout << "import private key success." << std::endl;
//     else
//         std::cout << "import private key failed." << std::endl;
// }

// void out_private_key()
// {
//     char out_data[1024] = {0};
//     int data_len = 1024;

//     int ret = g_AccountInfo.GetPrivateKeyHex(nullptr, out_data, data_len);
//     if (ret)
//         cout << "get private key success." << endl;
//     else
//         cout << "get private key error." << endl;

//     cout << "private key:" << out_data << endl;

//     memset(out_data, 0, 1024);

//     ret = g_AccountInfo.GetMnemonic(nullptr, out_data, data_len);
//     if (ret)
//         cout << "get mem success." << endl;
//     else
//         cout << "get mem error." << endl;

//     cout << "mem data:" << out_data << endl;
// }

// void get_account_balance()
// {
//     std::string FromAddr;
//     std::cout << "Input User's Address:"  << std::endl;
//     // std::cin >> FromAddr;
//     std::cin.get();
//     std::getline(std::cin, FromAddr);

//     std::string addr;
//     if (FromAddr.size() == 0)
//     {
//         addr = g_AccountInfo.DefaultKeyBs58Addr;
//     }
//     else
//     {
//         addr = FromAddr;
//     }

//     std::shared_ptr<GetBalanceReq> req = std::make_shared<GetBalanceReq>();
//     req->set_address(addr);

//     GetBalanceAck ack;

//     int ret = GetBalanceReqImpl(req, ack);

//     if (ret != 0)
//     {
//         std::cout << "Get Balance failed! ret: " << ret << std::endl;
//         return;
//     }

//     std::cout << "User Amount[" << addr << "] : " << ack.balance() << std::endl;
//     INFOLOG("User Amount[{}] : {}", addr.c_str(), ack.balance());
// }

// void print_all_block()
// {
//     std::string str = printBlocks(100);
//     std::cout << str << std::endl;
//     ofstream file("blockdata.txt", fstream::out);
//     str = printBlocks();
//     file << str;
//     file.close();
// }

// void rollback()
// {
//     std::cout << "1.Rollback block from Height" << std::endl;
//     std::cout << "2.Rollback block from Hash" << std::endl;
//     std::cout << "0.Quit" << std::endl;

//     int iSwitch = 0;
//     std::cin >> iSwitch;
//     switch (iSwitch)
//     {
//     case 0:
//     {
//         break;
//     }
//     case 1:
//     {
//         unsigned int height = 0;
//         std::cout << "Rollback block height: ";
//         std::cin >> height;
//         auto ret = ca_algorithm::RollBackToHeight(height);
//         if (0 != ret)
//         {
//             std::cout << std::endl
//                       << "ca_algorithm::RollBackToHeight:" << ret << std::endl;
//             break;
//         }
//         MagicSingleton<PeerNode>::GetInstance()->set_self_height();
//         break;
//     }
//     case 2:
//     {
//         std::string hash;
//         std::cout << "Rollback block hash: ";
//         std::cin >> hash;
//         auto ret = ca_algorithm::RollBackByHash(hash);
//         if (0 != ret)
//         {
//             std::cout << std::endl
//                       << "ca_algorithm::RollBackByHash:" << ret << std::endl;
//             break;
//         }

//         break;
//     }
//     default:
//     {
//         std::cout << "Input error !" << std::endl;
//         break;
//     }
//     }
// }

// void CompPublicNodeHash()
// {
//     uint64_t wait_time = 0;
//     std::cout << "Please enter the maximum waiting time (seconds)" << std::endl;
//     std::cin >> wait_time;
//     uint64_t start_height = 0, end_height = 0;
//     std::cout << "Please enter the starting height" << std::endl;
//     std::cin >> start_height;
//     std::cout << "Please enter the ending height" << std::endl;
//     std::cin >> end_height;
//     std::cout << "start:" << start_height << ", end:" << end_height << std::endl;
//     if (start_height >= end_height)
//     {
//         std::cout << "The starting height is greater than the ending height" << std::endl;
//         return;
//     }
//     if (end_height - start_height > 10000)
//     {
//         std::cout << "The interval cannot be greater than 10000" << std::endl;
//         return;
//     }
//     std::vector<Node> nodeInfos = MagicSingleton<PeerNode>::GetInstance()->get_nodelist();
//     std::string msg_id;
//     GLOBALDATAMGRPTR.CreateWait(wait_time, nodeInfos.size(), msg_id);
//     std::cout << "send data num:" << nodeInfos.size() << std::endl;
//     for (auto &node : nodeInfos)
//     {
//         SendFastSyncGetHashReq(node.base58address, msg_id, start_height, end_height);
//     }
//     std::vector<std::string> ret_data;
//     GLOBALDATAMGRPTR.WaitData(msg_id, ret_data);
//     std::cout << "return data num:" << ret_data.size() << std::endl;
//     if (ret_data.empty())
//     {
//         return;
//     }
//     // The number of hashes and existences that need to be stored in the map
//     Node node;
//     FastSyncGetHashAck ack;
//     for (auto &data : ret_data)
//     {
//         ack.Clear();
//         if (!ack.ParseFromString(data))
//         {
//             continue;
//         }
//         MagicSingleton<PeerNode>::GetInstance()->find_node(ack.self_node_id(), node);
//         for (FastSyncBlockHashs fastSyncBlockHashs : ack.hashs())
//         {
//             for(auto hash : fastSyncBlockHashs.hashs())
//             {
//                 std::cout << "ip: " << setw(15) << IpPort::ipsz(node.public_ip) << ", node: " << ack.self_node_id()
//                           << ", hash: " << hash << std::endl;
//             }
//         }
//     }
//     return;
// }

// void GetAbnormalSignAddrList()
// {
//     CBlockDataApi data_reader;
//     std::vector<std::string> abnormal_addr_list;
//     uint64_t cur_time=MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//     uint64_t end_time=MagicSingleton<TimeUtil>::GetInstance()->getMorningTime(cur_time)*1000000-1000000;//Convert to subtle
//     uint64_t start_time=end_time-(uint64_t)24*60*60*1000000;
//     std::unordered_map<std::string, uint64_t> addr_sign_cnt;
//     std::cout << "start_time: " << MagicSingleton<TimeUtil>::GetInstance()->formatUTCTimestamp(start_time) << std::endl;
// 	   std::cout << "end_time: " << MagicSingleton<TimeUtil>::GetInstance()->formatUTCTimestamp(end_time) << std::endl;

//     uint64_t node_height = 0;

//     if (DBStatus::DB_SUCCESS != data_reader.GetBlockTop(node_height))
//     {
//         return;
//     }
//     std::vector<CBlock> blocks;
//     while(node_height--)
//     {
//         uint64_t MaxTime = 0;
//         uint64_t MinTime = cur_time;

//         blocks.clear();
//         if (DBStatus::DB_SUCCESS != data_reader.GetBlocksByBlockHeight(node_height, blocks))
//         {
//             return;
//         }
//         for (auto &block : blocks)
//         {
//             for (auto &tx : block.txs())
//             {
//                 if(tx.time() > MaxTime)
//                 {
//                     MaxTime = tx.time();
//                 }
//                 if(tx.time() < MinTime)
//                 {
//                     MinTime = tx.time();
//                 }
//             }

//             if(MinTime >= end_time)
//             {
//                 continue;
//             }
//             if(MaxTime < start_time)
//             {
//                 break;
//             }

//             for (auto &tx : block.txs())
//             {
//                 if(start_time <= tx.time() && tx.time() < end_time)
//                 {
//                     if (GetTransactionType(tx) != kTransactionType_Gas)
//                     {
//                         continue;
//                     }
//                     for (auto &vout : tx.utxo().vout())
//                     {
//                         if (vout.value() <= 0)
//                         {
//                             continue;
//                         }
//                         addr_sign_cnt[vout.addr()]++;
//                     }

//                 }
//             }
//         }
//     }

//     uint64_t quarter_num = addr_sign_cnt.size() * 0.25;
//     uint64_t three_quarter_num = addr_sign_cnt.size() * 0.75;
//     if (quarter_num == three_quarter_num)
//     {
//         return;
//     }

//     std::vector<uint64_t> sign_cnt;     // Number of signatures stored
//     for (auto &item : addr_sign_cnt)
//     {
//         std::cout<<")("<<item.first<<'\t'<<item.second<<std::endl;
//         sign_cnt.push_back(item.second);
//     }
//     std::sort(sign_cnt.begin(), sign_cnt.end());

//     uint64_t sign_cnt_quarter_num_value = sign_cnt.at(quarter_num);
//     uint64_t sign_cnt_three_quarter_num_value = sign_cnt.at(three_quarter_num);
//     int64_t sign_cnt_lower_limit_value = sign_cnt_quarter_num_value -
//                                           ((sign_cnt_three_quarter_num_value - sign_cnt_quarter_num_value) * 1.5);
//     std::cout<<"Abnormally low: "<<sign_cnt_lower_limit_value<<std::endl;
//     for (auto &item : addr_sign_cnt)
//     {
//         if (item.second < sign_cnt_lower_limit_value)
//         {
//             abnormal_addr_list.push_back(item.first);
//         }
//     }
// }

// void GetStakeList()
// {
//     DBReader db_reader;
//     std::vector<std::string> addresses;
//     db_reader.GetStakeAddress(addresses);
//     std::cout << "StakeList :" << std::endl;
//     for (auto &it : addresses)
//     {
//         std::cout << it << std::endl;
//     }
// }
// int GetBounsAddrInfo()
// {
//     DBReader db_reader;
//     std::vector<std::string> addresses;
//     std::vector<std::string> bonusAddrs;
//     db_reader.GetBonusaddr(bonusAddrs);
//     for (auto &bonusAddr : bonusAddrs)
//     {
//         std::cout << YELLOW << "BonusAddr: " << bonusAddr << RESET << std::endl;
//         auto ret = db_reader.GetInvestAddrsByBonusAddr(bonusAddr, addresses);
//         if (ret != DBStatus::DB_SUCCESS && ret != DBStatus::DB_NOT_FOUND)
//         {
//             return -1;
//         }

//         uint64_t sum_invest_amount = 0;
//         std::cout << "InvestAddr:" << std::endl;
//         for (auto &address : addresses)
//         {
//             std::cout << address << std::endl;
//             std::vector<string> utxos;
//             ret = db_reader.GetBonusAddrInvestUtxosByBonusAddr(bonusAddr, address, utxos);
//             if (ret != DBStatus::DB_SUCCESS && ret != DBStatus::DB_NOT_FOUND)
//             {
//                 return -2;
//             }

//             uint64_t invest_amount = 0;
//             for (const auto &hash : utxos)
//             {
//                 std::string tx_raw;
//                 if (db_reader.GetTransactionByHash(hash, tx_raw) != DBStatus::DB_SUCCESS)
//                 {
//                     return -3;
//                 }
//                 CTransaction tx;
//                 if (!tx.ParseFromString(tx_raw))
//                 {
//                     return -4;
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
//         std::cout << "total invest amount :" << sum_invest_amount << std::endl;
//     }
//     return 0;
// }

#pragma region netMenu
// void menu_net()
// {
//     while (true)
//     {
//         std::cout << std::endl;
//         std::cout << "1.Send message To user." << std::endl;
//         std::cout << "2.Show my K bucket." << std::endl;
//         std::cout << "3.Kick out node." << std::endl;
//         std::cout << "4.Test echo." << std::endl;
//         std::cout << "5.Broadcast sending Messages." << std::endl;
//         std::cout << "6.Print req and ack." << std::endl;
//         std::cout << "7.Print buffers." << std::endl;
//         std::cout << "8.Big data send to user." << std::endl;
//         std::cout << "9.Print peer node cache." << std::endl;
//         std::cout << "10.Show my ID." << std::endl;
//         std::cout << "0.Exit" << std::endl;

//         std::string strKey;
//         std::cout << "please input your choice:";
//         std::cin >> strKey;

//         std::regex pattern("^[0-9]|([1][0])$");
//         if (!std::regex_match(strKey, pattern))
//         {
//             std::cout << "Input invalid." << std::endl;
//             return;
//         }
//         int key = std::stoi(strKey);
//         switch (key)
//         {
//         case 0:
//             return;
//         case 1:
//             send_message_to_user();
//             break;
//         case 2:
//             show_my_k_bucket();
//             break;
//         case 3:
//             kick_out_node();
//             break;
//         case 4:
//             test_echo();
//             break;
//         case 5:
//             net_com::test_broadcast_message();
//             break;
//         case 6:
//             print_req_and_ack();
//             break;
//         case 7:
//             MagicSingleton<BufferCrol>::GetInstance()->print_bufferes();
//             break;
//         case 8:
//             net_com::test_send_big_data();
//             break;
//         case 9:
//             MagicSingleton<PeerNodeCache>::GetInstance()->PrintToFile();
//             break;
//         case 10:
//             printf("MyID : %s\n", MagicSingleton<PeerNode>::GetInstance()->get_base58addr().c_str());
//             break;
//         default:
//             std::cout << "Invalid input." << std::endl;
//             continue;
//         }
//     }
// }

// void send_message_to_user()
// {
//     if (net_com::input_send_one_message() == 0)
//         DEBUGLOG("send one msg Succ.");
//     else
//         DEBUGLOG("send one msg Fail.");
// }

// void show_my_k_bucket()
// {
//     std::cout << "The K bucket is being displayed..." << std::endl;
//     auto nodelist = MagicSingleton<PeerNode>::GetInstance()->get_nodelist();
//     MagicSingleton<PeerNode>::GetInstance()->print(nodelist);
// }

// void kick_out_node()
// {
//     std::string id;
//     std::cout << "input id:" << std::endl;
//     std::cin >> id;
//     MagicSingleton<PeerNode>::GetInstance()->delete_node(id);
//     std::cout << "Kick out node succeed!" << std::endl;
// }

// void test_echo()
// {

//     std::string message;
//     std::cout << "please input message:" << std::endl;
//     std::cin >> message;

//     EchoReq echoReq;
//     echoReq.set_id(MagicSingleton<PeerNode>::GetInstance()->get_self_id());
//     echoReq.set_message(message);
//     bool isSucceed = net_com::broadcast_message(echoReq, net_com::Compress::kCompress_False, net_com::Encrypt::kEncrypt_False, net_com::Priority::kPriority_Low_0);
//     if (isSucceed == false)
//     {
//         ERRORLOG(":broadcast EchoReq failed!");
//         return;
//     }
// }

// void print_req_and_ack()
// {
//     double total = .0f;
//     std::cout << "------------------------------------------" << std::endl;
//     for (auto &item : global::reqCntMap)
//     {
//         total += (double)item.second.second;
//         std::cout.precision(3);
//         std::cout << item.first << ": " << item.second.first << " size: " << (double)item.second.second / 1024 / 1024 << " MB" << std::endl;
//     }
//     std::cout << "------------------------------------------" << std::endl;
//     std::cout << "Total: " << total / 1024 / 1024 << " MB" << std::endl;
// }

// void menu_blockinfo()
// {
//     while (true)
//     {
//         DBReader reader;
//         uint64_t top = 0;
//         reader.GetBlockTop(top);

//         std::cout << std::endl;
//         std::cout << "Height: " << top << std::endl;
//         std::cout << "1.Get the total number of transactions \n"
//                      "2.Get transaction block details\n"
//                      "5.Get device password \n"
//                      "6.Set device password\n"
//                      "7.Get device private key\n"
//                      "0.Exit \n";

//         std::string strKey;
//         std::cout << "please input your choice:";
//         std::cin >> strKey;

//         std::regex pattern("^[0-7]$");
//         if (!std::regex_match(strKey, pattern))
//         {
//             std::cout << "Input invalid." << std::endl;
//             return;
//         }
//         int key = std::stoi(strKey);
//         switch (key)
//         {
//         case 0:
//             return;
//             //         case 1:
//             //             get_all_tx_number(top);
//             //             break;
//         case 2:
//             get_tx_block_info(top);
//             break;
//             //         case 5:
//             //             std::cout << "none" << std::endl;
//             //             break;
//             //         case 6:
//             //             std::cout << "none" << std::endl;
//             //             break;
//             //         case 7:
//             //             std::cout << "none" << std::endl;
//             //             break;
//         default:
//             std::cout << "Invalid input." << std::endl;
//             continue;
//         }

//         sleep(1);
//     }
// }

// void get_all_tx_number(uint64_t &top)
// {
//     std::vector<std::string> vBlockHashs;
//     size_t b_count = 0;
//     CBlockDataApi data_reader;

//     for (size_t i = top; i > 0; --i)
//     {
//         data_reader.GetBlockHashsByBlockHeight(i, vBlockHashs);
//         b_count += vBlockHashs.size();
//         std::cout << "height------- " << i << std::endl;
//         std::cout << "count======= " << vBlockHashs.size() << std::endl;
//         vBlockHashs.clear();
//     }
//     std::cout << "b_count>>>>>>> " << b_count << std::endl;
// }

// void get_tx_block_info(uint64_t &top)
// {
//     auto amount = to_string(top);
//     std::string input_s, input_e;
//     uint64_t start, end;

//     std::cout << "amount: " << amount << std::endl;
//     std::cout << "pleace input start: ";
//     std::cin >> input_s;
//     if (input_s == "a" || input_s == "pa")
//     {
//         input_s = "0";
//         input_e = amount;
//     }
//     else
//     {
//         if (std::stoul(input_s) > std::stoul(amount))
//         {
//             std::cout << "input > amount" << std::endl;
//             return;
//         }
//         std::cout << "pleace input end: ";
//         std::cin >> input_e;
//         if (std::stoul(input_s) < 0 || std::stoul(input_e) < 0)
//         {
//             std::cout << "params < 0!!" << endl;
//             return;
//         }
//         if (std::stoul(input_s) > std::stoul(input_e))
//         {
//             input_s = input_e;
//         }
//         if (std::stoul(input_e) > std::stoul(amount))
//         {
//             input_e = std::to_string(top);
//         }
//     }
//     start = std::stoul(input_s);
//     end = std::stoul(input_e);

//     std::cout << "Print to screen[0] or file[1] ";
//     uint64_t nType = 0;
//     std::cin >> nType;
//     if (nType == 0)
//     {
//         printRocksdb(start, end, true, std::cout);
//     }
//     else if (nType == 1)
//     {
//         std::string fileName = "print_block_" + std::to_string(start) + "_" + std::to_string(end) + ".txt";
//         std::ofstream filestream;
//         filestream.open(fileName);
//         if (!filestream)
//         {
//             std::cout << "Open file failed!" << std::endl;
//             return;
//         }
//         printRocksdb(start, end, true, filestream);
//     }
// }

// void menu_test()
// {
//     // while (true)
//      {
//     //     std::cout << std::endl;
//     //     // std::cout << "1. Generate mnemonics."<<  std::endl;
//     //     // std::cout << "2. Simulate stake assets."<<  std::endl;
//     //     // std::cout << "3. Get transaction Hash list"<< std::endl;
//     //     std::cout << "4. Query balance according to utxo" << std::endl;
//     //     std::cout << "5. Imitate create tx" << std::endl;
//     //     // std::cout << "6. Simulate unstake assets"<< std::endl;
//     //     // std::cout << "7. Query account stake asset limit"<< std::endl;
//     //     std::cout << "8. Multi account transaction" << std::endl;
//     //     // std::cout << "9. Query transaction list"<< std::endl;
//     //     // std::cout << "10. Query transaction details"<< std::endl;
//     //     // std::cout << "11. Query block list"<< std::endl;
//     //     // std::cout << "12. Query block details"<< std::endl;
//     //     std::cout << "13. Query all stake addresses" << std::endl;
//     //     std::cout << "14. Automatic disordered transaction (simplified version)" << std::endl;
//     //     // std::cout << "15. Get the number of signatures in the previous day"<< std::endl;
//     //     // std::cout << "16. Get block information through transaction hash"<< std::endl;
//     //     // std::cout << "17. Get failed transaction list information"<< std::endl;
//     //     // std::cout << "18. Get whether the node height meets the height range"<< std::endl;
//     //     // std::cout << "19. Obtain stake and unstake"<< std::endl;
//     //     // std::cout << "20. Get QR code" << std::endl;
//     //     // std::cout << "21. Obtain the number of blocks per unit time and the total number of block awards according to the height "<<std::endl;
//     //     // std::cout << "22. Get all utxo"<< std::endl;
//     //     // std::cout << "23. Get block cache hash"<< std::endl;
//     //     // std::cout << "24. Get all transaction hashes in the height"<< std::endl;
//     //     // std::cout << "25. Get the utxo value and total value of all base58 in the height" << std::endl;
//     //     // std::cout << "26. Check the address and amount of the investee account" << std::endl;
//     //     std::cout << "27. print block cache info" << std::endl;
//     //     std::cout << "28. Output the block in database" << std::endl;
//     //     std::cout << "29. Output the block in cache" << std::endl;
//     //     // std::cout << "30. Flush blockcache" << std::endl;
//     //     std::cout << "30. PrintAllAccount" << std::endl;
//     //     std::cout << "31. Get the top 1000 high abnormal signatures" << std::endl;
//     //     std::cout << "32. Create multi thread automatic transaction" << std::endl;
//     //     std::cout << "33. Create multi thread automatic stake transaction" << std::endl;
//     //     std::cout << "34. Automatic investment" << std::endl;
//     //     std::cout << "35. print block cache" << std::endl;
//     //     std::cout << "36. Print Signable Verification Nodes" << std::endl;
//     //     std::cout << "37. Generate account addresses that EVM can." << std::endl;
//     //     std::cout << "38. get TX data." << std::endl;
//     //     std::cout << "39. get checksum EvmAddr." << std::endl;
//     //     std::cout << "40. Benchmark" << std::endl;
//     //     std::cout << "41. Benchmark Clear" << std::endl;
//     //     std::cout << "0. Exit" << std::endl;

//     //     std::string strKey;
//     //     std::cout << "please input your choice:";
//     //     std::cin >> strKey;
//     //     std::regex pattern("^[0-9]|([1][0-9])|([2][0-9])|([3][0-9])|([4][0-1])$");
//     //     if (!std::regex_match(strKey, pattern))
//     //     {
//     //         std::cout << "Input invalid." << std::endl;
//     //         return;
//     //     }
//     //     int key = std::stoi(strKey);
//     //     switch (key)
//     //     {
//     //     case 0:
//     //         return;
//     //     case 1:
//     //         gen_mnemonic();
//     //         // std::cout<<"Hit: "<<global::ca::blockCacheHit<<" : "<<global::ca::dbCacheHit<<" : "<<global::ca::dbHit<<std::endl;
//     //         break;
//     //         //         case 2:
//     //         //             handle_stake();
//     //         //             break;
//     //         //         case 3:
//     //         //             get_hashlist();
//     //         //             break;
//     //     case 4:
//     //         get_balance_by_utxo();
//     //         break;
//     //     case 5:
//     //         imitate_create_tx_struct();
//     //         break;
//     //         //         case 6:
//     //         //             handle_unstake();
//     //         //             break;
//     //         //         case 7:
//     //         //             get_stake();
//     //         //             break;
//     //     case 8:
//     //         multi_tx();
//     //         break;
//     //         //         case 9:
//     //         //             break;
//     //         //         case 10:
//     //         //             break;
//     //         //         case 11:
//     //         //             break;
//     //         //         case 12:
//     //         //             break;
//     //     case 13:
//     //         get_all_pledge_addr();
//     //         break;
//     //     case 14:
//     //         auto_tx();
//     //         break;
//             //         case 15:
//             //             GetAbnormalSignAddrList();
//             //             break;
//             //         case 16:
//             //             get_blockinfo_by_txhash();
//             //             break;
//             //         case 17:
//             //             get_failure_tx_list_info();
//             //             break;
//             //         case 18:
//             //             break;
//             //         case 19:
//             //             break;
//             //         case 20:
//             //             get_qrcode();
//             //             break;
//             //         case 21:
//             //             get_block_num();
//             //             break;
//             //         case 22:
//             //             get_utxo();
//             //             break;
//             //         case 23:
//             //             get_block_cache_hash();
//             //             break;
//             //         case 24:
//             //             get_tx_hash_by_height();
//             //             break;
//             //         case 25:
//             //             get_allutxoBlance();
//             //             break;
//             //         case 26:
//             //             get_investedNodeBlance();
//             //             break;
//         // case 27:
//         //     break;
//         // case 28:
//         //     print_database_block();
//         //     break;
//         // case 29:
//         //     break;
//         // case 30:
//         //     MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();
//         //     break;
//         // case 31:
//         //     get_the_top_1000_hign_abnormal_signatures();
//         //     break;
//         // case 32:
//         //     Create_multi_thread_automatic_transaction();
//         //     break;
//         // case 33:
//         //     Create_multi_thread_automatic_stake_transaction();
//         //     break;
//         // case 34:
//         //     Auto_investment();
//         //     break;
//         // case 35:
//         //     print_block_cache();
//         //     break;
//         // case 36:
//         //     print_verify_node();
//         //     break;
//         // case 37:
//         //     test_address_mapping();
//         //     break;
//         // case 38:
//         //     printTxdata();
//         //     break;
//         // case 39:
//         //     evmAddrConversion();
//         //     break;
//         // case 40:
//         //     printBenchmark();
//         //     break;
//         // case 41:
//         //     MagicSingleton<TFSBenchmark>::GetInstance()->Clear();
//         //     break;
//         default:
//             std::cout << "Invalid input." << std::endl;
//             continue;
//         }

//         sleep(1);
//     }
// }

// void gen_mnemonic()
// {
//     char out[1024 * 10] = {0};

//     // g_AccountInfo.GetMnemonic(nullptr, out, sizeof(out));
//     // printf("Mnemonic_data: [%s]\n", out);

//     std::string mnemonic;

//     ED defaultEd;
//     MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(defaultEd);
//     MagicSingleton<EDManager>::GetInstance()->GetMnemonic(defaultEd.base58Addr, mnemonic);
//     std::cout << "mnemonic : " << mnemonic << std::endl;
//     std::cout << "priStr : " << Str2Hex(defaultEd.priStr) << std::endl;
//     std::cout << "pubStr : " << Str2Hex(defaultEd.pubStr) << std::endl;

//     std::cout << "input mnemonic:" << std::endl;
//     std::string str;
//     std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
//     std::getline(std::cin, str);

//     int len = 0;
//     if (mnemonic.size() > str.size())
//     {
//         len = mnemonic.size();
//     }
//     else
//     {
//         len = str.size();
//     }

//     for (int i = 0; i < len; i++)
//     {
//         if (mnemonic[i] != str[i])
//         {
//             std::cout << "not equal!" << std::endl;
//         }
//     }

//     if (str != mnemonic)
//     {
//         std::cout << "mnemonic error !!! " << std::endl;
//     }
//     std::cout << "final mnemonic : " << str << std::endl;
//     // memset(out, 0x00, sizeof(out));
//     MagicSingleton<EDManager>::GetInstance()->ImportMnemonic(mnemonic);

//     // g_AccountInfo.GetKeyStore(nullptr, "123456", out, sizeof(out));
//     // std::string  keyStore;
//     // MagicSingleton<EDManager>::GetInstance()->GetKeyStore(out, "123456" , keyStore);
//     // std::cout << "keyStore : " << std::endl;
//     // printf("keystore_data %s\n", out);

//     // MagicSingleton<EDManager>::GetInstance()->GetPrivateKeyHex();
//     //  memset(out, 0x00, sizeof(out));
//     //  g_AccountInfo.GetPrivateKeyHex(nullptr, out, sizeof(out));
//     //  printf("PrivateKey_data  %s\n", out);

//     // memset(out, 0x00, sizeof(out));
//     // memcpy(out, g_AccountInfo.CurrentUser.Bs58Addr.c_str(), g_AccountInfo.CurrentUser.Bs58Addr.size());
//     // printf("PrivateKey_58Address  %s\n", out);
// }

// void printTxdata()
// {
//     std::string hash;
//     std::cout << "TX hash: ";
//     std::cin >> hash;

//     DBReader data_reader;

//     CTransaction tx;
//     std::string TxRaw;
//     auto ret = data_reader.GetTransactionByHash(hash, TxRaw);
//     if (ret != DBStatus::DB_SUCCESS)
//     {
//         ERRORLOG("GetTransactionByHash failed!");
//         return;
//     }
//     if (!tx.ParseFromString(TxRaw))
//     {
//         ERRORLOG("Transaction Parse failed!");
//         return;
//     }

//     nlohmann::json data_json = nlohmann::json::parse(tx.data());
//     std::string data = data_json.dump(4);
//     std::cout << data << std::endl;
// }

// void evmAddrConversion()
// {
//     std::string strInput;
//     std::cout << "Please enter non-checksummed version addr:" << std::endl;
//     std::cin >> strInput;
//     bool need0x = false;
//     if (strInput.substr(0, 2) == "0x")
//     {
//         strInput = strInput.substr(2);
//         need0x = true;
//     }

//     std::string checksum_addr = evm_utils::EvmAddrToChecksum(strInput);
//     if (need0x)
//     {
//         checksum_addr = "0x" + checksum_addr;
//     }

//     std::cout << checksum_addr << std::endl;
// }

// void printBenchmark()
// {
//     std::cout << "Print to screen[0] or file[1] ";
//     int nType = 0;
//     std::cin >> nType;
//     if (nType == 0)
//     {
//         MagicSingleton<TFSBenchmark>::GetInstance()->PrintBenchmarkSummary(false);
//     }
//     else if (nType == 1)
//     {
//         MagicSingleton<TFSBenchmark>::GetInstance()->PrintBenchmarkSummary(true);
//     }
//     return;
// }

// void get_hashlist()
// {
//     std::cout << "Inquiry address:";
//     std::string addr;
//     std::cin >> addr;

//     DBReader reader;

//     std::vector<std::string> txHashs;
//     reader.GetAllTransactionByAddreess(addr, txHashs);

//     auto txHashOut = [addr, txHashs](ostream & stream)
//     {

//         stream << "account:" << addr << " tx hash list " << std::endl;
//         for (auto hash : txHashs)
//         {
//             stream << hash << std::endl;
//         }

//         stream << "address: " << addr << " tx hash size: " << txHashs.size() << std::endl;
//     };

//     if (txHashs.size() < 10)
//     {
//         txHashOut(std::cout);
//     }
//     else
//     {
//         std::string fileName = "tx_" + addr + ".txt";
//         ofstream file(fileName);
//         if( !file.is_open() )
//         {
//             ERRORLOG("Open file failed!");
//             return;
//         }
//         txHashOut(file);
//         file.close();
//     }
// }

// void get_balance_by_utxo()
// {
//     std::cout << "Inquiry address:";
//     std::string addr;
//     std::cin >> addr;

//     DBReader reader;
//     std::vector<std::string> utxoHashs;
//     reader.GetUtxoHashsByAddress(addr, utxoHashs);

//     auto utxoOutput = [addr, utxoHashs, &reader](ostream &stream)
//     {
//         stream << "account:" << addr << " utxo list " << std::endl;

//         uint64_t total = 0;
//         for (auto i : utxoHashs)
//         {
//             std::string txRaw;
//             reader.GetTransactionByHash(i, txRaw);

//             CTransaction tx;
//             tx.ParseFromString(txRaw);

//             uint64_t value = 0;
//             for (int j = 0; j < tx.utxo().vout_size(); j++)
//             {
//                 CTxOutput txout = tx.utxo().vout(j);
//                 if (txout.addr() != addr)
//                 {
//                     continue;
//                 }
//                 value += txout.value();
//             }
//             stream << i << " : " << value << std::endl;
//             total += value;
//         }

//         stream << "address: " << addr << " UTXO total: " << utxoHashs.size() << " UTXO gross value:" << total << std::endl;
//     };

//     if (utxoHashs.size() < 10)
//     {
//         utxoOutput(std::cout);
//     }
//     else
//     {
//         std::string fileName = "utxo_" + addr + ".txt";
//         ofstream file(fileName);
//         if (!file.is_open())
//         {
//             ERRORLOG("Open file failed!");
//             return;
//         }
//         utxoOutput(file);
//         file.close();
//     }
// }

// int imitate_create_tx_struct()
// {
//     ED acc;
//     EVP_PKEY_free(acc.pkey);
//     if (MagicSingleton<EDManager>::GetInstance()->GetDefaultAccount(acc) != 0)
//     {
//         return -1;
//     }

//     const std::string addr = acc.base58Addr;
//     uint64_t time = global::ca::kGenesisTime;

//     CTransaction tx;
//     tx.set_version(0);
//     tx.set_time(time);
//     tx.set_n(0);
//     tx.set_identity(addr);
//     tx.set_type(global::ca::kGenesisSign);

//     CTxUtxo *utxo = tx.mutable_utxo();
//     utxo->add_owner(addr);
//     {
//         CTxInput *txin = utxo->add_vin();
//         CTxPrevOutput *prevOut = txin->add_prevout();
//         prevOut->set_hash(std::string(64, '0'));
//         prevOut->set_n(0);
//         txin->set_sequence(0);

//         std::string serVinHash = getsha256hash(txin->SerializeAsString());
//         std::string signature;
//         std::string pub;

//         if (acc.Sign(serVinHash, signature) == false)
//         {
//             return -2;
//         }

//         CSign *sign = txin->mutable_vinsign();
//         sign->set_sign(signature);
//         sign->set_pub(acc.pubStr);
//     }

//     {
//         CTxOutput *txout = utxo->add_vout();
//         txout->set_value(global::ca::kM2 * global::ca::kDecimalNum);
//         txout->set_addr(addr);
//     }

//     {
//         std::string serUtxo = getsha256hash(utxo->SerializeAsString());
//         std::string signature;
//         if (acc.Sign(serUtxo, signature) == false)
//         {
//             return -3;
//         }

//         CSign *multiSign = utxo->add_multisign();
//         multiSign->set_sign(signature);
//         multiSign->set_pub(acc.pubStr);
//     }
    
//     tx.set_txtype((uint32)global::ca::TxType::kTxTypeGenesis);

//     tx.set_hash(getsha256hash(tx.SerializeAsString()));

//     CBlock block;
//     block.set_time(time);
//     block.set_version(0);
//     block.set_prevhash(std::string(64, '0'));
//     block.set_height(0);

//     CTransaction *tx0 = block.add_txs();
//     *tx0 = tx;

//     nlohmann::json blockData;
//     blockData["Name"] = "Transformers";
//     blockData["Type"] = "Genesis";
//     block.set_data(blockData.dump());

//     block.set_merkleroot(ca_algorithm::CalcBlockMerkle(block));
//     block.set_hash(getsha256hash(block.SerializeAsString()));

//     std::string hex = Str2Hex(block.SerializeAsString());
//     std::cout << std::endl
//               << hex << std::endl;

//     return 0;
// }

// void get_stake()
// {
//     std::cout << "Query the amount of pledged assets of account:" << std::endl;
//     { // Query pledge

//         std::string addr;
//         std::cout << "Query account number:";
//         std::cin >> addr;

//         std::shared_ptr<GetStakeListReq> req = std::make_shared<GetStakeListReq>();
//         req->set_version(global::kVersion);
//         req->set_addr(addr);

//         GetStakeListAck ack;
//         GetStakeListReqImpl(req, ack);

//         if (ack.code() != 0)
//         {
//             ERRORLOG("get stake list failed!");
//             std::cout << "get stake list failed!" << std::endl;
//             return;
//         }

//         size_t size = (size_t)ack.list_size();
//         for (size_t i = 0; i < size; i++)
//         {
//             std::cout << "----- stake " << i << " start -----" << std::endl;
//             const StakeItem &  item = ack.list(i);
//             std::cout << "blockhash: " << item.blockhash() << std::endl;
//             std::cout << "blockheight: " << item.blockheight() << std::endl;
//             std::cout << "utxo: " << item.utxo() << std::endl;
//             std::cout << "amount: " << item.amount() << std::endl;
//             std::cout << "time: " << PrintTime(item.time()) << std::endl;
//             std::cout << "fromaddr: " << item.fromaddr() << std::endl;
//             std::cout << "toaddr: " << item.toaddr() << std::endl;
//             std::cout << "detail: " << item.detail() << std::endl;

//             std::cout << "----- stake " << i << " end -----" << std::endl
//                       << std::endl;
//         }
//     }
// }

// void multi_tx()
// {
//     int addrCount = 0;
//     std::cout << "Number of initiator accounts:";
//     std::cin >> addrCount;

//     std::vector<std::string> fromAddr;
//     for (int i = 0; i < addrCount; ++i)
//     {
//         std::string addr;
//         std::cout << "Initiating account" << i + 1 << ": ";
//         std::cin >> addr;
//         fromAddr.push_back(addr);
//     }

//     std::cout << "Number of receiver accounts:";
//     std::cin >> addrCount;

//     std::map<std::string, int64_t> toAddr;
//     for (int i = 0; i < addrCount; ++i)
//     {
//         std::string addr;
//         double amt = 0;
//         std::cout << "Receiving account" << i + 1 << ": ";
//         std::cin >> addr;
//         std::cout << "amount : ";
//         std::cin >> amt;
//         toAddr.insert(make_pair(addr, amt * global::ca::kDecimalNum));
//     }

//     DBReader db_reader;
//     uint64_t top = 0;
//     if (DBStatus::DB_SUCCESS != db_reader.GetBlockTop(top))
//     {
//         ERRORLOG("db get top failed!!");
//         return;
//     }

//     TxMsgReq txMsg;
//     TxHelper::vrfAgentType isNeedAgent_flag;
//     CTransaction outTx;
//     Vrf info_;
//     int ret = TxHelper::CreateTxTransaction(fromAddr, toAddr, top + 1, outTx,isNeedAgent_flag,info_);
//     if (ret != 0)
// 	{
// 		ERRORLOG("CreateTxTransaction error!!");
// 		return;
// 	}


// 	txMsg.set_version(global::kVersion);
//     TxMsgInfo * txMsgInfo = txMsg.mutable_txmsginfo();
//     txMsgInfo->set_type(0);
//     txMsgInfo->set_tx(outTx.SerializeAsString());
//     txMsgInfo->set_height(top);

//     if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf){
//         Vrf * new_info = txMsg.mutable_vrfinfo();
//         new_info->CopyFrom(info_);
//     }

//     auto msg = make_shared<TxMsgReq>(txMsg);

//     std::string defaultBase58Addr = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
//     if (isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf && outTx.identity() != defaultBase58Addr)
//     {

//         ret = DropshippingTx(msg, outTx);
//     }
//     else
//     {
//         ret = DoHandleTx(msg, outTx);
//     }
//     DEBUGLOG("Transaction result, ret:{}  txHash: {}", ret, outTx.hash());
// }

// void get_all_pledge_addr()
// {
//     DBReader reader;
//     std::vector<std::string> addressVec;
//     reader.GetStakeAddress(addressVec);

//     auto allPledgeOutput = [addressVec](ostream &stream)
//     {
//         stream << std::endl
//                << "---- Pledged address start ----" << std::endl;
//         for (auto &addr : addressVec)
//         {
//             uint64_t pledgeamount = 0;
//             SearchStake(addr, pledgeamount, global::ca::StakeType::kStakeType_Node);
//             stream << addr << " : " << pledgeamount << std::endl;
//         }
//         stream << "---- Number of pledged addresses:" << addressVec.size() << " ----" << std::endl
//                << std::endl;
//         stream << "---- Pledged address end  ----" << std::endl
//                << std::endl;
//     };

//     if (addressVec.size() <= 10)
//     {
//         allPledgeOutput(std::cout);
//     }
//     else
//     {
//         std::string fileName = "all_pledge.txt";

//         std::cout << "output to a file" << fileName << std::endl;

//         std::ofstream fileStream;
//         fileStream.open(fileName);
//         if (!fileStream)
//         {
//             std::cout << "Open file failed!" << std::endl;
//             return;
//         }

//         allPledgeOutput(fileStream);

//         fileStream.close();
//     }
// }

// void auto_tx()
// {
//     if (bIsCreateTx)
//     {
//         int i = 0;
//         std::cout << "1. Close the transaction" << std::endl;
//         std::cout << "0. Continue trading" << std::endl;
//         std::cout << ">>>" << std::endl;
//         std::cin >> i;
//         if (i == 1)
//         {
//             bStopTx = true;
//         }
//         else if (i == 0)
//         {
//             return;
//         }
//         else
//         {
//             std::cout << "Error!" << std::endl;
//             return;
//         }
//     }
//     else
//     {
//         bStopTx = false;
//         std::vector<std::string> addrs;

//         MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();
//         MagicSingleton<EDManager>::GetInstance()->GetAccountList(addrs);

//         double sleepTime = 0;
//         std::cout << "Interval time (seconds):";
//         std::cin >> sleepTime;
//         sleepTime *= 1000000;
//         std::thread th(TestCreateTx, addrs, (int)sleepTime);
//         th.detach();
//         return;
//     }
// }

// void get_blockinfo_by_txhash()
// {
//     DBReader reader;

//     std::cout << "Tx Hash : ";
//     std::string txHash;
//     std::cin >> txHash;

//     std::string blockHash;
//     reader.GetBlockHashByTransactionHash(txHash, blockHash);

//     if (blockHash.empty())
//     {
//         std::cout << RED << "Error : GetBlockHashByTransactionHash failed !" << RESET << std::endl;
//         return;
//     }

//     std::string blockStr;
//     reader.GetBlockByBlockHash(blockHash, blockStr);
//     CBlock block;
//     block.ParseFromString(blockStr);

//     std::cout << GREEN << "Block Hash : " << blockHash << RESET << std::endl;
//     std::cout << GREEN << "Block height : " << block.height() << RESET << std::endl;
// }

// void get_failure_tx_list_info()
// {
//     std::cout << "Failed to query transaction list:" << std::endl;

//     std::string addr;
//     std::cout << "Query account number:";
//     std::cin >> addr;

//     std::string txHash;
//     std::cout << "Enter hash (first direct enter):";
//     std::cin.get();
//     std::getline(std::cin, txHash);

//     size_t count = 0;
//     std::cout << "Enter count:";
//     std::cin >> count;

//     std::shared_ptr<GetTxFailureListReq> req = std::make_shared<GetTxFailureListReq>();
//     req->set_version(global::kVersion);
//     req->set_addr(addr);
//     req->set_txhash(txHash);
//     req->set_count(count);

//     GetTxFailureListAck ack;
//     GetTxFailureListReqImpl(req, ack);
//     if (ack.code() != 0)
//     {
//         ERRORLOG("get transaction failure failed!");
//         std::cout << "get transaction failure failed!!" << std::endl;
//         std::cout << "code: " << ack.code() << std::endl;
//         std::cout << "description: " << ack.message() << std::endl;
//         return;
//     }

//     std::cout << "failure total: " << ack.total() << std::endl;
//     std::cout << "last hash: " << ack.lasthash() << std::endl;
//     size_t size = (size_t)ack.list_size();
//     for (size_t i = 0; i < size; i++)
//     {
//         std::cout << "----- failure list " << i << " start -----" << std::endl;
//         const TxFailureItem &item = ack.list(i);
//         std::cout << "hash: " << item.txhash() << std::endl;
//         if (item.fromaddr_size() > 0)
//         {
//             std::cout << "from addr: " << item.fromaddr(0) << std::endl;
//         }
//         if (item.toaddr_size() > 0)
//         {
//             std::cout << "to addr: " << item.toaddr(0) << std::endl;
//         }
//         std::cout << "amout: " << item.amount() << std::endl;
//     }
// }

// void get_qrcode()
// {
//     std::string fileName("account.txt");

//     ca_console redColor(kConsoleColor_Red, kConsoleColor_Black, true);
//     std::cout << redColor.color() << "The output file is:" << fileName << " Please use courier NewFont view" << redColor.reset() << std::endl;

//     ofstream file;
//     file.open(fileName);

//     file << "Please use courier new font to view" << std::endl
//          << std::endl;

//     for (auto &item : g_AccountInfo.AccountList)
//     {
//         file << "Base58 addr: " << item.first << std::endl;

//         char out_data[1024] = {0};
//         int data_len = sizeof(out_data);
//         mnemonic_from_data((const uint8_t *)item.second.sPriStr.c_str(), item.second.sPriStr.size(), out_data, data_len);
//         file << "Mnemonic: " << out_data << std::endl;

//         std::string strPriHex = Str2Hex(item.second.sPriStr);
//         file << "Private key: " << strPriHex << std::endl;

//         file << "QRCode:";

//         QRCode qrcode;
//         uint8_t qrcodeData[qrcode_getBufferSize(5)];
//         qrcode_initText(&qrcode, qrcodeData, 5, ECC_MEDIUM, strPriHex.c_str());

//         file << std::endl
//              << std::endl;

//         for (uint8_t y = 0; y < qrcode.size; y++)
//         {
//             file << "        ";
//             for (uint8_t x = 0; x < qrcode.size; x++)
//             {
//                 file << (qrcode_getModule(&qrcode, x, y) ? "\u2588\u2588" : "  ");
//             }

//             file << std::endl;
//         }

//         file << std::endl
//              << std::endl
//              << std::endl
//              << std::endl
//              << std::endl
//              << std::endl;
//     }
// }

// void get_block_num()
// {
//     uint64_t block_height = 0;
//     std::cout << "Height:";
//     std::cin >> block_height;
//     if (0 == block_height)
//     {
//         if (DBStatus::DB_SUCCESS != DBReader().GetBlockTop(block_height))
//         {
//             std::cout << "Failed to get node height" << std::endl;
//             return;
//         }
//     }
//     int block_num = ca_algorithm::GetBlockNumInUnitTime(block_height);
//     if (block_num < 0)
//     {
//         std::cout << "Get the block return in unit time:" << block_num << std::endl;
//         return;
//     }
//     std::cout << std::endl
//               << "block num:" << block_num << std::endl;
// }

// void get_utxo()
// {
//     std::string addr;
//     std::cout << "Query account number: ";
//     std::cin >> addr;

//     std::string id;
//     std::cout << "Please enter ID: ";
//     std::cin >> id;

//     GetUtxoReq req;
//     req.set_version(global::kVersion);
//     req.set_address(addr);

//     net_register_callback<GetUtxoAck>([] (const std::shared_ptr<GetUtxoAck> & ack, const MsgData & msgdata)
//                                        {

//         if (ack->code() != 0)
//         {
//             std::cout << "code: " << ack->code() << std::endl;
//             return 0;
//         }

//         for(size_t i = 0; i != (size_t)ack->utxos_size(); ++i)
//         {
//             auto & item = ack->utxos(i);
//             std::cout << "hash: " << item.hash() << " value: " << item.value() << " n:" << item.n() << std::endl;
//         }

//         std::cout << "addr: " << ack->address() << "utxo count: " << ack->utxos_size() << std::endl;
//         return 0;});
//     net_send_message<GetUtxoReq>(id, req);
// }
// void get_tx_hash_by_height()
// {
//     int64_t start = 0;
//     int64_t end = 0;
//     std::cout << "Please input start height:";
//     std::cin >> start;

//     std::cout << "Please input end height:";
//     std::cin >> end;

//     if (end < start)
//     {
//         std::cout << "input invalid" << std::endl;
//         return;
//     }

//     DBReader db_reader;
//     uint64_t tx_total = 0;
//     uint64_t block_total = 0;
//     for (int64_t i = end; i >= start; --i)
//     {

//         std::vector<std::string> tmp_block_hashs;
//         if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashsByBlockHeight(i, tmp_block_hashs))
//         {
//             ERRORLOG("(get_tx_hash_by_height) GetBlockHashsByBlockHeight  Failed!!");
//             return;
//         }

//         int tx_hash_count = 0;
//         for (auto &blockhash : tmp_block_hashs)
//         {
//             std::string blockstr;
//             db_reader.GetBlockByBlockHash(blockhash, blockstr);
//             CBlock block;
//             block.ParseFromString(blockstr);

//             tx_hash_count += block.txs_size();
//         }
//         tx_total += tx_hash_count;
//         block_total += tmp_block_hashs.size();
//         std::cout << GREEN << "height: " << i << " block: " << tmp_block_hashs.size() << " tx: " << tx_hash_count / 3 << RESET << std::endl;
//     }

//     std::cout << GREEN << "block sum " << block_total << RESET << std::endl;
//     std::cout << GREEN << "tx sum " << tx_total / 3 << RESET << std::endl;

//     std::vector<std::string> start_hashes;
//     if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashsByBlockHeight(start, start_hashes))
//     {
//         ERRORLOG("GetBlockHashsByBlockHeight fail  top = {} ", start);
//         return;
//     }
//     //取出起始高度的块 并按照时间从小到大排序
//     std::vector<CBlock> start_blocks;
//     for (auto &hash : start_hashes)
//     {
//         std::string blockStr;
//         db_reader.GetBlockByBlockHash(hash, blockStr);
//         CBlock block;
//         block.ParseFromString(blockStr);
//         start_blocks.push_back(block);
//     }
//     std::sort(start_blocks.begin(), start_blocks.end(), [](const CBlock &x, const CBlock &y)
//               { return x.time() < y.time(); });

//     std::vector<std::string> end_hashes;
//     if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashsByBlockHeight(end, end_hashes))
//     {
//         ERRORLOG("GetBlockHashsByBlockHeight fail  top = {} ", end);
//         return;
//     }
//     //取出终止高度的块，并按照时间从小到大排序
//     std::vector<CBlock> end_blocks;
//     for (auto &hash : end_hashes)
//     {
//         std::string blockStr;
//         db_reader.GetBlockByBlockHash(hash, blockStr);
//         CBlock block;
//         block.ParseFromString(blockStr);
//         end_blocks.push_back(block);
//     }
//     std::sort(end_blocks.begin(), end_blocks.end(), [](const CBlock &x, const CBlock &y)
//               { return x.time() < y.time(); });

//     uint64_t time_diff = 0;
//     if (end_blocks[end_blocks.size() - 1].time() - start_blocks[0].time() != 0)
//     {
//         time_diff = (end_blocks[end_blocks.size() - 1].time() - start_blocks[0].time()) / 1000000;
//     }
//     else
//     {
//         time_diff = 1;
//     }
//     uint64_t tx_conut = tx_total / 3;
//     uint64_t tps = tx_conut / time_diff;
//     std::cout << "TPS : " << tps << std::endl;
// }

// void get_block_cache_hash()
// {
//     std::string strStart, strEnd;
//     std::regex pattern("^[1-9]\\d*$");

//     std::cout << "pleace input cache start : ";
//     std::cin >> strStart;
//     if (!std::regex_match(strStart, pattern))
//     {
//         std::cout << "invalid input!(start height should '> 0')" << std::endl;
//         return;
//     }
//     uint64_t start = std::stoull(strStart) - 1;

//     std::cout << "pleace input cache end: ";
//     std::cin >> strEnd;
//     uint64_t end = 0;
//     if (!std::regex_match(strEnd, pattern) || (end = std::stoull(strEnd) - 1) < start)
//     {
//         std::cout << "invalid input!(end height should be larger than start height)!" << std::endl;
//         return;
//     }

//     std::map<uint64_t, std::set<CBlock, CBlockCompare>> block_cache;
//     MagicSingleton<CBlockCache>::GetInstance()->GetCache(block_cache);

//     auto Blockcacheout = [block_cache,start,end](ostream & stream)
//     {
//         auto iter = block_cache.begin();
//         int block_size = block_cache.size();
//         for (int i = 0; i <= end && i < block_size ; ++i)
//         {
//             if (i < start)
//             {
//                 iter++;
//                 continue;
//             }
//             stream << "height: " << iter->first << std::endl;
//             stream << "block hash: " << std::endl;
//             for (const auto &block : iter->second)
//             {
//                 stream << block.hash() << std::endl;
//             }
//             iter++;
//         }
//     };

//     if (end - start < 10)
//     {
//         Blockcacheout(std::cout);
//     }
//     else
//     {
//         int64_t getLocTime = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//         std::string fileName = "blockcache_" + strStart + "_" + strEnd + "_" + MagicSingleton<TimeUtil>::GetInstance()->formatTimestamp(getLocTime) + ".txt";
//         ofstream file(fileName);
//         if (!file.is_open() )
//         {
//             ERRORLOG("Open file failed!");
//             return;
//         }
//         Blockcacheout(file);
//         file.close();
//     }
// }

// void menu_node()
// {
//     while (true)
//     {
//         std::cout << std::endl;
//         std::cout << "1.get a node info" << std::endl;
//         std::cout << "2.get all node info" << std::endl;
//         std::cout << "0.exit node" << std::endl;

//         std::string strKey;
//         std::cout << "please input your choice:";
//         std::cin >> strKey;

//         std::regex pattern("^[0-2]$");
//         if (!std::regex_match(strKey, pattern))
//         {
//             std::cout << "Input invalid." << std::endl;
//             return;
//         }
//         int key = std::stoi(strKey);
//         switch (key)
//         {
//         case 0:
//             return;
//         case 1:

//             break;
//         case 2:
//             break;
//         default:
//             std::cout << "Invalid input." << std::endl;
//             continue;
//         }
//     }
// }

// void get_allutxoBlance()
// {
//     // 1.程序起  计算all base58
//     std::set<std::string> list_base58;
//     uint64_t search_top;
//     std::cout<<"please input your search top>:";
//     std::cin >> search_top;

//     int ret = GetAllBase58(list_base58,search_top);
//     if (ret != 0)
//     {
//         DEBUGLOG("GetAllBase58 error ret={}", ret);
//         return;
//     }

//     std::string text_file;
//     std::cout << "please input your getbalance text file>:";
//     std::cin >> text_file;

//     PrintBalanceToTxt(list_base58, text_file);
// }

// void get_investedNodeBlance()
// {
//     std::string addr;
//     std::cout << "Please enter the address you need to inquire: " << std::endl;
//     std::cin >> addr;

//     std::shared_ptr<GetAllInvestAddressReq> req = std::make_shared<GetAllInvestAddressReq>();
//     req->set_version(global::kVersion);
//     req->set_addr(addr);

//     GetAllInvestAddressAck ack;
//     GetAllInvestAddressReqImpl(req, ack);
//     if (ack.code() != 0)
//     {
//         std::cout << "code: " << ack.code() << std::endl;
//         ERRORLOG("get_investedNodeBlance failed!");
//         return;
//     }

//     std::cout << "------------" << ack.addr() << "------------" << std::endl;

//     for (int i = 0; i < ack.list_size(); i++)
//     {
//         const InvestAddressItem info = ack.list(i);
//         std::cout << "addr:" << info.addr() << "\tamount:" << info.value() << std::endl;
//     }
// }
// void print_database_block()
// {
//     DBReader dbReader;
//     std::string str = printBlocks(100, false);
//     std::cout << str << std::endl;
// }

// void print_cache_block()
//{
//     std::string str = printBlocks(100, false, *MagicSingleton<CBlockCache>::GetInstance());
//     std::cout << str << std::endl;
// }

// void get_the_top_1000_hign_abnormal_signatures()
// {
//     int blockNum = 1000;
//     clock_t start = clock();

//     DBReader reader;
//     uint64_t top = 0;
//     if (DBStatus::DB_SUCCESS != reader.GetBlockTop(top))
//     {
//         std::cout << "GetBlockTop failed! " << std::endl;
//         return;
//     }

//     if (0 == top)
//     {
//         return;
//     }
//     uint64_t highest_height = top;
//     uint64_t lowest_height = top > 1000 ? top - 1000 : 1;
//     std::vector<std::string> block_hashes;
//     DBReader db_reader;
//     if (DBStatus::DB_SUCCESS != db_reader.GetBlockHashesByBlockHeight(lowest_height, highest_height, block_hashes))
//     {
//         std::cout << "error" << std::endl;
//         return;
//     }
//     std::vector<std::string> blocks;
//     if (DBStatus::DB_SUCCESS != db_reader.GetBlocksByBlockHash(block_hashes, blocks))
//     {
//         std::cout << "error" << std::endl;
//         return;
//     }
//     std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> addr_sign_cnt_and_award_amount;

//     CBlock block;
//     uint64_t count = 0;
//     std::vector<Node> nodelist = MagicSingleton<PeerNode>::GetInstance()->get_nodelist();
//     std::vector<std::string> pubid;
//     for (auto &item : nodelist)
//     {
//         pubid.push_back(item.base58address);
//     }

//     for (auto &block_raw : blocks)
//     {
//         if (!block.ParseFromString(block_raw))
//         {
//             std::cout << "error" << std::endl;
//             return;
//         }
//         for (auto &tx : block.txs())
//         {
//             if (GetTransactionType(tx) != kTransactionType_Gas)
//             {
//                 continue;
//             }
//             for (auto &vout : tx.utxo().vout())
//             {
//                 if (vout.value() <= 0)
//                 {
//                     continue;
//                 }

//                 if (pubid.cend() != std::find(pubid.cbegin(), pubid.cend(), vout.addr()))
//                 {
//                     continue;
//                 }

//                 auto it = addr_sign_cnt_and_award_amount.find(vout.addr());
//                 if (addr_sign_cnt_and_award_amount.end() == it)
//                 {
//                     addr_sign_cnt_and_award_amount.insert(std::make_pair(vout.addr(), std::make_pair(0, 0)));
//                 }
//                 auto &value = addr_sign_cnt_and_award_amount.at(vout.addr());
//                 ++value.first;
//             }
//         }
//     }
//     uint64_t quarter_num = addr_sign_cnt_and_award_amount.size() * 0.25;
//     uint64_t three_quarter_num = addr_sign_cnt_and_award_amount.size() * 0.75;
//     if (quarter_num == three_quarter_num)
//     {
//         return;
//     }

//     std::vector<uint64_t> sign_cnt; // 存放签名次数
//     for (auto &item : addr_sign_cnt_and_award_amount)
//     {
//         sign_cnt.push_back(item.second.first);
//         std::cout << "账号：" << item.first << "   次数：" << item.second.first << std::endl;
//     }
//     std::sort(sign_cnt.begin(), sign_cnt.end());
//     uint64_t sign_cnt_quarter_num_value = sign_cnt.at(quarter_num);
//     uint64_t sign_cnt_three_quarter_num_value = sign_cnt.at(three_quarter_num);
//     uint64_t sign_cnt_upper_limit_value = sign_cnt_three_quarter_num_value +
//                                           ((sign_cnt_three_quarter_num_value - sign_cnt_quarter_num_value) * 1.5);

//     uint32_t sign_abnormal_count = 0;
//     uint32_t abnormal_count = 0;
//     for (auto &item : addr_sign_cnt_and_award_amount)
//     {
//         if (item.second.first > sign_cnt_upper_limit_value)
//         {
//             ++sign_abnormal_count;
//             std::cout << "********** 签名次数异常账号：" << item.first << "   总次数 = " << item.second.first << std::endl;
//         }
//     }
//     std::cout << "********** 签名次数---1/4 = " << sign_cnt_quarter_num_value << std::endl;
//     std::cout << "********** 签名次数---3/4 = " << sign_cnt_three_quarter_num_value << std::endl;
//     std::cout << "********** 签名次数---差值 = " << sign_cnt_three_quarter_num_value - sign_cnt_quarter_num_value << std::endl;
//     std::cout << "********** 签名次数---上限 = " << sign_cnt_upper_limit_value << std::endl;
//     std::cout << "********** 签名次数---总帐号个数 = " << addr_sign_cnt_and_award_amount.size() << std::endl;
//     std::cout << "********** 签名次数---异常帐号个数 = " << sign_abnormal_count << std::endl;
//     std::cout << "********** 签名次数---异常帐号占比 = " << ((double)sign_abnormal_count / addr_sign_cnt_and_award_amount.size()) * 100 << "%" << std::endl;
// }

// void ThreadTest::TestCreateTx_2(const std::string &from, const std::string &to)
// {
//     // int intPart = rand() % 10;
//     std::cout << "from:" << from << std::endl;
//     std::cout << "to:" << to << std::endl;

//     uint64_t start_time = MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp();
//     bool Initiate = false;
//     ON_SCOPE_EXIT{
//         if(!Initiate)
//         {
//             MagicSingleton<TFSBenchmark>::GetInstance()->ClearTransactionInitiateMap();
//         }
//     };

//     int intPart = 0;
//     double decPart = (double)(rand() % 10) / 10000;
//     std::string amountStr = std::to_string(intPart + decPart);

//     std::vector<std::string> fromAddr;
//     fromAddr.emplace_back(from);
//     std::map<std::string, int64_t> toAddrAmount;
//     uint64_t amount = (stod(amountStr) + global::ca::kFixDoubleMinPrecision) * global::ca::kDecimalNum;

//     if (amount == 0)
//     {
//         std::cout << "amount = 0" << std::endl;
//         DEBUGLOG("amount = 0");
//         return;
//     }

//     toAddrAmount[to] = amount;

//     DBReader data_reader;
//     uint64_t top = 0;
//     if (DBStatus::DB_SUCCESS != data_reader.GetBlockTop(top))
//     {
//         ERRORLOG("db get top failed!!");
//         return;
//     }
    
//     CTransaction outTx;
//     TxHelper::vrfAgentType isNeedAgent_flag;
//     Vrf info_;
//     int ret = TxHelper::CreateTxTransaction(fromAddr, toAddrAmount, top + 1, outTx,isNeedAgent_flag,info_);
//     if (ret != 0)
//     {
//         ERRORLOG("CreateTxTransaction error!!");
//         return;
//     }
//     MagicSingleton<TranMonitor>::GetInstance()->AddTranMonitor(outTx);

//     TxMsgReq txMsg;
//     txMsg.set_version(global::kVersion);
//     TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
//     txMsgInfo->set_type(0);
//     txMsgInfo->set_tx(outTx.SerializeAsString());
//     txMsgInfo->set_height(top);


//     if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf){
//         Vrf * new_info = txMsg.mutable_vrfinfo();
//         new_info->CopyFrom(info_);

//     }

//     auto msg = make_shared<TxMsgReq>(txMsg);

//     std::string defaultBase58Addr = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
//     if (isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf && outTx.identity() != defaultBase58Addr)
//     {

//         ret = DropshippingTx(msg, outTx);
//     }
//     else
//     {
//         ret = DoHandleTx(msg, outTx);
//     }

//     // ret = DoHandleTx(msg, outTx);
//     DEBUGLOG("Transaction result,ret:{}  txHash:{}", ret, outTx.hash());
//     MagicSingleton<TranMonitor>::GetInstance()->SetDoHandleTxStatus(outTx, ret);
//     Initiate = true;
//     MagicSingleton<TFSBenchmark>::GetInstance()->AddTransactionInitiateMap(start_time, MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp());
    
//     std::cout << "=====Transaction initiator:" << from << std::endl;
//     std::cout << "=====Transaction recipient:" << to << std::endl;
//     std::cout << "=====Transaction amount:" << amountStr << std::endl;
//     std::cout << "=======================================================================" << std::endl
//               << std::endl
//               << std::endl;
// }

// bool bStopTx_2 = true;
// bool bIsCreateTx_2 = false;
// static int i = -1;
// static int i_count = 1;
// static int count_wheel = 0;
// int GetIndex(uint32_t &tranNum, std::vector<std::string> &addrs, bool flag = false)
// {
//     if ((i + 1) > ((tranNum * 2) - 1))
//     {
//         i = 0;
//     }
//     else
//     {
//         i += 1;
//     }
//     if (flag)
//     {
//         std::vector<CTransaction> vectTxs;
//         if (!MagicSingleton<TranMonitor>::GetInstance()->Find(addrs[i], vectTxs))
//         {
//             i = GetIndex(tranNum, addrs, flag);
//         }
//     }
//     return i;
// }
// void ThreadTest::set_StopTx_flag(const bool &flag)
// {
//     bStopTx_2 = flag;
// }



// void ThreadTest::get_StopTx_flag(bool &flag)
// {
//    flag =  bStopTx_2 ;
// }



// void ThreadTest::test_createTx(uint32_t tranNum, std::vector<std::string> addrs_,int timeout)
// {
//     DEBUGLOG("test_createTx start at {}", MagicSingleton<TimeUtil>::GetInstance()->getUTCTimestamp());
//     Cycliclist<std::string> addrs;

//     std::string defautlBase = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
//     for (auto &U : addrs_)
//     {
//         if (U == defautlBase)
//         {
//         }
//         else
//         {
//             addrs.push_back(U);
//         }
//     }

//     auto iter=addrs.begin();
//     while (bStopTx_2==false)
//     {
//         MagicSingleton<TFSBenchmark>::GetInstance()->SetTransactionInitiateBatchSize(tranNum);
//         for (int i = 0; i < tranNum; i++)
//         {
           
//             std::string from = iter->data;
//             iter++;
//             std::string to = iter->data;
//             std::thread th(ThreadTest::TestCreateTx_2, from, to);
//             th.detach();
            
//         }
//         sleep(timeout);
//     }
// }

// void Create_multi_thread_automatic_transaction()
// {
//     bool isRun=true;
//     while(isRun){
//     std::cout << "1. tx " << std::endl;
//     std::cout << "2. close" << std::endl;


//     int check=0;
//      std::cout << "chose:" ;
//      std::cin >> check;

//      if(check==1){
//        if(bStopTx_2==true){
//             //没有进行并发
//             bStopTx_2=false;
//        }else {
//             //  正在进行并发
//             std::cout << "has run" << std::endl;
//             continue;
//        }
//      }else if(check ==2){
//         bStopTx_2=true;
//         break;
//      }else{
//         std::cout<< " invalui" << std::endl;
//         continue;
//      }

//     int TxNum = 0;
//     int timeout = 0;
//     //交易间隔时间
//     std::cout << "Interval time (seconds):";
//     std::cin >> timeout;
//     //每次发的交易数
//     std::cout << "Interval frequency :" ;

//     std:: cin >> TxNum;


//     std::vector<std::string> addrs_;

   


//     MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();
//     MagicSingleton<EDManager>::GetInstance()->GetAccountList(addrs_);

     
//     std::thread th(ThreadTest::test_createTx,TxNum, addrs_, timeout);
//     th.detach();

//     }
    
// }

// void TestCreateStake_2(const std::string &from)
// {
//     TxHelper::PledgeType pledgeType = TxHelper::PledgeType::kPledgeType_Node;
//     uint64_t stake_amount = 10  * global::ca::kDecimalNum ;

//     DBReader data_reader;
//     uint64_t top = 0;
//     if (DBStatus::DB_SUCCESS != data_reader.GetBlockTop(top))
//     {
//         ERRORLOG("db get top failed!!");
//         return;
//     }


//     CTransaction outTx;
//     TxHelper::vrfAgentType isNeedAgent_flag;
//     Vrf info_;
//     std::vector<TxHelper::Utxo> outVin;
//     if(TxHelper::CreateStakeTransaction(from, stake_amount, top + 1, pledgeType, outTx, outVin,isNeedAgent_flag,info_) != 0)
//     {
//         return;
//     }
//     MagicSingleton<TranMonitor>::GetInstance()->AddTranMonitor(outTx);
//     std::cout << " from: " << from << " amout: " << stake_amount << std::endl;
//     TxMsgReq txMsg;
//     txMsg.set_version(global::kVersion);
//     TxMsgInfo * txMsgInfo = txMsg.mutable_txmsginfo();
//     txMsgInfo->set_type(0);
//     txMsgInfo->set_tx(outTx.SerializeAsString());
//     txMsgInfo->set_height(top);

//     if(isNeedAgent_flag== TxHelper::vrfAgentType::vrfAgentType_vrf)
//     {
//         Vrf * new_info = txMsg.mutable_vrfinfo();
//         new_info->CopyFrom(info_);
//     }
//     auto msg = std::make_shared<TxMsgReq>(txMsg);
//     int ret = 0;
//     std::string defaultBase58Addr = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();
//     if (isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf && outTx.identity() != defaultBase58Addr)
//     {

//         ret = DropshippingTx(msg, outTx);
//     }
//     else
//     {
//         ret = DoHandleTx(msg, outTx);
//     }

//     if (ret != 0)
//     {
//         ret -= 100;
//     }
//     DEBUGLOG("Transaction result,ret:{}  txHash:{}", ret, outTx.hash());
//     MagicSingleton<TranMonitor>::GetInstance()->SetDoHandleTxStatus(outTx, ret);
// }


// void Create_multi_thread_automatic_stake_transaction()
// {
//     std::vector<std::string> addrs;

//     MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();
//     MagicSingleton<EDManager>::GetInstance()->GetAccountList(addrs);

//     std::vector<std::string>::iterator it = std::find(addrs.begin(), addrs.end(), MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr());
//     if (it != addrs.end())
//     {
//         addrs.erase(it);
//     }

//     for (int i = 0; i <= addrs.size(); ++i)
//     {
//         std::thread th(TestCreateStake_2, addrs[i]);
//         th.detach();
//     }
// }

// void TestCreateInvestment(const std::string &strFromAddr, const std::string &strToAddr, const std::string &amountStr)
// {

//     TxHelper::InvestType investType = TxHelper::InvestType::kInvestType_NetLicence;

//     uint64_t invest_amount = std::stod(amountStr) * global::ca::kDecimalNum;

//     // DBReader db_reader;
//     DBReader data_reader;
//     uint64_t top = 0;
//     if (DBStatus::DB_SUCCESS != data_reader.GetBlockTop(top))
//     {
//         ERRORLOG("db get top failed!!");
//         return;
//     }


//     CTransaction outTx;
//     std::vector<TxHelper::Utxo> outVin;
//     TxHelper::vrfAgentType isNeedAgent_flag;
//     Vrf info_;
//     int ret = TxHelper::CreateInvestTransaction(strFromAddr, strToAddr, invest_amount, top + 1, investType,outTx, outVin,isNeedAgent_flag,info_);
// 	if(ret != 0)
// 	{
//         ERRORLOG("Failed to create investment transaction! The error code is:{}", ret);
//         return;
//     }
//     MagicSingleton<TranMonitor>::GetInstance()->AddTranMonitor(outTx);
//     TxMsgReq txMsg;
//     txMsg.set_version(global::kVersion);
//     TxMsgInfo *txMsgInfo = txMsg.mutable_txmsginfo();
//     txMsgInfo->set_type(0);
//     txMsgInfo->set_tx(outTx.SerializeAsString());
//     txMsgInfo->set_height(top);

//     if (isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf)
//     {
//         Vrf * new_info=txMsg.mutable_vrfinfo();
//         new_info->CopyFrom(info_);
//     }

//     auto msg = std::make_shared<TxMsgReq>(txMsg);

//     std::string defaultBase58Addr = MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr();

//     if (isNeedAgent_flag == TxHelper::vrfAgentType::vrfAgentType_vrf && outTx.identity() != defaultBase58Addr)
//     {

//         ret = DropshippingTx(msg, outTx);
//     }
//     else
//     {
//         ret = DoHandleTx(msg, outTx);
//     }

//     MagicSingleton<TranMonitor>::GetInstance()->SetDoHandleTxStatus(outTx, ret);
//     std::cout << "=====Transaction initiator:" << strFromAddr << std::endl;
//     std::cout << "=====Transaction recipient:" << strToAddr << std::endl;
//     std::cout << "=====Transaction amount:" << amountStr << std::endl;
//     std::cout << "=======================================================================" << std::endl
//               << std::endl
//               << std::endl
//               << std::endl;
// }

// void Auto_investment()
// {

//     std::cout << "input aummot: ";
//     std::string aummot;
//     std::cin >> aummot;

//     std::vector<std::string> addrs;

//     MagicSingleton<EDManager>::GetInstance()->PrintAllAccount();
//     MagicSingleton<EDManager>::GetInstance()->GetAccountList(addrs);

//     std::vector<std::string>::iterator it = std::find(addrs.begin(), addrs.end(), MagicSingleton<EDManager>::GetInstance()->GetDefaultBase58Addr());
//     if (it != addrs.end())
//     {
//         addrs.erase(it);
//     }

//     int i = 0;
//     while (i < addrs.size())
//     {
//         std::string from;
//         std::string to;
//         from = addrs[i];
//         if ((i + 1) >= addrs.size())
//         {
//             i = 0;
//         }
//         else
//         {
//             i += 1;
//         }

//         to = addrs[i];

//         if (from != "")
//         {
//             if (!MagicSingleton<EDManager>::GetInstance()->IsExist(from))
//             {
//                 DEBUGLOG("Illegal account.");
//                 return;
//             }
//         }
//         else
//         {
//             DEBUGLOG("Illegal account. from base58addr is null !");
//             return;
//         }
//         std::thread th(TestCreateInvestment, from, to, aummot);
//         th.detach();
//         if (i == 0)
//         {
//             return;
//         }
//         sleep(1);
//     }
// }

// void print_verify_node()
// {
//     std::vector<Node> nodelist = MagicSingleton<PeerNode>::GetInstance()->get_nodelist();

//     vector<Node> result_node;
//     for (const auto &node : nodelist)
//     {
//         int ret = VerifyBonusAddr(node.base58address);

//         int64_t stake_time = ca_algorithm::GetPledgeTimeByAddr(node.base58address, global::ca::StakeType::kStakeType_Node);
//         if (stake_time > 0 && ret == 0)
//         {
//             result_node.push_back(node);
//         }
//     }

//     std::string fileName = "verify_node.txt";
//     std::ofstream filestream;
//     filestream.open(fileName);
//     if (!filestream)
//     {
//         std::cout << "Open file failed!" << std::endl;
//         return;
//     }

//     filestream << "------------------------------------------------------------------------------------------------------------" << std::endl;
//     for (auto &i : result_node)
//     {
//         filestream
//             << "  base58(" << i.base58address << ")"
//             << std::endl;
//     }
//     filestream << "------------------------------------------------------------------------------------------------------------" << std::endl;
//     filestream << "PeerNode size is: " << result_node.size() << std::endl;
// }

// void print_block_cache()
// {
//     std::cout << "input height :";
//     int height;
//     std::cin >> height;
//     std::map<uint64_t, std::set<CBlock, CBlockCompare>> _cache;
//     MagicSingleton<CBlockCache>::GetInstance()->GetCache(_cache);
//     auto iter = _cache.begin();
//     for (; iter != _cache.end(); ++iter)
//     {
//         if (iter->first == height)
//         {
//             for (auto block : iter->second)
//             {
//                 std::cout << block.hash() << std::endl;
//             }
//         }
//     }
// }

// int GetAllBase58(std::set<std::string> &userlist, uint64_t& top)
// {
//     CBlockDataApi data_reader;
//     uint64_t best_top;
//     if (DBStatus::DB_SUCCESS != data_reader.GetBlockTop(best_top))
//     {
//         ERRORLOG("DB read failed!");
//         return -1;
//     }
//     if(top > best_top)
//     {
//         ERRORLOG("top > best_top");
//         return -2;
//     }
//     DEBUGLOG("start PrintUserBase58ToText");
//     for (int i = 0; i <= top; i++)
//     {
//         cout << "top = " << i << endl;

//         std::vector<CBlock> blocks;
//         if (DBStatus::DB_SUCCESS != data_reader.GetBlocksByBlockHeight(i, blocks))
//         {
//             return -6;
//         }

//         for (auto &Block : blocks)
//         {
//             for (int j = 0; j < Block.txs_size(); j++)
//             {
//                 CTransaction tx = Block.txs(j);
//                 for (int k = 0; k < tx.utxo().vout_size(); k++)
//                 {
//                     CTxOutput txout = tx.utxo().vout(k);
//                     userlist.insert(txout.addr());
//                 }
//             }
//         }
//     }

//     DEBUGLOG("join to set successful!");

//     return 0;
// }

// int PrintBalanceToTxt(const std::set<std::string> &base58_list, const std::string &textpath)
// {
//     if (base58_list.empty())
//     {
//         ERRORLOG("base58_list is empty !");
//         return -1;
//     }
//     if (textpath.size() == 0)
//     {
//         ERRORLOG("textpath is zero");
//         return -2;
//     }
//     stringstream os;
//     DBReader DB_read;
//     ofstream file(textpath);
//     if (!file.is_open())
//     {
//         ERRORLOG("PrintBalanceToTxt::open feil failed!");
//         return -3;
//     }
//     int i = 0;
//     for (auto base58 : base58_list)
//     {
//         i++;
//         uint64_t tatol = 0;

//         os << "\t" << base58 << ":\n";
//         cout<<"addr>:"<<base58<<"--进度-->:"<<i<<"/"<<base58_list.size()<<endl;
//         std::vector<std::string> addr_utxo_hashs;

//         if (DBStatus::DB_SUCCESS != DB_read.GetUtxoHashsByAddress(base58, addr_utxo_hashs))
//         {
//             ERRORLOG("PrintBalanceToTxt:GetUtxoHashsByAddress failed!");
//             continue;
//         }

//         auto utxoOutput = [base58, addr_utxo_hashs, &DB_read](ostream &stream)
//         {
//             stream << " ordinary utxo list :\n" << "开始--------------账户:" << base58 <<std::endl;

//             uint64_t total = 0;
//             for (auto i : addr_utxo_hashs)
//             {
//                 std::string txRaw;
//                 DB_read.GetTransactionByHash(i, txRaw);

//                 CTransaction tx;
//                 tx.ParseFromString(txRaw);

//                 uint64_t value = 0;
//                 for (int j = 0; j < tx.vout_size(); j++)
//                 {
//                     CTxout txout = tx.vout(j);
//                     if (txout.scriptpubkey() != base58)
//                     {
//                         continue;
//                     }
//                     value += txout.value();
//                 }
//                 stream << i << " : " << value << std::endl;
//                 total += value;
//             }

//             stream << "普通--结束----------------地址: " << base58 << " 普通UTXO 总数：" << addr_utxo_hashs.size() << " 普通UTXO 总值：" << total << std::endl
//                                <<std::endl
//                                <<std::endl
//                                <<std::endl;
//         };

//         utxoOutput(file);
//         std::vector<string> utxoes;
//         DBReader db_reader;
//         auto db_status = db_reader.GetStakeAddressUtxo(base58, utxoes);
//         if(db_status != DBStatus::DB_SUCCESS)
//         {
//             continue;
//         }
//         stringstream os;
//         uint64_t total_pledge = 0;

//         os << "质押的utxo list:"<<"\n";
//         for (size_t i = 0 ; i < utxoes.size(); i++)
//         {

//             std::string strUtxo = utxoes[i];

//             std::string serTxRaw;
//             db_status = db_reader.GetTransactionByHash(strUtxo, serTxRaw);
//             if (DBStatus::DB_SUCCESS != db_status)
//             {
//                 ERRORLOG("Get stake tx error");
//                 continue;
//             }

//             CTransaction tx;
//             tx.ParseFromString(serTxRaw);

//             if (tx.vout_size() != 2)
//             {
//                 ERRORLOG("invalid tx");
//                 continue;
//             }

//             if (tx.hash().length() == 0)
//             {
//                 ERRORLOG("Get stake tx error");
//                 continue;
//             }

//             uint64_t value = 0;
//             for (int i = 0; i < tx.vout_size(); i++)
//             {
//                 CTxout txout = tx.vout(i);
//                 if (txout.scriptpubkey() == global::ca::kVirtualStakeAddr)
//                 {
//                     value = txout.value();
//                     total_pledge += value;
//                     break;
//                 }

//             }
//             os << utxoes[i] << " : " << value << std::endl;

//         }
//         os << "地址: " << base58 << " 质押UTXO 总数：" << utxoes.size() << "质押 UTXO 总值：" << total_pledge <<"\n";
//         os << "质押--结束:-----------------账户:"<<base58<<"\n"<<"\n"<<"\n";

//         file << os.str();
//     }

//     file.close();

//     return 0;
// }




int ca_endTimerTask()
{
    // global::ca::kBlockPoolTimer.Cancel();
    global::ca::kDataBaseTimer.Cancel();
    return 0;
}

void ca_cleanup()
{
    // MagicSingleton<CBlockCache>::GetInstance()->FlushBlockManually();
    ca_endTimerTask();
   // DBDestory();
}