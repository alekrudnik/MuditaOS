
/*
 * @file eMMC.cpp
 * @author Mateusz Piesta (mateusz.piesta@mudita.com)
 * @date 20.05.19
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */


#include "eMMC.hpp"

#if defined(TARGET_RT1051)
#include "board/rt1051/bsp/eMMC/fsl_mmc.h"
#elif defined(TARGET_Linux)

#else
#error "Unsupported target 1"
#endif


namespace bsp
{

    RetCode eMMC::Init()
    {
    #if defined(TARGET_RT1051)

        mmcCard.busWidth = kMMC_DataBusWidth8bit;
        mmcCard.busTiming = kMMC_HighSpeedTiming;
        mmcCard.enablePreDefinedBlockCount = true;
        mmcCard.host.base = BOARD_EMMC_USDHCx;
        mmcCard.host.sourceClock_Hz = BOARD_EMMC_USDHCx_CLK_FREQ;
        
        auto ret = MMC_Init(&mmcCard);
        if(ret != kStatus_Success){       
            return RetCode::Failure;
        }
        else{
            userPartitionBlocks = mmcCard.userPartitionBlocks;
            return RetCode::Success;
        }
    #else
    #error "Unsupported target"
    #endif

    }

    RetCode eMMC::DeInit()
    {
    #if defined(TARGET_RT1051)
        MMC_Deinit(&mmcCard);
        return RetCode::Success;
    #else
    #error "Unsupported target"
    #endif
    }

    RetCode eMMC::ReadBlocks(uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
    {
    #if defined(TARGET_RT1051)
        auto ret = MMC_ReadBlocks(&mmcCard,buffer,startBlock,blockCount);
        if(ret != kStatus_Success){
            return RetCode::Failure;
        }
        else{
            return RetCode::Success;
        }
    #else
    #error "Unsupported target"
    #endif
    }

    RetCode eMMC::WriteBlocks(const uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
    {
    #if defined(TARGET_RT1051)
        auto ret = MMC_WriteBlocks(&mmcCard,buffer,startBlock,blockCount);
        if(ret != kStatus_Success){
            return RetCode::Failure;
        }
        else{
            return RetCode::Success;
        }
    #else
    #error "Unsupported target"
    #endif
    }

    RetCode eMMC::SwitchPartition(eMMC::Partition partition)
    {
    #if defined(TARGET_RT1051)
        auto ret = MMC_SelectPartition(&mmcCard, static_cast<mmc_access_partition_t>(partition));
        if(ret != kStatus_Success){
            return RetCode::Failure;
        }
        else{
            return RetCode::Success;
        }
    #else
    #error "Unsupported target"
    #endif
    }

}