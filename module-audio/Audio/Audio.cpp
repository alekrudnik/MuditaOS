#include "Audio.hpp"
#include "Operation/Operation.hpp"
#include <log/log.hpp>
#include <Utils.hpp>

namespace audio
{

    Audio::Audio(std::function<int32_t(AudioEvents event)> asyncCallback)
        : currentOperation(), asyncCallback(asyncCallback)
    {

        auto ret = Operation::Create(Operation::Type::Idle, "");
        if (ret) {
            currentOperation = std::move(ret.value());
        }
    }

    Position Audio::GetPosition()
    {
        return currentOperation != nullptr ? currentOperation->GetPosition() : -1; // TODO: need to be fixed
    }

    std::optional<Tags> Audio::GetFileTags(const char *filename)
    {
        auto ret = decoder::Create(filename);
        if (ret == nullptr) {
            return {};
        }
        else {
            return *ret->fetchTags();
        };
    }

    int32_t Audio::SendEvent(const Operation::Event evt, const EventData *data)
    {
        return currentOperation != nullptr ? currentOperation->SendEvent(evt, data)
                                           : static_cast<int32_t>(RetCode::OperationNotSet);
    }

    int32_t Audio::SetOutputVolume(Volume vol)
    {
        auto volToSet = vol;
        if (vol > 1) {
            volToSet = 1;
        }
        if (vol < 0) {
            volToSet = 0;
        }

        return currentOperation != nullptr ? currentOperation->SetOutputVolume(volToSet)
                                           : static_cast<int32_t>(RetCode::OperationNotSet);
    }

    int32_t Audio::SetInputGain(Gain gain)
    {
        auto gainToSet = gain;
        if (gain > 10) {
            gainToSet = 10.0;
        }
        if (gain < 0) {
            gainToSet = 0;
        }
        return currentOperation != nullptr ? currentOperation->SetInputGain(gainToSet)
                                           : static_cast<int32_t>(RetCode::OperationNotSet);
    }

    int32_t Audio::Start(Operation::Type op, const char *fileName)
    {

        auto ret = Operation::Create(op, fileName);
        if (ret) {

            switch (op) {
            case Operation::Type::Playback:
                currentState = State::Playback;
                break;
            case Operation::Type::Recorder:
                currentState = State::Recording;
                break;
            case Operation::Type::Router:
                currentState = State::Routing;
                break;
            case Operation::Type::Idle:
                break;
            }
            currentOperation = std::move(ret.value());
        }
        else {
            // If creating operation failed fallback to IdleOperation which is guaranteed to work
            LOG_ERROR("Failed to create operation type %d", static_cast<int>(op));
            currentOperation = Operation::Create(Operation::Type::Idle, "").value_or(nullptr);
            currentState     = State ::Idle;
            return static_cast<int32_t>(RetCode::OperationCreateFailed);
        }

        return currentOperation->Start(asyncCallback);
    }

    int32_t Audio::Stop()
    {
        if (currentState == State::Idle) {
            return static_cast<int32_t>(RetCode::Success);
        }

        auto retStop =
            currentOperation != nullptr ? currentOperation->Stop() : static_cast<int32_t>(RetCode::OperationNotSet);
        if (retStop != 0) {
            LOG_ERROR("Operation STOP failure: %" PRIu32 " see RetCode enum for audio for more information", retStop);
        }

        auto ret = Operation::Create(Operation::Type::Idle, "");
        if (ret) {
            currentState     = State::Idle;
            currentOperation = std::move(ret.value());
            return static_cast<int32_t>(RetCode::Success);
        }
        else {
            return static_cast<int32_t>(RetCode::OperationCreateFailed);
        }
    }

    int32_t Audio::Pause()
    {
        if (currentState == State::Idle) {
            return static_cast<int32_t>(RetCode::InvokedInIncorrectState);
        }

        return currentOperation != nullptr ? currentOperation->Pause() : static_cast<int32_t>(RetCode::OperationNotSet);
    }

    int32_t Audio::Resume()
    {
        if (currentState == State::Idle) {
            return static_cast<int32_t>(RetCode::InvokedInIncorrectState);
        }
        return currentOperation != nullptr ? currentOperation->Resume()
                                           : static_cast<int32_t>(RetCode::OperationNotSet);
    }

} // namespace audio
