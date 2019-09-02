#pragma once

#include <memory>
#include <string>

#include "AppWindow.hpp"
#include "ContactInputBox.hpp"
#include <widgets/BoxLayout.hpp>

#include "../ContactBuilder.hpp"
#include "PhonebookData.hpp"

namespace gui {

class PhonebookNewContact : public AppWindow {
  protected:
    static const int side_margin;
    gui::ContactInputBox *box1, *box2;
    std::unique_ptr<PhonebookContactData> data;

  public:
    PhonebookNewContact(app::Application *app);
    virtual ~PhonebookNewContact();

    // virtual methods
    bool onInput(const InputEvent &inputEvent) override;
    void onBeforeShow(ShowMode mode, uint32_t command, SwitchData *data) override;
    virtual bool handleSwitchData(SwitchData *data) override;

    void rebuild() override;
    void buildInterface() override;
    void destroyInterface() override;

  private:
    void addContact(ContactBuilder &&contact);
    void removeContact();
    void obligatoryData();
    void dataTaken(UTF8 &&text);
};

} /* namespace gui */
