/*
 * @file ApplicationDesktop.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 18 cze 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */

#include "windows/PinWindow.hpp"
#include "windows/DesktopMainWindow.hpp"

#include "Application.hpp"
#include "ApplicationDesktop.hpp"

#include "MessageType.hpp"


namespace app {

ApplicationDesktop::ApplicationDesktop(std::string name) :
	Application( name ) {
}

ApplicationDesktop::~ApplicationDesktop() {
}

// Invoked upon receiving data message
sys::Message_t ApplicationDesktop::DataReceivedHandler(sys::DataMessage* msgl) {

	auto retMsg = Application::DataReceivedHandler(msgl);
	//if message was handled by application's template there is no need to process further.
	if( (reinterpret_cast<sys::ResponseMessage*>( retMsg.get() )->retCode ==
		sys::ReturnCodes::Success ) ){
		return retMsg;
	}

	//this variable defines whether message was processed.
	bool handled = true;

	if( handled )
		return std::make_shared<sys::ResponseMessage>();
	else
		return std::make_shared<sys::ResponseMessage>(sys::ReturnCodes::Unresolved);
}

// Invoked during initialization
sys::ReturnCodes ApplicationDesktop::InitHandler() {

	auto ret = Application::InitHandler();
	if( ret != sys::ReturnCodes::Success )
		return ret;

	createUserInterface();

	setActiveWindow("MainWindow");

	return ret;
}

sys::ReturnCodes ApplicationDesktop::DeinitHandler() {
	return sys::ReturnCodes::Success;
}

sys::ReturnCodes ApplicationDesktop::WakeUpHandler() {
	return sys::ReturnCodes::Success;
}


sys::ReturnCodes ApplicationDesktop::SleepHandler() {
	return sys::ReturnCodes::Success;
}

void ApplicationDesktop::createUserInterface() {

	gui::Window* window = nullptr;

	window = new gui::DesktopMainWindow(this);
	windows.insert(std::pair<std::string,gui::Window*>(window->getName(), window));

	window = new gui::PinWindow(this);
	windows.insert(std::pair<std::string,gui::Window*>( window->getName(), window));
}


void ApplicationDesktop::destroyUserInterface() {
}

} /* namespace gui */
