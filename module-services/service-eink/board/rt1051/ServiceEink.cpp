/*
 * ServiceEink.cpp
 *
 *  Created on: 22 maj 2019
 *      Author: robert
 */


//module-utils
#include "log/log.hpp"
//module-vfs
#include "vfs.hpp"
//module-bsp
extern "C"  {
#include "ED028TC1.h"
#include "chip.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "eink/dma_config.h"
}

#include "ServiceEink.hpp"

/// This is DMA handle for internal frame buffer memory-to-memory copying operation
static edma_handle_t            s_einkMemcpyDma_handle;

/**
 * @brief This function is a callback for the memory to memory Eink HW copying completed event
 * @param
 */
static void s_EinkServiceDMAMemcpyCallback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (isIRQ())
    {
        BaseType_t xhigherpriorityTaskToBeWokenUp = 0;
        xTaskNotifyFromISR((TaskHandle_t)param, 0, eNoAction, &xhigherpriorityTaskToBeWokenUp);
        portEND_SWITCHING_ISR( xhigherpriorityTaskToBeWokenUp );
    }
    else
    {
        xTaskNotify((TaskHandle_t)param, 0, eNoAction);
    }
}

ServiceEink::ServiceEink(const std::string& name)
	: sys::Service(name),
	  timerID { 0 },
	  selfRefereshTriggerCount{ 0 },
	  temperatureMeasurementTriggerCount{ 0 },
	  powerOffTriggerCount{ 0 }
{
	LOG_INFO("[ServiceEink] Initializing");

	memset(&waveformSettings, 0, sizeof(EinkWaveFormSettings_t));
	waveformSettings.temperature = -1000;

	EinkStatus_e einkStatus = EinkResetAndInitialize();

	if (einkStatus != EinkOK)
	{
		LOG_FATAL("Error: Could not initialize Eink display!\n");
	}

	DMAMUX_EnableAlwaysOn(BSP_EINK_MEMCPY_DMA_DMAMUX_BASE, BSP_EINK_MEMCPY_DMA_CH, true);
	DMAMUX_EnableChannel(BSP_EINK_MEMCPY_DMA_DMAMUX_BASE, BSP_EINK_MEMCPY_DMA_CH);

	EDMA_CreateHandle(&s_einkMemcpyDma_handle, BSP_EINK_MEMCPY_DMA_DMA_BASE, BSP_EINK_MEMCPY_DMA_CH);
	EDMA_SetCallback(&s_einkMemcpyDma_handle, s_EinkServiceDMAMemcpyCallback, NULL);



	EinkPowerOn();

	 uint8_t s_einkAmbientTemperature = EinkGetTemperatureInternal();
	 LOG_INFO("EInk measured temperature: %d\u00B0C", s_einkAmbientTemperature);


	// Make the saturation to the lower limit
	if (s_einkAmbientTemperature < 0)
		s_einkAmbientTemperature = 0;

	// Make the saturation to the higher limit
	if (s_einkAmbientTemperature > 49)
		s_einkAmbientTemperature = 49;

	// Clear the temperature timer count


//	EinkUpdateWaveform(EinkWaveformGC16, s_einkAmbientTemperature);
	deepClearScreen( s_einkAmbientTemperature );
	EinkRefreshImage(0, 0, BOARD_EINK_DISPLAY_RES_X, BOARD_EINK_DISPLAY_RES_Y, EinkDisplayTimingsDeepCleanMode);
	EinkPowerOff();

	timerID = CreateTimer(1000,true);
	ReloadTimer(timerID);
}

ServiceEink::~ServiceEink(){
	LOG_INFO("[ServiceEink] Cleaning resources");
	//release data from settings
	if( waveformSettings.LUTCData != nullptr )
		delete [] waveformSettings.LUTCData;
	if( waveformSettings.LUTDData != nullptr )
		delete [] waveformSettings.LUTDData;
	//set sure that any new temperature will cause to load data from file.
	waveformSettings.temperature = -1000;

}

// Invoked upon receiving data message
sys::Message_t ServiceEink::DataReceivedHandler(sys::DataMessage* msgl) {
	return std::make_shared<sys::ResponseMessage>();
}

// Invoked when timer ticked
void ServiceEink::TickHandler(uint32_t id) {


}

// Invoked during initialization
sys::ReturnCodes ServiceEink::InitHandler() {
	return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceEink::DeinitHandler() {
	return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceEink::WakeUpHandler() {
	return sys::ReturnCodes::Success;
}


sys::ReturnCodes ServiceEink::SleepHandler() {
	return sys::ReturnCodes::Success;
}

bool ServiceEink::changeWaveform( EinkWaveforms_e mode, const int32_t temperature ) {

    // If neither the temperature nor the waveform has changed - do nothing
    if ((temperature == waveformSettings.temperature) &&
        (mode == waveformSettings.mode ))
    {
        return EinkOK;
    }

    const uint32_t LUTD_SIZE = 16385;
	const uint32_t LUTC_SIZE = 64;
	const uint32_t LUTR_SIZE = 256; ///< Needed due to \ref EINK_LUTS_FILE_PATH structure

	const uint32_t LUTS_TOTAL_SIZE = LUTD_SIZE + LUTC_SIZE + LUTR_SIZE;

	waveformSettings.temperature = temperature;

	unsigned int segment = 0;

	if (temperature < 38)
	{
		segment = temperature/3;
	}
	else
	{
		if (temperature < 43)
		{
			segment = 12;
		}
		else
		{
			segment = 13;
		}
	}

	uint32_t offset = 0;

	switch (mode)
	{
		case EinkWaveformINIT:
			offset = LUTS_TOTAL_SIZE*segment;
		break;

		case EinkWaveformA2:
			offset = LUTS_TOTAL_SIZE*(14+segment);
		break;

		case EinkWaveformDU2:
			offset = LUTS_TOTAL_SIZE*(28+segment);
		break;

		case EinkWaveformGLD16:
			offset = LUTS_TOTAL_SIZE*(42+segment);
		break;

		case EinkWaveformGC16:
		default:
			offset = LUTS_TOTAL_SIZE*(56+segment);
		break;
	}


    //Open file
	auto file = vfs.fopen( "sys/Luts.bin", "rb" );
    if ( file == nullptr )
    {
        LOG_FATAL("Could not find the LUTS.bin file. Returning");
        return false;
    }
    // Allocate memory for the LUTD data. +1 for the EinkLUTD command for SPI transfer
    if( waveformSettings.LUTDData != nullptr )
    	delete [] waveformSettings.LUTDData;

    waveformSettings.LUTDSize = 0;
    waveformSettings.LUTDData = new uint8_t[ LUTD_SIZE + 1];
//    uint8_t* bufferLutD = (uint8_t*)malloc(LUTD_SIZE + 1);

    if(waveformSettings.LUTDData == nullptr )
    {
        LOG_ERROR("Could not allocate memory for the LUTD array");
//        vfs_fclose(lutFileDescriptor);
        vfs.fclose( file );
//        return EinkNoMem;
        return false;
    }

    // Allocate memory for the LUTC data. +1 for the EinkLUTC command for SPI transfer
    if( waveformSettings.LUTCData != nullptr )
    	delete [] waveformSettings.LUTCData;

    waveformSettings.LUTCSize = LUTC_SIZE;
    waveformSettings.LUTCData = new uint8_t[ LUTC_SIZE + 1];
    if ( waveformSettings.LUTCData  == nullptr )
    {
        LOG_ERROR("Could not allocate memory for the LUTC array");
//        free(bufferLutD);
        vfs.fclose( file );
//        vfs_fclose(lutFileDescriptor);
//        return EinkNoMem;
        return false;
    }

    waveformSettings.LUTDData[0] = EinkLUTD;
    waveformSettings.LUTCData[0] = EinkLUTC;

    ///LUTD
    vfs.fseek( file, offset, FF_SEEK_SET );
    vfs.fread( &waveformSettings.LUTDData[1], 1, LUTD_SIZE, file );

    uint8_t frameCount = waveformSettings.LUTDData[1] + 1; // 0x00 - 1 frame, ... , 0x0F - 16 frames
    waveformSettings.LUTDSize = frameCount * 64 + 1 + 1; // (frameCount * 64) - size of actual LUT; (+1) - the byte containing frameCount; (+1) - EinkLUTD command

    //LUTC
	offset += LUTD_SIZE;
	vfs.fseek( file, offset, FF_SEEK_SET );
	vfs.fread( &waveformSettings.LUTCData[1], 1, LUTC_SIZE, file );

	vfs.fclose( file );

//	if (BSP_EinkWriteData( waveformSettings.LUTDData, waveformSettings.LUTDSize, SPI_AUTOMATIC_CS) != 0)
//	{
//		LOG_ERROR("Eink: transmitting the LUTD failed");
//		EinkResetAndInitialize();
//		return EinkSPIErr;
//	}
//	if (BSP_EinkWriteData(waveformSettings.LUTCData, waveformSettings.LUTCSize + 1, SPI_AUTOMATIC_CS) != 0)
//	{
//		LOG_ERROR("Eink: transmitting the LUTC failed");
//		EinkResetAndInitialize();
//		return EinkSPIErr;
//	}

	return true;
//	return EinkOK;
}

bool ServiceEink::deepClearScreen(int8_t temperature)
{
    EinkWaveforms_e wv = waveformSettings.mode;

	changeWaveform( EinkWaveformINIT, temperature );
    //EinkFillScreenWithColor(EinkDisplayColorWhite);
    EinkFillScreenWithColor(EinkDisplayColorBlack);
    //EinkFillScreenWithColor(EinkDisplayColorWhite);
    //EinkFillScreenWithColor(EinkDisplayColorBlack);
//    EinkFillScreenWithColor(EinkDisplayColorWhite);

    //changeWaveform(wv, temperature);

    return true;
}




