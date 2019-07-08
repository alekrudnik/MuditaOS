/*
 *  @file ServiceCellular.cpp
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 03.07.19
 *  @brief  
 *  @copyright Copyright (C) 2019 mudita.com
 *  @details
 */



#include "ServiceCellular.hpp"
#include "Service/Service.hpp"
#include "Service/Message.hpp"

#include "Modem/MuxDaemon.hpp"

#include "MessageType.hpp"

const char *ServiceCellular::serviceName = "ServiceCellular";


ServiceCellular::ServiceCellular()
        : sys::Service(serviceName, 1024 * 4, sys::ServicePriority::Idle) {
    LOG_INFO("[ServiceCellular] Initializing");

    testTimerID = CreateTimer(3000,true);
    ReloadTimer(testTimerID);

    muxdaemon = MuxDaemon::Create();


/*    vTaskDelay(5000);
    muxdaemon.reset();

    muxdaemon = MuxDaemon::Create();*/

}

ServiceCellular::~ServiceCellular() {

    LOG_INFO("[ServiceCellular] Cleaning resources");
}

// Invoked when timer ticked
void ServiceCellular::TickHandler(uint32_t id) {
    char* resp = "AT\r";
    //muxdaemon->WriteMuxFrame(2, reinterpret_cast<unsigned char *>(resp),strlen(resp), static_cast<unsigned char>(MuxDefines::GSM0710_TYPE_UIH));
}

// Invoked during initialization
sys::ReturnCodes ServiceCellular::InitHandler() {

    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceCellular::DeinitHandler() {

    return sys::ReturnCodes::Success;
}

sys::ReturnCodes ServiceCellular::WakeUpHandler() {
    return sys::ReturnCodes::Success;
}


sys::ReturnCodes ServiceCellular::SleepHandler() {
    return sys::ReturnCodes::Success;
}

sys::Message_t ServiceCellular::DataReceivedHandler(sys::DataMessage *msgl) {
    std::shared_ptr<sys::ResponseMessage> responseMsg;

    switch (static_cast<MessageType >(msgl->messageType)) {
        default:
            break;
    }
}