#pragma once

#include "../Common.hpp"
#include "module-utils/log/log.hpp"

namespace gui
{

    class Item;

    // Class holds GUI Items for 4 possible directions for navigating using keyboard.
    class Navigation
    {
      protected:
        Item *left  = nullptr;
        Item *up    = nullptr;
        Item *right = nullptr;
        Item *down  = nullptr;

      public:
        /// Sets pointer to the widget that should receive focus after receiving navigation event.
        void setDirectionItem(NavigationDirection direction, Item *item);
        /// Retrives item from specified durection
        Item *getDirectionItem(const NavigationDirection direction) const;
        /// Clears Item's pointer for specified direction.
        void clearDirection(const NavigationDirection direction);

        virtual ~Navigation();
    };

} // namespace gui
