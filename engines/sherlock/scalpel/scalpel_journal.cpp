/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sherlock/journal.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_journal.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/tattoo/tattoo_journal.h"

namespace Sherlock {

namespace Scalpel {

#define JOURNAL_BUTTONS_Y_INTL 178
#define JOURNAL_BUTTONS_Y_ZH 181
#define JOURNAL_SEARCH_LEFT 15
#define JOURNAL_SEARCH_TOP_INTL 186
#define JOURNAL_SEARCH_TOP_ZH 184
#define JOURNAL_SEARCH_RIGHT 296
#define JOURNAL_SEACRH_MAX_CHARS 50

// Positioning of buttons in the journal view
static const int JOURNAL_POINTS_INTL[9][3] = {
	{ 6, 68, 37 },
	{ 69, 131, 100 },
	{ 132, 192, 162 },
	{ 193, 250, 221 },
	{ 251, 313, 281 },
	{ 6, 82, 44 },
	{ 83, 159, 121 },
	{ 160, 236, 198 },
	{ 237, 313, 275 }
};

static const int JOURNAL_POINTS_ZH[9][3] = {
	{ 0, 52, 26 },
	{ 52, 121, 87 },
	{ 122, 157, 140 },
	{ 157, 194, 176 },
	{ 194, 265, 230 },
	{ 265, 320, 293 },
	{ 270, 320, 295 },
	{ 270, 320, 295 },
	{ 0, 0, 0 }
};

static const int SEARCH_POINTS_INTL[3][3] = {
	{ 51, 123, 86 },
	{ 124, 196, 159 },
	{ 197, 269, 232 }
};

static const int SEARCH_POINTS_ZH[3][3] = {
	{ 206, 243, 225 },
	{ 243, 279, 261 },
	{ 279, 315, 297 }
};

/*----------------------------------------------------------------*/

ScalpelJournal::ScalpelJournal(SherlockEngine *vm) : Journal(vm) {
	if (_vm->_interactiveFl) {
		// Load the journal directory and location names
		loadLocations();
	}

	_fixedTextWatsonsJournal = FIXED(Journal_WatsonsJournal);
	_fixedTextExit = FIXED(Journal_Exit);
	_fixedTextBack10 = FIXED(Journal_Back10);
	_fixedTextUp = FIXED(Journal_Up);
	_fixedTextDown = FIXED(Journal_Down);
	_fixedTextAhead10 = FIXED(Journal_Ahead10);
	_fixedTextSearch = FIXED(Journal_Search);
	_fixedTextFirstPage = FIXED(Journal_FirstPage);
	_fixedTextLastPage = FIXED(Journal_LastPage);
	_fixedTextPrintText = FIXED(Journal_PrintText);

	_hotkeyExit = toupper(_fixedTextExit[0]);
	_hotkeyBack10 = toupper(_fixedTextBack10[0]);
	_hotkeyUp = toupper(_fixedTextUp[0]);
	_hotkeyDown = toupper(_fixedTextDown[0]);
	_hotkeyAhead10 = toupper(_fixedTextAhead10[0]);
	_hotkeySearch = toupper(_fixedTextSearch[0]);
	_hotkeyFirstPage = toupper(_fixedTextFirstPage[0]);
	_hotkeyLastPage = toupper(_fixedTextLastPage[0]);
	_hotkeyPrintText = toupper(_fixedTextPrintText[0]);

	_fixedTextSearchExit = FIXED(JournalSearch_Exit);
	_fixedTextSearchBackward = FIXED(JournalSearch_Backward);
	_fixedTextSearchForward = FIXED(JournalSearch_Forward);
	_fixedTextSearchNotFound = FIXED(JournalSearch_NotFound);

	_hotkeySearchExit = toupper(_fixedTextSearchExit[0]);
	_hotkeySearchBackward = toupper(_fixedTextSearchBackward[0]);
	_hotkeySearchForward = toupper(_fixedTextSearchForward[0]);
}

Common::Rect ScalpelJournal::getButtonRect(JournalButton btn) {
	int idx = btn - 1;
	if (_vm->getLanguage() == Common::ZH_TWN) {
		if (btn >= BTN_FIRST_PAGE) {
			return Common::Rect(JOURNAL_POINTS_ZH[idx][0], JOURNAL_BUTTONS_Y_ZH - (btn - BTN_FIRST_PAGE + 1) * 19,
				    JOURNAL_POINTS_ZH[idx][1], JOURNAL_BUTTONS_Y_ZH + 19 - (btn - BTN_FIRST_PAGE + 1) * 19);
		} else
			return Common::Rect(JOURNAL_POINTS_ZH[idx][0], JOURNAL_BUTTONS_Y_ZH,
					    JOURNAL_POINTS_ZH[idx][1], JOURNAL_BUTTONS_Y_ZH + 19);
	} else {
		if (btn >= BTN_SEARCH)
			return Common::Rect(JOURNAL_POINTS_INTL[idx][0], JOURNAL_BUTTONS_Y_INTL + 11,
					    JOURNAL_POINTS_INTL[idx][1], JOURNAL_BUTTONS_Y_INTL + 21);
		else
			return Common::Rect(JOURNAL_POINTS_INTL[idx][0], JOURNAL_BUTTONS_Y_INTL,
					    JOURNAL_POINTS_INTL[idx][1], JOURNAL_BUTTONS_Y_INTL + 10);
	}
}

Common::Point ScalpelJournal::getButtonTextPoint(JournalButton btn) {
	int idx = btn - 1;
	if (_vm->getLanguage() == Common::ZH_TWN) {
		if (btn >= BTN_FIRST_PAGE)
			return Common::Point(JOURNAL_POINTS_ZH[idx][2], JOURNAL_BUTTONS_Y_ZH + 2 - (btn - BTN_FIRST_PAGE + 1) * 19);
		else
			return Common::Point(JOURNAL_POINTS_ZH[idx][2], JOURNAL_BUTTONS_Y_ZH + 2);
	} else {
		if (btn >= BTN_SEARCH)
			return Common::Point(JOURNAL_POINTS_INTL[idx][2], JOURNAL_BUTTONS_Y_INTL + 11);
		else
			return Common::Point(JOURNAL_POINTS_INTL[idx][2], JOURNAL_BUTTONS_Y_INTL);
	}
}

Common::Rect ScalpelJournal::getSearchButtonRect(int idx) {
	if (_vm->getLanguage() == Common::ZH_TWN) {
		return Common::Rect(SEARCH_POINTS_ZH[idx][0], 175, SEARCH_POINTS_ZH[idx][1], 194);
	} else {
		return Common::Rect(SEARCH_POINTS_INTL[idx][0], 174, SEARCH_POINTS_INTL[idx][1], 184);
	}
}

Common::Point ScalpelJournal::getSearchButtonTextPoint(int idx) {
	if (_vm->getLanguage() == Common::ZH_TWN) {
		return Common::Point(SEARCH_POINTS_ZH[idx][2], 177);
	} else {
		return Common::Point(SEARCH_POINTS_INTL[idx][2], 174);
	}
}

void ScalpelJournal::loadLocations() {
	Resources &res = *_vm->_res;

	_directory.clear();
	_locations.clear();


	Common::SeekableReadStream *dir = res.load("talk.lib");
	dir->skip(4);		// Skip header

	// Get the numer of entries
	_directory.resize(dir->readUint16LE());

	// Read in each entry
	char buffer[17];
	for (uint idx = 0; idx < _directory.size(); ++idx) {
		dir->read(buffer, 17);
		buffer[16] = '\0';

		_directory[idx] = Common::String(buffer);
	}

	delete dir;

	if (IS_3DO) {
		// 3DO: storage of locations is currently unknown TODO
		return;
	}

	// Load in the locations stored in journal.txt
	Common::SeekableReadStream *loc = res.load("journal.txt");

	while (loc->pos() < loc->size()) {
		Common::String line;
		char c;
		while ((c = loc->readByte()) != 0)
			line += c;

		// WORKAROUND: Special fixes for faulty translations
		// Was obviously not done in the original interpreter
		if (_vm->getLanguage() == Common::ES_ESP) {
			// Spanish version
			// We fix all sorts of typos
			// see bug #6931
			if (line == "En el cajellon destras del teatro Regency") {
				line = "En el callejon detras del teatro Regency";
			} else if (line == "En el apartamente de Simon Kingsley") {
				line = "En el apartamento de Simon Kingsley";
			} else if (line == "Bajo la muelle de Savoy Pier") {
				line = "Bajo el muelle de Savoy Pier";
			} else if (line == "En le viejo Sherman") {
				line = "En el viejo Sherman";
			} else if (line == "En la entrada de la cada de Anna Carroway") {
				line = "En la entrada de la casa de Anna Carroway";
			}
		}

		_locations.push_back(line);
	}

	delete loc;
}

void ScalpelJournal::drawFrame() {
	Resources &res = *_vm->_res;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	byte palette[Graphics::PALETTE_SIZE];

	// Load in the journal background
	Common::SeekableReadStream *bg = res.load("journal.lbv");
	bg->read(screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT);
	bg->read(palette, Graphics::PALETTE_SIZE);
	delete bg;

	// Translate the palette for display
	for (int idx = 0; idx < Graphics::PALETTE_SIZE; ++idx)
		palette[idx] = VGA_COLOR_TRANS(palette[idx]);

	// Set the palette and print the title
	screen.setPalette(palette);
	if (_vm->getLanguage() == Common::ZH_TWN) {
		screen.gPrint(Common::Point(111, 13), BUTTON_BOTTOM, "%s", _fixedTextWatsonsJournal.c_str());
		screen.gPrint(Common::Point(110, 12), INV_FOREGROUND, "%s", _fixedTextWatsonsJournal.c_str());
	} else {
		screen.gPrint(Common::Point(111, 18), BUTTON_BOTTOM, "%s", _fixedTextWatsonsJournal.c_str());
		screen.gPrint(Common::Point(110, 17), INV_FOREGROUND, "%s", _fixedTextWatsonsJournal.c_str());
	}

	// Draw the buttons
	screen.makeButton(getButtonRect(BTN_EXIT), getButtonTextPoint(BTN_EXIT), _fixedTextExit);
	screen.makeButton(getButtonRect(BTN_BACK10), getButtonTextPoint(BTN_BACK10), _fixedTextBack10);
	screen.makeButton(getButtonRect(BTN_UP), getButtonTextPoint(BTN_UP), _fixedTextUp);
	screen.makeButton(getButtonRect(BTN_DOWN), getButtonTextPoint(BTN_DOWN), _fixedTextDown);
	screen.makeButton(getButtonRect(BTN_AHEAD110), getButtonTextPoint(BTN_AHEAD110), _fixedTextAhead10);
	screen.makeButton(getButtonRect(BTN_SEARCH), getButtonTextPoint(BTN_SEARCH), _fixedTextSearch);
	screen.makeButton(getButtonRect(BTN_FIRST_PAGE), getButtonTextPoint(BTN_FIRST_PAGE), _fixedTextFirstPage);
	screen.makeButton(getButtonRect(BTN_LAST_PAGE), getButtonTextPoint(BTN_LAST_PAGE), _fixedTextLastPage);

	// WORKAROUND: Draw Print Text button as disabled, since we don't support it in ScummVM
	// In Chinese version skip it altogether to make space for hotkeys
	if (_vm->getLanguage() != Common::ZH_TWN) {
		screen.makeButton(getButtonRect(BTN_PRINT_TEXT), getButtonTextPoint(BTN_PRINT_TEXT), _fixedTextPrintText);
		screen.buttonPrint(getButtonTextPoint(BTN_PRINT_TEXT), COMMAND_NULL, false, _fixedTextPrintText);
	}
}

void ScalpelJournal::drawInterface() {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;

	drawFrame();

	if (_journal.empty()) {
		_up = _down = 0;
	} else {
		drawJournal(0, 0);
	}

	doArrows();

	// Show the entire screen
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
}

void ScalpelJournal::doArrows() {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	byte color;

	color = (_page > 1) ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(getButtonTextPoint(BTN_BACK10), color, false, _fixedTextBack10);
	screen.buttonPrint(getButtonTextPoint(BTN_UP), color, false, _fixedTextUp);

	color = _down ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(getButtonTextPoint(BTN_DOWN), color, false, _fixedTextDown);
	screen.buttonPrint(getButtonTextPoint(BTN_AHEAD110), color, false, _fixedTextAhead10);
	screen.buttonPrint(getButtonTextPoint(BTN_LAST_PAGE), color, false, _fixedTextLastPage);

	color = _journal.size() > 0 ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(getButtonTextPoint(BTN_SEARCH), color, false, _fixedTextSearch);
	if (_vm->getLanguage() != Common::ZH_TWN) {
		screen.buttonPrint(getButtonTextPoint(BTN_PRINT_TEXT), COMMAND_NULL, false, _fixedTextPrintText);
	}

	color = _page > 1 ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(getButtonTextPoint(BTN_FIRST_PAGE), color, false, _fixedTextFirstPage);
}

JournalButton ScalpelJournal::getHighlightedButton(const Common::Point &pt) {
	if (getButtonRect(BTN_EXIT).contains(pt))
		return BTN_EXIT;

	if (getButtonRect(BTN_BACK10).contains(pt) && _page > 1)
		return BTN_BACK10;

	if (getButtonRect(BTN_UP).contains(pt) && _up)
		return BTN_UP;

	if (getButtonRect(BTN_DOWN).contains(pt) && _down)
		return BTN_DOWN;

	if (getButtonRect(BTN_AHEAD110).contains(pt) && _down)
		return BTN_AHEAD110;

	if (getButtonRect(BTN_SEARCH).contains(pt) && !_journal.empty())
		return BTN_SEARCH;

	if (getButtonRect(BTN_FIRST_PAGE).contains(pt) && _up)
		return BTN_FIRST_PAGE;

	if (getButtonRect(BTN_LAST_PAGE).contains(pt) && _down)
		return BTN_LAST_PAGE;

	if (_vm->getLanguage() != Common::ZH_TWN && getButtonRect(BTN_PRINT_TEXT).contains(pt) && !_journal.empty())
		return BTN_PRINT_TEXT;

	return BTN_NONE;
}

bool ScalpelJournal::handleEvents(int key) {
	Events    &events    = *_vm->_events;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	bool doneFlag = false;

	Common::Point pt = events.mousePos();
	JournalButton btn = getHighlightedButton(pt);
	byte color;

	if (events._pressed || events._released) {
		// Exit button
		color = (btn == BTN_EXIT) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;
		screen.buttonPrint(getButtonTextPoint(BTN_EXIT), color, true, _fixedTextExit);

		// Back 10 button
		if (btn == BTN_BACK10) {
			screen.buttonPrint(getButtonTextPoint(BTN_BACK10), COMMAND_HIGHLIGHTED, true, _fixedTextBack10);
		} else if (_page > 1) {
			screen.buttonPrint(getButtonTextPoint(BTN_BACK10), COMMAND_FOREGROUND, true, _fixedTextBack10);
		}

		// Up button
		if (btn == BTN_UP) {
			screen.buttonPrint(getButtonTextPoint(BTN_UP), COMMAND_HIGHLIGHTED, true, _fixedTextUp);
		} else if (_up) {
			screen.buttonPrint(getButtonTextPoint(BTN_UP), COMMAND_FOREGROUND, true, _fixedTextUp);
		}

		// Down button
		if (btn == BTN_DOWN) {
			screen.buttonPrint(getButtonTextPoint(BTN_DOWN), COMMAND_HIGHLIGHTED, true, _fixedTextDown);
		} else if (_down) {
			screen.buttonPrint(getButtonTextPoint(BTN_DOWN), COMMAND_FOREGROUND, true, _fixedTextDown);
		}

		// Ahead 10 button
		if (btn == BTN_AHEAD110) {
			screen.buttonPrint(getButtonTextPoint(BTN_AHEAD110), COMMAND_HIGHLIGHTED, true, _fixedTextAhead10);
		} else if (_down) {
			screen.buttonPrint(getButtonTextPoint(BTN_AHEAD110), COMMAND_FOREGROUND, true, _fixedTextAhead10);
		}

		// Search button
		if (btn == BTN_SEARCH) {
			color = COMMAND_HIGHLIGHTED;
		} else if (_journal.empty()) {
			color = COMMAND_NULL;
		} else {
			color = COMMAND_FOREGROUND;
		}
		screen.buttonPrint(getButtonTextPoint(BTN_SEARCH), color, true, _fixedTextSearch);

		// First Page button
		if (btn == BTN_FIRST_PAGE) {
			color = COMMAND_HIGHLIGHTED;
		} else if (_up) {
			color = COMMAND_FOREGROUND;
		} else {
			color = COMMAND_NULL;
		}
		screen.buttonPrint(getButtonTextPoint(BTN_FIRST_PAGE), color, true, _fixedTextFirstPage);

		// Last Page button
		if (btn == BTN_LAST_PAGE) {
			color = COMMAND_HIGHLIGHTED;
		} else if (_down) {
			color = COMMAND_FOREGROUND;
		} else {
			color = COMMAND_NULL;
		}
		screen.buttonPrint(getButtonTextPoint(BTN_LAST_PAGE), color, true, _fixedTextLastPage);

		if (_vm->getLanguage() != Common::ZH_TWN) {
			// Print Text button
			screen.buttonPrint(getButtonTextPoint(BTN_PRINT_TEXT), COMMAND_NULL, true, _fixedTextPrintText);
		}
	}

	if (btn == BTN_EXIT && events._released) {
		// Exit button pressed
		doneFlag = true;

	} else if (((btn == BTN_BACK10 && events._released) || key == _hotkeyBack10) && (_page > 1)) {
		// Scrolll up 10 pages
		if (_page < 11)
			drawJournal(1, (_page - 1) * LINES_PER_PAGE);
		else
			drawJournal(1, 10 * LINES_PER_PAGE);

		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_UP && events._released) || key == _hotkeyUp) && _up) {
		// Scroll up
		drawJournal(1, LINES_PER_PAGE);
		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_DOWN && events._released) || key == _hotkeyDown) && _down) {
		// Scroll down
		drawJournal(2, LINES_PER_PAGE);
		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_AHEAD110 && events._released) || key == _hotkeyAhead10) && _down) {
		// Scroll down 10 pages
		if ((_page + 10) > _maxPage)
			drawJournal(2, (_maxPage - _page) * LINES_PER_PAGE);
		else
			drawJournal(2, 10 * LINES_PER_PAGE);

		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_SEARCH && events._released) || key == _hotkeySearch) && !_journal.empty()) {
		screen.buttonPrint(getButtonTextPoint(BTN_SEARCH), COMMAND_FOREGROUND, true, _fixedTextSearch);
		bool notFound = false;

		do {
			int dir;
			if ((dir = getSearchString(notFound)) != 0) {
				int savedIndex = _index;
				int savedSub = _sub;
				int savedPage = _page;

				if (drawJournal(dir + 2, 1000 * LINES_PER_PAGE) == 0) {
					_index = savedIndex;
					_sub = savedSub;
					_page = savedPage;

					drawFrame();
					drawJournal(0, 0);
					notFound = true;
				} else {
					doneFlag = true;
				}

				doArrows();
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
			} else {
				doneFlag = true;
			}
		} while (!doneFlag);
		doneFlag = false;

	} else if (((btn == BTN_FIRST_PAGE && events._released) || key == _hotkeyFirstPage) && _up) {
		// First page
		_index = _sub = 0;
		_up = _down = false;
		_page = 1;

		drawFrame();
		drawJournal(0, 0);
		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_LAST_PAGE && events._released) || key == _hotkeyLastPage) && _down) {
		// Last page
		if ((_page + 10) > _maxPage)
			drawJournal(2, (_maxPage - _page) * LINES_PER_PAGE);
		else
			drawJournal(2, 1000 * LINES_PER_PAGE);

		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	}

	events.wait(2);

	return doneFlag;
}

int ScalpelJournal::getSearchString(bool printError) {
	Events    &events    = *_vm->_events;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Talk &talk = *_vm->_talk;
	int xp;
	int yp;
	bool flag = false;
	Common::String name;
	int done = 0;
	byte color;
	bool isChinese = _vm->getLanguage() == Common::ZH_TWN;

	// Draw search panel
	if (isChinese)
		screen.makePanel(Common::Rect(6, 171, 318, 199));
	else
		screen.makePanel(Common::Rect(6, 171, 313, 199));

	screen.makeButton(getSearchButtonRect(0), getSearchButtonTextPoint(0), _fixedTextSearchExit, !isChinese);
	screen.makeButton(getSearchButtonRect(1), getSearchButtonTextPoint(1), _fixedTextSearchBackward, !isChinese);
	screen.makeButton(getSearchButtonRect(2), getSearchButtonTextPoint(2), _fixedTextSearchForward, !isChinese);

	if (isChinese)
		screen.makeField(Common::Rect(12, 175, 205, 194));
	else
		screen.makeField(Common::Rect(12, 185, 307, 196));

	if (printError) {
		screen.gPrint(Common::Point((SHERLOCK_SCREEN_WIDTH - screen.stringWidth(_fixedTextSearchNotFound)) / 2, 185),
			INV_FOREGROUND, "%s", _fixedTextSearchNotFound.c_str());
	} else if (!_find.empty()) {
		// There's already a search term, display it already
		screen.gPrint(Common::Point(15, 185), TALK_FOREGROUND, "%s", _find.c_str());
		name = _find;
	}

	if (_vm->getLanguage() == Common::ZH_TWN)
		screen.slamArea(6, 171, 312, 28);
	else
		screen.slamArea(6, 171, 307, 28);

	if (printError) {
		// Give time for user to see the message
		events.setButtonState();
		for (int idx = 0; idx < 40 && !_vm->shouldQuit() && !events.kbHit() && !events._released; ++idx) {
			events.pollEvents();
			events.setButtonState();
			events.wait(2);
		}

		events.clearKeyboard();
		screen._backBuffer1.fillRect(Common::Rect(13, 186, 306, 195), BUTTON_MIDDLE);

		if (!_find.empty()) {
			screen.gPrint(Common::Point(15, 185), TALK_FOREGROUND, "%s", _find.c_str());
			name = _find;
		}

		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	}

	xp = JOURNAL_SEARCH_LEFT + screen.stringWidth(name);
	yp = isChinese ? JOURNAL_SEARCH_TOP_ZH : JOURNAL_SEARCH_TOP_INTL;

	do {
		events._released = false;
		JournalButton found = BTN_NONE;

		while (!_vm->shouldQuit() && !events.kbHit() && !events._released) {
			found = BTN_NONE;
			if (talk._talkToAbort)
				return 0;

			// Check if key or mouse button press has occurred
			events.setButtonState();
			Common::Point pt = events.mousePos();

			flag = !flag;
			screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), flag ? INV_FOREGROUND : BUTTON_MIDDLE);

			if (events._pressed || events._released) {
				if (getSearchButtonRect(0).contains(pt)) {
					found = BTN_EXIT;
					color = COMMAND_HIGHLIGHTED;
				} else {
					color = COMMAND_FOREGROUND;
				}
				screen.buttonPrint(getSearchButtonTextPoint(0), color, false, _fixedTextSearchExit, !isChinese);

				if (getSearchButtonRect(1).contains(pt)) {
					found = BTN_BACKWARD;
					color = COMMAND_HIGHLIGHTED;
				} else {
					color = COMMAND_FOREGROUND;
				}
				screen.buttonPrint(getSearchButtonTextPoint(1), color, false, _fixedTextSearchBackward, !isChinese);

				if (getSearchButtonRect(2).contains(pt)) {
					found = BTN_FORWARD;
					color = COMMAND_HIGHLIGHTED;
				} else {
					color = COMMAND_FOREGROUND;
				}
				screen.buttonPrint(getSearchButtonTextPoint(2), color, false, _fixedTextSearchForward, !isChinese);
			}

			events.wait(2);
		}

		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();

			if ((keyState.keycode == Common::KEYCODE_BACKSPACE) && (name.size() > 0)) {
				screen.vgaBar(Common::Rect(xp - screen.charWidth(name.lastChar()), yp, xp + 8, yp + 9), BUTTON_MIDDLE);
				xp -= screen.charWidth(name.lastChar());
				screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), INV_FOREGROUND);
				name.deleteLastChar();

			} else  if (keyState.keycode == Common::KEYCODE_RETURN) {
				done = 1;

			}  else if (keyState.keycode == Common::KEYCODE_ESCAPE) {
				screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), BUTTON_MIDDLE);
				done = -1;

			} else if (keyState.ascii >= ' ' && keyState.ascii <= 'z' && keyState.keycode != Common::KEYCODE_AT &&
				name.size() < JOURNAL_SEACRH_MAX_CHARS && (xp + screen.charWidth(keyState.ascii)) < JOURNAL_SEARCH_RIGHT) {
				char ch = toupper(keyState.ascii);
				screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), BUTTON_MIDDLE);
				screen.print(Common::Point(xp, yp), TALK_FOREGROUND, "%c", ch);
				xp += screen.charWidth(ch);
				name += ch;
			}
		}

		if (events._released) {
			switch (found) {
			case BTN_EXIT:
				done = -1; break;
			case BTN_BACKWARD:
				done = 2; break;
			case BTN_FORWARD:
				done = 1; break;
			default:
				break;
			}
		}
	} while (!done && !_vm->shouldQuit());

	if (done != -1) {
		_find = name;
	} else {
		done = 0;
	}

	// Redisplay the journal screen
	drawFrame();
	drawJournal(0, 0);
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	return done;
}

void ScalpelJournal::resetPosition() {
	_index = _sub = _up = _down = 0;
	_page = 1;
}

void ScalpelJournal::record(int converseNum, int statementNum, bool replyOnly) {
	// there seems to be no journal in the 3DO version
	if (!IS_3DO)
		Journal::record(converseNum, statementNum, replyOnly);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
