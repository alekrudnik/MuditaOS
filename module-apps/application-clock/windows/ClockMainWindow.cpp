/*
 * @file ClockMainWindow.cpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 6 cze 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */

//module-gui
#include <module-apps/application-clock/windows/ClockMainWindow.hpp>
#include "gui/widgets/Window.hpp"
#include "gui/widgets/Item.hpp"
#include "gui/widgets/Image.hpp"
#include "gui/widgets/Label.hpp"
#include "gui/widgets/BoxLayout.hpp"
#include "gui/core/ImageManager.hpp"
#include "../ClockData.hpp"


namespace gui {

ClockMainWindow::ClockMainWindow() : gui::Window("Main") {

	setSize( 480, 600 );

	uint32_t xOffset = 0;
	uint32_t yOffset = 0;

	hourLabel = new gui::Label(this, 100+xOffset,300-160+yOffset,280,150);
	hourLabel->setFilled( false );
	hourLabel->setBorderColor( gui::ColorNoColor );
	hourLabel->setFont("gt_pressura_regular_140");
	hourLabel->setText("00");
	hourLabel->setAlignement( gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER));

	minuteLabel = new gui::Label(this, 100+xOffset,310+yOffset,280,150);
	minuteLabel->setFilled( false );
	minuteLabel->setBorderColor( gui::ColorFullBlack );
	minuteLabel->setFont("gt_pressura_regular_140");
	minuteLabel->setText("00");
	minuteLabel->setRadius( 20 );
	minuteLabel->setPenWidth(1);
	minuteLabel->setFillColor( gui::Color(10,0));
	minuteLabel->setAlignement( gui::Alignment(gui::Alignment::ALIGN_HORIZONTAL_CENTER, gui::Alignment::ALIGN_VERTICAL_CENTER));

	progressBar = new gui::Progress(this, 480/2-90+xOffset, 300-6+yOffset, 180, 12 );
	progressBar->setTotalProgress(59);
	progressBar->setCurrentProgress(0);
}

ClockMainWindow::~ClockMainWindow() {
	// TODO Auto-generated destructor stub
}

bool ClockMainWindow::handleSwitchData( SwitchData* data ) {
	ClockData* clockData = reinterpret_cast<ClockData*>( data );

	hour = clockData->hour % 24;
	minute = clockData->minute % 60;

	return true;
}

void ClockMainWindow::updateLabels() {
	std::string h;
	if( hour<10 ) {
		h += "0";
	}
	h += std::to_string(hour);
	hourLabel->setText( h );

	std::string m;
	if( minute < 10 ) {
		m+= "0";
	}
	m += std::to_string(minute);
	minuteLabel->setText( m );
}

bool ClockMainWindow::incrementHour() {
	hour++;
	if( hour > 23 ) {
		hour = 0;
		return true;
	}
	return false;
}
bool ClockMainWindow::incrementMinute() {
	minute++;
	if( minute > 59 ) {
		incrementHour();
		minute = 0;
		return true;
	}
	return false;
}

bool ClockMainWindow::incrementSecond(){
	bool ret = false;
	seconds++;
	if( seconds > 59 ) {
		incrementMinute();
		seconds = 0;
		ret = true;
	}
	progressBar->setCurrentProgress(seconds);

	return ret;
}

} /* namespace gui */
