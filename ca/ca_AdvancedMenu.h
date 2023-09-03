#ifndef __CA_ADVANCEDMENU_H_
#define __CA_ADVANCEDMENU_H_

#include <cstdint>




#pragma region FirstLevelMenu
//rootMenu
void menu_advanced();
#pragma endregion

//caMenu
void menu_ca();
// void gen_key();
// void in_private_key();
// void out_private_key();
// void get_account_balance();
// void print_all_block();
// void rollback();
// void CompPublicNodeHash();
// void GetAbnormalSignAddrList();
// void GetStakeList();

// int GetBounsAddrInfo();
//netMenu
void menu_net();
void send_message_to_user();
void show_my_k_bucket();
void kick_out_node();
void test_echo();
void print_req_and_ack();

#pragma region ThreeLevelMenu
//blockinfoMenu
void menu_blockinfo();
// void get_all_tx_number(uint64_t& top);
// void get_tx_block_info(uint64_t& top);

//testMenu
// void menu_test();
// void gen_mnemonic();
// void get_hashlist();
// void get_balance_by_utxo();
// void set_signfee();
// int imitate_create_tx_struct();
// void get_stake();
// void multi_tx();

// void get_all_pledge_addr();
// void auto_tx();
// void get_blockinfo_by_txhash();
// void get_failure_tx_list_info();
// void get_the_top_1000_hign_abnormal_signatures();
// void Create_multi_thread_automatic_transaction();
// void Create_multi_thread_automatic_stake_transaction();
// void Auto_investment();
// void print_verify_node();

// void get_qrcode();
// void get_block_cache_hash();
// void get_block_num();
// void get_utxo();
// void get_tx_hash_by_height();
// int PrintBalanceToTxt(const std::set<std::string>& base58_list,const std::string& textpath);
// int8_t CheckTransactionType(const CTransaction& tx,std::string& pType);
// void get_allutxoBlance();
// void get_investedNodeBlance();
// int GetAllBase58(std::set<std::string>& userlist,uint64_t& top);
// void print_database_block();
//void print_cache_block();
//nodeMenu
void menu_node();
// void print_block_cache();
// void printTxdata();
// void evmAddrConversion();
// void GenEvmAddress();
// void printBenchmark();

// namespace ThreadTest
// {
//     void TestCreateTx_2(const std::string& from,const std::string& to);
//     void test_createTx(uint32_t tranNum, std::vector<std::string> addrs, int sleepTime);
//     void set_StopTx_flag(const bool &flag);
//     void get_StopTx_flag(bool &flag);
// }




#pragma endregion
#endif
