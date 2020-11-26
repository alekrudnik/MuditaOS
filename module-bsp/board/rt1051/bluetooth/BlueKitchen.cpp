// Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "bsp/bluetooth/Bluetooth.hpp"
#include "BluetoothWorker.hpp"
#include "log/log.hpp"
#include "FreeRTOS.h"
#include "fsl_lpuart.h"
#include "board.h"

using namespace bsp;

BlueKitchen::BlueKitchen(unsigned int in_size, unsigned int out_size) : BluetoothCommon(in_size, out_size)
{
    to_read       = 0;
    read_buff     = NULL;
    read_ready_cb = NULL;
    write_done_cb = NULL;
    in.threshold  = 128;
}

BlueKitchen::~BlueKitchen()
{}

BlueKitchen *BlueKitchen::getInstance()
{
    static BlueKitchen *k = NULL;
    if (k == NULL) {
        /// outcomming & incomming heap allocated buffers sizes
        /// packet on IP network cna have MTU 1500, so big enough buffers were added to not throttle comms
        k = new BlueKitchen(2048, 8000);
    }
    return k;
}

// request... from circ buffer to give data to buf
ssize_t BlueKitchen::read(char *buf, size_t nbytes)
{
    LOG_DEBUG("BlueKitchen requested %d bytes to read", nbytes);

    ssize_t i            = 0;
    BaseType_t taskwoken = 0;
    uint8_t val;

    to_read = nbytes;

    read_buff = reinterpret_cast<char *>(buf);
    read_len  = nbytes;

    if (BluetoothCommon::read(reinterpret_cast<char *>(buf), nbytes) == nbytes) {
        val = Bt::Message::EvtReceiving;
        xQueueSendFromISR(qHandle, &val, &taskwoken);
    }
    else {
        val = Bt::Message::EvtReceivingError;
        xQueueSendFromISR(qHandle, &val, &taskwoken);
    }
    portEND_SWITCHING_ISR(taskwoken);

    return i;
}

void BlueKitchen::set_flowcontrol(int on)
{
    // TODO
}

#include <sstream>

ssize_t BlueKitchen::write(char *buf, size_t size)
{

    LOG_DEBUG("BlueKitchen sends %d bytes", size);

    ssize_t i            = 0;
    BaseType_t taskwoken = 0;
    uint8_t val;

#define DO_DEBUG_HCI_COMS
#ifdef DO_DEBUG_HCI_COMS
    std::stringstream ss;
    for (int i = 0; i < size; ++i) {
        ss << " 0x" << std::hex << (int)buf[i];
    }
    LOG_DEBUG("--> [%d]>%s<", size, ss.str().c_str());
#endif
    if (BluetoothCommon::write(buf, size) == size) {
        val = Bt::Message::EvtSending;
        xQueueSendFromISR(qHandle, &val, &taskwoken);
    }
    else {
        val = Bt::Message::EvtSendingError;
        xQueueSendFromISR(qHandle, &val, &taskwoken);
    }
    portEND_SWITCHING_ISR(taskwoken);

    return i;
}
