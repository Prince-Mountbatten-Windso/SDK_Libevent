#include <iostream>
#include "utils/MagicSingleton.h"
#include "utils/EDManager.h"
#include "utils/base64_2.h"
#include "utils/bip39.h"
#include "jcAPI.h"


using namespace std;

extern "C" {
//g++ -fPIC -c jcAPI.cpp
//g++ -shared *.o -o libjcAPI.so libphone.a

    //生成钱包
    int  GenWallet_( char *out_private_key, int *out_private_key_len,
                  char *out_public_key, int *out_public_key_len, 
                  char *out_bs58addr, int *out_bs58addr_len,
				  char *mnemonic)
    {   
        std::string out_pri_key;
        std::string out_pub_key;
		std::string base58;
		const uint32_t BUFF_SIZE = 512;
		const uint32_t OUTA_SIZE = 1024;

		ED  ed;
		out_pub_key = ed.pubStr;
		out_pri_key = ed.priStr;
		base58 = ed.base58Addr;
        
		
		memcpy(out_private_key, out_pri_key.data(), out_pri_key.size());
        *out_private_key_len = out_pri_key.size();

		std::string pri_hex = Str2Hex(out_pri_key);
		std::cout << "pri_hex = "<< pri_hex << std::endl;
		std::cout << "out_private_key_len = " << pri_hex.size() << std::endl;

        memcpy(out_public_key, out_pub_key.data(), out_pub_key.size());
        *out_public_key_len = out_pub_key.size();
		

		std::string pub_hex = Str2Hex(out_pub_key);
		std::cout << "pub_hex = " << pub_hex << std::endl;
		std::cout << "out_private_key_len = " << pub_hex.size() << std::endl;
		
		
		memcpy(out_bs58addr, base58.data(), base58.size());
        *out_bs58addr_len = base58.size();

		std::cout << "out_bs58addr = " << base58 << std::endl;
		std::cout << "out_bs58addr_len = " << base58.size() << std::endl;

		char out_data[OUTA_SIZE] = { 0 };
		int data_len = OUTA_SIZE;
		mnemonic_from_data((const uint8_t*)out_pri_key.data(), out_pri_key.size(), out_data, data_len);
		memcpy(mnemonic, out_data, strlen(out_data));
		std::cout << "out_data = "<< mnemonic << std::endl;
		std::cout << "data_len = "<< data_len << std::endl;

		std::cout << "---------------------------------------------" << std::endl;
		
        return 0;
    }

	int  GenSign_(const char* pri, int pri_len,const char* msg, int msg_len,char* signature_msg, int* out_len)
	{
		std::string pri_str(pri, pri_len);
		
		MagicSingleton<ED>::GetInstance()->ImportPrivateKeyHexNewAccount(pri_str);
		
		std::string message(msg, msg_len);
		std::string signature;
		
		if (!MagicSingleton<ED>::GetInstance()->Sign(message, signature))
		{
			return -1;
		}

		base64_encode((unsigned char *)signature.data(), signature.size(), (unsigned char *)signature_msg);
        *out_len = strlen(signature_msg);
		// memcpy(signature_msg, signature.data(), signature.size());
		// *out_len = signature.size();

		// std::string signature_message = Str2Hex(signature);
		 std::cout << "signature_msg = " << signature_msg << std::endl;
		 std::cout << "out_len = " << *out_len << std::endl;
		// std::cout << "signature_message size = " << signature_message.size() << std::endl;
		return 0;
	}


int  KeyFromPrivate_(const char * pridata,int pri_len, char *out_public_key, int *out_public_key_len,
		char *out_bs58addr, int *out_bs58addr_len, char *mnemonic)
{
		const uint32_t BUFF_SIZE = 512;
		const uint32_t OUTA_SIZE = 1024;
		std::string pristrdata(pridata,pri_len);
		MagicSingleton<ED>::GetInstance()->ImportPrivateKeyHexNewAccount(pristrdata);
		std::string strPub;
		std::string strpriv;
		std::string base58;
		MagicSingleton<ED>::GetInstance()->GetPrivateKey(strpriv);
		MagicSingleton<ED>::GetInstance()->GetPublicKey(strPub);
		MagicSingleton<ED>::GetInstance()->GetBase58Address(base58);
	
	
		memcpy(out_public_key, strPub.data(), strPub.size());
		*out_public_key_len = strPub.size();

		// char* pub_hex = new char[BUFF_SIZE * 2]{ 0 };
		// encode_hex(pub_hex, strPub.data(), strPub.size());
		
		std::string pub_hex = Str2Hex(strPub);
		std::cout << "pub_hex = " << pub_hex << std::endl;
		//std::cout << "out_public_key_len = " << *out_public_key_len << std::endl;
		std::cout << "pub_len = " << pub_hex.size() << std::endl;
		

		memcpy(out_bs58addr, base58.data(), base58.size());
		*out_bs58addr_len = base58.size();
		std::cout << "out_bs58addr = " << base58 << std::endl;
		std::cout << "out_bs58addr_len = " << base58.size() << std::endl;

		char out_data[OUTA_SIZE] = { 0 };
		int data_len = OUTA_SIZE;
		mnemonic_from_data((const uint8_t*)strpriv.data(), strpriv.size(), out_data, data_len);
		memcpy(mnemonic, out_data, strlen(out_data));
		std::cout << "out_data = "<< mnemonic << std::endl;
		std::cout << "data_len = "<< data_len << std::endl;


		// char out_data[OUTA_SIZE] = { 0 };
		// int data_len = OUTA_SIZE;
		// size_t buf_len = sizeof(out_data);
		// mnemonic_from_data((const uint8_t*)strpriv.data(), strpriv.size(), out_data, data_len);
		// memcpy(mnemonic, out_data, strlen(out_data));
		// std::cout << "mnemonic = " << mnemonic << std::endl;
		// std::cout << "buf_len = " << strlen(out_data) << std::endl;
	
		//delete[]pub_hex;
		return 0;
	}

	
	int  GenerateKeyFromMnemonic_(const char *mnemonic,
		char *out_private_key, int *out_private_len,
		char *out_public_key, int *out_public_key_len,
		char *out_bs58addr, int *out_bs58addr_len)
	{
		const uint32_t BUFF_SIZE = 512;
		char out[33] = {0};
		int outLen = 0;
		
		if(!mnemonic_check((char *)mnemonic, out, &outLen))
		{
			cout<<"mnemonic_check((char *)mnemonic, out, &outLen)"<<endl;
			return false;
		}
		
		std::string mnemonic_key(out,outLen);

		std::string pri_mnemonic_key =  Str2Hex(mnemonic_key);
		MagicSingleton<ED>::GetInstance()->ImportMnemonicSetKey(pri_mnemonic_key);
		std::string strPub;
		std::string strpriv;
		std::string base58;
		MagicSingleton<ED>::GetInstance()->GetPrivateKey(strpriv);
		MagicSingleton<ED>::GetInstance()->GetPublicKey(strPub);
		MagicSingleton<ED>::GetInstance()->GetBase58Address(base58);

		
		memcpy(out_private_key, strpriv.data(), strpriv.size());
		*out_private_len = strpriv.size();


		std::string pri_hex = Str2Hex(strpriv);
		std::cout << "*********pri_hex = " << pri_hex << std::endl;
		
		std::cout << "*******pri_len = " << pri_hex.size() << std::endl;
		memcpy(out_public_key, strPub.data(), strPub.size());
		*out_public_key_len = strPub.size();

		std::string pub_hex = Str2Hex(strPub);
		std::cout << "**************pub_hex = " << pub_hex << std::endl;
		std::cout << "**************pub_len = " << pub_hex.size() << std::endl;
		

		memcpy(out_bs58addr, base58.data(), base58.size());
		*out_bs58addr_len = base58.size();
		std::cout << "out_bs58addr = " << base58 << std::endl;
		std::cout << "out_bs58addr_len = " << base58.size() << std::endl;
		return 0;
	}


}
