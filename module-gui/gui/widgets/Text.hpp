/*
 * @file Text.hpp
 * @author Robert Borzecki (robert.borzecki@mudita.com)
 * @date 1 sie 2019
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */
#ifndef MODULE_GUI_GUI_WIDGETS_TEXT_HPP_
#define MODULE_GUI_GUI_WIDGETS_TEXT_HPP_

#include <list>

#include "utf8/UTF8.hpp"

#include "BoxLayout.hpp"
#include "Label.hpp"
#include "Rect.hpp"

namespace gui {

/*
 * @brief Widget that holds multiple lines of text. This widget can expand vertically if needed to hold lines of text.
 */
class Text: public Rect{
public:
	enum class LineEndType {
		EOT,     //end of source text
		BREAK,   //line had enter defined as a /r
		CONTINUE, //line was broken due to not enough space to hold all characters
		CONTINUE_SPACE //line was broken on the space character because next word doesn't fit current line.
	};

	/**
	 * Types of expanding the text field
	 */
	enum class ExpandMode {
		EXPAND_UP,
		EXPAND_DOWN,
		EXPAND_NONE //defult
	};

	/**
	 * Modes of work of the text editor
	 */
	enum class EditMode	{
		BROWSE,
		EDIT
	};

	/**
	 * @brief Type of the text field
	 */
	enum class TextType{
		SINGLE_LINE = 1,
		MULTI_LINE
	};

	enum class MoveDirection {
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT
	};

protected:
	class TextLine {
	public:
		UTF8 text;
		uint32_t startIndex = 0;
		uint32_t endIndex = 0;
		LineEndType endType = LineEndType::EOT;
		uint32_t pixelLength = 0;

		TextLine( const UTF8& text, uint32_t startIndex, uint32_t endIndex, LineEndType endType, uint32_t pixelLength );
	};

	//holds list of all lines that  text was divided to.
	std::list<TextLine*> textLines;
	//holds list of labels for displaying currently visible text lines.
	std::list<gui::Label*> labelLines;
	//pointer to the first visible line of text
	std::list<TextLine*>::iterator firstLine = textLines.end();
	//pointer to the last visible line.
	std::list<TextLine*>::iterator lastLine = textLines.end();
	//row where cursor is located ( 0 - first row from top )
	uint32_t cursorRow = 0;
	// column where cursor is located( 0 - position before first character in the line )
	uint32_t cursotColumn = 0;



	EditMode editMode = EditMode::EDIT;
	ExpandMode expandMode = ExpandMode::EXPAND_NONE;
	TextType textType = TextType::MULTI_LINE;
	//maximum number of lines until which widget will expand its size;
	uint32_t maxExpansionLines = 0;
	//width of the cursor
	uint32_t cursorWidth = 2;
	//font pointer
	Font* font;
	//index of the first visible row of the text
	uint32_t firstRow = 0;
	//internal copy of the text that is displayed

	void splitTextToLines( const UTF8& text);
	/**
	 * Iterate over lines starting from the one that is provided. Function concatenate lines and performs new split.
	 * Function stops on the last line or it there is a lines break ( enter ) or if last concatenated line doesn;t change after update.
	 */

	void reworkLines( TextLine* textLine );
	/**
	 * Moves cursor in specified direction
	 */
	bool moveCursor( const MoveDirection& direction );
	/**
	 * Function is used to move text up and down in the browsing mode
	 */
	bool handleBrowsing( const InputEvent& inputEvent );
	/**
	 * Function is used to move cursor in all directions when in the Wdit mode.
	 */
	bool handleNavigation( const InputEvent& inputEvent );
	/**
	 * Handles enter key
	 */
	bool handleEnter();
	bool handleBackspace();
	/**
	 * Inserts character provided from external source into currently selected line.
	 */
	bool handleChar( const InputEvent& inputEvent );
	/**
	 * Updates cursor position in the text widget
	 */
	void updateCursor();
	/**
	 * Returns text line where cursor is located
	 */
	TextLine* getCursorTextLine();

public:
	Text();
	Text( Item* parent, const uint32_t& x, const uint32_t& y, const uint32_t& w, const uint32_t& h,
			const UTF8& text = "", ExpandMode expandMode = ExpandMode::EXPAND_NONE, TextType textType = TextType::MULTI_LINE );
	virtual ~Text();

	void setEditMode( EditMode mode );
	void setCursorWidth( uint32_t w );
	virtual void setText( const UTF8& text );
	virtual void clear();
	virtual UTF8 getText();
	void setFont( const UTF8& fontName );

	//virtual methods from Item
	std::list<DrawCommand*> buildDrawList() override;
	void setPosition( const short& x, const short& y ) override;
	void setSize( const short& w, const short& h ) override;
	bool onInput( const InputEvent& inputEvent ) override;
	bool onActivated( void* data ) override ;
	bool onDimensionChanged( const BoundingBox& oldDim, const BoundingBox& newDim) override;
};

} /* namespace gui */

#endif /* MODULE_GUI_GUI_WIDGETS_TEXT_HPP_ */
