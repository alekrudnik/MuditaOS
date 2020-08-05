#pragma once
#include "CalendarListItem.hpp"
#include <Label.hpp>
#include <Text.hpp>
#include <BoxLayout.hpp>

namespace gui

{
    class RepeatAndReminderItem : public CalendarListItem
    {
        gui::HBox *hBox           = nullptr;
        gui::VBox *repeatVBox     = nullptr;
        gui::VBox *reminderVBox   = nullptr;
        gui::Label *repeatTitle   = nullptr;
        gui::Label *repeat        = nullptr;
        gui::Label *reminderTitle = nullptr;
        gui::Label *reminder      = nullptr;

      public:
        RepeatAndReminderItem();
        virtual ~RepeatAndReminderItem() = default;

        void descriptionHandler();
        bool onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim) override;
    };

} /* namespace gui */
