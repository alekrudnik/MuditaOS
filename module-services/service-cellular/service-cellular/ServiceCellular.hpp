// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "CellularCall.hpp"
#include "CellularMessage.hpp"
#include "USSD.hpp"
#include "PacketData.hpp"
#include "PacketDataCellularMessage.hpp"
#include <service-cellular/connection-manager/ConnectionManager.hpp>

#include <modem/ATURCStream.hpp>
#include <modem/mux/DLCChannel.h>
#include <modem/mux/CellularMux.h>
#include <modem/mux/CellularMuxTypes.h>
#include <SMSRecord.hpp>
#include <Service/Common.hpp>
#include <Service/Message.hpp>
#include <Service/Service.hpp>
#include <Service/CpuSentinel.hpp>
#include <Timers/TimerHandle.hpp>
#include <bsp/common.hpp>
#include <utf8/UTF8.hpp>
#include <service-db/Settings.hpp>
#include <module-services/service-db/agents/settings/SystemSettings.hpp>
#include <module-sys/PhoneModes/Observer.hpp>
#include <service-db/DBServiceName.hpp>
#include <service-db/DBNotificationMessage.hpp>

#include <optional> // for optional
#include <memory>   // for unique_ptr, allocator, make_unique, shared_ptr
#include <string>   // for string
#include <vector>   // for vector
#include <cstdint>


namespace db
{
    namespace query
    {
        class SMSSearchByTypeResult;
    } // namespace query
} // namespace db

namespace packet_data
{
    class PacketData;
    class PDPContext;
} // namespace packet_data

namespace constants
{
    using namespace std::chrono_literals;
    inline constexpr std::chrono::milliseconds sleepTimerInterval{500ms};
    inline constexpr std::chrono::milliseconds enterSleepModeTime{5s};
} // namespace constants

class ConnectionManager;

namespace internal
{
    class ServiceCellularPriv;
}

class ServiceCellular : public sys::Service
{

  public:
    ServiceCellular();

    ~ServiceCellular() override;

    sys::MessagePointer DataReceivedHandler(sys::DataMessage *msgl, sys::ResponseMessage *resp = nullptr) override
    {
        return std::make_shared<sys::ResponseMessage>();
    }
    // Invoked during initialization
    sys::ReturnCodes InitHandler() override;
    sys::ReturnCodes DeinitHandler() override;
    void ProcessCloseReason(sys::CloseReason closeReason) override;
    sys::ReturnCodes SwitchPowerModeHandler(const sys::ServicePowerMode mode) override final;

    /** Register message handlers.
     */
    void registerMessageHandlers();

    static const char *serviceName;

    auto sendSMS(SMSRecord record) -> bool;
    auto receiveSMS(std::string messageNumber) -> bool;

    /**
     * @brief Its getting selected SIM card own number.
     * @param destination Reference to destination string.
     * @return true when succeed, false when fails
     */
    bool getOwnNumber(std::string &destination);
    /**
     * @brief Its getting IMSI from selected SIM card.
     * @param fullNumber Its returning full IMSI number when fullNumber is true, otherwise its returning only country
     * identification number
     * @param destination Reference to destination string.
     * @return true when succeed, false when fails
     */
    bool getIMSI(std::string &destination, bool fullNumber = false);
    std::vector<std::string> getNetworkInfo();

    /** group of action/messages send "outside" eg. GUI
     * requestPin is call anytime modem need pin, here should be called any action
     * which allow user input (or mockup) pin. Then send appropriate action to notify the modem
     * \param attempts Attempts counter for current action
     * \param msg Literal name of action eg. SIM PIN
     * \return
     */
    bool requestPin(unsigned int attempts, const std::string msg);

    /** requestPuk is call anytime modem need puk, here should be called any action
     * which allow user input (or mockup) puk and new pin. Then send appropriate action to notify the modem
     * \param attempts Attempts counter for current action
     * \param msg Literal name of action eg. SIM PUK
     * \return
     */
    bool requestPuk(unsigned int attempts, const std::string msg);

    /** Call in case of SIM card unlocked, MT ready. Place for sending message/action inform rest
     * \return
     */
    bool sendSimUnlocked();

    /** Call in case of SIM card locked (card fail, eg. to many bad PUK). Place for sending message/action inform rest
     * \return
     */
    bool sendSimBlocked();

    /** From this point should be send message/action call interaction in other layers eg. GUI
     * \param cme_error
     * \return
     */
    bool sendUnhandledCME(unsigned int cme_error);

    /** Similar to sendBadPin
     * \return
     */
    bool sendBadPin();

    /** Message send, when modem return incorrect password for PIN message.
     * Probably modem firmware depend. On current version last bad message (attempts=1) return PUK request
     * and generate PUK URC, so finally action on puk request will be call. This implementation allow to
     * rethrow URC (so achive similar behavior in all cases).
     * \return
     */
    bool sendBadPuk();

    /** Place to send action notifying eg. GUI
     * \param res
     * \return
     */
    //    bool sendChangePinResult(cellular::service::sim::Result res);

    /// sim functionality

    /** Function checks if sim pin is locked (enabled)
     * @return True if sim pin is locked, False if it's not
     */
    bool isPinLocked();

    /** Function ready for change pin action send to Service Cellular form eg. GUI
     * \param oldPin
     * \param newPin
     * \return
     */
    bool changePin(const std::string oldPin, const std::string newPin);
    bool unlockSimPin(std::string pin);
    bool unlockSimPuk(std::string puk, std::string pin);
    bool setPinLock(bool lock, const std::string pin);

  private:
    at::ATURCStream atURCStream;
    std::unique_ptr<CellularMux> cmux = std::make_unique<CellularMux>(PortSpeed_e::PS460800, this);
    std::shared_ptr<sys::CpuSentinel> cpuSentinel;

    // used for polling for call state
    sys::TimerHandle callStateTimer;
    sys::TimerHandle stateTimer;
    sys::TimerHandle ussdTimer;

    // used to enter modem sleep mode
    sys::TimerHandle sleepTimer;

    // used to manage network connection in Messages only mode
    sys::TimerHandle connectionTimer;

    std::unique_ptr<settings::Settings> settings;

    void SleepTimerHandler();
    void CallStateTimerHandler();
    DLCChannel::Callback_t notificationCallback = nullptr;

    std::unique_ptr<packet_data::PacketData> packetData;
    std::unique_ptr<sys::phone_modes::Observer> phoneModeObserver;
    std::unique_ptr<ConnectionManager> connectionManager;
    bsp::Board board = bsp::Board::none;

    /// URC GSM notification handler
    std::optional<std::shared_ptr<CellularMessage>> identifyNotification(const std::string &data);

    std::vector<std::string> messageParts;

    CellularCall::CellularCall ongoingCall;

    ussd::State ussdState = ussd::State::none;

    enum class ResetType
    {
        SoftReset,  //<! AT CFUN reset
        PowerCycle, //<! PWRKEY pin toggle
        HardReset   //<! RESET_N pin
    };

    bool resetCellularModule(ResetType type);
    bool isAfterForceReboot                    = false;
    bool nextPowerStateChangeAwaiting          = false;

    /// one point of state change handling
    void change_state(cellular::StateChange *msg);

    /// @defgroup state_handlers     all functions on State::ST:: change requests
    /// @{
    /// modem has started to turn off
    bool handle_power_down_started();
    /// wait some time or for change of a status pin
    bool handle_power_down_waiting();
    /// what to do after a full power down
    bool handle_power_down();
    /// idle handler
    bool handle_idle();
    ///  wait for start permission handlers
    bool handle_wait_for_start_permission();
    ///  start the module in proper way
    bool handle_power_up_request();
    /// cellular power up procedure
    bool handle_status_check();
    /// cellular power up procedure
    bool handle_power_up_in_progress_procedure();
    /// cellular power up procedure
    bool handle_power_up_procedure();
    /// detect communication baud rate
    bool handle_baud_detect();
    /// configure basic modem parameters
    bool handle_start_conf_procedure();
    /// configure modem audio parameters
    bool handle_audio_conf_procedure();
    /// modem on event is used in desktop to follow up sim selection
    bool handle_modem_on();
    /// URCReady event is set when serwice is ready to handle URC notifications
    bool handle_URCReady();
    /// check one time modem configuration for sim (hot swap)
    /// if hot swap is not enabled full modem restart is needed (right now at best reboot)
    bool handle_sim_sanity_check();
    /// select sim from settings
    bool handle_select_sim();
    /// initialize sim (GSM commands for initialization)
    bool handle_sim_init();
    /// modem failure handler
    bool handle_failure();
    /// fatal failure handler, if we have power switch - we could handle it here
    /// \note some run state should be added to ignore non system messages now...
    bool handle_fatal_failure();
    bool handle_ready();
    /// Process change of power state
    void handle_power_state_change();

    bool handle_apn_conf_procedure();

    bool handleTextMessagesInit();
    [[nodiscard]] SMSRecord createSMSRecord(const UTF8 &decodedMessage,
                                            const UTF8 &receivedNumber,
                                            const time_t messageDate,
                                            const SMSType &smsType = SMSType::INBOX) const noexcept;
    bool dbAddSMSRecord(const SMSRecord &record);
    void onSMSReceived(const utils::PhoneNumber::View &number);
    [[nodiscard]] bool receiveAllMessages();
    /// @}

    bool transmitDtmfTone(uint32_t digit);
    /// Handle message CellularGetChannelMessage
    void handle_CellularGetChannelMessage();

    bool SetScanMode(std::string mode);
    std::string GetScanMode();

    uint32_t stateTimeout = 0;
    void startStateTimer(uint32_t timeout);
    void stopStateTimer();
    void handleStateTimer();

    // db response handlers
    auto handle(db::query::SMSSearchByTypeResult *response) -> bool;

    // ussd handlers
    uint32_t ussdTimeout = 0;
    void setUSSDTimer();
    bool handleUSSDRequest(CellularUSSDMessage::RequestType requestType, const std::string &request = "");
    bool handleUSSDURC();
    void handleUSSDTimer();

    bool handleSimState(at::SimState state, const std::string &message);
    auto handleSimPinMessage(sys::Message *msgl) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleSimPukMessage(sys::Message *msgl) -> std::shared_ptr<sys::ResponseMessage>;

    std::shared_ptr<cellular::RawCommandRespAsync> handleCellularStartOperatorsScan(
        CellularStartOperatorsScanMessage *msg);

    std::shared_ptr<CellularSetOperatorAutoSelectResponse> handleCellularSetOperatorAutoSelect(
        CellularSetOperatorAutoSelectMessage *msg);
    std::shared_ptr<CellularGetCurrentOperatorResponse> handleCellularGetCurrentOperator(
        CellularGetCurrentOperatorMessage *msg);
    std::shared_ptr<CellularGetAPNResponse> handleCellularGetAPNMessage(CellularGetAPNMessage *msg);
    std::shared_ptr<CellularSetAPNResponse> handleCellularSetAPNMessage(CellularSetAPNMessage *msg);
    std::shared_ptr<CellularNewAPNResponse> handleCellularNewAPNMessage(CellularNewAPNMessage *msg);
    std::shared_ptr<CellularSetOperatorResponse> handleCellularSetOperator(CellularSetOperatorMessage *msg);
    std::shared_ptr<CellularSetDataTransferResponse> handleCellularSetDataTransferMessage(
        CellularSetDataTransferMessage *msg);
    std::shared_ptr<CellularGetDataTransferResponse> handleCellularGetDataTransferMessage(
        CellularGetDataTransferMessage *msg);
    std::shared_ptr<CellularActivateContextResponse> handleCellularActivateContextMessage(
        CellularActivateContextMessage *msg);
    std::shared_ptr<CellularDeactivateContextResponse> handleCellularDeactivateContextMessage(
        CellularDeactivateContextMessage *msg);
    std::shared_ptr<CellularGetActiveContextsResponse> handleCellularGetActiveContextsMessage(
        CellularGetActiveContextsMessage *msg);
    friend class CellularUrcHandler;
    friend class SimCard;
    friend class CellularRequestHandler;
    friend class NetworkSettings;
    friend class packet_data::PDPContext;
    friend class packet_data::PacketData;
    friend class ConnectionManagerCellularCommands;

    void volteChanged(const std::string &value);
    void apnListChanged(const std::string &value);
    bool volteOn = false;

    auto handleCellularAnswerIncomingCallMessage(CellularMessage *msg) -> std::shared_ptr<CellularResponseMessage>;
    auto handleCellularCallRequestMessage(CellularCallRequestMessage *msg) -> std::shared_ptr<CellularResponseMessage>;
    void handleCellularHangupCallMessage(CellularHangupCallMessage *msg);
    auto handleDBQueryResponseMessage(db::QueryResponse *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularListCallsMessage(CellularMessage *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleDBNotificatioMessage(db::NotificationMessage *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularRingingMessage(CellularRingingMessage *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularIncominCallMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularCallerIdMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSimProcedureMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetIMSIMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetOwnNumberMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetNetworkInfoMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSelectAntennaMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSetScanModeMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetScanModeMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetFirmwareVersionMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleEVMStatusMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetCsqMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetCregMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetNwinfoMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularGetAntennaMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularDtmfRequestMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularUSSDMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleSimStateMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleStateRequestMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;

    auto handleCallActiveNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCallAbortedNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handlePowerUpProcedureCompleteNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handlePowerDownDeregisteringNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handlePowerDownDeregisteredNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleNewIncomingSMSNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleRawCommandNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleSimReadyNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleSmsDoneNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleSignalStrengthUpdateNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleNetworkStatusUpdateNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleSimNotReadyNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleUrcIncomingNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSetFlightModeMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSetRadioOnOffMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSendSMSMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularRingNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularCallerIdNotification(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;
    auto handleCellularSetConnectionFrequencyMessage(sys::Message *msg) -> std::shared_ptr<sys::ResponseMessage>;

    auto isIncommingCallAllowed() -> bool;

    auto hangUpCall() -> bool;

  private:
    std::unique_ptr<internal::ServiceCellularPriv> priv;
};

namespace sys
{
    template <> struct ManifestTraits<ServiceCellular>
    {
        static auto GetManifest() -> ServiceManifest
        {
            ServiceManifest manifest;
            manifest.name         = ServiceCellular::serviceName;
            manifest.dependencies = {service::name::db};
            return manifest;
        }
    };
} // namespace sys
