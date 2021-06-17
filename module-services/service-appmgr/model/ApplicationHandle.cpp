﻿// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <service-appmgr/model/ApplicationHandle.hpp>
#include <apps-common/ApplicationLauncher.hpp>

namespace app::manager
{
    ApplicationHandle::ApplicationHandle(std::unique_ptr<app::ApplicationLauncher> &&_launcher)
        : launcher{std::move(_launcher)}
    {
        if (!launcher) {
            throw std::invalid_argument{"The application launcher must not be null."};
        }
    }

    auto ApplicationHandle::valid() const noexcept -> bool
    {
        return launcher && launcher->handle;
    }

    auto ApplicationHandle::name() const -> ApplicationName
    {
        return launcher->getName();
    }

    auto ApplicationHandle::state() const noexcept -> State
    {
        return valid() ? launcher->handle->getState() : State::NONE;
    }

    void ApplicationHandle::setState(State state) noexcept
    {
        if (valid()) {
            launcher->handle->setState(state);
        }
    }

    auto ApplicationHandle::preventsAutoLocking() const noexcept -> bool
    {
        return launcher->preventsAutoLocking();
    }

    auto ApplicationHandle::closeable() const noexcept -> bool
    {
        return launcher->isCloseable();
    }

    auto ApplicationHandle::started() const noexcept -> bool
    {
        const auto appState = state();
        return appState == State::ACTIVE_FORGROUND || appState == State::ACTIVE_BACKGROUND ||
               appState == State::ACTIVATING;
    }

    auto ApplicationHandle::handles(actions::ActionId action) const noexcept -> bool
    {
        const auto manifest = getManifest();
        return manifest.contains(action);
    }

    void ApplicationHandle::run(sys::phone_modes::PhoneMode mode, sys::Service *caller)
    {
        launcher->run(mode, caller);
    }

    void ApplicationHandle::runInBackground(sys::phone_modes::PhoneMode mode, sys::Service *caller)
    {
        launcher->runBackground(mode, caller);
    }

    void ApplicationHandle::close() noexcept
    {
        launcher->handle = nullptr;
    }

    auto ApplicationHandle::getManifest() const -> const ApplicationManifest &
    {
        return launcher->getManifest();
    }
} // namespace app::manager
