
/*
 * @file InputSerialWorker.hpp
 * @author Mateusz Piesta (mateusz.piesta@mudita.com)
 * @date 24.06.19
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */


#ifndef PUREPHONE_INPUTSERIALWORKER_HPP
#define PUREPHONE_INPUTSERIALWORKER_HPP

#include "FreeRTOS.h"
#include "task.h"
#include "GSM0710.hpp"

class MuxDaemon;

void workerTaskFunction( void* ptr );

class InputSerialWorker {
public:

    InputSerialWorker(MuxDaemon* mux);
    ~InputSerialWorker();

private:

    friend void workerTaskFunction( void* ptr );

    // Pointer to muxDaemon which is owner of this worker
    MuxDaemon* muxDaemon;

    int ReadIncomingData();
    int ExtractFrames();
    int HandleCtrlChannelCommands(GSM0710Frame* frame);

    //worker's task handle
    xTaskHandle taskHandle=nullptr;
    const uint32_t taskPriority = 0;

    std::unique_ptr<GSM0710Buffer> inputBuffer=nullptr;

};


#endif //PUREPHONE_INPUTSERIALWORKER_HPP
