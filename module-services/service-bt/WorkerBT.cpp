#include "WorkerBT.hpp"

bool WorkerBT::handleMessage(uint32_t queueID)
{

    QueueHandle_t queue = queues[queueID];

    std::string qname = queueNameMap[queue];
    LOG_INFO("[ServiceDesktop:Worker] Received data from queue: %s", qname.c_str());

    static std::string receiveMsg;
    // static std::string *sendMsg;

    if (qname == SERVICE_QUEUE_NAME) {
        LOG_ERROR("[ServiceDesktop:Worker] Service Queue invoked but not implemented!");
    }

    if (qname == RECEIVE_QUEUE_BUFFER_NAME) {
        if (xQueueReceive(queue, &receiveMsg, 0) != pdTRUE) {
            return false;
        }
        LOG_DEBUG("%d> %s",receiveMsg.length(), receiveMsg.c_str());
        auto send = new std::string(receiveMsg);
        bsp::usbCDCSend(send);
    }

    // TODO: Consider moving sendBuffer receive to bsp driver
    ///if (qname == SEND_QUEUE_BUFFER_NAME) {
    ///    if (xQueueReceive(queue, &sendMsg, 0) != pdTRUE)
    ///        return false;

    ///    bsp::usbCDCSend(sendMsg);
    ///}

    return true;
}

bool WorkerBT::init(std::list<sys::WorkerQueueInfo> queues)
{
    LOG_DEBUG("init worker bt");
    Worker::init(queues);


    LOG_DEBUG("get bt instajce");
    bt = bsp::BlueKitchen::getInstance();
    // TODO here initialization and crap...

    LOG_DEBUG("start usb");
    auto queue = Worker::getQueueByName(WorkerBT::RECEIVE_QUEUE_BUFFER_NAME);
    if( queue == nullptr ) 
    {
        LOG_FATAL("No queue for USB");
    }
    // now start usb
    if ((bsp::usbCDCInit(queue) < 0)) {
        LOG_ERROR("won't start desktop service without serial port");
        return false;
    }
    LOG_DEBUG("WorkerBT initialized - usb Started");
    return true;
}

bool WorkerBT::deinit(void)
{
    Worker::deinit();
    return true;
}
