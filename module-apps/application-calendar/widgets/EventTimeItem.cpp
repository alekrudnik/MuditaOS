#include "EventTimeItem.hpp"
#include "application-calendar/widgets/CalendarStyle.hpp"
#include <ListView.hpp>
#include <Style.hpp>
#include <Utils.hpp>

namespace gui
{

    EventTimeItem::EventTimeItem(const std::string &description,
                                 bool mode24H,
                                 std::function<void(const UTF8 &text)> bottomBarTemporaryMode,
                                 std::function<void()> bottomBarRestoreFromTemporaryMode)
        : mode24H{mode24H}, bottomBarTemporaryMode(std::move(bottomBarTemporaryMode)),
          bottomBarRestoreFromTemporaryMode(std::move(bottomBarRestoreFromTemporaryMode))
    {
        setMinimumSize(style::window::default_body_width, style::window::calendar::item::eventTime::height);

        setEdges(RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        setMargins(gui::Margins(style::margins::small, style::window::calendar::item::eventTime::margin, 0, 0));

        vBox = new gui::VBox(this, 0, 0, 0, 0);
        vBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        vBox->activeItem = false;

        descriptionLabel = new gui::Label(vBox, 0, 0, 0, 0);
        descriptionLabel->setMinimumSize(style::window::default_body_width,
                                         style::window::calendar::item::eventTime::separator);
        descriptionLabel->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        descriptionLabel->setAlignment(Alignment(gui::Alignment::Horizontal::Left, gui::Alignment::Vertical::Center));
        descriptionLabel->setFont(style::window::font::small);
        descriptionLabel->activeItem = false;

        hBox = new gui::HBox(vBox, 0, 0, 0, 0);
        hBox->setMinimumSize(style::window::default_body_width,
                             style::window::calendar::item::eventTime::height -
                                 style::window::calendar::item::eventTime::separator);
        hBox->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        hBox->activeItem = false;

        hourInput = new gui::Text(hBox, 0, 0, 0, 0);
        hourInput->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
        hourInput->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        hourInput->setFont(style::window::font::largelight);
        hourInput->setInputMode(new InputMode({InputMode::digit}));
        hourInput->setPenFocusWidth(style::window::default_border_focus_w);
        hourInput->setPenWidth(style::window::default_border_rect_no_focus);
        hourInput->setEditMode(gui::EditMode::EDIT);

        colonLabel = new gui::Label(hBox, 0, 0, 0, 0);
        colonLabel->setMinimumSize(style::window::calendar::item::eventTime::separator,
                                   style::window::calendar::item::eventTime::height -
                                       style::window::calendar::item::eventTime::separator);
        colonLabel->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_NO_EDGES);
        colonLabel->setAlignment(Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        colonLabel->setFont(style::window::font::medium);
        colonLabel->setText(":");
        colonLabel->activeItem = false;

        minuteInput = new gui::Text(hBox, 0, 0, 0, 0);
        minuteInput->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
        minuteInput->setAlignment(gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
        minuteInput->setFont(style::window::font::largelight);
        minuteInput->setInputMode(new InputMode({InputMode::digit}));
        minuteInput->setPenFocusWidth(style::window::default_border_focus_w);
        minuteInput->setPenWidth(style::window::default_border_rect_no_focus);
        minuteInput->setEditMode(gui::EditMode::EDIT);

        descriptionLabel->setText(description);

        focusChangedCallback = [&](Item &item) {
            setFocusItem(focus ? hBox : nullptr);
            if (!item.focus) {
                validateHour();
            }
            return true;
        };

        applyInputCallbacks();
        prepareForTimeMode();
    }

    void EventTimeItem::applyInputCallbacks()
    {
        inputCallback = [&](Item &item, const InputEvent &event) {
            auto focusedItem = getFocusItem();
            if (event.state != gui::InputEvent::State::keyReleasedShort) {
                return false;
            }
            if (event.keyCode == gui::KeyCode::KEY_ENTER || event.keyCode == gui::KeyCode::KEY_RF) {
                return false;
            }

            if (focusedItem->onInput(event)) {
                if (mode24H && atoi(hourInput->getText().c_str()) > timeConstants::max_hour_24H_mode) {
                    hourInput->setText("00");
                }
                else if (!mode24H && atoi(hourInput->getText().c_str()) > timeConstants::max_hour_12H_mode) {
                    hourInput->setText("12");
                }
                if (atoi(minuteInput->getText().c_str()) > timeConstants::max_minutes) {
                    minuteInput->setText("00");
                }

                if (this->descriptionLabel->getText() ==
                    utils::localize.get("app_calendar_new_edit_event_start").c_str()) {
                    auto hour = atoi(hourInput->getText().c_str()) + 1;
                    if (!mode24H) {
                        if (mode12hInput->getText() == timeConstants::after_noon) {
                            if (hour == 12) {
                                hour = 11;
                                secondItem->minuteInput->setText(std::to_string(timeConstants::max_minutes));
                            }
                            else {
                                secondItem->minuteInput->setText(minuteInput->getText());
                            }
                            secondItem->mode12hInput->setText(mode12hInput->getText());
                        }
                        else {
                            if (hour == 12) {
                                secondItem->mode12hInput->setText(timeConstants::after_noon);
                            }
                            secondItem->minuteInput->setText(minuteInput->getText());
                        }
                        if (hour > timeConstants::max_hour_12H_mode) {
                            hour = 1;
                            secondItem->mode12hInput->setText(mode12hInput->getText());
                            secondItem->minuteInput->setText(minuteInput->getText());
                        }
                    }
                    else {
                        secondItem->minuteInput->setText(minuteInput->getText());
                        if (hour > timeConstants::max_hour_24H_mode) {
                            hour = timeConstants::max_hour_24H_mode;
                            secondItem->minuteInput->setText(minuteInput->getText());
                        }
                    }
                    secondItem->hourInput->setText(std::to_string(hour));
                }
                return true;
            }
            else if (hBox->onInput(event)) {
                return true;
            }

            return false;
        };

        onSaveCallback = [&](std::shared_ptr<EventsRecord> record) {
            validateHour();
            auto hour = atoi(hourInput->getText().c_str());
            if (!mode24H) {
                hour = convertTimeTo24hMode(hour, mode12hInput->getText());
            }
            if (this->descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_end")) {

                record->date_till = record->date_till - record->date_till % 10000;
                record->date_till = record->date_till + hour * 100 + atoi(minuteInput->getText().c_str());
            }
            else if (this->descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_start")) {
                record->date_from = record->date_from - record->date_from % 10000;
                record->date_from = record->date_from + hour * 100 + atoi(minuteInput->getText().c_str());
            }
        };

        hourInput->inputCallback = [&](Item &item, const InputEvent &event) {
            if (event.state != gui::InputEvent::State::keyReleasedShort) {
                return false;
            }
            if (hourInput->getText().length() > 1 && event.keyCode != gui::KeyCode::KEY_LEFT &&
                event.keyCode != gui::KeyCode::KEY_RIGHT && event.keyCode != gui::KeyCode::KEY_PND &&
                event.keyCode != gui::KeyCode::KEY_UP && event.keyCode != gui::KeyCode::KEY_DOWN) {
                return true;
            }
            return false;
        };

        minuteInput->inputCallback = [&](Item &item, const InputEvent &event) {
            if (event.state != gui::InputEvent::State::keyReleasedShort) {
                return false;
            }
            if (minuteInput->getText().length() > 1 && event.keyCode != gui::KeyCode::KEY_LEFT &&
                event.keyCode != gui::KeyCode::KEY_RIGHT && event.keyCode != gui::KeyCode::KEY_PND &&
                event.keyCode != gui::KeyCode::KEY_UP && event.keyCode != gui::KeyCode::KEY_DOWN) {
                return true;
            }
            return false;
        };
    }

    void EventTimeItem::prepareForTimeMode()
    {
        if (!mode24H) {
            mode12hInput = new gui::Label(hBox, 0, 0, 0, 0);
            mode12hInput->setEdges(gui::RectangleEdgeFlags::GUI_RECT_EDGE_BOTTOM);
            mode12hInput->setAlignment(
                gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Center));
            mode12hInput->setFont(style::window::font::largelight);
            mode12hInput->setPenFocusWidth(style::window::default_border_focus_w);
            mode12hInput->setPenWidth(style::window::default_border_rect_no_focus);
            mode12hInput->setText(timeConstants::before_noon);
            mode12hInput->inputCallback = [&](Item &item, const InputEvent &event) {
                if (event.state != gui::InputEvent::State::keyReleasedShort) {
                    return false;
                }
                if (event.keyCode == gui::KeyCode::KEY_LF) {
                    if (mode12hInput->getText() == timeConstants::before_noon) {
                        mode12hInput->setText(timeConstants::after_noon);
                    }
                    else {
                        mode12hInput->setText(timeConstants::before_noon);
                    }
                    return true;
                }
                return false;
            };
            mode12hInput->focusChangedCallback = [&](Item &item) {
                if (item.focus) {
                    bottomBarTemporaryMode(utils::localize.get("common_switch"));
                }
                else {
                    bottomBarRestoreFromTemporaryMode();
                }
                return true;
            };

            mode12hInput->setMinimumSize(style::window::calendar::item::eventTime::time_input_12h_w,
                                         style::window::calendar::item::eventTime::height -
                                             style::window::calendar::item::eventTime::separator);
            mode12hInput->setMargins(gui::Margins(style::window::calendar::item::eventTime::separator, 0, 0, 0));
            hourInput->setMinimumSize(style::window::calendar::item::eventTime::time_input_12h_w,
                                      style::window::calendar::item::eventTime::height -
                                          style::window::calendar::item::eventTime::separator);
            minuteInput->setMinimumSize(style::window::calendar::item::eventTime::time_input_12h_w,
                                        style::window::calendar::item::eventTime::height -
                                            style::window::calendar::item::eventTime::separator);

            onLoadCallback = [&](std::shared_ptr<EventsRecord> event) {
                if (this->descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_start")) {
                    hourInput->setText(std::to_string(event->date_from % 10000 / 100));
                    minuteInput->setText(std::to_string(event->date_from % 100));
                }
                else if (this->descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_end")) {
                    hourInput->setText(std::to_string(event->date_till % 10000 / 100));
                    minuteInput->setText(std::to_string(event->date_till % 100));
                }
            };
        }
        else {
            hourInput->setMinimumSize(style::window::calendar::item::eventTime::time_input_24h_w,
                                      style::window::calendar::item::eventTime::height -
                                          style::window::calendar::item::eventTime::separator);
            minuteInput->setMinimumSize(style::window::calendar::item::eventTime::time_input_24h_w,
                                        style::window::calendar::item::eventTime::height -
                                            style::window::calendar::item::eventTime::separator);

            onLoadCallback = [&](std::shared_ptr<EventsRecord> event) {
                if (this->descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_start")) {
                    hourInput->setText(std::to_string(event->date_from % 10000 / 100));
                    minuteInput->setText(std::to_string(event->date_from % 100));
                }
                else if (this->descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_end")) {
                    hourInput->setText(std::to_string(event->date_till % 10000 / 100));
                    minuteInput->setText(std::to_string(event->date_till % 100));
                }
            };
        }
    }

    bool EventTimeItem::onDimensionChanged(const BoundingBox &oldDim, const BoundingBox &newDim)
    {
        vBox->setPosition(0, 0);
        vBox->setSize(newDim.w, newDim.h);
        return true;
    }

    void EventTimeItem::setConnectionToSecondItem(gui::EventTimeItem *item)
    {
        this->secondItem = item;
    }

    int EventTimeItem::convertTimeTo24hMode(int mode12h, const std::string &type)
    {
        if (type == timeConstants::before_noon) {
            if (mode12h != timeConstants::max_hour_12H_mode) {
                return mode12h;
            }
            else {
                return 0;
            }
        }
        else {
            if (mode12h != timeConstants::max_hour_12H_mode) {
                return mode12h + timeConstants::max_hour_12H_mode;
            }
            else {
                return mode12h;
            }
        }
    }

    void EventTimeItem::validateHour()
    {
        if (descriptionLabel->getText() == utils::localize.get("app_calendar_new_edit_event_end")) {
            if (!mode24H) {
                validateHourFor12hMode();
            }
            else {
                validateHourFor24hMode();
            }
        }
    }

    void EventTimeItem::validateHourFor12hMode()
    {
        auto start_hour =
            convertTimeTo24hMode(atoi(secondItem->hourInput->getText().c_str()), secondItem->mode12hInput->getText());
        auto end_hour = convertTimeTo24hMode(atoi(hourInput->getText().c_str()), mode12hInput->getText());
        if (start_hour > end_hour || (start_hour == end_hour && atoi(secondItem->minuteInput->getText().c_str()) >
                                                                    atoi(minuteInput->getText().c_str()))) {
            auto hour = atoi(secondItem->hourInput->getText().c_str()) + 1;
            if (secondItem->mode12hInput->getText() == timeConstants::after_noon) {
                if (hour == 12) {
                    hour = 11;
                }
                mode12hInput->setText(mode12hInput->getText());
                minuteInput->setText(std::to_string(timeConstants::max_minutes));
            }
            else {
                if (hour == 12) {
                    mode12hInput->setText(timeConstants::after_noon);
                }
                minuteInput->setText(minuteInput->getText());
            }
            if (hour > timeConstants::max_hour_12H_mode) {
                hour = 1;
                mode12hInput->setText(secondItem->mode12hInput->getText());
                minuteInput->setText(secondItem->minuteInput->getText());
            }
            hourInput->setText(std::to_string(hour));
        }
    }

    void EventTimeItem::validateHourFor24hMode()
    {
        auto start_hour = atoi(secondItem->hourInput->getText().c_str());
        auto end_hour   = atoi(hourInput->getText().c_str());
        if (start_hour > end_hour || (start_hour == end_hour && atoi(secondItem->minuteInput->getText().c_str()) >
                                                                    atoi(minuteInput->getText().c_str()))) {
            auto hour = atoi(secondItem->hourInput->getText().c_str()) + 1;
            if (hour > timeConstants::max_hour_24H_mode) {
                hour = timeConstants::max_hour_24H_mode;
                minuteInput->setText(std::to_string(timeConstants::max_minutes));
            }
            else {
                minuteInput->setText(secondItem->minuteInput->getText());
            }
            hourInput->setText(std::to_string(hour));
        }
    }

} /* namespace gui */
