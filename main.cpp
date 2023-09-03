#include <iostream>
#include <string>
#include <vector>
#include<unistd.h>
#include <regex>
#include <cstring>
#include <limits>
#include <iosfwd>
#include "main.h"


//#include "include/logging.h"
//#include "common/global.h"
#include "net/net_api.h"
#include "utils/MagicSingleton.h"
//#include "ca/ca_handle_event.h"
#include "utils/base58.h"
// #include "proto/ca_protomsg.pb.h"
// #include "proto/sdk.pb.h"
#include "net/debug.h"
#include "net/Net.h"
// #include "net/message_process.h"
#include "utils/EDManager.h"
//#include "common/config.h"
#include "ca/ca_AdvancedMenu.h"
// #include "net/dispatcher.h"
//#include "net/PEvent"
#include "net/connect.h"

#include "ca/ca.h"
#include "ca/jcAPI.h"



using namespace std;


int main(int argc, char *argv[])
{
	InitStart();
	std::string path = "/root/zhao_sdk/sdk_libevent/build_dev_debug/bin/dong/";
	InitAccount(path.data(),path.size());
	InitAccount(path.data(),path.size());
	std::string message;
	int code = -1;
	int type = -1 ;
	phone_input(message.data(),code,type);
	set_phoneptr(phone_input);
	uint64_t gas = 0;
	phone_gas(gas);
	set_phonegasptr(phone_gas);
	menu();
	return 0;
}




// bool InitAccount(const std::string &path)
// {
// 	if(!path.empty())
// 	{
// 		MagicSingleton<Recver>::GetInstance()->set_accountpath(path.data());
// 	}
// 	MagicSingleton<EDManager>::GetInstance();
// 	return true;
// }

void phone_gas(int gas)
{
	cout<<"phone_gas gas = "<< gas <<endl;
	return ;
}

int phone_input(const char *message, int code,int type)
{
	cout<<"phone_input message = "<< message<<endl;
	cout<<"phone_input code = "<< code <<endl;
	cout<<"type = "<< type << endl;
	return 0;
}


// bool init(const std::string &path)
// {
// 	// Initialize account
//     if(!InitAccount(path))
// 	{
// 		std::cout << "Failed to initialize certification!" << std::endl;
// 		return false;		
// 	}
// 	return true;
// }



void GenWallet_test()
{   
    const int BUFF_SIZE = 128;
    char *out_private_key = new char[BUFF_SIZE]{0};
    int *out_private_key_len = new int{BUFF_SIZE};
    char *out_public_key = new char[BUFF_SIZE]{0};
    int *out_public_key_len = new int{BUFF_SIZE}; 
    char *out_bs58addr = new char[BUFF_SIZE]{0};
    int *out_bs58addr_len = new int{BUFF_SIZE};
    char *out_mnemonic = new char[1024]{0};

   GenWallet_(out_private_key, out_private_key_len, out_public_key, out_public_key_len, out_bs58addr, out_bs58addr_len,out_mnemonic);
 }

void KeyFromPrivate_test()
{   
    const int BUFF_SIZE = 128;
    char *out_private_key = new char[BUFF_SIZE]{0};
    int *out_private_key_len = new int{BUFF_SIZE};
    char *out_public_key = new char[BUFF_SIZE]{0};
    int *out_public_key_len = new int{BUFF_SIZE}; 
    char *out_bs58addr = new char[BUFF_SIZE]{0};
    int *out_bs58addr_len = new int{BUFF_SIZE};
    char *out_mnemonic = new char[1024]{0};

    std::string pri_key;
    std::cout << "Please input private key :" << std::endl;
    std::cin >> pri_key;
    KeyFromPrivate_(pri_key.data(),pri_key.size(), out_public_key, out_public_key_len,out_bs58addr, out_bs58addr_len, out_mnemonic);
  
 }

void GenerateKeyFromMnemonic__test()
{   
    const int BUFF_SIZE = 128;
    char *out_private_key = new char[BUFF_SIZE]{0};
    int *out_private_key_len = new int{BUFF_SIZE};
    char *out_public_key = new char[BUFF_SIZE]{0};
    int *out_public_key_len = new int{BUFF_SIZE}; 
    char *out_bs58addr = new char[BUFF_SIZE]{0};
    int *out_bs58addr_len = new int{BUFF_SIZE};
    char *out_mnemonic = new char[1024]{0};
    std::string str;
    std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
    std::getline(std::cin,str);
    GenerateKeyFromMnemonic_(str.data(),out_private_key, out_private_key_len,out_public_key, out_public_key_len,out_bs58addr, out_bs58addr_len);
}

void GenSign__test()
{   
    //const char* msg = 
    std::string msg = "fdafdae";
    //int msg_len = strlen(msg);
    //std::cout << "msg_len " << msg_len << std::endl;
    char *signature_msg = new char[91]{0};
    int *out_len = new int{91};


    std::string pri_key;
    std::cout << "Please input private key :" << std::endl;
    std::cin >> pri_key;

    
    GenSign_(pri_key.data(), pri_key.size(), msg.data(), msg.size(), signature_msg, out_len);
    std::string signature(signature_msg,*out_len);
    int ret = MagicSingleton<ED>::GetInstance()->Verify(msg,signature);
    // if(ed.Verify(sig_name, signature) == false)
    //     {
    //         std::cout << "evp verify fail" << std::endl;
    //     }
    std::cout << "out_len " << *out_len << std::endl;
    std::cout << "ret " << ret << std::endl;
 }


void menu()
{
	
	while (true)
	{
		std::cout << std::endl << std::endl;
		std::cout << "1.Transaction" << std::endl;
		std::cout << "2.Stake" << std::endl;
		std::cout << "3.Unstake" << std::endl;
		std::cout << "4.Invest" << std::endl;
		std::cout << "5.Disinvest" << std::endl;
		std::cout << "6.Bonus"  << std::endl;
		// std::cout << "7.Deploy contract"  << std::endl;
		// std::cout << "8.Call contract"  << std::endl;
		std::cout << "9.Advanced_Menu" << std::endl;
    	std::cout << "10.PrintAccountInfo" << std::endl;
		std::cout << "11.Generate wallet" << std::endl;
		std::cout << "12.Generate Key From Mnemonic" << std::endl;
		std::cout << "13.Key From Private" << std::endl;
		std::cout << "14.Gen Sign" << std::endl;
		std::cout << "15.connect node" << std::endl;
		std::cout << "0.Exit" << std::endl;


		std::string strKey;
		std::cout << "Please input your choice: "<< std::endl;
		std::cin >> strKey;	    
		std::regex pattern("^[0-9]|([1][0-9])$");
		if(!std::regex_match(strKey, pattern))
        {
            std::cout << "Invalid input." << std::endl;
            continue;
        }
        int key = std::stoi(strKey);
		switch (key)
		{			
			case 0:
				std::cout << "Exiting, bye!" << std::endl;
				return ;
			case 1:
			{
				std::string strFromAddr;
				std::cout << "input FromAddr :" << std::endl;
				std::cin >> strFromAddr;
				if (!CheckBase58Addr(strFromAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return ;
				}
				
				std::string strToAddr;
				std::cout << "input ToAddr :" << std::endl;
				std::cin >> strToAddr;
				if (!CheckBase58Addr(strToAddr))
				{
					std::cout << "input ToAddr error!" << std::endl;
					return ;
				}
				
				std::string strAmt;
				std::cout << "input amount :" << std::endl;
				std::cin >> strAmt;
				std::regex pattern("^\\d+(\\.\\d+)?$");

				handle_transaction(strFromAddr.data(),strFromAddr.size(),strToAddr.data(),strToAddr.size(),strAmt.data(),strAmt.size());
				break;
			}
			case 2:
			{
				std::cout << std::endl
              			   << std::endl;

				std::string strFromAddr;
				std::cout << "input FromAddr :" << std::endl;
				std::cin >> strFromAddr;
				if (!CheckBase58Addr(strFromAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return ;
				}
				
				std::string strStakeFee;
				std::cout << "Please enter the amount to stake:" << std::endl;
				std::cin >> strStakeFee;
				std::regex pattern("^\\d+(\\.\\d+)?$");
				if (!std::regex_match(strStakeFee, pattern))
				{
					std::cout << "input stake amount error " << std::endl;
					return;
				}
				
				handle_stake(strFromAddr.data(),strFromAddr.size(),strStakeFee.data(),strStakeFee.size());
			 	break;
			}
			case 3:
			{
				std::cout << std::endl
              	<< std::endl;

				std::string strFromAddr;
				std::cout << "Please enter unstake addr:" << std::endl;
				std::cin >> strFromAddr;
				if (!CheckBase58Addr(strFromAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return ;
				}
				
				std::string strUtxoHash;
				std::cout << "utxo:";
				std::cin >> strUtxoHash;

				handle_unstake(strFromAddr.data(),strFromAddr.size(),strUtxoHash.data(),strUtxoHash.size());
				break;
			}
			case 4:
			{
				std::cout << std::endl
              			   << std::endl;

				std::string strFromAddr;
				std::cout << "input FromAddr :" << std::endl;
				std::cin >> strFromAddr;
				if (!CheckBase58Addr(strFromAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return ;
				}
				
				std::string strToAddr;
				std::cout << "input ToAddr :" << std::endl;
				std::cin >> strToAddr;
				if (!CheckBase58Addr(strToAddr))
				{
					std::cout << "input ToAddr error!" << std::endl;
					return ;
				}
			
				std::string strInvestFee;
    			std::cout << "Please enter the amount to invest:" << std::endl;
    			std::cin >> strInvestFee;
				std::regex pattern("^\\d+(\\.\\d+)?$");
				if (!std::regex_match(strInvestFee, pattern))
				{
					std::cout << "input stake amount error " << std::endl;
					return;
				}
				
				handle_invest(strFromAddr.data(),strFromAddr.size(),strToAddr.data(),strToAddr.size(),strInvestFee.data(),strInvestFee.size());
                break;
			}
			case 5:
			{
				std::cout << std::endl
              	<< std::endl;

				std::string strFromAddr;
				std::cout << "Please enter your addr:" << std::endl;
				std::cin >> strFromAddr;
				if (!CheckBase58Addr(strFromAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return ;
				}
				
				
				std::string strToAddr;
				std::cout << "Please enter the addr you want to divest from:" << std::endl;
				std::cin >> strToAddr;
				if (!CheckBase58Addr(strToAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return;
				}
				
				std::string strUtxoHash;
				std::cout << "Please enter the utxo you want to divest:";
				std::cin >> strUtxoHash;
				
				handle_disinvest(strFromAddr.data(),strFromAddr.size(),strToAddr.data(),strToAddr.size(),strUtxoHash.data(),strUtxoHash.size());
				break;
			}
			case 6:
			{
				std::cout << std::endl
              	<< std::endl;

				std::string strFromAddr;
				std::cout << "Please enter your addr:" << std::endl;
				std::cin >> strFromAddr;
				if (!CheckBase58Addr(strFromAddr))
				{
					std::cout << "Input addr error!" << std::endl;
					return ;
				}			
				
				handle_bonus(strFromAddr.data(),strFromAddr.size());
                break;
			}
			 case 9:
                menu_advanced();
		 		break;
      		case 10:
			{
				require_balance_height();
		 		break;
			}
			case 11:
			{
				GenWallet_test();
				break;
			}			
			case 12:
			{
				GenerateKeyFromMnemonic__test();
				break;
			}
            case 13:
			{
				KeyFromPrivate_test();
				break;
			}
            case 14:
			{
				GenSign__test();
				break; 
			} 
			case 15:
			{
				uint32_t port = 41516 ;
				std::string ip = "192.168.1.67";
				Require_config_random_node(ip.data(),ip.size(),port);
				// std::cout << std::endl
              	// << std::endl;

				// std::string addr;
				// std::cout << "Please enter your addr:" << std::endl;
				// std::cin >> addr;
				// Set_defaultAccount(addr);
				break;	
			}
			// case 16:
			// {
			// 	std::cout << "please input num" << std::endl;
			// 	int num = 0;
			// 	std::cin >> num;
			// 	if (num <= 0)
			// 	{
			// 		return;
			// 	}

			// 	std::cout << "please input Normal addr or MultiSign addr" << std::endl;
			// 	std::cout << "0. Normal addr" << std::endl;
			// 	std::cout << "1. MultiSign addr" << std::endl;

			// 	int iVer = 0;
			// 	std::cin >> iVer;
			// 	Add_Account(num,iVer);
			// 	break;	
			// }
			// case 17:
			// {
			// 	std::cout << std::endl
            //   	<< std::endl;

			// 	std::string addr;
			// 	std::cout << "Please enter your addr:" << std::endl;
			// 	std::cin >> addr;
			// 	Delete_Account(addr);
			// 	break;	
			// }
			// case 18:
			// {
			// 	std::cout << std::endl
            //   	<< std::endl;

			// 	std::string addr;
			// 	std::cout << "Please enter your addr:" << std::endl;
			// 	std::cin >> addr;
			// 	Import_Account(addr);
			// 	break;	
			// }
			case 19:
			{
				std::cout << std::endl
              	<< std::endl;

				std::string addr;
				std::cout << "Please enter your addr:" << std::endl;
				std::cin >> addr;
				// std::string Mnemonic;
				// std::string PriHex;
				// std::string PubHex;

				const int BUFF_SIZE = 128;
				char *out_private_key = new char[BUFF_SIZE]{0};
				int *out_private_key_len = new int{BUFF_SIZE};
				char *out_public_key = new char[BUFF_SIZE]{0};
				int *out_public_key_len = new int{BUFF_SIZE}; 
				char *out_mnemonic = new char[1024]{0};
				Export_private_key(addr.data(),addr.size(),out_mnemonic, 
                        out_private_key,out_private_key_len,
                        out_public_key,out_public_key_len);
				std::cout<<"main-->Mnemonic"<<out_mnemonic<<std::endl;
				std::cout<<"main-->PriHex"<<out_private_key<<std::endl;
				std::cout<<"main-->PubHex"<<out_public_key<<std::endl;
				break;	
			}

			default:
			std::cout << "Invalid input." << std::endl;
			continue;
		}
		sleep(1);
	}
}
	
    













