#ifndef __CA_GLOBAL_H__
#define __CA_GLOBAL_H__
#include <unordered_set>

#include "common/global.h"
#include "proto/ca_protomsg.pb.h"
#include "utils/CTimer.hpp"
// #include "ca_txconfirmtimer.h"



namespace global{

    namespace ca{

        // data
        #ifdef PRIMARYCHAIN
            static const std::string kInitAccountBase58Addr = "1zzF8jNBAXFTPJzhvY45zLKgQmnPLBZ93";
            static const std::string kGenesisBlockRaw = "1080c0f3fe9087fd021a40636231336163616665623232323033356338663838666163336261343961396663356537616530376634656130316636396631346361663732626664313734302240303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303240356231373262643032303961303230333861396630383932333132656335343535333935356238666266663039343639396434666562613263626166613031343aff031080c0f3fe9087fd0222223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e437054392a403562313732626430323039613032303338613966303839323331326563353435353339353562386662666630393436393964346665626132636261666130313432fe020a223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e4370543912b6010a420a403030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303012700a407f73b768efd00ca1d8407088441f2be25c79fef65bd1865840badac03383c64de79c5ef6a019676a50ce63c169a125e6adde0b0ac3e2aa21785a99a551b67a09122c302a300506032b6570032100284f67841c6164c647cd2c0428e8aff2925059710a3a6bcbc57acdb67ed28e261a2d088080b68be8ceb70c12223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e4370543922700a403991dec26c413c1e4415cf45ba93d291d03b88f88303db0f9b8ac7a822e8bdf1f47b75b2e727310627290980b1198714b8ecd3cd36cc987f757dcb8b6f4ce10b122c302a300506032b6570032100284f67841c6164c647cd2c0428e8aff2925059710a3a6bcbc57acdb67ed28e263a0747656e6573697348ffffffff0f42287b224e616d65223a225472616e73666f726d657273222c2254797065223a2247656e65736973227d";
            static const uint64_t kGenesisTime = 1675900800000000;
            static const std::string kConfigJson = "{\"http_callback\":{\"ip\":\"\",\"path\":\"\",\"port\":0},\"http_port\":11190,\"info\":{\"logo\":\"\",\"name\":\"\"},\"ip\":\"\",\"log\":{\"console\":false,\"level\":\"OFF\",\"path\":\"./logs\"},\"server\":[\"36.153.199.186\",\"36.154.216.186\"],\"server_port\":11187,\"sync_data\":{\"count\":50,\"interval\":100},\"thread_num\":256,\"version\":\"1.0\"}";
        #elif TESTCHAIN
            static const std::string kInitAccountBase58Addr = "1zzF8jNBAXFTPJzhvY45zLKgQmnPLBZ93";
            static const std::string kGenesisBlockRaw = "1080c0f3fe9087fd021a40636231336163616665623232323033356338663838666163336261343961396663356537616530376634656130316636396631346361663732626664313734302240303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303240356231373262643032303961303230333861396630383932333132656335343535333935356238666266663039343639396434666562613263626166613031343aff031080c0f3fe9087fd0222223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e437054392a403562313732626430323039613032303338613966303839323331326563353435353339353562386662666630393436393964346665626132636261666130313432fe020a223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e4370543912b6010a420a403030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303012700a407f73b768efd00ca1d8407088441f2be25c79fef65bd1865840badac03383c64de79c5ef6a019676a50ce63c169a125e6adde0b0ac3e2aa21785a99a551b67a09122c302a300506032b6570032100284f67841c6164c647cd2c0428e8aff2925059710a3a6bcbc57acdb67ed28e261a2d088080b68be8ceb70c12223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e4370543922700a403991dec26c413c1e4415cf45ba93d291d03b88f88303db0f9b8ac7a822e8bdf1f47b75b2e727310627290980b1198714b8ecd3cd36cc987f757dcb8b6f4ce10b122c302a300506032b6570032100284f67841c6164c647cd2c0428e8aff2925059710a3a6bcbc57acdb67ed28e263a0747656e6573697348ffffffff0f42287b224e616d65223a225472616e73666f726d657273222c2254797065223a2247656e65736973227d";
            static const uint64_t kGenesisTime = 1675900800000000;
            static const std::string kConfigJson = "{\"http_callback\":{\"ip\":\"\",\"path\":\"\",\"port\":0},\"http_port\":11190,\"info\":{\"logo\":\"\",\"name\":\"\"},\"ip\":\"\",\"log\":{\"console\":false,\"level\":\"OFF\",\"path\":\"./logs\"},\"server\":[\"36.153.199.186\",\"36.154.216.186\"],\"server_port\":11187,\"sync_data\":{\"count\":50,\"interval\":100},\"thread_num\":256,\"version\":\"1.0\"}";
        #else // DEVCHAIN
            static const std::string kInitAccountBase58Addr = "1zzF8jNBAXFTPJzhvY45zLKgQmnPLBZ93";
            static const std::string kGenesisBlockRaw = "1080c0f3fe9087fd021a40636231336163616665623232323033356338663838666163336261343961396663356537616530376634656130316636396631346361663732626664313734302240303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303240356231373262643032303961303230333861396630383932333132656335343535333935356238666266663039343639396434666562613263626166613031343aff031080c0f3fe9087fd0222223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e437054392a403562313732626430323039613032303338613966303839323331326563353435353339353562386662666630393436393964346665626132636261666130313432fe020a223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e4370543912b6010a420a403030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303012700a407f73b768efd00ca1d8407088441f2be25c79fef65bd1865840badac03383c64de79c5ef6a019676a50ce63c169a125e6adde0b0ac3e2aa21785a99a551b67a09122c302a300506032b6570032100284f67841c6164c647cd2c0428e8aff2925059710a3a6bcbc57acdb67ed28e261a2d088080b68be8ceb70c12223148344b5a7a31565a734d75674a717a793644747664684d6f343531636e4370543922700a403991dec26c413c1e4415cf45ba93d291d03b88f88303db0f9b8ac7a822e8bdf1f47b75b2e727310627290980b1198714b8ecd3cd36cc987f757dcb8b6f4ce10b122c302a300506032b6570032100284f67841c6164c647cd2c0428e8aff2925059710a3a6bcbc57acdb67ed28e263a0747656e6573697348ffffffff0f42287b224e616d65223a225472616e73666f726d657273222c2254797065223a2247656e65736973227d";
            static const uint64_t kGenesisTime = 1675900800000000;
            static const std::string kConfigJson = "{\"http_callback\":{\"ip\":\"\",\"path\":\"\",\"port\":0},\"http_port\":11190,\"info\":{\"logo\":\"\",\"name\":\"\"},\"ip\":\"\",\"log\":{\"console\":false,\"level\":\"OFF\",\"path\":\"./logs\"},\"server\":[\"36.153.199.186\",\"36.154.216.186\"],\"server_port\":11187,\"sync_data\":{\"count\":50,\"interval\":100},\"thread_num\":256,\"version\":\"1.0\"}";
        #endif

        // consensus
        static const int kConsensus = 8;

        // timer
        static CTimer kBlockPoolTimer("blockpool");
        static CTimer kSeekBlockTimer("SeekBlock");
        static CTimer kDataBaseTimer("database");
        // mutex
        static std::mutex kBonusMutex;
        static std::mutex kInvestMutex;
        static std::mutex kBlockBroadcastMutex;

        // ca
        const uint64_t kDecimalNum = 100000000;
        const double   kFixDoubleMinPrecision = 0.000000005;
        const uint64_t kTotalAwardAmount = 130000000;
        const uint64_t kM2 = 70000000;
        const uint64_t kMinStakeAmt = (uint64_t)((double)1000 * kDecimalNum);
        const uint64_t kMinInvestAmt = (uint64_t)((double)10000 * kDecimalNum);
        const std::string kGenesisSign = "Genesis";
        const std::string kTxSign = "Tx";
        const std::string kGasSign = "Gas";
        const std::string kBurnSign = "Burn";
        const std::string kVirtualStakeAddr = "VirtualStake";
        const std::string kVirtualInvestAddr = "VirtualInvest";
        const std::string kVirtualBurnGasAddr = "VirtualBurnGas";
        const uint64_t kUpperBlockHeight = 4;
        const uint64_t kLowerBlockHeight = 1;
        const std::string kStakeTypeNet = "Net";
        const std::string kInvestTypeNormal = "Normal";
        const uint64_t kMinUnstakeHeight = 500;
        const uint64_t kMaxBlockSize = 1024 * 1024 * 1;
        const std::string kVirtualDeployContractAddr = "VirtualDeployContract";

        const int KSign_node_threshold = 10;
        const int kNeed_node_threshold = 30;

        const uint64_t kMaxSendSize = 100;

        const int TxTimeoutMin = 30;

        const uint64_t kVerifyRange = 600;

        enum class StakeType
        {
            kStakeType_Unknown = 0,
            kStakeType_Node = 1
        };
        
        // Transacatione Type
        enum class TxType
        {
            kTxTypeGenesis = -1,
            kTxTypeUnknown, // unknown
            kTxTypeTx, //normal transaction
            kTxTypeStake, //stake
            kTxTypeUnstake, //unstake
            kTxTypeInvest, //invest
            kTxTypeDisinvest, //disinvest
            kTxTypeDeclaration, //declaration
            kTxTypeDeployContract,
            kTxTypeCallContract,
            kTxTypeBonus = 99//bonus
        };

        // Sync
        enum class SaveType
        {
            SyncNormal,
            SyncFromZero,
            Broadcast,
            Unknow
        };

        enum class BlockObtainMean
        {
            Normal,
            ByPreHash,
            ByUtxo
        };
        const uint64_t sum_hash_range = 100;
        // const std::string sync_buffer_path = "./buffer/";

        //test
        static int TxNumber = 0;
    }
}


#endif