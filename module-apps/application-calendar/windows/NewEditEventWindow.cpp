#include <module-db/Interface/EventsRecord.hpp>
#include <module-services/service-db/api/DBServiceAPI.hpp>
#include <module-db/queries/calendar/QueryEventsAdd.hpp>
#include "NewEditEventWindow.hpp"

namespace gui
{

    NewEditEventWindow::NewEditEventWindow(app::Application *app, std::string name)
        : AppWindow(app, style::window::calendar::name::new_edit_event),
          newEditEventModel{std::make_shared<NewEditEventModel>(this->application)}
    {
        buildInterface();
    }

    void NewEditEventWindow::rebuild()
    {
        erase();
        buildInterface();
    }

    void NewEditEventWindow::buildInterface()
    {
        AppWindow::buildInterface();

        topBar->setActive(gui::TopBar::Elements::TIME, true);
        bottomBar->setActive(gui::BottomBar::Side::RIGHT, true);
        bottomBar->setActive(gui::BottomBar::Side::CENTER, true);
        bottomBar->setText(gui::BottomBar::Side::RIGHT, utils::localize.get(style::strings::common::back));
        bottomBar->setText(gui::BottomBar::Side::CENTER, utils::localize.get(style::strings::common::save));

        list = new gui::ListView(this,
                                 style::window::calendar::listView_x,
                                 style::window::calendar::listView_y,
                                 style::window::calendar::listView_w,
                                 style::window::calendar::listView_h,
                                 newEditEventModel);
        setFocusItem(list);
    }

    void NewEditEventWindow::onBeforeShow(gui::ShowMode mode, gui::SwitchData *data)
    {
        switch (eventAction) {
        case EventAction::None:
            break;
        case EventAction::Add: {
            setTitle(utils::localize.get("app_calendar_new_event_title"));
            break;
        }
        case EventAction::Edit:
            setTitle(utils::localize.get("app_calendar_edit_event_title"));
            break;
        }
        if (mode == ShowMode::GUI_SHOW_INIT) {
            list->rebuildList();
            newEditEventModel->loadData(eventRecord);
        }
    }

    bool NewEditEventWindow::onInput(const gui::InputEvent &inputEvent)
    {
        if (AppWindow::onInput(inputEvent)) {
            return true;
        }

        if (!inputEvent.isShortPress()) {
            return false;
        }

        if (inputEvent.keyCode == gui::KeyCode::KEY_ENTER) {
            LOG_DEBUG("Save Event");
            bool edit;
            if (eventAction == EventAction::Edit) {
                edit = true;
            }
            else if (eventAction == EventAction::Add) {
                edit = false;
            }

            newEditEventModel->saveData(eventRecord, edit, prevWindowName);
            return true;
        }

        return false;
    }

    bool NewEditEventWindow::handleSwitchData(gui::SwitchData *data)
    {
        if (data == nullptr) {
            return false;
        }
        auto rec = dynamic_cast<EventRecordData *>(data);
        if (rec != nullptr) {

            eventRecord = rec->getData();
            prevWindowName = rec->getWindowName();
        }

        if (data->getDescription() == "Edit") {
            eventAction = EventAction::Edit;
        }
        else if (data->getDescription() == "New") {
            eventAction = EventAction::Add;
        }
        return true;
    }
} /* namespace gui */
