/*
 * BottomBar.cpp
 *
 *  Created on: 13 mar 2019
 *      Author: robert
 */
#include "Label.hpp"
#include "BottomBar.hpp"
#include "Margins.hpp"
#include "utf8/UTF8.hpp"

namespace gui {

BottomBar::BottomBar() {

	Margins margins {20,0,20,0};
	left = prepareLabel( Side::LEFT );
	center = prepareLabel( Side::CENTER );
	right = prepareLabel( Side::RIGHT );

	left->setMargins(margins);
	center->setMargins(margins);
	right->setMargins(margins);


	addWidget(left);
	addWidget(center);
	addWidget(right);

	setFillColor( ColorFullWhite );
	setBorderColor( ColorNoColor );
	setFilled(true);
	setSize(480, 50);
}
BottomBar::BottomBar( Item* parent, uint32_t x, uint32_t y, uint32_t w, uint32_t h ) : Rect{ parent, x, y, w, h } {

	Margins margins {20,0,20,0};
	left = prepareLabel( Side::LEFT );
	center = prepareLabel( Side::CENTER );
	right = prepareLabel( Side::RIGHT );

	left->setMargins(margins);
	center->setMargins(margins);
	right->setMargins(margins);

	addWidget(left);
	addWidget(center);
	addWidget(right);

	setFillColor( ColorFullWhite );
	setBorderColor( ColorNoColor );
	setFilled(true);
	setSize(480, 50);
	updateDrawArea();
}
BottomBar::~BottomBar() {
}

gui::Label* BottomBar::prepareLabel( BottomBar::Side side ) {
	Label* label = new Label( this, 0,0,0,0 );
	label->setBorderColor( Color{15,15} );
	switch( side ) {
		case Side::LEFT:
			label->setAlignement( gui::Alignment(	gui::Alignment::ALIGN_HORIZONTAL_LEFT, gui::Alignment::ALIGN_VERTICAL_CENTER ));
			label->setFont("gt_pressura_regular_18");
			break;
		case Side::CENTER:
			label->setAlignement( gui::Alignment(	gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER ));
			label->setFont("gt_pressura_bold_18");
			break;
		case Side::RIGHT:
			label->setAlignement( gui::Alignment(	gui::Alignment::ALIGN_HORIZONTAL_RIGHT, gui::Alignment::ALIGN_VERTICAL_CENTER ));
			label->setFont("gt_pressura_regular_18");
			break;
	};

	label->setFilled(false);

	return label;
}

void BottomBar::setActive( BottomBar::Side side, bool active ) {
	switch( side ) {
		case Side::LEFT:
			left->setVisible(active);
			break;
		case Side::CENTER:
			center->setVisible(active);
			break;
		case Side::RIGHT:
			right->setVisible(active);
			break;
	};
}
void BottomBar::setText( BottomBar::Side side, const UTF8& str ) {
	switch( side ) {
		case Side::LEFT:
			left->setText(str);
			break;
		case Side::CENTER:
			center->setText(str);
			break;
		case Side::RIGHT:
			right->setText(str);
			break;
	};
}

bool BottomBar::onDimensionChanged( const BoundingBox& oldDim, const BoundingBox& newDim) {
	Rect::onDimensionChanged(oldDim, newDim);
	left->setSize(newDim.w, newDim.h );
	center->setSize(newDim.w, newDim.h );
	right->setSize(newDim.w, newDim.h );
	return true;
}

} /* namespace gui */
