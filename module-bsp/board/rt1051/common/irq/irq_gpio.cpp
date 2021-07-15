// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "irq_gpio.hpp"

#include "board.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "fsl_common.h"

#include "board/rt1051/bsp/eink/bsp_eink.h"
#include "bsp/battery-charger/battery_charger.hpp"
#include "bsp/cellular/bsp_cellular.hpp"
#include "bsp/keyboard/keyboard.hpp"
#include "bsp/headset/headset.hpp"
#include "bsp/BoardDefinitions.hpp"
#include "bsp/magnetometer/magnetometer.hpp"
#include "bsp/light_sensor/light_sensor.hpp"
#include "bsp/bell_switches/bell_switches.hpp"

#if 0 // TODO:M.P implement the rest of BSP drivers

#include "bsp_cellular.h"

#include "bsp_usbc.h"
#include "log.h"

#endif

namespace bsp
{

    void irq_gpio_Init(void)
    {
        DisableIRQ(GPIO1_Combined_0_15_IRQn);
        DisableIRQ(GPIO1_Combined_16_31_IRQn);
        DisableIRQ(GPIO2_Combined_0_15_IRQn);
        DisableIRQ(GPIO2_Combined_16_31_IRQn);
        DisableIRQ(GPIO3_Combined_16_31_IRQn);

        GPIO_PortDisableInterrupts(GPIO1, UINT32_MAX);
        GPIO_PortDisableInterrupts(GPIO2, UINT32_MAX);
        GPIO_PortDisableInterrupts(GPIO3, UINT32_MAX);

        // Clear all IRQs
        GPIO_PortClearInterruptFlags(GPIO1, UINT32_MAX);
        GPIO_PortClearInterruptFlags(GPIO2, UINT32_MAX);
        GPIO_PortClearInterruptFlags(GPIO3, UINT32_MAX);

        EnableIRQ(GPIO1_Combined_0_15_IRQn);
        NVIC_SetPriority(GPIO1_Combined_0_15_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

        EnableIRQ(GPIO1_Combined_16_31_IRQn);
        NVIC_SetPriority(GPIO1_Combined_16_31_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

        EnableIRQ(GPIO2_Combined_0_15_IRQn);
        NVIC_SetPriority(GPIO2_Combined_0_15_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

        EnableIRQ(GPIO2_Combined_16_31_IRQn);
        NVIC_SetPriority(GPIO2_Combined_16_31_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

        EnableIRQ(GPIO3_Combined_16_31_IRQn);
        NVIC_SetPriority(GPIO3_Combined_16_31_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    }

    extern "C"
    {

        void GPIO1_Combined_0_15_IRQHandler(void)
        {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask                   = GPIO_GetPinsInterruptFlags(GPIO1);

            if (irq_mask & (1 << BSP_CELLULAR_STATUS_PIN)) {
                xHigherPriorityTaskWoken |= cellular::status::statusIRQhandler();
            }

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO1, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO1_Combined_16_31_IRQHandler(void)
        {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask                   = GPIO_GetPinsInterruptFlags(GPIO1);

            if (irq_mask & (1 << static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ))) {
                xHigherPriorityTaskWoken |= bsp::magnetometer::IRQHandler();
                LOG_DEBUG("magneto IRQ! >%s<",
                          GPIO_PinRead(GPIO1, static_cast<uint32_t>(BoardDefinitions::MAGNETOMETER_IRQ)) ? "high"
                                                                                                         : "low");
            }

            if (irq_mask & (1 << BSP_BLUETOOTH_UART_CTS_PIN)) {
                LOG_DEBUG("CTS IRQ!");
            }

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO1, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO2_Combined_0_15_IRQHandler(void)
        {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask                   = GPIO_GetPinsInterruptFlags(GPIO2);

            if (irq_mask & (1 << BOARD_KEYBOARD_RF_BUTTON_PIN)) {
                xHigherPriorityTaskWoken |= keyboard_right_functional_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_BATTERY_CHARGER_INOKB_PIN)) {}

            if (irq_mask & (1 << BOARD_BATTERY_CHARGER_WCINOKB_PIN)) {}

            if (irq_mask & (1 << BOARD_BATTERY_CHARGER_INTB_PIN)) {
                xHigherPriorityTaskWoken |= bsp::battery_charger::INTB_IRQHandler();
            }

            if (irq_mask & (1 << BSP_CELLULAR_SIM_TRAY_INSERTED_PIN)) {
                xHigherPriorityTaskWoken |= bsp::cellular::sim::trayIRQHandler();
            }

            if (irq_mask & (1 << static_cast<uint32_t>(BoardDefinitions::LIGHT_SENSOR_IRQ))) {
                xHigherPriorityTaskWoken |= bsp::light_sensor::IRQHandler();
            }

            if (irq_mask & ((1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_CENTER))
                             | (1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_RIGHT)) 
                             | (1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LATCH)))) {
                xHigherPriorityTaskWoken |= bsp::bell_switches::bell_switches_Cent_Right_Latch_IRQHandler(irq_mask);
            }

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO2, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO2_Combined_16_31_IRQHandler(void)
        {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask                   = GPIO_GetPinsInterruptFlags(GPIO2);

            if (irq_mask & (1 << BOARD_KEYBOARD_IRQ_GPIO_PIN)) {
                xHigherPriorityTaskWoken |= keyboard_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_USBC_NINT_PIN)) {
                // TODO:M.P xHigherPriorityTaskWoken |= bsp_usbc_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_JACKDET_IRQ_GPIO_PIN)) {
                xHigherPriorityTaskWoken |= bsp::headset::headset_IRQHandler();
            }

            if (irq_mask & (1 << BSP_CELLULAR_RI_PIN)) {
                bsp::cellular::ringIndicator::riIRQHandler();
            }

            if (irq_mask & (1 << static_cast<uint32_t>(BoardDefinitions::BELL_SWITCHES_LEFT))) {
                xHigherPriorityTaskWoken |= bsp::bell_switches::bell_switches_Left_IRQHandler();
            }

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO2, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO3_Combined_16_31_IRQHandler(void)
        {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask                   = GPIO_GetPinsInterruptFlags(GPIO3);

            if (irq_mask & (1 << BOARD_EINK_BUSY_GPIO_PIN)) {

                xHigherPriorityTaskWoken |= BSP_EinkBusyPinStateChangeHandler();
            }

            // Clear all IRQs on the GPIO3 port
            GPIO_PortClearInterruptFlags(BOARD_EINK_BUSY_GPIO, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO5_Combined_0_15_IRQHandler(void)
        {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask                   = GPIO_GetPinsInterruptFlags(GPIO5);

            if (irq_mask & (1 << static_cast<uint32_t>(BoardDefinitions::BELL_WAKEUP))) {
                xHigherPriorityTaskWoken |= bsp::bell_switches::bell_wakeup_IRQHandler();
            }

            // Clear all IRQs on the GPIO3 port
            GPIO_PortClearInterruptFlags(GPIO5, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }
    }
} // namespace bsp
