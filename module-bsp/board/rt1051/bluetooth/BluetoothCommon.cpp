// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <module-bsp/drivers/dmamux/DriverDMAMux.hpp>
#include <module-bsp/bsp/BoardDefinitions.hpp>
#include <sstream>
#include "BluetoothWorker.hpp"
#include "bsp/bluetooth/Bluetooth.hpp"
#include "log/log.hpp"
#include "FreeRTOS.h"
#include "fsl_lpuart.h"
#include "board.h"

using namespace bsp;

lpuart_edma_handle_t BluetoothCommon::uartDmaHandle = {};
uint8_t BluetoothCommon::dmaRXbuf[384]             = {0};
uint32_t BluetoothCommon::dmaRXreadCount            = 0;

// TODO it's plain copy same as in cellular - this is kind of wrong
uint32_t UartGetPeripheralClock();

void BTdev::_circ::sem_take()
{
    if (!(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)) {
        xSemaphoreTake(sem, 0);
    }
    else {
        BaseType_t px;
        xSemaphoreTakeFromISR(sem, &px);
    }
}

void BTdev::_circ::sem_give()
{
    if (!(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)) {
        xSemaphoreGive(sem);
    }
    else {
        BaseType_t px;
        xSemaphoreGiveFromISR(sem, &px);
    }
}

BTdev::_circ::_circ(unsigned int size, int threshold) : head(0), tail(0), threshold(threshold), size(size), len(0)
{
    buff = new char[size];
    sem  = xSemaphoreCreateBinary();
}

BTdev::_circ::~_circ()
{
    vSemaphoreDelete(sem);
    delete[] buff;
}

BluetoothCommon::BluetoothCommon(unsigned int in_size, unsigned int out_size, int threshold)
    : BTdev(in_size, out_size, threshold)
{
    configure_uart_io();
    configure_lpuart();
    configure_cts_irq();
    LOG_INFO("Bluetooth HW init done!");
}

BluetoothCommon::~BluetoothCommon()
{}

void BluetoothCommon::open()
{
    LOG_INFO("Bluetooth HW open!");
    set_reset(true);
    set_irq(true);
    is_open = true;
    set_rts(true);
}

void BluetoothCommon::close()
{
    LOG_INFO("close!");
    // TODO destroy semaphore
    set_rts(false);
    set_irq(false);
    is_open = false;
    set_reset(false);
}

void BluetoothCommon::sleep_ms(ssize_t ms)
{
    ulTaskNotifyTake(pdTRUE, ms);
}

/*
ssize_t BluetoothCommon::write(char *buf, size_t nbytes)
{
    LOG_INFO("write -> [%.*s]", nbytes, buf);
    ssize_t i = 0;
    //     if CTS set -> ignore return 0, can use threshold_guard here too
    for (i = 0; i < nbytes; ++i) {
        if (out.push(*(buf + i)) != 0) {
            LOG_ERROR("Cant push!");
            break;
        }
    }
    return i;
}
*/

ssize_t BluetoothCommon::write(char *buf, size_t nbytes)
{
    //    std::stringstream ss;
    //    for (auto i = 0; i < nbytes; ++i) {
    //        ss << " 0x" << std::hex << (int)buf[i];
    //    }
    //    LOG_DEBUG("write DMA -> [%d]>%s<", nbytes, ss.str().c_str());

    lpuart_transfer_t sendXfer = {0};

    sendXfer.data     = reinterpret_cast<uint8_t *>(buf);
    sendXfer.dataSize = nbytes;

    uartDmaHandle.userData = xTaskGetCurrentTaskHandle();

    // Sends out.
    LPUART_EnableTx(BSP_BLUETOOTH_UART_BASE, true);

    ssize_t ret = 0;

    SCB_CleanInvalidateDCache();
    auto sent = LPUART_SendEDMA(BSP_BLUETOOTH_UART_BASE, &uartDmaHandle, &sendXfer);
    switch (sent) {
    case kStatus_Success: {
        // orchestrate a DMA tx
        LOG_DEBUG("DMA Tx pending");
        ret = nbytes;
    } break;
    case kStatus_LPUART_TxBusy:
        LOG_ERROR("Previous DMA xfer is still pending");
        //        could've checked beforehand with uartDmaHandle.txState == kLPUART_TxIdle
        ret = -1;
        break;
    case kStatus_InvalidArgument:
        LPUART_EnableTx(BSP_BLUETOOTH_UART_BASE, false);
        ret = -1;
        break;
    }
    return ret;
}

ssize_t BluetoothCommon::write_blocking(char *buf, ssize_t len)
{
    ssize_t ret = -1;

    // flush RX circ buffer
    bsp::BlueKitchen *bt = bsp::BlueKitchen::getInstance();
    bt->in.flush();

    auto wrote = write(buf, len);

    if (wrote == len) { // success orchestrating a transfer
        constexpr auto write_blocking_timeout = pdMS_TO_TICKS(100);
        auto ulNotificationValue              = ulTaskNotifyTake(pdFALSE, write_blocking_timeout);
        if (ulNotificationValue != 0) { // success completing a transfer
            LOG_DEBUG("DMA Tx wrote");
            ret = len;
        } else {
            LOG_ERROR("DMA Tx timeout");
            ret = -1;
        }
    }
    else{
        LOG_WARN("DMA Tx not wrote (%d/%d)", wrote, len);
    }
    return ret;
}

BTdev::Error BluetoothCommon::set_baudrate(uint32_t bd)
{
    LOG_INFO("Set baudrate: %" PRIu32, bd);
    Error ret = Success;
    int err   = 0;
    if ((err = LPUART_SetBaudRate(BSP_BLUETOOTH_UART_BASE, bd, UartGetPeripheralClock())) != 0) {
        LOG_ERROR("BT error: baudrate [%lu] set err: %d", bd, err);
        ret = ErrorBSP;
    }
    return ret;
}

// set flow on -> true, set flow off -> false
BTdev::Error BluetoothCommon::set_rts(bool on)
{
    LOG_DEBUG("set RTS: %c", on ? 'O' : 'X');
    GPIO_PinWrite(BSP_BLUETOOTH_UART_RTS_PORT, BSP_BLUETOOTH_UART_RTS_PIN, on ? 0U : 1U);
    return Success;
}

BTdev::Error BluetoothCommon::set_reset(bool on)
{
    LOG_INFO("reset %s", on ? "on" : "off");
    GPIO_PinWrite(BSP_BLUETOOTH_SHUTDOWN_PORT, BSP_BLUETOOTH_SHUTDOWN_PIN, on ? 1U : 0U);
    return Success;
}

int BluetoothCommon::read_cts()
{
    return GPIO_PinRead(BSP_BLUETOOTH_UART_CTS_PORT, BSP_BLUETOOTH_UART_CTS_PIN);
}

uint32_t UartGetPeripheralClock()
{
    const int UART_PERIPHERAL_PLL_DIVIDER = 6;
    uint32_t freq                         = 0;
    /* To make it simple, we assume default PLL and divider settings, and the only variable
       from application is use PLL3 source or OSC source */
    if (CLOCK_GetMux(kCLOCK_UartMux) == 0) /* PLL3 div6 80M */
    {
        freq = (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / UART_PERIPHERAL_PLL_DIVIDER) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }
    else {
        freq = CLOCK_GetOscFreq() / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }

    return freq;
}

void BluetoothCommon::configure_uart_io()
{
    gpio_pin_config_t gpio_init_structure;
    gpio_init_structure.direction     = kGPIO_DigitalOutput;
    gpio_init_structure.interruptMode = kGPIO_IntRisingOrFallingEdge;
    gpio_init_structure.outputLogic   = 1;
    GPIO_PinInit(BSP_BLUETOOTH_UART_RTS_PORT, BSP_BLUETOOTH_UART_RTS_PIN, &gpio_init_structure);
    gpio_init_structure.direction     = kGPIO_DigitalInput;
    gpio_init_structure.interruptMode = kGPIO_IntRisingOrFallingEdge;
    gpio_init_structure.outputLogic   = 0;
    GPIO_PinInit(BSP_BLUETOOTH_UART_CTS_PORT, BSP_BLUETOOTH_UART_CTS_PIN, &gpio_init_structure);
    gpio_init_structure.direction     = kGPIO_DigitalOutput;
    gpio_init_structure.interruptMode = kGPIO_NoIntmode;
    GPIO_PinInit(BSP_BLUETOOTH_OSC_EN_PORT, BSP_BLUETOOTH_OSC_EN_PIN, &gpio_init_structure);
    GPIO_PinWrite(BSP_BLUETOOTH_OSC_EN_PORT, BSP_BLUETOOTH_OSC_EN_PIN, 1U);
    gpio_init_structure.direction = kGPIO_DigitalOutput;
    GPIO_PinInit(BSP_BLUETOOTH_SHUTDOWN_PORT, BSP_BLUETOOTH_SHUTDOWN_PIN, &gpio_init_structure);
}

void BluetoothCommon::configure_lpuart()
{
    lpuart_config_t bt_c;
    LPUART_GetDefaultConfig(&bt_c);
    bt_c.baudRate_Bps  = baudrate;
    bt_c.dataBitsCount = kLPUART_EightDataBits;
    bt_c.parityMode    = kLPUART_ParityDisabled;
    bt_c.isMsb         = false;
    bt_c.rxIdleType    = kLPUART_IdleTypeStartBit;
    bt_c.rxIdleConfig  = kLPUART_IdleCharacter1;
    bt_c.enableTx      = false;
    bt_c.enableRx      = false;

    if (LPUART_Init(BSP_BLUETOOTH_UART_BASE, &bt_c, UartGetPeripheralClock()) != kStatus_Success) {
        LOG_ERROR("BT: UART config error Could not initialize the uart!");
        return;
    }

    LPUART_ClearStatusFlags(BSP_BLUETOOTH_UART_BASE, 0xFFFFFFFF);
    NVIC_ClearPendingIRQ(LPUART2_IRQn);
    NVIC_SetPriority(LPUART2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(LPUART2_IRQn);

    //    set_reset(true);
    dmamux = drivers::DriverDMAMux::Create(static_cast<drivers::DMAMuxInstances>(BoardDefinitions ::BLUETOOTH_DMAMUX),
                                           drivers::DriverDMAMuxParams{});
    dma    = drivers::DriverDMA::Create(static_cast<drivers::DMAInstances>(BoardDefinitions ::BLUETOOTH_DMA),
                                     drivers::DriverDMAParams{});

    uartTxDmaHandle = dma->CreateHandle(static_cast<uint32_t>(BoardDefinitions::BLUETOOTH_TX_DMA_CHANNEL));
    uartRxDmaHandle = dma->CreateHandle(static_cast<uint32_t>(BoardDefinitions::BLUETOOTH_RX_DMA_CHANNEL));

    dmamux->Enable(static_cast<uint32_t>(BoardDefinitions::BLUETOOTH_TX_DMA_CHANNEL), kDmaRequestMuxLPUART2Tx);
    dmamux->Enable(static_cast<uint32_t>(BoardDefinitions::BLUETOOTH_RX_DMA_CHANNEL), kDmaRequestMuxLPUART2Rx);

    LPUART_TransferCreateHandleEDMA(BSP_BLUETOOTH_UART_BASE,
                                    &uartDmaHandle,
                                    uartDmaCallback,
                                    nullptr,
                                    reinterpret_cast<edma_handle_t *>(uartTxDmaHandle->GetHandle()),
                                    reinterpret_cast<edma_handle_t *>(uartRxDmaHandle->GetHandle()));
}

void BluetoothCommon::uartDmaCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    BaseType_t higherPriorTaskWoken = 0;

    switch (status) {
    case kStatus_LPUART_TxIdle: {
        BaseType_t taskwoken = 0;
        uint8_t val          = Bt::Message::EvtSent;
        bsp::BlueKitchen *bt = bsp::BlueKitchen::getInstance();

        LOG_DEBUG("DMA irq: TX done");
        LPUART_EnableTx(BSP_BLUETOOTH_UART_BASE, false);

        //        vTaskNotifyGiveFromISR((TaskHandle_t)userData, &higherPriorTaskWoken);
        //        portEND_SWITCHING_ISR(higherPriorTaskWoken);

        xQueueSendFromISR(bt->qHandle, &val, &taskwoken);
        break;
    }
    case kStatus_LPUART_RxIdle:
        LOG_DEBUG("DMA irq: RX done");
        break;
    default:
        LOG_DEBUG("DMA irq: something else! (%ld). Quite impossible", status);
        break;
    }
}

void BluetoothCommon::configure_cts_irq()
{
    DisableIRQ(GPIO1_Combined_16_31_IRQn);
    GPIO_PortClearInterruptFlags(GPIO1, 0xFFFFFFFF);
    GPIO_PortEnableInterrupts(GPIO1, (1 << BSP_BLUETOOTH_UART_CTS_PIN));
    EnableIRQ(GPIO1_Combined_16_31_IRQn);
    NVIC_SetPriority(GPIO1_Combined_16_31_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
}
void BluetoothCommon::set_irq(bool enable)
{
    // printf("%s\n", __FUNCTION__);
    LPUART_EnableRx(BSP_BLUETOOTH_UART_BASE, false);
    LPUART_EnableTx(BSP_BLUETOOTH_UART_BASE, false);
    LPUART_ClearStatusFlags(BSP_BLUETOOTH_UART_BASE, 0xFFFFFFFF);
    auto n = LPUART_GetEnabledInterrupts(BSP_BLUETOOTH_UART_BASE);
    n += 0;
    if (enable) {
        LPUART_EnableInterrupts(BSP_BLUETOOTH_UART_BASE,
                                kLPUART_RxActiveEdgeInterruptEnable | kLPUART_IdleLineInterruptEnable |
                                    kLPUART_RxOverrunInterruptEnable);
    }
    else {
        LPUART_DisableInterrupts(BSP_BLUETOOTH_UART_BASE,
                                 kLPUART_RxDataRegFullInterruptEnable | kLPUART_IdleLineInterruptEnable |
                                     kLPUART_RxOverrunInterruptEnable);
    }
    //     LPUART_EnableInterrupts(BSP_BLUETOOTH_UART_BASE,
    //     kLPUART_RxDataRegFullInterruptEnable|kLPUART_TxDataRegEmptyInterruptEnable|kLPUART_TransmissionCompleteInterruptEnable|kLPUART_RxOverrunInterruptEnable
    //     );
    LPUART_EnableRx(BSP_BLUETOOTH_UART_BASE, true);
    LPUART_EnableTx(BSP_BLUETOOTH_UART_BASE, true);
}

extern "C"
{
    void LPUART2_IRQHandler(void)
    {
        uint32_t isrReg      = LPUART_GetStatusFlags(BSP_BLUETOOTH_UART_BASE);
        BaseType_t taskwoken = 0;
        uint8_t val          = Bt::Message::EvtReceived;
        bsp::BlueKitchen *bt = bsp::BlueKitchen::getInstance();

        auto statusRxDma = LPUART_TransferGetReceiveCountEDMA(
            BSP_BLUETOOTH_UART_BASE, &BluetoothCommon::uartDmaHandle, &BluetoothCommon::dmaRXreadCount);
        LOG_DEBUG("DMA irq: %ld RXed so far (status: %ld)", BluetoothCommon::dmaRXreadCount, statusRxDma);

        if (isrReg & kLPUART_RxActiveFlag) {
            LOG_DEBUG("LPUART IRQ new byte incoming RX");

            if (statusRxDma == kStatus_NoTransferInProgress) {
                // start RXfer if there is byte incoming and no pending RXfer
                lpuart_transfer_t receiveXfer;
                receiveXfer.data     = reinterpret_cast<uint8_t *>(BluetoothCommon::dmaRXbuf);
                receiveXfer.dataSize = sizeof(BluetoothCommon::dmaRXbuf) / sizeof(BluetoothCommon::dmaRXbuf[0]);

                BluetoothCommon::dmaRXreadCount = 0;
                LPUART_ReceiveEDMA(BSP_BLUETOOTH_UART_BASE, &BluetoothCommon::uartDmaHandle, &receiveXfer);
            }
            else {
                // transfer pending, no need to start again
                LOG_DEBUG("LPUART IRQ new byte incoming RX - RX pending");
            }
        }

        if (isrReg & kLPUART_IdleLineFlag) {
            LOG_DEBUG("LPUART IRQ line idle");

            // DMA transaction is over. copy DMA buffer over to bt.in circ buffer

            if (statusRxDma == kStatus_Success) {
                LPUART_TransferAbortReceiveEDMA(BSP_BLUETOOTH_UART_BASE, &BluetoothCommon::uartDmaHandle);
                LOG_DEBUG("LPUART IRQ aborted RX");
            }
            LOG_DEBUG("compare count/to_read_req : %ld/%ld", BluetoothCommon::dmaRXreadCount, bt->to_read_req);
            // copy DMA bytes to bt->in
            for (auto dma = 0; dma < BluetoothCommon::dmaRXreadCount; dma++) {
                if (bt->in.push(BluetoothCommon::dmaRXbuf[dma])) {
                    val = Bt::Message::EvtRecUnwanted;
                    xQueueSendFromISR(bt->qHandle, &val, &taskwoken);
                }
            }

            if (bt->to_read != 0 && (bt->in.len >= bt->to_read)) {
                bt->to_read = 0;
                assert(bt->qHandle);
                val = Bt::Message::EvtReceived;
                xQueueSendFromISR(bt->qHandle, &val, &taskwoken);
                portEND_SWITCHING_ISR(taskwoken);
            }

            BluetoothCommon::dmaRXreadCount = 0;
        }
        if (isrReg & kLPUART_RxDataRegFullFlag) {}
        if (isrReg & kLPUART_RxOverrunFlag) {
            val = Bt::Message::EvtUartError;
            xQueueSendFromISR(bt->qHandle, &val, &taskwoken);
        }
        LPUART_ClearStatusFlags(BSP_BLUETOOTH_UART_BASE, isrReg);
    }
}
