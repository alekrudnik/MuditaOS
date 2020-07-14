#include "application-calendar/models/CalendarModel.hpp"
#include "CalendarItem.hpp"

#include "ListView.hpp"
#include "gui/widgets/Label.hpp"

#include <Style.hpp>
#include <time/time_conversion.hpp>

namespace gui
{

    CalendarItem::CalendarItem()
    {
        setMinimumSize(style::window::default_body_width, style::window::calendar::item::height);
        setMaximumSize(style::window::default_body_width, style::window::calendar::item::height);

        setRadius(0);
        setPenFocusWidth(2);
        setPenWidth(0);

        title = new gui::Label(this, 0, 0, 0, 0);
        title->setPenFocusWidth(0);
        title->setPenWidth(0);
        title->setFont(style::window::font::bigbold);
        title->setAlignment(
            gui::Alignment{gui::Alignment::ALIGN_HORIZONTAL_LEFT, gui::Alignment::ALIGN_VERTICAL_CENTER});

        description = new gui::Label(this, 0, 0, 0, 0);
        description->setPenFocusWidth(0);
        description->setPenWidth(0);
        description->setFont(style::window::font::medium);
        description->setAlignment(
            gui::Alignment{gui::Alignment::ALIGN_HORIZONTAL_LEFT, gui::Alignment::ALIGN_VERTICAL_CENTER});

        description->setText(utils::localize.get("common_information"));
        title->setText("10:00 - 11:15 AM");
    }

    bool CalendarItem::onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim)
    {
        title->setPosition(style::window::calendar::item::label_x, style::window::calendar::item::title_y);
        title->setSize(style::window::calendar::item::title_w, style::window::calendar::item::title_h);

        description->setPosition(style::window::calendar::item::label_x, style::window::calendar::item::descripton_y);
        description->setSize(newDim.w - 2 * style::window::calendar::item::label_x,
                             newDim.h - style::window::calendar::item::title_h);

        return true;
    }

} /* namespace gui */
