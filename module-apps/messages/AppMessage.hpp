/*
 * @file AppMessages.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 15 cze 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_MESSAGES_APPMESSAGE_HPP_
#define MODULE_APPS_MESSAGES_APPMESSAGE_HPP_

#include <memory>
#include <string>
#include "SwitchData.hpp"
#include "gui/input/InputEvent.hpp"

namespace app {

/*
 * @brief Template for all messages that go to application manager
 */
class AppMessage: public sys::DataMessage {
protected:
	//name of the application to which switch is performed.
	std::string application;
public:
	AppMessage( MessageType messageType, const std::string& application ) :
		sys::DataMessage( static_cast<uint32_t>(messageType)),
		application{application} {};
	virtual ~AppMessage() {};

	std::string getApplicationName() { return application;};
};

//this message is used to notify application about switching event. Application will gain or lose focus upon receiving this message.
//Application gains focus when it was in init or active background state. Application lose focus when it was in active foreground state.
class AppSwitchMessage : public AppMessage {
protected:
	std::string window;
	std::unique_ptr<gui::SwitchData> data;
public:
	AppSwitchMessage( const std::string& application, const std::string& window, std::unique_ptr<gui::SwitchData> data ) :
		AppMessage( MessageType::AppSwitch, application),
		window{window},
		data {std::move(data)} {};
	virtual ~AppSwitchMessage() {};

	std::string getWindowName() { return window; };
	std::unique_ptr<gui::SwitchData>& getData() { return data; };
};

class AppRefreshMessage : public AppMessage {
protected:
	gui::RefreshModes mode;
public:
	AppRefreshMessage( const std::string& application, gui::RefreshModes mode ) :
		AppMessage( MessageType::AppRefresh, application),
		mode{ mode }{};
	virtual ~AppRefreshMessage() {};

	const gui::RefreshModes& getMode() { return mode; };
};

class AppSwitchWindowMessage : public AppMessage {
protected:
	std::string window;
	uint32_t command;
	std::unique_ptr<gui::SwitchData> data;
public:
	AppSwitchWindowMessage( const std::string& window, uint32_t command, std::unique_ptr<gui::SwitchData> data ) :
		AppMessage( MessageType::AppSwitchWindow, "" ),
		window{window},
		command{ command },
		data {std::move(data)} {};
	virtual ~AppSwitchWindowMessage() {};

	std::string getWindowName() { return window; };
	const uint32_t& getCommand() { return command; };
	std::unique_ptr<gui::SwitchData>& getData() { return data; };
};

class AppInputEventMessage : public AppMessage {
protected:
	gui::InputEvent event;
public:
	AppInputEventMessage( gui::InputEvent evt ) :
		AppMessage( MessageType::AppInputEvent, "" ),
		event{evt} {};
	virtual ~AppInputEventMessage() {};

	const gui::InputEvent& getEvent() { return event; };
};

};

#endif /* MODULE_APPS_MESSAGES_APPMESSAGE_HPP_ */
