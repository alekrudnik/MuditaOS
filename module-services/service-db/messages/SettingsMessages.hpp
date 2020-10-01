#pragma once

#include <Service/Message.hpp>
#include <MessageType.hpp>

#include <memory>
#include <utility>
#include <variant>
#include <set>
#include <list>

namespace Settings
{
    struct EntryPath
    {
        std::string mode;
        std::string service;
        std::string profile;
        std::string variable;

        [[nodiscard]] auto to_string(std::string sep = "\\") const -> std::string
        {
            return mode + sep + service + sep + profile + sep + variable;
        }
    };

    namespace Messages
    {
        class SettingsMessage : public sys::DataMessage
        {
          public:
            explicit SettingsMessage(MessageType type = MessageType::Settings) : sys::DataMessage(type){};
            ~SettingsMessage() override = default;
        };

        /// Variable manipulation
        class Variable : public SettingsMessage
        {
          public:
            Variable() = default;
            explicit Variable(EntryPath path, std::optional<std::string> value = {})
                : SettingsMessage(), path(std::move(path)), value(std::move(value))
            {}

            [[nodiscard]] auto getValue() const -> std::optional<std::string>
            {
                return value;
            }

            [[nodiscard]] auto getPath() const -> EntryPath
            {
                return path;
            }

          protected:
            EntryPath path;
            std::optional<std::string> value;
        };

        class GetVariable : public Variable
        {
          public:
            GetVariable() = default;
            explicit GetVariable(EntryPath path) : Variable(path)
            {}
        };

        class SetVariable : public Variable
        {
          public:
            SetVariable() = default;
            SetVariable(EntryPath path, std::string value) : Variable(path, value)
            {}
        };

        class RegisterOnVariableChange : public Variable
        {
          public:
            RegisterOnVariableChange() = default;
            explicit RegisterOnVariableChange(EntryPath path) : Variable(path)
            {}
        };

        class UnregisterOnVariableChange : public Variable
        {
          public:
            UnregisterOnVariableChange() = default;
            explicit UnregisterOnVariableChange(EntryPath path) : Variable(path)
            {}
        };

        class VariableChanged : public Variable
        {
          public:
            VariableChanged() = default;
            explicit VariableChanged(EntryPath path, std::string value, std::string old_value)
                : Variable(path, value), old_value(std::move(old_value))
            {}

            [[nodiscard]] auto getOldValue() const -> std::string
            {
                return old_value;
            }

          protected:
            std::string old_value;
        };

        /// Profiles manipulation
        class ListProfiles : public SettingsMessage
        {
          public:
            [[nodiscard]] auto getProfiles() const -> std::set<std::string>
            {
                return profiles;
            }

          private:
            std::set<std::string> profiles;
        };

        class ProfileSettingsMessage : public SettingsMessage
        {
          public:
            [[nodiscard]] auto getProfileName() const -> std::string
            {
                return profile;
            }

          protected:
            ProfileSettingsMessage() = default;
            explicit ProfileSettingsMessage(std::string name) : SettingsMessage(), profile(std::move(name))
            {}

          protected:
            std::string profile;
        };

        class SetCurrentProfile : public ProfileSettingsMessage
        {
          public:
            SetCurrentProfile() = default;
            explicit SetCurrentProfile(std::string profile) : ProfileSettingsMessage(profile)
            {}
        };

        class AddProfile : public ProfileSettingsMessage
        {
          public:
            AddProfile() = default;
            explicit AddProfile(std::string profile) : ProfileSettingsMessage(profile)
            {}
        };

        class GetCurrentProfile : public ProfileSettingsMessage
        {
          public:
            GetCurrentProfile() = default;
            explicit GetCurrentProfile(std::string profile) : ProfileSettingsMessage(profile)
            {}
        };

        class RegisterOnProfileChange : public SettingsMessage
        {};

        class UnregisterOnProfileChange : public SettingsMessage
        {};

        class CurrentProfileChanged : public ProfileSettingsMessage
        {
          public:
            CurrentProfileChanged() = default;
            explicit CurrentProfileChanged(std::string profile) : ProfileSettingsMessage(profile)
            {}
        };

        /// Modes manipulation
        class ListModes : public SettingsMessage
        {
          public:
            [[nodiscard]] auto getModes() const -> std::set<std::string>
            {
                return modes;
            }

          private:
            std::set<std::string> modes;
        };

        class Mode : public SettingsMessage
        {
          public:
            [[nodiscard]] auto getModeName() const -> std::string
            {
                return mode;
            }

          protected:
            Mode() = default;
            explicit Mode(std::string mode) : SettingsMessage(), mode(std::move(mode))
            {}

          protected:
            std::string mode;
        };

        class SetCurrentMode : public ProfileSettingsMessage
        {
          public:
            SetCurrentMode() = default;
            explicit SetCurrentMode(std::string mode) : ProfileSettingsMessage(mode)
            {}
        };

        class AddMode : public ProfileSettingsMessage
        {
          public:
            AddMode() = default;
            explicit AddMode(std::string mode) : ProfileSettingsMessage(mode)
            {}
        };

        class GetCurrentMode : public ProfileSettingsMessage
        {
          public:
            GetCurrentMode() = default;
            explicit GetCurrentMode(std::string profile) : ProfileSettingsMessage(profile)
            {}
        };

        class RegisterOnModeChange : public SettingsMessage
        {};

        class UnregisterOnModeChange : public SettingsMessage
        {};

        class CurrentModeChanged : public ProfileSettingsMessage
        {
          public:
            CurrentModeChanged() = default;
            explicit CurrentModeChanged(std::string profile) : ProfileSettingsMessage(profile)
            {}
        };

        class ValueResponse : sys::ResponseMessage
        {
          public:
            ValueResponse() = default;
            ValueResponse(std::string value) : sys::ResponseMessage(), value(std::move(value))
            {}

            [[nodiscard]] auto getValue() const -> std::string
            {
                return value;
            }

          private:
            std::string value;
        };

        class VariableResponse : public sys::ResponseMessage
        {
          public:
            explicit VariableResponse(EntryPath path,
                                      std::optional<std::string> value,
                                      sys::ReturnCodes code = sys::ReturnCodes::Success)
                : sys::ResponseMessage(code), path(std::move(path)), value(std::move(value))
            {}

            [[nodiscard]] auto getValue() const -> std::optional<std::string>
            {
                return value;
            }

            [[nodiscard]] auto getPath() const -> std::string
            {
                return path.to_string();
            }

          protected:
            EntryPath path;
            std::optional<std::string> value;
        };

        class ProfileResponse : public ValueResponse
        {
          public:
            ProfileResponse() = default;
            explicit ProfileResponse(std::string profile) : ValueResponse(profile)
            {}
        };

        class ModeResponse : public ValueResponse
        {
          public:
            ModeResponse() = default;
            explicit ModeResponse(std::string mode) : ValueResponse(mode)
            {}
        };

        class ListResponse : public sys::ResponseMessage
        {
          public:
            ListResponse() = default;
            explicit ListResponse(std::list<std::string> value, sys::ReturnCodes code = sys::ReturnCodes::Success)
                : sys::ResponseMessage(code), value(std::move(value))
            {}

            [[nodiscard]] auto getValue() const -> std::list<std::string>
            {
                return value;
            }

          protected:
            std::list<std::string> value;
        };

        class ProfileListResponse : public ListResponse
        {
          public:
            ProfileListResponse() = default;
            explicit ProfileListResponse(std::list<std::string> profiles) : ListResponse(profiles)
            {}
        };

        class ModeListResponse : public ListResponse
        {
          public:
            ModeListResponse() = default;
            explicit ModeListResponse(std::list<std::string> modes) : ListResponse(modes)
            {}
        };

    } // namespace Messages
} // namespace Settings
