/*
 * WorkerEvent.cpp
 *
 *  Created on: May 31, 2019
 *      Author: kuba
 */

#include <string.h>

extern "C" {
	#include "FreeRTOS.h"
	#include "task.h"
}


#include "Service/Service.hpp"
#include "Service/Message.hpp"
#include "Service/Worker.hpp"
#include "MessageType.hpp"

#include "WorkerEvent.hpp"
#include "EventManager.hpp"
#include "service-evtmgr/messages/EVMessages.hpp"
#include "module-bsp/bsp/keyboard/keyboard.hpp"

bool WorkerEvent::handleMessage( uint32_t queueID ) {

	QueueHandle_t queue = queues[queueID];

	//service queue
	if( queueID == static_cast<uint32_t>(WorkerEventQueues::queueService) ) {
		sys::WorkerCommand wcmd;
		if( xQueueReceive(queue, &wcmd, 0 ) != pdTRUE ) {
			return false;
		}
		wcmd.command = 1;
		//place some code here to handle messages from service

	}

	if( queueID == static_cast<uint32_t>(WorkerEventQueues::queueKeyboardIRQ) )
	{
		uint8_t notification;
		if( xQueueReceive(queue, &notification, 0 ) != pdTRUE ) {
				return false;
		}
		uint8_t state, code;
		bsp::keyboard_get_data(notification, state, code);

		processKeyEvent(static_cast<bsp::KeyEvents>(state), static_cast<bsp::KeyCodes>(code));

	}
	return true;
}

bool WorkerEvent::init( std::list<sys::WorkerQueueInfo> queues )
{
	Worker::init(queues);
	std::vector<xQueueHandle> qhanldes = this->getQueues();
	bsp::keyboard_Init(qhanldes[static_cast<int32_t>(WorkerEventQueues::queueKeyboardIRQ)]);

	return true;
}
bool WorkerEvent::deinit(void)
{
	Worker::stop();
	Worker::deinit();
	bsp::keyboard_Deinit();

	return true;
}



 void WorkerEvent::processKeyEvent(bsp::KeyEvents event, bsp::KeyCodes code)
 {
	auto message = std::make_shared<sevm::KbdMessage>(MessageType::KBDKeyEvent);

	message->keyCode = code;

	if(event == bsp::KeyEvents::Pressed)
	{
		if(lastState == bsp::KeyEvents::Pressed) {
			return;
		}

		message->keyState =  sevm::KeyboardEvents::keyPressed;
		message->keyPressTime = xTaskGetTickCount();
		message->keyRelaseTime = 0;

		// Slider sends only press, not release state so it would block the entire keyboard
		if( (code != bsp::KeyCodes::SSwitchUp) && (code != bsp::KeyCodes::SSwitchMid) && (code != bsp::KeyCodes::SSwitchDown) ) {
			lastPressed = code;
			lastState = event;
		}
	}
	else
	{
		if( lastState != bsp::KeyEvents::Pressed)
		{
			return;
		}
		if( lastPressed != code)
		{
			return;
		}

		lastState = bsp::KeyEvents::Released;
		{
			message->keyState =  sevm::KeyboardEvents::keyReleasedShort;
			message->keyRelaseTime = xTaskGetTickCount();
		}
	}
	sys::Bus::SendUnicast(message, "EventManager", this->service);
 }

