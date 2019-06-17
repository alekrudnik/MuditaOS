/*
 *  @file irq_gpio.c
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 4 oct 2018
 *  @brief
 *  @copyright Copyright (C) 2018 mudita.com
 *  @details
 */

#include "irq_gpio.hpp"

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_common.h"


#include "keyboard/keyboard.hpp"


#include "bsp_eink.h"

#if 0 //TODO:M.P implement the rest of BSP drivers


#include "bsp_jackdet.h"
#include "bsp_cellular.h"
#include "bsp_battery_charger.h"

#include "bsp_usbc.h"
#include "log.h"

#endif

namespace bsp {


        void irq_gpio_Init(void) {
            DisableIRQ(GPIO2_Combined_0_15_IRQn);
            DisableIRQ(GPIO2_Combined_16_31_IRQn);
            DisableIRQ(GPIO3_Combined_16_31_IRQn);

            GPIO_PortEnableInterrupts(GPIO2, 0xFFFFFFFF);
            GPIO_PortEnableInterrupts(GPIO3, 0xFFFFFFFF);

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO2, 0xFFFFFFFF);
            GPIO_PortClearInterruptFlags(GPIO3, 0xFFFFFFFF);

            GPIO_PortDisableInterrupts(GPIO2, 0xFFFFFFFF);
            GPIO_PortDisableInterrupts(GPIO3, 0xFFFFFFFF);

            EnableIRQ(GPIO2_Combined_0_15_IRQn);
            NVIC_SetPriority(GPIO2_Combined_0_15_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

            EnableIRQ(GPIO2_Combined_16_31_IRQn);
            NVIC_SetPriority(GPIO2_Combined_16_31_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

            EnableIRQ(GPIO3_Combined_16_31_IRQn);
            NVIC_SetPriority(GPIO3_Combined_16_31_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
        }


        extern "C"
        {

        void GPIO2_Combined_0_15_IRQHandler(void) {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask = GPIO_GetPinsInterruptFlags(GPIO2);

            if (irq_mask & (1 << BOARD_KEYBOARD_RF_BUTTON_PIN)) {
                xHigherPriorityTaskWoken |= keyboard_right_functional_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_BATTERY_CHARGER_INOKB_PIN)) {
                //TODO:M.P xHigherPriorityTaskWoken |= BSP_BatteryChargerINOKB_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_BATTERY_CHARGER_WCINOKB_PIN)) {
                //TODO:M.P xHigherPriorityTaskWoken |= BSP_BatteryChargerWCINOKB_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_BATTERY_CHARGER_INTB_PIN)) {
                //TODO:M.P xHigherPriorityTaskWoken |= BSP_BatteryChargerINTB_IRQHandler();
            }

            if (irq_mask & (1 << BSP_CELLULAR_SIM_CARD_1_INSERTED_PIN)) {
                //TODO:M.P BSP_CellularSimCard1InsertionEjectionIrqHandler(&xHigherPriorityTaskWoken);
            }

            if (irq_mask & (1 << BSP_CELLULAR_SIM_CARD_2_INSERTED_PIN)) {
                //TODO:M.P BSP_CellularSimCard2InsertionEjectionIrqHandler(&xHigherPriorityTaskWoken);
            }

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO2, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO2_Combined_16_31_IRQHandler(void) {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask = GPIO_GetPinsInterruptFlags(GPIO2);


            if (irq_mask & (1 << BOARD_KEYBOARD_IRQ_GPIO_PIN)) {
                xHigherPriorityTaskWoken |= keyboard_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_USBC_NINT_PIN)) {
                //TODO:M.P xHigherPriorityTaskWoken |= bsp_usbc_IRQHandler();
            }

            if (irq_mask & (1 << BOARD_JACKDET_IRQ_GPIO_PIN)) {
                //TODO:M.P xHigherPriorityTaskWoken |= bsp_jackdet_IRQHandler();
            }

            if (irq_mask & (1 << BSP_CELLULAR_RI_PIN)) {
                //TODO:M.P BSP_CellularUartRingIndicatorIrqHandler();
            }

            // Clear all IRQs
            GPIO_PortClearInterruptFlags(GPIO2, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        void GPIO3_Combined_16_31_IRQHandler(void) {
            BaseType_t xHigherPriorityTaskWoken = 0;
            uint32_t irq_mask = GPIO_GetPinsInterruptFlags(GPIO3);

            if (irq_mask & (1 << BOARD_EINK_BUSY_GPIO_PIN)) {

                xHigherPriorityTaskWoken |= BSP_EinkBusyPinStateChangeHandler();

            }

            // Clear all IRQs on the GPIO3 port
            GPIO_PortClearInterruptFlags(BOARD_EINK_BUSY_GPIO, irq_mask);

            // Switch context if necessary
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
        }

        }
}

