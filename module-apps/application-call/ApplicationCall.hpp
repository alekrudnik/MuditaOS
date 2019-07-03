/*
 * @file ApplicationCall.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 1 lip 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_APPS_APPLICATION_CALL_APPLICATIONCALL_HPP_
#define MODULE_APPS_APPLICATION_CALL_APPLICATIONCALL_HPP_

#include "Application.hpp"
#include "SystemManager/SystemManager.hpp"
#include "Service/Message.hpp"

namespace app {

/*
 *
 */
class ApplicationCall: public Application {
public:
	ApplicationCall( std::string name="ApplicationCall");
	virtual ~ApplicationCall();
	sys::Message_t DataReceivedHandler(sys::DataMessage* msgl) override;
	sys::ReturnCodes InitHandler() override;
	sys::ReturnCodes DeinitHandler() override;
	sys::ReturnCodes WakeUpHandler() override;
	sys::ReturnCodes SleepHandler() override;

	void createUserInterface() ;
	void destroyUserInterface();

	/**
	* @brief Sends information from application to manager about result of application's init function.
	* If successful message will contain name and true value, otherwise false value will be transmitted.
	*/
   static bool messageRegisterApplication( sys::Service* sender, const bool& status );
};

class ApplicationCallLauncher : public ApplicationLauncher {
public:
	ApplicationCallLauncher() : ApplicationLauncher("ApplicationCall", true) {};
	bool run(sys::SystemManager* sysmgr) override {
		return sysmgr->CreateService(std::make_shared<ApplicationCall>(name),sysmgr,1000);
	};
};

} /* namespace app */

#endif /* MODULE_APPS_APPLICATION_CALL_APPLICATIONCALL_HPP_ */
