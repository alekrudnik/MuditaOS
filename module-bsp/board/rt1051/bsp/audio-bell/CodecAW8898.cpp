// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "CodecAW8898.hpp"
#include "AW8898_regs.hpp"
#include "bsp/BoardDefinitions.hpp"

using namespace drivers;

CodecAW8898::CodecAW8898() : i2cAddr{}
{

    i2cAddr.deviceAddress  = AW8898_I2C_ADDR;
    i2cAddr.subAddressSize = 1; // AW8898 uses 1byte addressing
    i2c                    = DriverI2C::Create(
        static_cast<I2CInstances>(BoardDefinitions::AUDIOCODEC_I2C),
        DriverI2CParams{.baudrate = static_cast<uint32_t>(BoardDefinitions::AUDIOCODEC_I2C_BAUDRATE)});
    Reset();
}

CodecAW8898::~CodecAW8898()
{
    Reset();
}

CodecRetCode CodecAW8898::Start(const CodecParams &param)
{

    const CodecParamsAW8898 &params = static_cast<const CodecParamsAW8898 &>(param);

    // Software reset - p.19
    i2cAddr.subAddress                   = AW8898_REG_ID;
    aw8898_reg_idcode_t dev_id = { .idcode = AW8898_SW_RESET_MAGIC };
    i2c->Write(i2cAddr, (uint8_t *)&dev_id, 2);

    // Power up sequence - table 2, p.18
    /*
        1   Wait for VDD, DVDD supply power up  ;   mode: Power-Down
        2   I2S + Data Path Configuration       ;   mode: Stand-By
        3.1 Enable system (SYSCTRL.PWDN=0)      ;   mode: Configuring
        3,2 Bias, OSC, PLL active
        3.3 Waiting for PLL locked
        4.1 Enable Boost and amplifier
            (SYSCTRL.AMPPD=0)
            Boost and Amplifier boot up         ;   mode: Operating
        4.2 Wait SYSST.SWS=1                    ;   mode: Operating
        5   Release HARD-Mute
            Data Path active                    ;   mode: Operating
    */

    /* 2. I2S:
        I2SCTRL.I2SMD -> mode
        I2SCTRL.I2SSR -> samplerate
        I2SCTRL.I2SBCK -> bit clock BCK frequency = SampleRate * SlotLength * SlotNumber 
            SampleRate: Sample rate for this digital audio interface; SlotLength: The length of one audio slot in unit of BCK clock; SlotNumber:  How  many  slots  supported  in  this  audio  interface.  For  example:  2-slot  supported  in  I2S  mode, 
            4-slot supported in TDM mode.
        
    */

    aw8898_reg_i2sctrl_t i2s_setup = 
    {
        .inplev = 1,    //attenuate input
        .chsel = 3,     //mono; (L+R)/2
        .i2smd = 0,     //standard I2S
        .i2sfs = 0,     //16 bit
        .i2sbck = 0    //21*fs(16*2)
    };

    switch (params.sampleRate) {

    case CodecParamsAW8898::SampleRate::Rate8KHz:
        i2s_setup.i2ssr = 0;     //8 kHz
        break;

    case CodecParamsAW8898::SampleRate::Rate16KHz:
        i2s_setup.i2ssr = 3;     //16 kHz
        break;

    case CodecParamsAW8898::SampleRate::Rate44K1Hz:
        i2s_setup.i2ssr = 7;     //44.1 kHz
        break;

    case CodecParamsAW8898::SampleRate::Rate48KHz:
        i2s_setup.i2ssr = 8;     //48 kHz
        break;

    case CodecParamsAW8898::SampleRate::Rate32KHz:
        i2s_setup.i2ssr = 6;     //32 kHz
        break;

    case CodecParamsAW8898::SampleRate::Rate96KHz:
        i2s_setup.i2ssr = 9;     //96 kHz
        break;

    default:
        return CodecRetCode::InvalidSampleRate;
    }
    i2cAddr.subAddress = AW8898_REG_I2SCTRL;
    i2c->Write(i2cAddr, (uint8_t *)&i2s_setup, 2);

    aw8898_reg_i2stxcfg_t i2s_tx_setup = 
    {
        .fsync_type = 0,    //one slot wide sync pulse
        .slot_num = 0,      // 2 slots
        .i2s_tx_slotvld = 0,   //send on slot 0
        .i2s_rx_slotvld = 3,   //slot 0,1
        .drvstren = 0,      //I2S_DATAO pad drive 2mA
        .dohz = 0          //unused channel data set to 0
    };
    i2cAddr.subAddress = AW8898_REG_I2STXCFG;
    i2c->Write(i2cAddr, (uint8_t *)&i2s_setup, 2);

    /* 3.1 Enable system (SYSCTRL.PWDN=0) */
    aw8898_reg_sysctrl_t sys_setup = 
    {
        // all values default to 0 except:
        .i2sen = 1,     //enable I2S
        .pwdn = 0      //power up
    };
    i2cAddr.subAddress = AW8898_REG_SYSCTRL;
    i2c->Write(i2cAddr, (uint8_t *)&sys_setup, 2);

    /* 3,2 Bias, OSC, PLL active */
    
    /* 3.3 Waiting for PLL locked */
    i2cAddr.subAddress = AW8898_REG_SYSST;
    uint16_t sys_status = 0;
    while ((sys_status & 0x0001) != 1) //wait for PLLS = 1 (locked)
    {
        i2c->Read(i2cAddr, (uint8_t *)&sys_status, 2);
    }

    /* 4.1 Enable Boost and amplifier
            (SYSCTRL.AMPPD=0)
    */
    i2cAddr.subAddress = AW8898_REG_SYSCTRL;
    i2c->Modify(i2cAddr, 0x0002, false, 2);   //(SYSCTRL.AMPPD=0)

    /* 4.2 Wait SYSST.SWS=1 */
    i2cAddr.subAddress = AW8898_REG_SYSST;
    while ((sys_status & (1 << 8)) == 0) //wait for SWS = 1 
    {
        i2c->Read(i2cAddr, (uint8_t *)&sys_status, 2);
    }

    // Store param configuration
    currentParams = params;

    auto currVol = currentParams.outVolume;

    SetOutputVolume(currVol);

    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Pause()
{
    // Turn off device
    i2cAddr.subAddress = AW8898_REG_SYSCTRL;
    i2c->Modify(i2cAddr, 0x0002, true, 2);   //(SYSCTRL.AMPPD=1)
    i2c->Modify(i2cAddr, 0x0001, true, 2);   //(SYSCTRL.PWDN=1)

    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Resume()
{
    // Turn on device
    i2cAddr.subAddress = AW8898_REG_SYSCTRL;
    i2c->Modify(i2cAddr, 0x0001, false, 2);   //(SYSCTRL.PWDN=0)
    /* 3.3 Waiting for PLL locked */
    i2cAddr.subAddress = AW8898_REG_SYSST;
    uint16_t sys_status = 0;
    while ((sys_status & 0x0001) != 1) //wait for PLLS = 1 (locked)
    {
        i2c->Read(i2cAddr, (uint8_t *)&sys_status, 2);
    }
    i2c->Modify(i2cAddr, 0x0002, false, 2);   //(SYSCTRL.AMPPD=0)
    
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Stop()
{
    Pause();
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Ioctrl(const CodecParams &param)
{

    const CodecParamsAW8898 &params = static_cast<const CodecParamsAW8898 &>(param);

    CodecRetCode ret = CodecRetCode::Success;

    switch (params.opCmd) {
    case CodecParamsAW8898::Cmd::SetOutVolume:
        ret = SetOutputVolume(params.outVolume);
        break;

    case CodecParamsAW8898::Cmd::SetInGain:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::SetInput:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::SetOutput:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::MicBiasCtrl:
        ret = CodecRetCode::Success;
        break;
    case CodecParamsAW8898::Cmd::Reset:
        ret = Reset();
        break;
    case CodecParamsAW8898::Cmd::SetMute:
        ret = SetMute(params.muteEnable);
        break;
    default:
        break;
    }

    return ret;
}

CodecRetCode CodecAW8898::SetOutputVolume(const float vol)
{
    uint8_t mute = 0;

    // If volume set to 0 then mute output
    i2cAddr.subAddress                   = AW8898_REG_PWMCTRL;
    if (vol == 0)
        i2c->Modify(i2cAddr, 0x0001, true, 2);   //(PWMCTRL.HMUTE=1) - enable mute
    else
        i2c->Modify(i2cAddr, 0x0001, false, 2);   //(PWMCTRL.HMUTE=0) - disable mute

    // volume is encoded with 8 bits. vol is in range 0-10
    i2cAddr.subAddress = AW8898_REG_HAGCCFG7;
    aw8898_reg_hagccfg7_t regval = {};
    i2c->Read(i2cAddr, (uint8_t *)&regval, 2);
    regval.volume = static_cast<uint8_t>(25.5 * vol);
    i2c->Write(i2cAddr, (uint8_t *)&regval, 2);

    currentParams.outVolume = vol;
    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::Reset()
{
    // Software reset - p.19
    i2cAddr.subAddress                   = AW8898_REG_ID;
    aw8898_reg_idcode_t dev_id = { AW8898_SW_RESET_MAGIC };
    i2c->Write(i2cAddr, (uint8_t *)&dev_id, 2);

    return CodecRetCode::Success;
}

CodecRetCode CodecAW8898::SetMute(const bool enable)
{
    i2cAddr.subAddress                   = AW8898_REG_PWMCTRL;
    i2c->Modify(i2cAddr, 0x0001, enable ? true : false, 2);   //(PWMCTRL.HMUTE)
    return CodecRetCode::Success;
}

std::optional<uint32_t> CodecAW8898::Probe()
{
    uint16_t id = 0;

    i2cAddr.subAddress = AW8898_REG_ID;
    i2c->Write(i2cAddr, (uint8_t *)&id, 2);
    return id;
}
