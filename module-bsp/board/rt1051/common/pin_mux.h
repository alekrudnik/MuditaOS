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

#define PINMUX_AUDIOCODEC_SAIx_MCLK      IOMUXC_GPIO_AD_B0_10_SAI2_MCLK
#define PINMUX_AUDIOCODEC_SAIx_TX_DATA00 IOMUXC_GPIO_AD_B0_09_SAI2_TX_DATA
#define PINMUX_AUDIOCODEC_SAIx_RX_DATA00 IOMUXC_GPIO_AD_B0_08_SAI2_RX_DATA
#define PINMUX_AUDIOCODEC_SAIx_BCLK      IOMUXC_GPIO_AD_B0_05_SAI2_TX_BCLK
#define PINMUX_AUDIOCODEC_SAIx_TX_SYNC   IOMUXC_GPIO_AD_B0_04_SAI2_TX_SYNC

#define PINMUX_AUDIOCODEC_SCL IOMUXC_GPIO_B0_04_LPI2C2_SCL
#define PINMUX_AUDIOCODEC_SDA IOMUXC_GPIO_B0_05_LPI2C2_SDA

    void PINMUX_InitAudioCodec(void);

#define PINMUX_KEYBOARD_SCL   IOMUXC_GPIO_B0_04_LPI2C2_SCL
#define PINMUX_KEYBOARD_SDA   IOMUXC_GPIO_B0_05_LPI2C2_SDA
#define PINMUX_KEYBOARD_RESET IOMUXC_GPIO_B1_13_GPIO2_IO29
#define PINMUX_KEYBOARD_IRQ   IOMUXC_GPIO_B1_12_GPIO2_IO28

    void PINMUX_InitKeyboard(void);

/**
 * EINK PINMUX DEFINITIONS
 */
#define PINMUX_EINK_SCK   IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK
#define PINMUX_EINK_SDO   IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO
#define PINMUX_EINK_SDI   IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI
#define PINMUX_EINK_CS    IOMUXC_GPIO_SD_B0_01_GPIO3_IO13
#define PINMUX_EINK_BUSY  IOMUXC_GPIO_SD_B0_05_GPIO3_IO17
#define PINMUX_EINK_RESET IOMUXC_GPIO_SD_B0_04_GPIO3_IO16
    void PINMUX_InitEINK(void);

/**
 * CELLULAR PINMUX DEFINITIONS
 */
#define PINMUX_CELLULAR_UART_TX           IOMUXC_GPIO_AD_B0_12_LPUART1_TX
#define PINMUX_CELLULAR_UART_RX           IOMUXC_GPIO_AD_B0_13_LPUART1_RX
#define PINMUX_CELLULAR_UART_CTS          IOMUXC_GPIO_AD_B0_14_LPUART1_CTS_B
#define PINMUX_CELLULAR_UART_RTS          IOMUXC_GPIO_AD_B0_15_LPUART1_RTS_B
#define PINMUX_CELLULAR_AP_READY          IOMUXC_GPIO_B1_00_GPIO2_IO16
#define PINMUX_CELLULAR_RESET             IOMUXC_GPIO_B1_01_GPIO2_IO17
#define PINMUX_CELLULAR_PWRKEY            IOMUXC_GPIO_B1_02_GPIO2_IO18
#define PINMUX_CELLULAR_WAKEUP_IN         IOMUXC_GPIO_B1_06_GPIO2_IO22
#define PINMUX_CELLULAR_SIM_SEL           IOMUXC_GPIO_B1_04_GPIO2_IO20
#define PINMUX_CELLULAR_RI                IOMUXC_GPIO_B1_05_GPIO2_IO21
#define PINMUX_CELLULAR_DTR               IOMUXC_GPIO_EMC_41_GPIO3_IO27
#define PINMUX_CELLULAR_SIM_TRAY_INSERTED IOMUXC_GPIO_B0_11_GPIO2_IO11
#define PINMUX_CELLULAR_NC                IOMUXC_GPIO_B0_10_GPIO2_IO10
#define PINMUX_CELLULAR_SIM_PRESENCE      IOMUXC_GPIO_B0_09_GPIO2_IO09
#define PINMUX_CELLULAR_ANTENNA_SELECT    IOMUXC_GPIO_B0_02_GPIO2_IO02
#define PINMUX_CELLULAR_STATUS            IOMUXC_GPIO_AD_B0_02_GPIO1_IO02
#define PINMUX_CELLULAR_USB_BOOT_PIN      IOMUXC_GPIO_B1_08_GPIO2_IO24

#define PINMUX_CELLULAR_SAIx_TX_DATA00 IOMUXC_GPIO_AD_B1_13_SAI1_TX_DATA00
#define PINMUX_CELLULAR_SAIx_RX_DATA00 IOMUXC_GPIO_AD_B1_12_SAI1_RX_DATA00
#define PINMUX_CELLULAR_SAIx_BCLK      IOMUXC_GPIO_AD_B1_14_SAI1_TX_BCLK
#define PINMUX_CELLULAR_SAIx_TX_SYNC   IOMUXC_GPIO_AD_B1_15_SAI1_TX_SYNC

    void PINMUX_InitCellular(void);

/**
 * BLUETOOTH PINMUX DEFINITIONS
 */
#define PINMUX_BLUETOOTH_UART_TX   IOMUXC_GPIO_AD_B1_02_LPUART2_TX
#define PINMUX_BLUETOOTH_UART_RX   IOMUXC_GPIO_AD_B1_03_LPUART2_RX
#define PINMUX_BLUETOOTH_UART_CTS  IOMUXC_GPIO_AD_B1_00_LPUART2_CTS_B
#define PINMUX_BLUETOOTH_UART_RTS  IOMUXC_GPIO_AD_B1_01_LPUART2_RTS_B
#define PINMUX_BLUETOOTH_OSC_EN    IOMUXC_GPIO_B0_00_GPIO2_IO00
#define PINMUX_BLUETOOTH_NSHUTDOWN IOMUXC_GPIO_B0_01_GPIO2_IO01

    void PINMUX_InitBluetoothPins(void);

/**
 * USB TYPE-C CONTROLLER PINMUX DEFINITIONS
 */
#define PINMUX_USBC_CONTROLLER_NINT    IOMUXC_GPIO_B1_09_GPIO2_IO25
#define PINMUX_USB_FUNCTION_MUX_SELECT IOMUXC_GPIO_AD_B1_09_GPIO1_IO25
    void PINMUX_InitUSBC(void);

/**
 * LED DRIVER PINMUX DEFINITIONS
 */
#define PINMUX_LEDDRIVER_NRST IOMUXC_GPIO_AD_B0_03_GPIO1_IO03
    void PINMUX_InitLEDDRIVER(void);

/**
 * HEADSET (JACK DETECTION, MIC BIAS) PINMUX DEFINITIONS
 */
#define PINMUX_JACKDET_IRQ IOMUXC_GPIO_B1_14_GPIO2_IO30
#define PINMUX_MIC_LDO_EN  IOMUXC_GPIO_B1_03_GPIO2_IO19
    void PINMUX_InitHeadset(void);

/**
 * BATTERY CHARGER PINMUX DEFINITIONS
 */
#define PINUMX_BATTERY_CHARGER_INOKB_IRQ   IOMUXC_GPIO_B0_12_GPIO2_IO12
#define PINUMX_BATTERY_CHARGER_WCINOKB_IRQ IOMUXC_GPIO_B0_13_GPIO2_IO13
#define PINUMX_BATTERY_CHARGER_INTB_IRQ    IOMUXC_GPIO_B0_14_GPIO2_IO14

    void PINMUX_InitBatteryCharger(void);

#define PINMUX_KEYBOARD_POWER_ENABLE_PAD IOMUXC_GPIO_B0_07_GPIO2_IO07
#define PINMUX_KEYBOARD_RF_BUTTON_PAD    IOMUXC_GPIO_B0_06_GPIO2_IO06

    void PINMUX_InitPowerButton(void);

#define PINMUX_ALS_ADC IOMUXC_GPIO_AD_B1_08_GPIO1_IO24 // gpio_ad_b1_08 - gpio1.24 - alt5
#define PINMUX_ALS_GB1 IOMUXC_GPIO_AD_B1_10_GPIO1_IO26 // GPIO_AD_B1_10
#define PINMUX_ALS_GB2 IOMUXC_GPIO_AD_B1_11_GPIO1_IO27 // GPIO_AD_B1_11
    void PINMUX_InitALS(void);

#define PINMUX_POWER_SW   IOMUXC_GPIO_B0_06_GPIO2_IO06
#define PINMUX_POWER_HOLD IOMUXC_GPIO_B0_07_GPIO2_IO07
    void PINMUX_InitPowerSW(void);

#define PINMUX_VIBRATOR_EN_PIN IOMUXC_GPIO_AD_B0_00_GPIO1_IO00
    void PINMUX_InitVibrator(void);

#define PINMUX_TORCH_EN_PIN IOMUXC_GPIO_AD_B1_05_GPIO1_IO21
    void PINMUX_InitTorch(void);

#define PINMUX_MAGNETOMETER_IRQ_PIN IOMUXC_GPIO_AD_B1_04_GPIO1_IO20
    void PINMUX_InitMagnetometer(void);

#define PINMUX_EINK_FORNTLIGHT_PWM IOMUXC_GPIO_AD_B0_01_FLEXPWM2_PWMB03
    void PINMUX_InitEinkFrontlight(void);

#define PINMUX_LIGHT_SENSOR_IRQ_PIN IOMUXC_GPIO_B0_15_GPIO2_IO15
    void PINMUX_InitLightSensor(void);

/**
* DCDC INVERTER MODE PINMUX DEFINITIONS
*/
#define PINMUX_DCDC_MODE_PIN      IOMUXC_GPIO_B1_10_GPIO2_IO26
    void PINMUX_InitDcdcInverter(void);

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
