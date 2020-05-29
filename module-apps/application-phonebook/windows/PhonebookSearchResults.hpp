#pragma once

#include "../data/PhonebookItemData.hpp"
#include "../models/SearchResultsModel.hpp"
#include "../widgets/PhonebookListView.hpp"

namespace gui
{
    class PhonebookSearchResults : public AppWindow
    {
      protected:
        SearchResultsModel *searchResultsModel = nullptr;
        PhonebookListView *searchResultList    = nullptr;

        Image *newContactImage = nullptr;

      public:
        PhonebookSearchResults(app::Application *app);
        virtual ~PhonebookSearchResults();

        // virtual methods
        bool onInput(const InputEvent &inputEvent) override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        bool handleSwitchData(SwitchData *data) override;
        void rebuild() override;
        void buildInterface() override;
        void destroyInterface() override;
    };

} /* namespace gui */
