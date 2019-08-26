/*
 * ListItem.h
 *
 *  Created on: 11 mar 2019
 *      Author: robert
 */

#ifndef MIDDLEWARES_GUI_WIDGETS_LISTITEM_HPP_
#define MIDDLEWARES_GUI_WIDGETS_LISTITEM_HPP_

#include "Rect.hpp"

namespace gui {

class ListItem: public Rect {
public:
	//TODO move this to protected area and provide getters and setters
	int minHeight = 0;
	int minWidth = 0;
	int maxHeight = 0xFFFF;
	int maxWidth = 0xFFFF;

	ListItem();
	virtual ~ListItem();
};

} /* namespace gui */

#endif /* MIDDLEWARES_GUI_WIDGETS_LISTITEM_HPP_ */
