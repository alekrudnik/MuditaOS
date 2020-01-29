/*
 * Item.cpp
 *
 *  Created on: 4 mar 2019
 *      Author: robert
 */

#include "Item.hpp"
#include "BoundingBox.hpp"
#include "Margins.hpp"
#include "Navigation.hpp"
#include <algorithm>
#include <cstring>
#include <list>

namespace gui {

Item::Item() :
	focus {false},
	type {ItemType::ITEM},
	parent{nullptr},
	radius{0},
	visible{true},
	verticalPolicy{ LayoutVerticalPolicy::LAYOUT_POLICY_VERTICAL_EXPAND },
	horizontalPolicy { LayoutHorizontalPolicy::LAYOUT_POLICY_HORIZONTAL_EXPAND },
	minHeight { 0 },
	minWidth { 0 },
	maxHeight { 0xFFFF },
	maxWidth { 0xFFFF },
	navigationDirections{ nullptr } {

	focusChangedCallback = [](Item&){ return false;};
	activatedCallback = [](Item&){ return false;};
	inputCallback = [](Item&, const InputEvent& inputEvent){ return false;};
	dimensionChangedCallback = [](Item&,  void* data){ return false;};
	contentCallback = [=](Item&){ if( parent != nullptr ) return parent->onContent(); return false;};
}

Item::~Item() {
	for( auto& widget : children )
		delete widget;

	if( navigationDirections )
		delete navigationDirections;
}

bool Item::addWidget( Item* item ) {
    if( item == nullptr ) return false;
	if( item->parent ) {
		item->parent->removeWidget(item);
	}

	item->parent = this;
	children.push_back(item);

	item->updateDrawArea();
	return true;
}

bool Item::removeWidget( Item* item ) {
	if(item == nullptr) return false;
	
	auto fi = std::find( children.begin(), children.end(), item );
	if( fi != children.end() ) {
		children.erase(fi);
		return true;
	}
	return false;
}

void Item::setVisible( bool value ) {
	visible = value;
}

std::list<DrawCommand*> Item::buildDrawList() {

	std::list<DrawCommand*> commands;

	for( auto& widget : children ) {
		std::list<DrawCommand*> widgetCommands = widget->buildDrawList();
		if( !widgetCommands.empty() )
			commands.insert( commands.end(), widgetCommands.begin(), widgetCommands.end() );
	}

	return commands;
}

void Item::setPosition( const short& x, const short& y ) {

	BoundingBox oldArea = widgetArea;
	widgetArea.x = x;
	widgetArea.y = y;
	updateDrawArea();

	onDimensionChanged(oldArea, widgetArea);
}

void Item::setX(const uint32_t x)
{
	BoundingBox oldArea = widgetArea;
	widgetArea.x = x;
	updateDrawArea();

	onDimensionChanged(oldArea, widgetArea);
}

void Item::setY(const uint32_t y)
{
	BoundingBox oldArea = widgetArea;
	widgetArea.y = y;
	updateDrawArea();

	onDimensionChanged(oldArea, widgetArea);
}

void Item::setSize( const short& w, const short& h ) {

	BoundingBox oldArea = widgetArea;
	widgetArea.w = w;
	if( widgetArea.w < 0 )
		widgetArea.w = 0;
	widgetArea.h = h;
	if( widgetArea.h < 0 )
		widgetArea.h = 0;
    if (w > maxWidth)
    {
        maxWidth = w;
    }
    if (h > maxHeight)
    {
        maxHeight = h;
    }
    updateDrawArea();
    onDimensionChanged(oldArea, widgetArea);
}

void Item::setBoundingBox(const BoundingBox &new_box)
{
    BoundingBox oldArea = widgetArea;
    widgetArea = new_box;
    updateDrawArea();
    onDimensionChanged(oldArea, widgetArea);
}

void Item::setRadius( int value ) {
	if( value < 0 )
		value = 0;
	radius = value;
}

void Item::updateDrawArea() {
	Item* parentItem = parent;

	BoundingBox result = widgetArea;

	//iterate up to top widget in hierarchy
	while( parentItem != nullptr ) {

		result.x += parentItem->widgetArea.x;
		result.y += parentItem->widgetArea.y;
		BoundingBox newResult;
		if( BoundingBox::intersect(parentItem->widgetArea, result, newResult) == false ) {
			result.clear();
			break;
		}
		result = newResult;

		parentItem = parentItem->parent;
	}

	drawArea = result;

	for( gui::Item* it : children )
		it->updateDrawArea();
}

void Item::updateInnerArea(){
    BoundingBox result = drawArea;
    if (innerMargins.left){
        result.x += innerMargins.left;
        result.w -= innerMargins.left;
    }
    if (innerMargins.right){
        result.w -= innerMargins.right;
    }
    if (innerMargins.top){
        result.y += innerMargins.top;
        result.h -= innerMargins.top;
    }
    if (innerMargins.bottom){
        result.h -= innerMargins.bottom;
    }

    innerArea = result;

    for( gui::Item* it : children )
        it->updateInnerArea();
}

Item* Item::getNavigationItem( NavigationDirection direction ) {
	if( navigationDirections != nullptr ) {
		return navigationDirections->getDirectionItem(direction);
	}
	return nullptr;
}

void Item::setNavigationItem( gui::NavigationDirection direction, Item* item ) {
	if( navigationDirections == nullptr )
		navigationDirections = new Navigation();
	navigationDirections->setDirectionItem(direction, item);
}

uint32_t Item::getMinHeight() {
	return minHeight;
}

uint32_t Item::getMinWidth() {
	return minWidth;
}

uint32_t Item::getMaxHeight() {
	return maxHeight;
}

uint32_t Item::getMaxWidth() {
	return maxWidth;
}

void Item::setMaxSize( const uint16_t& w, const uint16_t& h) {
	maxWidth = w;
	maxHeight = h;
}
void Item::setMinSize( const uint16_t& w, const uint16_t& h) {
	minWidth = w;
	minHeight = h;
}

bool Item::handleNavigation(const InputEvent inputEvent)
{
    gui::Item *newFocusItem = nullptr;
    if ((focusItem != nullptr) && (inputEvent.state == InputEvent::State::keyReleasedShort))
    {
        if (itemNavigation && itemNavigation(inputEvent))
        {
            return true;
        }
        switch (inputEvent.keyCode)
        {
        case gui::KeyCode::KEY_LEFT:
            newFocusItem = focusItem->getNavigationItem(gui::NavigationDirection::LEFT);
            break;
        case gui::KeyCode::KEY_RIGHT:
            newFocusItem = focusItem->getNavigationItem(gui::NavigationDirection::RIGHT);
            break;
        case gui::KeyCode::KEY_UP:
            newFocusItem = focusItem->getNavigationItem(gui::NavigationDirection::UP);
            break;
        case gui::KeyCode::KEY_DOWN:
            newFocusItem = focusItem->getNavigationItem(gui::NavigationDirection::DOWN);
            break;
        case gui::KeyCode::KEY_ENTER:
            if (focusItem != nullptr)
                return focusItem->onActivated(nullptr);
            break;
        default:
            break;
        }
    }
    if (newFocusItem != nullptr)
    {
        setFocusItem(newFocusItem);
        return true;
    }
    return false;
}

} /* namespace gui */
