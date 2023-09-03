#ifndef _CA_HANDLE_EVENT_H_
#define _CA_HANDLE_EVENT_H_

#include <memory>
#include "proto/interface.pb.h"
#include "proto/block.pb.h"
#include  "proto/interface.pb.h"
#include "proto/sdk.pb.h"
#include "../net/message_queue.h"


//void RegisterCaCallback();

//void HandleGetUtxoAck(const std::shared_ptr<GetUtxoAck> &ack, const MsgData &from);

int  HandleGetUtxo(const std::shared_ptr<GetBalanceAck> &ack, const MsgData &from);

void set_Fromaddr(const std::vector<std::string> &fromaddr);
void set_toaddr(const std::string &toaddr);
void set_amount(const std::string &amount);
void set_utxo(const std::string &utxo);
void set_curtime(const uint64_t &time);


int  SendToNode(std::shared_ptr<TxMsgReq> msg, std::pair<std::string, uint64_t>);
int  HandleGetSDKInfoReq(const std::vector<std::string> &fromAddr);




int HandleSdkTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);
int HandleTransactionTxMsgAck(const std::shared_ptr<TxMsgAck> &ack, const MsgData &from);




int HandleTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);

int HandleStakedTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);

int HandleUnStakedTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);

int HandleinvestTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);

int HandledisinvestTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);

int HandleBonusTransaction(const std::shared_ptr<GetSDKAck> &ack, const MsgData &from);

#endif