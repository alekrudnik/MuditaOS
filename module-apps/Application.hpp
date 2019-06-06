/*
 * @file Application.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 1 cze 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_HPP_
#define MODULE_APPS_APPLICATION_HPP_

#include <map>
//module-gui
#include "gui/Common.hpp"
#include "gui/widgets/Window.hpp"
//module-sys
#include "Service/Service.hpp"
#include "Service/Message.hpp"
#include "Service/Common.hpp"
#include "SystemManager/SystemManager.hpp"

namespace app {

//class template that stores information that was sent along with switch message
class SwitchData {
public:

	SwitchData( uint8_t* data, uint32_t size ) : data{nullptr}, size{0} {
//		this->data = new uint8_t[size];
//		if( data ) {
//			memcpy( this->data, data, size );
//			this->size = size;
//		}
//		else
//			this->size = 0;
	}
	~SwitchData() {
		if( data )
			delete []data;
	}

	uint8_t* data;
	uint32_t size;
};
/*
 * @brief This is template for creating new applications
 */
class Application: public sys::Service {
public:
	enum class State {
		//Application: Object has been created and underlying service is waiting to execute init handler method.
		//Application Manager: Launcher for the application has been provided. Application can be started using provided launcher. The other possibility
		//is that Appication Manager received CLOSING_FINISHED message.
		DEACTIVATED,
		//Application: Set after entering the init handler of the application. In this state application will request
		//in a blocking way the db configuration of the phone. Before exiting the init handler application must send APP_REGISTER message to
		//Application Manager.
		//Application Manager: Initialization is triggered by the switch message sent by other application to the application manager.
		//Launcher for the application has been executed upon receiving switch command. Optional switch data has been
		//saved and it will be provided when application manager recives APP_READY message.
		INITIALIZING,
		//Application manager sent variant of switch command to the selected application and it's now waiting for confirmation
		//from the application
		ACTIVATING,
		//Application is working and has focus and can render
		ACTIVE_FORGROUND,
		//Applicatino lost focus but it is still working
		ACTIVE_BACKGROUND,
		//Application: Close request message has been received. As a response application must send close request acknowledge message.
		//Application must start closing all workers and releasing resources. After all workers are closed and resources released application
		//sends to application manager CLOSING_FINISHED message
		//Application Manager sent signal to application that it should be closed. Application must perform all necessary cleanup
		//and request System Manager to close it.
		DEACTIVATING
	};
public:
	Application(std::string name,uint32_t stackDepth=4096,sys::ServicePriority priority=sys::ServicePriority::Idle);
	virtual ~Application();

	/**
	 * Method responsible for rendering currently active window.
	 */
	void render( gui::RefreshModes mode );
	/**
	 * Method responsible for setting application to the state where incoming user input is blocked
	 */
	void blockEvents(bool isBlocked );
	/**
	 * Generic function for sending switch command. This will switch window within active application.
	 */
	int switchWindow( const std::string& windowName, uint32_t cmd, uint32_t dataSize=0, uint8_t* data=nullptr );
	/**
	 * Generic function for returning to switch window in active application without performing initialization of the window.
	 */
	int switchBackWindow( const std::string& windowName, uint32_t cmd, uint32_t dataSize=0, uint8_t* data=nullptr );
	/**
	 * Method allows refreshing currently active window
	 */
	int refreshWindow(gui::RefreshModes mode);
	/**
	 * Sets active window of the application. This doesn't cause refresh. value -1 is for undefined state
	 * and should be set when application is left using back button.
	 */
	void setActiveWindow( const std::string& windowName );

//	settings_t* getSettings() {
//		return settings;
//	}

protected:
	/**
	 * Placeholder that can be used to create window and widgets.
	 */
	virtual void createUserInterface() = 0;
	/**
	 * Placeholder for closing application's windows.
	 */
	virtual void destroyUserInterface() = 0;
	/**
	 * Map containing application's windows
	 */
	std::map<std::string, gui::Window*> windows;
	gui::Window* currentWindow = nullptr;
	gui::Window* previousWindow = nullptr;
	/**
	 * Flag defines whether keyboard input is processed
	 */
	bool acceptInput = false;
	/**
	 * State of the application
	 */
	State state = State::DEACTIVATED;



};

class ApplicationLauncher {
protected:
	//name of the application to run
	std::string name;
	//defines whether application can be closed when it looses focus
	bool closeable = true;
public:
	ApplicationLauncher( std::string name, bool isCloseable ) : name{name}, closeable{isCloseable} {};
	virtual ~ApplicationLauncher() {};
	std::string getName() { return name;};
	bool isCloseable() { return closeable; };
	//virtual method to run the application
	virtual bool run(sys::SystemManager* sysmgr) {return true;};
};

} /* namespace app */

#endif /* MODULE_APPS_APPLICATION_HPP_ */
