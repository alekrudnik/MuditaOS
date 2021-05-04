// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "LockBoxInconstantSize.hpp"
#include <locks/data/LockData.hpp>

namespace gui
{
    class LockInputWindow;
}

namespace gui
{
    class SimLockBox : public LockBoxInconstantSize
    {
      public:
        explicit SimLockBox(LockInputWindow *LockBaseWindow,
                            locks::SimInputTypeAction simLockInputTypeAction = locks::SimInputTypeAction::UnlockWithPin)
            : LockBoxInconstantSize(LockBaseWindow), LockWindow(LockBaseWindow)
        {
            applyLockActionText(simLockInputTypeAction);
        }

      private:
        void buildLockBox(unsigned int pinSize) final;
        void setVisibleStateInputRequired(InputActionType type) final;
        void setVisibleStateInputInvalid(InputErrorType type, unsigned int value) final;
        void setVisibleStateBlocked() final;
        void setVisibleStateError(unsigned int errorCode) final;
        void applyLockActionText(locks::SimInputTypeAction simLockInputTypeAction);

        [[nodiscard]] top_bar::Configuration configureTopBar();

        LockInputWindow *LockWindow;
        std::string textForInputRequired;
        std::string textForInvalidInput;
        std::string textForInvalidInputLastAttempt;
        std::string textForInvalidInputLastAttemptWarning;
    };
} // namespace gui
