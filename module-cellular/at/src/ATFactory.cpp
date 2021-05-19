// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ATFactory.hpp"
#include "cmd/CSCA.hpp"

namespace at
{
    using namespace std::chrono_literals;

    std::initializer_list<std::pair<const AT, const Cmd>> initializer = {
        {AT::AT, {"AT"}},
        {AT::ECHO_OFF, {"ATE0"}},
        {AT::FACTORY_RESET, {"AT&F"}},
        {AT::SW_INFO, {"ATI\r"}},
        {AT::FLOW_CTRL_ON, {"AT+IFC=2,2\r\n"}},
        {AT::FLOW_CTRL_OFF, {"AT+IFC=0,0"}},
        {AT::URC_NOTIF_CHANNEL, {"AT+QCFG=\"cmux/urcport\",1"}},
        {AT::RI_PIN_AUTO_CALL, {"AT+QCFG=\"urc/ri/ring\",\"auto\""}},
        {AT::RI_PIN_OFF_CALL, {"AT+QCFG=\"urc/ri/ring\",\"off\""}},
        {AT::RI_PIN_PULSE_SMS, {"AT+QCFG=\"urc/ri/smsincoming\",\"pulse\",450"}},
        {AT::RI_PIN_OFF_SMS, {"AT+QCFG=\"urc/ri/smsincoming\",\"off\""}},
        {AT::RI_PIN_PULSE_OTHER, {"AT+QCFG=\"urc/ri/other\",\"pulse\""}},
        {AT::URC_DELAY_ON, {"AT+QCFG=\"urc/delay\",1"}},
        {AT::URC_UART1, {"AT+QURCCFG=\"urcport\",\"uart1\""}},
        {AT::AT_PIN_READY_LOGIC, {"AT+QCFG=\"apready\",1,1,200"}},
        {AT::CSQ_URC_ON, {"AT+QINDCFG=\"csq\",1"}},
        {AT::CSQ_URC_OFF, {"AT+QINDCFG=\"csq\",0"}},
        {AT::CRC_ON, {"AT+CRC=1"}},
        {AT::CALLER_NUMBER_PRESENTATION, {"AT+CLIP=1", default_long_timeout}},
        {AT::SMS_TEXT_FORMAT, {"AT+CMGF=1"}},
        {AT::SMS_UCSC2, {"AT+CSCS=\"UCS2\""}},
        {AT::SMS_GSM, {"AT+CSCS=\"GSM\""}},
        {AT::QSCLK_ON, {"AT+QSCLK=1"}},
        {AT::QDAI, {"AT+QDAI?"}},
        {AT::QDAI_INIT, {"AT+QDAI=1,0,0,3,0,1,1,1"}},
        {AT::SET_URC_CHANNEL, {"AT+QCFG=\"cmux/urcport\",2"}},
        {AT::CSQ, {"AT+CSQ"}},
        {AT::CLCC, {"AT+CLCC"}},
        {AT::CMGD, {"AT+CMGD="}},
        {AT::CNUM, {"AT+CNUM"}},
        {AT::CIMI, {"AT+CIMI"}},
        {AT::QCMGR, {"AT+QCMGR=", 180s}},
        {AT::ATH, {"ATH", 100s}},
        {AT::QHUP_BUSY, {"AT+QHUP=17", 100s}},
        {AT::ATA, {"ATA", 100s}},
        {AT::ATD, {"ATD", 6s}},
        {AT::IPR, {"AT+IPR="}},
        {AT::CMUX, {"AT+CMUX="}},
        {AT::CFUN, {"AT+CFUN=", default_long_timeout}},
        {AT::CFUN_RESET, {"AT+CFUN=1,1", default_long_timeout}},
        {AT::CFUN_MIN_FUNCTIONALITY, {"AT+CFUN=0", default_long_timeout}},
        {AT::CFUN_FULL_FUNCTIONALITY, {"AT+CFUN=1", default_long_timeout}},
        {AT::CFUN_DISABLE_TRANSMITTING, {"AT+CFUN=4", default_long_timeout}},
        {AT::CMGS, {"AT+CMGS=\"", 180s}}, //
        {AT::QCMGS, {"AT+QCMGS=\"", 180s}},
        {AT::CREG, {"AT+CREG?"}},
        {AT::QNWINFO, {"AT+QNWINFO"}},
        {AT::COPS, {"AT+COPS", 200s}},
        {AT::QSIMSTAT, {"AT+QSIMSTAT?"}},
        {AT::SIM_DET, {"AT+QSIMDET?"}},
        {AT::SIM_DET_ON, {"AT+QSIMDET=1,0"}},
        {AT::SIMSTAT_ON, {"AT+QSIMSTAT=1"}},
        {AT::SET_SCANMODE, {"AT+QCFG=\"nwscanmode\","}},
        {AT::GET_SCANMODE, {"AT+QCFG=\"nwscanmode\""}},
        {AT::QGMR, {"AT+QGMR"}},
        {AT::STORE_SETTINGS_ATW, {"AT&W"}},
        {AT::CEER, {"AT+CEER"}},
        {AT::QIGETERROR, {"AT+QIGETERROR"}},
        {AT::VTS, {"AT+VTS=", default_long_timeout}},
        {AT::QLDTMF, {"AT+QLDTMF=1,"}},
        {AT::CUSD_OPEN_SESSION, {"AT+CUSD=1", 150s}},
        {AT::CUSD_CLOSE_SESSION, {"AT+CUSD=2", 150s}},
        {AT::CUSD_SEND, {"AT+CUSD=1,", 150s}},
        {AT::SET_SMS_STORAGE, {"AT+CPMS=\"SM\",\"SM\",\"SM\""}},
        {AT::CPIN, {"AT+CPIN=", default_long_timeout}},
        {AT::GET_CPIN, {"AT+CPIN?", default_long_timeout}},
        {AT::QPINC, {"AT+QPINC=", default_long_timeout}},
        {AT::CLCK, {"AT+CLCK=", default_long_timeout}},
        {AT::CPWD, {"AT+CPWD=", default_long_timeout}},
        {AT::ENABLE_TIME_ZONE_UPDATE, {"AT+CTZU=3"}},
        {AT::SET_TIME_ZONE_REPORTING, {"AT+CTZR=2"}},
        {AT::DISABLE_TIME_ZONE_UPDATE, {"AT+CTZU=0"}},
        {AT::DISABLE_TIME_ZONE_REPORTING, {"AT+CTZR=0"}},
        {AT::ENABLE_NETWORK_REGISTRATION_URC, {"AT+CREG=2"}},
        {AT::SET_SMS_TEXT_MODE_UCS2, {"AT+CSMP=17,167,0,8"}},
        {AT::LIST_MESSAGES, {"AT+CMGL=\"ALL\""}},
        {AT::GET_IMEI, {"AT+GSN"}},
        {AT::CCFC, {"AT+CCFC="}},
        {AT::CCWA, {"AT+CCWA="}},
        {AT::CCWA_GET, {"AT+CCWA?"}},
        {AT::CHLD, {"AT+CHLD=\""}},
        {AT::CLIP, {"AT+CLIP=", default_long_timeout}},
        {AT::CLIP_GET, {"AT+CLIP?", default_long_timeout}},
        {AT::CLIR, {"AT+CLIR=", default_long_timeout}},
        {AT::CLIR_GET, {"AT+CLIR?", default_long_timeout}},
        {AT::CLIR_RESET, {"AT+CLIR=0", default_long_timeout}},
        {AT::CLIR_ENABLE, {"AT+CLIR=1", default_long_timeout}},
        {AT::CLIR_DISABLE, {"AT+CLIR=2", default_long_timeout}},
        {AT::COLP, {"AT+COLP", default_long_timeout}},
        {AT::COLP_GET, {"AT+COLP?", default_long_timeout}},
        {AT::COLP_ENABLE, {"AT+COLP=1", default_long_timeout}},
        {AT::COLP_DISABLE, {"AT+COLP=0", default_long_timeout}},
        {AT::CSSN, {"AT+CSSN=\""}},
        {AT::QICSGP, {"AT+QICSGP"}},
        {AT::QIACT, {"AT+QIACT", 150s}},
        {AT::QIDEACT, {"AT+QIDEACT", 40s}},
        {AT::QRXGAIN, {"AT+QRXGAIN=40000"}},
        {AT::CLVL, {"AT+CLVL=3"}},
        {AT::QMIC, {"AT+QMIC=15000,15000"}},
        {AT::QEEC, {"AT+QEEC="}},
        {AT::QNVFR, {"AT+QNVFR=", default_long_timeout}},
        {AT::QNVFW, {"AT+QNVFW=", default_long_timeout}},
        {AT::QMBNCFG, {"AT+QMBNCFG=", default_long_timeout}},
        {AT::QCFG_IMS, {"AT+QCFG=\"ims\""}},
        {AT::RING_URC_ON, {"AT+QINDCFG=\"ring\",1"}},
        {AT::RING_URC_OFF, {"AT+QINDCFG=\"ring\",0"}},
        {AT::ACT_URC_OFF, {"AT+QINDCFG=\"act\",0"}},
        {AT::ACT_URC_ON, {"AT+QINDCFG=\"act\",1"}},
        {AT::SMS_URC_ON, {"AT+QINDCFG=\"smsincoming\",1"}},
        {AT::SMS_URC_OFF, {"AT+QINDCFG=\"smsincoming\",0"}},

    };

    auto factory(AT at) -> const Cmd &
    {
        static auto fact = std::map<AT, const Cmd>(initializer);
        if (fact.count(at) != 0u) {
            return fact.at(at);
        }
        LOG_ERROR("no such at command defined: %d", static_cast<int>(at));
        return fact.at(AT::AT);
    }
} // namespace at
