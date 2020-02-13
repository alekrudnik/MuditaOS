/*
 * BoxLayout.hpp
 *
 *  Created on: 16 maj 2019
 *      Author: robert
 */

#ifndef GUI_WIDGETS_BOXLAYOUT_HPP_
#define GUI_WIDGETS_BOXLAYOUT_HPP_

#include <cstdint>

#include "Layout.hpp"
#include "Rect.hpp"

namespace gui {

    class BoxLayout : public Rect
    {
      protected:
        struct BoxElement
        {
            BoxElement(Item *item) : item{item}, noUpdate{false} {};
            Item *item;
            bool noUpdate;
        };

        template <Axis axis> uint16_t &size(Item *it)
        {
            return it->widgetArea.size(axis);
        };
        template <Axis axis> uint32_t sizeUsed(Item *it)
        {
            uint32_t sum = 0;
            std::for_each(it->children.begin(), it->children.end(), [&](auto &el) { sum += el->widgetArea.size(axis); });
            return sum;
        };
        template <Axis axis> uint32_t sizeLeft(Item *it)
        {
            return size<axis>(it) - sizeUsed<axis>(it);
        };
        template <Axis axis> uint32_t inAxisMax(Item *it)
        {
            return (axis == Axis::Y) ? (it->getMaxHeight()) : (it->getMaxWidth());
        };

        template <Axis axis> void resizeItems();

        virtual void resizeItems();
        bool reverse_order = false;
        /// get next navigation item including `from` item, ecludes not visible items and not acvite items
        std::list<Item *>::iterator nextNavigationItem(std::list<Item *>::iterator from);

      public:
        BoxLayout();
        BoxLayout(Item *parent, const uint32_t &x, const uint32_t &y, const uint32_t &w, const uint32_t &h);
        virtual bool onFocus(bool state) override;
        virtual ~BoxLayout() = default;

        // virtual methods from Item
        void setPosition(const short &x, const short &y) override;
        void setSize(const short &w, const short &h) override;
        bool addWidget(gui::Item *item) override;
        bool removeWidget(Item *item) override;
        std::list<DrawCommand *> buildDrawList() override;
        /// add item if it will fit in box, return true on success
        /// axis sets direction to define space left in container
        template <Axis axis> bool addWidget(Item *item);
        /// set navigation from last to fist element in box
        void setNavigation();
        void setVisible(bool value) override;
        /// set visible but from previous scope... (page, element etc)
        void setVisible(bool value, bool previous);
        void setReverseOrder(bool reverse_order)
        {
            this->reverse_order = reverse_order;
            resizeItems();
        }
        bool getReverseOrder() const;
        /// callback for situaton when we reached top/bottom/left/right of box
        /// if we want to do sth special (i.e. request new items)
        std::function<bool(const InputEvent &inputEvent)> borderCallback = nullptr;
    };

class HBox : public BoxLayout {
public:
  void resizeItems() override;
  HBox();
  HBox(Item *parent, const uint32_t &x, const uint32_t &y, const uint32_t &w, const uint32_t &h);
  virtual ~HBox() = default;
  virtual bool addWidget(Item *item) override;
};

class VBox : public BoxLayout {
public:
	void resizeItems() override;
	VBox();
	VBox( Item* parent, const uint32_t& x, const uint32_t& y, const uint32_t& w, const uint32_t& h);
	virtual ~VBox() = default;
    virtual bool addWidget(Item *item) override;
};

} /* namespace gui */

#endif /* GUI_WIDGETS_BOXLAYOUT_HPP_ */
