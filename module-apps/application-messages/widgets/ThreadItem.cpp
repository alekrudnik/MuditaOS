#include "ThreadItem.hpp"
#include "time/time_conversion.hpp"

#include "service-db/api/DBServiceAPI.hpp"

#include <Style.hpp>
#include "../MessagesStyle.hpp"

namespace gui
{
    ThreadItem *ThreadItem::makeThreadItem(ThreadModel *model, std::shared_ptr<ThreadRecord> thread)
    {
        ThreadItem *threadItem = nullptr;
        threadItem             = thread->msgRead > 0 ? new ThreadItemNotRead(model) : new ThreadItem(model);

        threadItem->setThreadItem(thread);

        return threadItem;
    }

    ThreadItem::ThreadItem(ThreadModel *model)
    {
        this->model = model;
    }

    void ThreadItem::setPreview()
    {
        UTF8 prefix;
        switch (thread->type) {
        case SMSType::DRAFT:
            prefix = utils::localize.get("app_messages_thread_draft");
            break;
        case SMSType::FAILED:
            prefix = utils::localize.get("app_messages_thread_not_sent");
            break;
        case SMSType::OUTBOX:
        case SMSType::QUEUED:
            prefix = utils::localize.get("app_messages_thread_you");
            break;
        default:
            break;
        }
        preview->setText(prefix + thread->snippet);
    }

    void ThreadItem::setThreadItem(std::shared_ptr<ThreadRecord> thread)
    {
        this->thread = thread;

        if (model != nullptr) {
            auto contactRec = DBServiceAPI::ContactGetByID(model->getApplication(), thread->contactID);
            auto cont       = contactRec->front();
            contact->setText(cont.getFormattedName());
        }

        timestamp->setText(utils::time::DateTime(thread->date));

        setPreview();
    }

    ThreadItemWithIndicator::ThreadItemWithIndicator(ThreadModel *model, const UTF8 &indicatorName) : ThreadItem(model)
    {
        indicator = new gui::Image(this, 0, 0, indicatorName);
    }

    void ThreadItemWithIndicator::onDimensionChangedBottom(const BoundingBox & /*oldDim*/, const BoundingBox &newDim)
    {
        namespace msgStyle = style::messages::threadItem;

        const auto indicatorWidth = indicator->getSize(gui::Axis::X) + 6;
        preview->setPosition(msgStyle::leftMargin + indicatorWidth, newDim.h / 2);
        preview->setSize(newDim.w - msgStyle::previewWidthOffset - indicatorWidth,
                         newDim.h / 2 - msgStyle::bottomMargin);

        indicator->setPosition(msgStyle::leftMargin,
                               newDim.h / 2 + preview->getSize(gui::Axis::Y) / 2 -
                                   indicator->getSize(gui::Axis::Y) / 2); // align with text
    }

    ThreadItemNotRead::ThreadItemNotRead(ThreadModel *model)
        : ThreadItemWithIndicator(model, ThreadItemNotRead::indicatorName)
    {}
} /*namespace gui*/
