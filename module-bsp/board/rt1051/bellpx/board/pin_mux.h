// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/*
 * The Clear BSD License
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*! @brief Direction type  */
typedef enum _pin_mux_direction
{
    kPIN_MUX_DirectionInput         = 0U, /* Input direction */
    kPIN_MUX_DirectionOutput        = 1U, /* Output direction */
    kPIN_MUX_DirectionInputOrOutput = 2U  /* Input or output direction */
} pin_mux_direction_t;

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C"
{
#endif

    /*!
     * @brief Calls initialization functions.
     *
     */
    void PINMUX_InitBootPins(void);

    /**
     * DEBUG UART PINMUX DEFINITIONS
     */

#define PINMUX_DEBUGUART_TX IOMUXC_GPIO_AD_B1_06_LPUART3_TX
#define PINMUX_DEBUGUART_RX IOMUXC_GPIO_AD_B1_07_LPUART3_RX

    void PINMUX_InitDEBUG_UART(void);

    void PINMUX_InitSDRAM(void);

#define PINMUX_EMMC_DATA0 IOMUXC_GPIO_SD_B1_03_USDHC2_DATA0
#define PINMUX_EMMC_DATA1 IOMUXC_GPIO_SD_B1_02_USDHC2_DATA1
#define PINMUX_EMMC_DATA2 IOMUXC_GPIO_SD_B1_01_USDHC2_DATA2
#define PINMUX_EMMC_DATA3 IOMUXC_GPIO_SD_B1_00_USDHC2_DATA3
#define PINMUX_EMMC_DATA4 IOMUXC_GPIO_SD_B1_08_USDHC2_DATA4
#define PINMUX_EMMC_DATA5 IOMUXC_GPIO_SD_B1_09_USDHC2_DATA5
#define PINMUX_EMMC_DATA6 IOMUXC_GPIO_SD_B1_10_USDHC2_DATA6
#define PINMUX_EMMC_DATA7 IOMUXC_GPIO_SD_B1_11_USDHC2_DATA7
#define PINMUX_EMMC_CLK   IOMUXC_GPIO_SD_B1_04_USDHC2_CLK
#define PINMUX_EMMC_CMD   IOMUXC_GPIO_SD_B1_05_USDHC2_CMD
#define PINMUX_EMMC_RESET IOMUXC_GPIO_SD_B1_06_USDHC2_RESET_B

    void PINMUX_InitEMMC(void);

//#define PINMUX_AUDIOCODEC_SAIx_MCLK      IOMUXC_GPIO_AD_B0_10_SAI2_MCLK
#define PINMUX_AUDIOCODEC_SAIx_TX_DATA00 IOMUXC_GPIO_AD_B1_13_SAI1_TX_DATA00
#define PINMUX_AUDIOCODEC_SAIx_RX_DATA00 IOMUXC_GPIO_AD_B1_12_SAI1_RX_DATA00
#define PINMUX_AUDIOCODEC_SAIx_BCLK      IOMUXC_GPIO_AD_B1_14_SAI1_TX_BCLK
#define PINMUX_AUDIOCODEC_SAIx_TX_SYNC   IOMUXC_GPIO_AD_B1_15_SAI1_TX_SYNC

#define PINMUX_AUDIOCODEC_SCL IOMUXC_GPIO_B0_04_LPI2C2_SCL
#define PINMUX_AUDIOCODEC_SDA IOMUXC_GPIO_B0_05_LPI2C2_SDA

    void PINMUX_InitAudioCodec(void);
/**
 * EINK PINMUX DEFINITIONS
 */
#define PINMUX_EINK_SCK   IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK
#define PINMUX_EINK_SDO   IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO
#define PINMUX_EINK_SDI   IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI
#define PINMUX_EINK_CS    IOMUXC_GPIO_SD_B0_01_GPIO3_IO13
#define PINMUX_EINK_BUSY  IOMUXC_GPIO_SD_B0_05_GPIO3_IO17
#define PINMUX_EINK_RESET IOMUXC_GPIO_SD_B0_04_GPIO3_IO16
#define PINMUX_EINK_PWREN IOMUXC_GPIO_AD_B1_03_GPIO1_IO19
    void PINMUX_InitEINK(void);

/**
 * BATTERY CHARGER PINMUX DEFINITIONS
 */
#define PINUMX_BATTERY_CHARGER_CHGOK IOMUXC_GPIO_AD_B0_00_GPIO1_IO00
#define PINUMX_BATTERY_CHARGER_ACOK  IOMUXC_GPIO_B1_06_GPIO2_IO22
#define PINUMX_BATTERY_CHARGER_CHGEN IOMUXC_GPIO_B1_07_GPIO2_IO23

    void PINMUX_InitBatteryCharger(void);

    /**
     * BELL EINK FRONTLIGHT PWM
     */

#define PINMUX_EINK_FORNTLIGHT_PWM IOMUXC_GPIO_B1_15_FLEXPWM4_PWMA03
    void PINMUX_InitEinkFrontlight(void);

/**
 * BELL BUTTONS
 */
#define PINMUX_BUTTON_SW1     IOMUXC_GPIO_B1_08_GPIO2_IO24
#define PINMUX_BUTTON_SW2     IOMUXC_GPIO_B1_09_GPIO2_IO25
#define PINMUX_BUTTON_SW_ENC  IOMUXC_GPIO_B1_00_GPIO2_IO16
#define PINMUX_BUTTON_SW_PUSH IOMUXC_GPIO_B1_10_GPIO2_IO26

    void PINMUX_InitButtons(void);

/**
 * BELL ROTARY ENCODER
 */
#define PINMUX_ENCODER_2 IOMUXC_GPIO_B0_06_QTIMER3_TIMER0
#define PINMUX_ENCODER_1 IOMUXC_GPIO_B0_07_QTIMER3_TIMER1

    void PINMUX_InitRotaryEncoder(void);

/**
 * BELL DOME SWITCH
 */
#define PINMUX_DOME_SWITCH IOMUXC_GPIO_B1_11_GPIO2_IO27
    void PINMUX_DomeSwitch(void);

/**
 * BELL WAKEUP
 */
#define PINMUX_WAKEUP IOMUXC_SNVS_WAKEUP_GPIO5_IO00
    void PINMUX_Wakeup(void);

/**
 * BELL WDOG_B
 */
#define PINMUX_WDOG_B IOMUXC_GPIO_B1_13_GPIO2_IO29
    void PINMUX_WDOG_B_Init(void);

/**
 * I2C4 pins init
 */
#define PINMUX_I2C4_SCL IOMUXC_GPIO_AD_B0_12_LPI2C4_SCL
#define PINMUX_I2C4_SDA IOMUXC_GPIO_AD_B0_13_LPI2C4_SDA

    void PINMUX_InitI2C4(void);

/**
 * Fuel gauge GPIO
 */
#define PINUMX_FUEL_GAUGE_FG_INT IOMUXC_GPIO_AD_B0_03_GPIO1_IO03

    void PINMUX_InitFuelGauge(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
