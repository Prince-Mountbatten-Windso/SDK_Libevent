#ifndef TFS_CA_H
#define TFS_CA_H

#include <iostream>
#include <thread>
#include <shared_mutex>
#include "proto/transaction.pb.h"
#include "proto/ca_protomsg.pb.h"
#include "proto/sdk.pb.h"


extern "C" {

void InitStart();

bool InitAccount(const char *path,int path_len);

void handle_SetdefaultAccount();


void Set_defaultAccount(const char* base58 ,int base58_len);



void Add_Account(int num,int iVer);




bool Delete_Account(const char* base58 ,int base58_len);


bool Import_Account(const char* pri_key ,int pri_key_len);



void Export_private_key(const char * base58,int base58_len,
                        char *mnemonic, 
                        char *out_private_key, int *out_private_len,
                        char *out_public_key,int *out_public_key_len);




typedef int (* txdata_callback)(const char *, int,int);
typedef void(* txgas_callback)(int) ;

void set_phonegasptr(txgas_callback gasback_ptr);

void set_phoneptr(txdata_callback back_ptr);


bool Require_config_random_node(const char *ip ,uint32_t ip_len,uint32_t port);


bool require_balance_height();

bool handle_transaction(const char *FromAddr,int fromlen,
                        const  char *ToAddr, int tolen,
                        const char *Amt,int amtlen);


bool handle_stake(const char *FromAddr,int fromlen,
                  const char * StakeFee,int amtlen);


bool handle_unstake(const char *FromAddr,int fromlen,
                    const char *UtxoHash,int hashlen);




bool handle_invest(const char *FromAddr,int fromlen,
                   const  char *ToAddr, int tolen,
                   const char *Amt,int amtlen);





bool handle_disinvest(const char *FromAddr,int fromlen,
                      const char *ToAddr, int tolen,
                      const char * UtxoHash,int hashlen);


bool handle_bonus(const char *FromAddr,int fromlen);

void handle_AccountManger();


void gen_key();



 void handle_export_private_key();


}

#endif
