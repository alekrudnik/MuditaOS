﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "TS0710.h"
#include "bsp/cellular/bsp_cellular.hpp"
#include "bsp/cellular/CellularResult.hpp"
#include "projdefs.h"
#include <service-cellular/ServiceCellular.hpp>
#include <service-cellular/SignalStrength.hpp>
#include <service-cellular/CellularMessage.hpp>
#include <cassert>
#include <memory>
#include <module-os/RTOSWrapper/include/ticks.hpp>
#include <sstream>

std::map<TypeOfFrame_e, std::string> TypeOfFrame_text = {{TypeOfFrame_e::SABM, "SABM"},
                                                         {TypeOfFrame_e::UA, "UA"},
                                                         {TypeOfFrame_e::DM, "DM"},
                                                         {TypeOfFrame_e::DISC, "DISC"},
                                                         {TypeOfFrame_e::UIH, "UIH"},
                                                         {TypeOfFrame_e::UI, "UI"},
                                                         {TypeOfFrame_e::I, "I"}};
std::map<PortSpeed_e, int> QuectelCMUXPortSpeeds_text = {{PortSpeed_e::PS9600, 1},
                                                         {PortSpeed_e::PS19200, 2},
                                                         {PortSpeed_e::PS38400, 3},
                                                         {PortSpeed_e::PS57600, 4},
                                                         {PortSpeed_e::PS115200, 5},
                                                         {PortSpeed_e::PS230400, 6},
                                                         {PortSpeed_e::PS460800, 7}};
std::map<PortSpeed_e, int> ATPortSpeeds_text          = {{PortSpeed_e::PS9600, 9600},
                                                {PortSpeed_e::PS19200, 19200},
                                                {PortSpeed_e::PS38400, 38400},
                                                {PortSpeed_e::PS57600, 57600},
                                                {PortSpeed_e::PS115200, 115200},
                                                {PortSpeed_e::PS230400, 230400},
                                                {PortSpeed_e::PS460800, 460800}};
/**
 * TS0710 implementation
 */

#ifndef SERIAL_PORT
#define SERIAL_PORT "/dev/null"
#endif

#define USE_DAEFAULT_BAUDRATE 1

static constexpr uint16_t threadSizeWords = 2048;

TS0710::TS0710(PortSpeed_e portSpeed, sys::Service *parent)
{
    LOG_INFO("Serial port: '%s'", SERIAL_PORT);
    SetStartParams(portSpeed);
    Init(parent);
}

TS0710::~TS0710()
{
    for (auto it : channels) {
        delete it;
    }
    channels.clear();
    TS0710_CLOSE pv_TS0710_Close = TS0710_CLOSE();
    mode                         = Mode::AT;
    if (taskHandle) {
        vTaskDelete(taskHandle);
    }

    delete parser;
}

void TS0710::Init(sys::Service *parent)
{
    pv_cellular = bsp::Cellular::Create(SERIAL_PORT, 115200).value_or(nullptr);
    parser      = new ATParser(pv_cellular.get());
    pv_parent   = parent;

    BaseType_t task_error =
        xTaskCreate(workerTaskFunction, "TS0710Worker", threadSizeWords, this, taskPriority, &taskHandle);
    if (task_error != pdPASS) {
        LOG_ERROR("Failed to start inputSerialWorker task");
        return;
    }
}

void TS0710::SetStartParams(PortSpeed_e portSpeed)
{
    startParams.PortSpeed               = portSpeed;
    startParams.MaxFrameSize            = 127; // maximum for Basic mode
    startParams.AckTimer                = 10;  // 100ms default
    startParams.MaxNumOfRetransmissions = 3;   // default
    startParams.MaxCtrlRespTime         = 30;  // 300ms default
    startParams.WakeUpRespTime          = 10;  // 10s default
    startParams.ErrRecovWindowSize      = 2;   // 2 default
}

TS0710_Frame::frame_t createCMUXExitFrame()
{
    TS0710_Frame::frame_t frame;
    frame.Address = 0 | static_cast<unsigned char>(MuxDefines ::GSM0710_CR);
    frame.Control = TypeOfFrame_e::UIH;
    frame.data.push_back(static_cast<uint8_t>(MuxDefines::GSM0710_CONTROL_CLD) |
                         static_cast<uint8_t>(MuxDefines::GSM0710_EA) | static_cast<uint8_t>(MuxDefines::GSM0710_CR));
    frame.data.push_back(0x01);
    return frame;
}

enum class BaudTestStep
{
    baud460800_NoCmux,
    baud460800_Cmux,
    baud115200_NoCmux,
    baud115200_Cmux,
    baud_NotFound,
};

const char *c_str(BaudTestStep step)
{
    switch (step) {
    case BaudTestStep::baud460800_NoCmux:
        return "baud460800_NoCmux";
    case BaudTestStep::baud460800_Cmux:
        return "baud460800_Cmux";
    case BaudTestStep::baud115200_NoCmux:
        return "baud115200_NoCmux";
    case BaudTestStep::baud115200_Cmux:
        return "baud115200_Cmux";
    default:
        return "baud_NotFound";
    }
}

bool BaudDetectTestAT(ATParser *parser, BaudTestStep &step, BaudTestStep nextStep)
{
    if (parser != nullptr) {
        LOG_DEBUG("=> Baud detection step: %s -> %s", c_str(step), c_str(nextStep));
        step = nextStep;
        if (parser->cmd(at::AT::AT)) {
            return true;
        }
    }
    return false;
}

void CloseCmux(std::unique_ptr<bsp::Cellular> &pv_cellular)
{
    LOG_INFO("Closing mux mode");
    TS0710_Frame::frame_t frame = createCMUXExitFrame();
    pv_cellular->Write((void *)frame.serialize().data(), frame.serialize().size());
    vTaskDelay(1000); // GSM module needs some time to close multiplexer
}

void TS0710::setMode(TS0710::Mode mode)
{
    this->mode = mode;
}

TS0710::ConfState TS0710::BaudDetectOnce()
{
    bool result           = false;
    BaudTestStep lastStep = BaudTestStep::baud_NotFound;
    BaudTestStep step     = BaudTestStep::baud460800_NoCmux;

    while (!result) {
        switch (step) {
        case BaudTestStep::baud460800_NoCmux:
            pv_cellular->SetSpeed(ATPortSpeeds_text[PortSpeed_e::PS460800]);
            lastStep = step;
            result   = BaudDetectTestAT(parser, step, BaudTestStep::baud460800_Cmux);
            break;
        case BaudTestStep::baud460800_Cmux:
            CloseCmux(pv_cellular);
            lastStep = step;
            result   = BaudDetectTestAT(parser, step, BaudTestStep::baud115200_NoCmux);
            break;
        case BaudTestStep::baud115200_NoCmux:
            pv_cellular->SetSpeed(ATPortSpeeds_text[PortSpeed_e::PS115200]);
            lastStep = step;
            result   = BaudDetectTestAT(parser, step, BaudTestStep::baud115200_Cmux);
            break;
        case BaudTestStep::baud115200_Cmux:
            CloseCmux(pv_cellular);
            lastStep = step;
            result   = BaudDetectTestAT(parser, step, BaudTestStep::baud_NotFound);
            break;
        case BaudTestStep::baud_NotFound:
            pv_cellular->SetSpeed(ATPortSpeeds_text[PortSpeed_e::PS115200]); // set port speed to default 115200
            LOG_ERROR("No Baud found for modem.");
            return ConfState::Failure;
            break;
        }
    }
    LOG_DEBUG("Baud found: %s", c_str(lastStep));
    return ConfState::Success;
}

TS0710::ConfState TS0710::BaudDetectProcedure(uint16_t timeout_s)
{
    at::Result ret;
    bool timed_out     = false;
    auto timeout_ticks = cpp_freertos::Ticks::GetTicks() + pdMS_TO_TICKS(timeout_s * 1000);

    while (!timed_out) {
        auto baud_result = BaudDetectOnce();
        timed_out        = cpp_freertos::Ticks::GetTicks() > timeout_ticks;
        if (baud_result == ConfState::Success) {
            return ConfState::Success;
        }
    }
    pv_cellular->SetSpeed(ATPortSpeeds_text[PortSpeed_e::PS115200]); // set port speed to default 115200
    LOG_DEBUG("No Baud found.");
    return ConfState::Failure;
}

// TODO:M.P Fetch configuration from JSON/XML file
TS0710::ConfState TS0710::ConfProcedure()
{
    LOG_DEBUG("Configuring modem...");

    if (!parser->cmd(at::AT::FACTORY_RESET)) {
        return ConfState::Failure;
    }
    if (!parser->cmd(at::AT::ECHO_OFF)) {
        return ConfState::Failure;
    }

    at::AT flowCmd;
    if (hardwareControlFlowEnable) {
        flowCmd = (at::AT::FLOW_CTRL_ON);
    }
    else {
        flowCmd = (at::AT::FLOW_CTRL_OFF);
    }
    if (!parser->cmd(flowCmd)) {
        return ConfState::Failure;
    }

    LOG_INFO("GSM modem info:");
    auto ret = parser->cmd(at::AT::SW_INFO);
    if (ret) {
        for (uint32_t i = 0; i < ret.response.size() - 1; ++i) // skip final "OK"
        {
            LOG_INFO("%s", ret.response[i].c_str());
        }
    }

    auto commands = at::getCommadsSet(at::commadsSet::modemInit);

    for (auto command : commands) {
        if (!parser->cmd(command)) {
            return ConfState::Failure;
        }
    }

    LOG_WARN("TODO: determine while this retry loop is necessary");
    bool timed_out                 = false;
    constexpr uint32_t qsclkTmeout = 30;
    const auto qsclkTmeoutTicks =
        cpp_freertos::Ticks::GetTicks() + pdMS_TO_TICKS(qsclkTmeout * utils::time::milisecondsInSecond);
    while (!timed_out) {
        if (parser->cmd(at::AT::QSCLK_ON)) {
            break;
        }
        // if error then limit polling - 1 poll per sec modem normaly takes ~ 20 sec to start anyway
        vTaskDelay(pdMS_TO_TICKS(utils::time::milisecondsInSecond));
        timed_out = cpp_freertos::Ticks::GetTicks() > qsclkTmeoutTicks;
        if (timed_out) {
            return ConfState::Failure;
        }
    }

    return ConfState ::Success;
}

TS0710::ConfState TS0710::AudioConfProcedure()
{
    auto ret = parser->cmd(at::AT::QDAI);
    // There is possibility for SendATCommand to capture invalid response (it can be ERROR or async URC)
    // Hence we are checking here for beginning of valid response for at::QDAI command. AudioConfProcedure
    // procedure will be invoked from AudioService's context as many times as needed.
    if (!ret) {
        return ConfState ::Failure;
    }
    else if (ret.response[0].compare("+QDAI: 1,0,0,3,0,1,1,1") == 0) {
        parser->cmd(at::AT::QRXGAIN);
        parser->cmd(at::AT::CLVL);
        parser->cmd(at::AT::QMIC);
        SetupEchoCalceller(EchoCancellerStrength::Tuned);
        return ConfState ::Success;
    }
    else {
        if (!parser->cmd(at::AT::QDAI_INIT)) {
            return ConfState ::Failure;
        }
        else {
            pv_cellular->Restart();
            LOG_DEBUG("GSM module first run, performing reset...");
            return ConfState::ModemNeedsReset;
        }
    }
}

TS0710::ConfState TS0710::StartMultiplexer()
{
    LOG_DEBUG("Configuring multiplexer...");

    // 1. start CMUX by AT command AT+CMUX=...(with given parameters) & get response
    /*
    AT+CMUX=<mode>[,<subset>[,<portspeed>[,<N1>[,<T1>[,<N2>[,<T2>[,<T3>[,<k>]]]]]]]]
    <mode> 0 Basic option.
    <subset> This parameter defines the way in which the MUX control channel is set up.
        Virtual channel may subsequently be set up differently, but in the absence of any
        negotiation for the settings of a virtual channel, it shall be set up according to thecontrol channel <subset>
    setting. 0 UIH frames used only. 1 UI frames used only. 2 I frames used only. <portspeed> 1 9600bit/s 2 19200bit/s
        3 38400bit/s
        4 57600bit/s
        5 115200bit/s
        6 230400bit/s
        7 460800bit/s
    <N1> Maximum frame size, the range is 1-32768, 127 is the default value for basic option (see <mode>).
    <T1> The time UE waits for an acknowledgement before resorting to other action (e.g.
        transmitting a frame).The step size is ten milliseconds, the range is 1-255. The default value is 100ms.
    <N2> Maximum number of re-transmissions, the range is 0-255, the default value is 3.
    <T2> Response timer for MUX control channel, the step size is ten milliseconds, the range is 2-255, 30 is the
    default value. <T3> Wake up response timer in seconds. The range is 1-255, 10 is the default value. <k> Window size
    (It is not supported for UC20)

    PortSpeed_e PortSpeed;          //!< Port speed
    int MaxFrameSize;               //!< Maximum Frame Size [ 1-128 in Basic mode, 1-512 in HDLC modes, default: 31 for
    basic option & 64 for advanced ] int AckTimer;                   //!< Acknowledgement Timer [0,01s-2,55s, default:
    0,1s] int MaxNumOfRetransmissions;    //!< Maximum number of retransmissions [0-100, default: 3] int
    MaxCtrlRespTime;            //!< Response timer for the multiplexer control channel [0,01s-2,55s, default: 0,3s] int
    WakeUpRespTime;             //!< Wake up response timer [1s-255s, default: 10s] int ErrRecovWindowSize;         //!<
    Window size for error recovery mode [1-7, default: 2]
    */

    // enable cmux
    LOG_DEBUG("Enabling CMUX");
    std::stringstream ss;
    ss << std::string(at::factory(at::AT::CMUX));
    ss << "0,0";
    ss << "," << QuectelCMUXPortSpeeds_text[startParams.PortSpeed];
    ss << "," << startParams.MaxFrameSize;
    ss << "," << startParams.AckTimer;
    ss << "," << startParams.MaxNumOfRetransmissions;
    ss << "," << startParams.MaxCtrlRespTime;
    ss << "," << startParams.WakeUpRespTime;
    ss << "," << startParams.ErrRecovWindowSize;
    ss << "\r";

    if (!parser->cmd(ss.str())) {
        LOG_ERROR("CMUX setup error");
        return ConfState::Failure;
    }

    mode                          = Mode::CMUX_SETUP;
    TS0710_START *pv_TS0710_Start = new TS0710_START(TS0710_START::Mode_e::Basic, startParams, pv_cellular.get());
    // wait for confirmation
    if (pv_TS0710_Start->ConnectionStatus()) {
        channels.push_back(pv_TS0710_Start->getCtrlChannel()); // store control channel
    }
    delete pv_TS0710_Start;

    controlCallback = [this](std::string &data) {
        auto frameData = data;

        if (frameData.size() < 4) {
            LOG_ERROR("frame too short");
            return;
        }
        switch (frameData[0]) {
        case 0xE3: { // MSC
            LOG_PRINTF("[MSC ch #%i] ", frameData[2] >> 2);
            if (frameData[3] & 0x80)
                LOG_PRINTF("DV ");
            if (frameData[3] & 0x40)
                LOG_PRINTF("IC ");
            if (frameData[3] & 0x08)
                LOG_PRINTF("RTR ");
            if (frameData[3] & 0x04) {
                LOG_PRINTF("RTC ");
                this->getCellular()->SetSendingAllowed(true);
            }
            else
                this->getCellular()->SetSendingAllowed(false);
            if (frameData[3] & 0x02)
                LOG_PRINTF("FC ");

            LOG_PRINTF("\n");
        } break;
        }
    };

    // channels[0]->setCallback(controlCallback);

    // TODO: Open remaining channels
    OpenChannel(Channel::Commands);
    OpenChannel(Channel::Notifications);
    OpenChannel(Channel::Data);

    mode           = Mode::CMUX;
    DLC_channel *c = get(Channel::Commands);
    if (c != nullptr) {
        // Route URCs to second (Notifications) MUX channel
        c->cmd(at::AT::SET_URC_CHANNEL);
        LOG_DEBUG("Sending test ATI");
        auto res = c->cmd(at::AT::SW_INFO);
        res      = c->cmd(at::AT::CSQ);
        if (res) {
            auto beg       = res.response[0].find(' ');
            auto end       = res.response[0].find(',', 1);
            auto input_val = res.response[0].substr(beg + 1, end - beg - 1);
            auto strength  = 0;
            try {
                strength = std::stoi(input_val);
            }
            catch (const std::exception &e) {
                LOG_ERROR("Conversion error of %s, taking default value %d", input_val.c_str(), strength);
            }

            SignalStrength signalStrength(strength);
            if (signalStrength.isValid()) {
                Store::GSM::get()->setSignalStrength(signalStrength.data);
                auto msg = std::make_shared<CellularNotificationMessage>(
                    CellularNotificationMessage::Type::SignalStrengthUpdate);
                pv_parent->bus.sendMulticast(msg, sys::BusChannel::ServiceCellularNotifications);
            }
        }
        else {
            LOG_ERROR("signal strength not set");
        }
    }
    else {
        LOG_FATAL("No channel");
        return ConfState::Failure;
    }

    return ConfState::Success;
}

static bool parseCellularResultCMUX(std::vector<uint8_t> &currentFrame, std::vector<uint8_t> &previousData)
{
    static bool frameStartDetected = false;
    for (auto el : previousData) {
        if (frameStartDetected || el == TS0710_FLAG) {
            frameStartDetected = true;
            currentFrame.push_back(el);

            // Check if frame is complete only in case of TS0710_FLAG
            if (el == TS0710_FLAG) {
                if (TS0710_Frame::isComplete(currentFrame)) {
                    frameStartDetected = false;
                    return true;
                }
            }
        }
    }

    return false;
}

static void sendFrameToChannel(TS0710 *inst, bsp::cellular::CellularFrameResult &frameResult)
{
    for (auto *chan : inst->getChannels()) {
        if (frameResult.getFrame().isMyChannel(chan->getDLCI())) {
            chan->ParseInputData(&frameResult);
        }
    }
}

[[noreturn]] void workerTaskFunction(void *ptr)
{
    TS0710 *inst = static_cast<TS0710 *>(ptr);
    bsp::cellular::CellularDMAResult result;
    std::vector<uint8_t> previousData;
    std::vector<uint8_t> currentFrame;

    while (true) {
        // TODO get real maximum CellularDMAResult instance result
        auto receivedBytes = inst->pv_cellular->Read(&result, 1024, UINT32_MAX);

        if (receivedBytes > 0) {
            // AT mode is used only during initialization phase
            if (inst->mode == TS0710::Mode::AT) {
                LOG_DEBUG("[Worker] Processing AT response");
                inst->parser->ProcessNewData(inst->pv_parent, result);
            }
            // CMUX mode is default operation mode
            else if (inst->mode == TS0710::Mode::CMUX) {
                // LOG_DEBUG("[Worker] Processing CMUX response");
                previousData.insert(previousData.end(), result.getData().begin(), result.getData().end());

                if (parseCellularResultCMUX(currentFrame, previousData)) {
                    bsp::cellular::CellularFrameResult frameResult{currentFrame, result.getResultCode()};
                    sendFrameToChannel(inst, frameResult);
                    currentFrame.clear();
                }
                else {
                    if (!currentFrame.empty() && currentFrame[0] == TS0710_FLAG) {
                        previousData = currentFrame;
                    }
                }
            }
        }
    }
}

void TS0710::SelectAntenna(bsp::cellular::antenna antenna)
{
    pv_cellular->SelectAntenna(antenna);
}

bsp::cellular::antenna TS0710::GetAntenna()
{
    return pv_cellular->GetAntenna();
}

void TS0710::InformModemHostWakeup(void)
{
    return pv_cellular->InformModemHostWakeup();
}

bool TS0710::IsModemActive(void)
{
    return bsp::cellular::status::getStatus() == bsp::cellular::status::value::ACTIVE;
}

void TS0710::TurnOnModem(void)
{
    return pv_cellular->PowerUp();
}

void TS0710::ResetModem(void)
{
    return pv_cellular->Restart();
}

void TS0710::TurnOffModem(void)
{
    return pv_cellular->PowerDown();
}

void TS0710::EnterSleepMode(void)
{
    return pv_cellular->EnterSleep();
}

void TS0710::ExitSleepMode(void)
{
    return pv_cellular->ExitSleep();
}

void TS0710::SetupEchoCalceller(EchoCancellerStrength strength)
{
    switch (strength) {
    case EchoCancellerStrength::LeastAggressive:
        // Aggressive settings
        parser->cmd(at::factory(at::AT::QEEC) + "0,2048");
        parser->cmd(at::factory(at::AT::QEEC) + "5,14");
        parser->cmd(at::factory(at::AT::QEEC) + "10,140");
        parser->cmd(at::factory(at::AT::QEEC) + "21,16000");
        parser->cmd(at::factory(at::AT::QEEC) + "22,300");
        parser->cmd(at::factory(at::AT::QEEC) + "24,450");
        parser->cmd(at::factory(at::AT::QEEC) + "33,640");
        break;
    case EchoCancellerStrength::Medium:
        // Aggressive settings
        parser->cmd(at::factory(at::AT::QEEC) + "0,2048");
        parser->cmd(at::factory(at::AT::QEEC) + "5,14");
        parser->cmd(at::factory(at::AT::QEEC) + "10,160");
        parser->cmd(at::factory(at::AT::QEEC) + "21,19000");
        parser->cmd(at::factory(at::AT::QEEC) + "22,600");
        parser->cmd(at::factory(at::AT::QEEC) + "24,600");
        parser->cmd(at::factory(at::AT::QEEC) + "33,768");
        break;
    case EchoCancellerStrength::Aggressive:
        // Aggressive settings
        parser->cmd(at::factory(at::AT::QEEC) + "0,2048");
        parser->cmd(at::factory(at::AT::QEEC) + "5,14");
        parser->cmd(at::factory(at::AT::QEEC) + "10,160");
        parser->cmd(at::factory(at::AT::QEEC) + "21,25000");
        parser->cmd(at::factory(at::AT::QEEC) + "22,12000");
        parser->cmd(at::factory(at::AT::QEEC) + "24,768");
        parser->cmd(at::factory(at::AT::QEEC) + "33,896");
        break;
    case EchoCancellerStrength::Tuned:
        /*
        The following steps describe the echo tuning workflow.
        1. Tune the echo path delay.
            a) Start with AT+QEEC=5,65436        （delay= -100）
            b) Make call to check the echo performance and record the value.
            c) Increase the delay in steps of 50, until 400, and check the echo performance at each delay value.
            d) Set the parameter to the value that yielded the best echo performance.

        2. Tune the tail of echo.
            a) Start with AT+QEEC=21,20000
            b) Make call to check the echo performance and record the value.
            c) Increase the DENS_tail_alpha in steps of 500, until 30000, and check the echo performance at each value.
            d) Set the parameter to the value that yielded the best echo performance.
            e) Start with AT+QEEC=22,6000
            f) Make call to check the echo performance and record the value.
            g) Increase the DENS_tail_portion in steps of 500, until 30000, and check the echo performance at each
        value. h) Set the parameter to the value that yielded the best echo performance.
        */
        parser->cmd(at::factory(at::AT::QEEC) + "0,2048");
        parser->cmd(at::factory(at::AT::QEEC) + "5,40"); // best performance on experiments in step 1
        parser->cmd(at::factory(at::AT::QEEC) + "10,160");
        parser->cmd(at::factory(at::AT::QEEC) + "21,26600"); // best performance on experiments in step 2c
        parser->cmd(at::factory(at::AT::QEEC) + "22,20000"); // best performance on experiments in step 2g
        parser->cmd(at::factory(at::AT::QEEC) + "24,768");
        parser->cmd(at::factory(at::AT::QEEC) + "33,896");
    };
}
