#pragma once

#include "PhoneNumber.hpp"

namespace gui
{
    class PinLockHandler;

    class PinLock
    {
      public:
        enum class LockType
        {
            Screen,
            SIM,
            PUK,
            Unknown
        };
        enum class InfoName
        {
            LockName,
            PhoneNum
        };
        enum class State
        {
            EnterPin,
            InvalidPin,
            VerifiedPin,
            Blocked,
            Unlocked
        };

        [[nodiscard]] State getState() const noexcept
        {
            return state;
        }
        [[nodiscard]] unsigned int getPinSize() const noexcept
        {
            return pinValue.size();
        }
        /// returns current position of a PIN character to be inserted
        [[nodiscard]] unsigned int getCharCount() const noexcept
        {
            return charCount;
        }
        [[nodiscard]] unsigned int getRemainingAttempts() const noexcept
        {
            return remainingAttempts;
        }
        [[nodiscard]] bool canPut() const noexcept
        {
            return getCharCount() != getPinSize();
        }
        void putNextChar(unsigned int c) noexcept;
        void verifyPin() noexcept;
        /// removes a last character passed to Lock via putNextChar. The last character can not be popped
        void popChar() noexcept;
        /// clear all characters passed to the Lock
        void clearAttempt() noexcept;
        /// if Lock is in the State::InvalidPin state, changes it's state to the State::EnterPin
        void consumeInvalidPinState() noexcept;

        [[nodiscard]] bool isLocked() const noexcept;
        bool unlock() noexcept;
        void lock() noexcept;

        [[nodiscard]] std::string getLockInfo(const InfoName name) const;
        [[nodiscard]] LockType getLockType() const noexcept
        {
            return type;
        }
        PinLock(gui::PinLockHandler *);

      private:
        /// for PIN verification purposes as PIN storage and management is out of scope of PinLock class
        gui::PinLockHandler *handler;

        LockType type                  = LockType::Unknown;
        State state                    = State::EnterPin;
        unsigned int remainingAttempts = 0;
        /// code of the entered character on specified position
        std::vector<unsigned int> pinValue;
        /// flag defines number of entered pin characters
        unsigned int charCount = 0;
        std::map<InfoName, std::string> additionalLockInfo;

        void reset(LockType, State, unsigned int remainingAttempts, unsigned int pinSize) noexcept;

        friend class gui::PinLockHandler;
    };

} // namespace gui
