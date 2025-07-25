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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/font/base_font_bitmap.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_frame.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_frame.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_file_manager.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(BaseFontBitmap, false)

//////////////////////////////////////////////////////////////////////
BaseFontBitmap::BaseFontBitmap(BaseGame *inGame) : BaseFont(inGame) {
	_subframe = nullptr;
	_sprite = nullptr;
	_widthsFrame = 0;
	memset(_widths, 0, NUM_CHARACTERS);
	_tileWidth = _tileHeight = _numColumns = 0;
	_fontextFix = false;
	_freezable = false;
	_wholeCell = false;
}


//////////////////////////////////////////////////////////////////////
BaseFontBitmap::~BaseFontBitmap() {
	delete _subframe;
	delete _sprite;
	_subframe = nullptr;
	_sprite = nullptr;
}


//////////////////////////////////////////////////////////////////////
void BaseFontBitmap::drawText(const byte *text, int x, int y, int width, TTextAlign align, int maxHeight, int maxLength) {
	textHeightDraw(text, x, y, width, align, true, maxHeight, maxLength);
}


//////////////////////////////////////////////////////////////////////
int BaseFontBitmap::getTextHeight(const byte *text, int width) {
	return textHeightDraw(text, 0, 0, width, TAL_LEFT, false);
}


//////////////////////////////////////////////////////////////////////
int BaseFontBitmap::getTextWidth(const byte *text, int maxLength) {
	AnsiString str;

	if (_gameRef->_textEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::utf8ToWide(Utf8String((const char *)text));
		str = StringUtil::wideToAnsi(wstr);
	} else {
		str = AnsiString((const char *)text);
	}

	if (maxLength >= 0 && str.size() > (uint32)maxLength) {
		str = Common::String(str.c_str(), (uint32)maxLength);
	}
	//str.substr(0, maxLength); // TODO: Remove

	int textWidth = 0;
	for (int i = 0; (uint32)i < str.size(); i++) {
		textWidth += getCharWidth((byte)str[i]);
	}

	return textWidth;
}


//////////////////////////////////////////////////////////////////////
int BaseFontBitmap::textHeightDraw(const byte *text, int x, int y, int width, TTextAlign align, bool draw, int maxHeight, int maxLength) {
	if (maxLength == 0) {
		return 0;
	}

	if (text == nullptr || text[0] == '\0') {
		return _tileHeight;
	}

	AnsiString str;

	if (_gameRef->_textEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::utf8ToWide(Utf8String((const char *)text));
		str = StringUtil::wideToAnsi(wstr);
	} else {
		str = AnsiString((const char *)text);
	}
	if (str.empty()) {
		return 0;
	}

	int lineLength = 0;
	int realLength = 0;
	int numLines = 0;

	int i;

	int index = -1;
	int start = 0;
	int end = 0;
	int last_end = 0;

	bool done = false;
	bool newLine = false;
	bool longLine = false;
#ifdef ENABLE_FOXTAIL
	bool minimizeSpacing = BaseEngine::instance().isFoxTail();
#endif

	if (draw) {
		_gameRef->_renderer->startSpriteBatch();
	}

	while (!done) {
		if (maxHeight > 0 && (numLines + 1)*_tileHeight > maxHeight) {
			if (draw) {
				_gameRef->_renderer->endSpriteBatch();
			}
			return numLines * _tileHeight;
		}

		index++;

		if (str[index] == ' ' && (maxHeight < 0 || maxHeight / _tileHeight > 1)) {
			end = index - 1;
			realLength = lineLength;
		}

		if (str[index] == '\n') {
			end = index - 1;
			realLength = lineLength;
			newLine = true;
		}

		if (lineLength + getCharWidth(str[index]) > width && last_end == end) {
			end = index - 1;
			realLength = lineLength;
			newLine = true;
			longLine = true;
		}

		if ((int)str.size() == (index + 1) || (maxLength >= 0 && index == maxLength - 1)) {
			done = true;
			if (!newLine) {
				end = index;
				lineLength += getCharWidth(str[index]);
				realLength = lineLength;
			}
		} else {
			lineLength += getCharWidth(str[index]);
		}

		if ((lineLength > width) || done || newLine) {
			if (end < 0) {
				done = true;
			}
			int startX;
			switch (align) {
			case TAL_CENTER:
				startX = x + (width - realLength) / 2;
				break;
			case TAL_RIGHT:
				startX = x + width - realLength;
				break;
			case TAL_LEFT:
				startX = x;
				break;
			default:
				error("BaseFontBitmap::TextHeightDraw - Unhandled enum");
				break;
			}
			for (i = start; i < end + 1; i++) {
				if (draw) {
					drawChar(str[i], startX, y);
				}
				startX += getCharWidth(str[i]);
			}
			y += _tileHeight;
#ifdef ENABLE_FOXTAIL
			if (minimizeSpacing) {
				y -= 3;
			}
#endif
			last_end = end;
			if (longLine) {
				end--;
			}
			start = end + 2;
			index = end + 1;
			lineLength = 0;
			newLine = false;
			longLine = false;
			numLines++;
		}
	}

	if (draw) {
		_gameRef->_renderer->endSpriteBatch();
	}

	return numLines * _tileHeight;
}


//////////////////////////////////////////////////////////////////////
void BaseFontBitmap::drawChar(byte c, int x, int y) {
	if (_fontextFix) {
		c--;
	}

	int row, col;

	row = c / _numColumns;
	col = c % _numColumns;

	Rect32 rect;
	/* l t r b */
	int tileWidth;
	if (_wholeCell) {
		tileWidth = _tileWidth;
	} else {
		tileWidth = _widths[c];
	}

	rect.setRect(col * _tileWidth, row * _tileHeight, col * _tileWidth + tileWidth, (row + 1) * _tileHeight);
	bool handled = false;
	if (_sprite) {
		_sprite->getCurrentFrame();
		if (_sprite->_currentFrame >= 0 && _sprite->_currentFrame < (int32)_sprite->_frames.getSize() && _sprite->_frames[_sprite->_currentFrame]) {
			if (_sprite->_frames[_sprite->_currentFrame]->_subframes.getSize() > 0) {
				_sprite->_frames[_sprite->_currentFrame]->_subframes[0]->_surface->displayTrans(x, y, rect);
			}
			handled = true;
		}
	}
	if (!handled && _subframe) {
		_subframe->_surface->displayTrans(x, y, rect, _subframe->_alpha);
	}
}


//////////////////////////////////////////////////////////////////////
bool BaseFontBitmap::loadFile(const Common::String &filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "BaseFontBitmap::LoadFile failed for file '%s'", filename.c_str());
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename.c_str());

	if (DID_FAIL(ret = loadBuffer(buffer))) {
		_gameRef->LOG(0, "Error parsing FONT file '%s'", filename.c_str());
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(FONTEXT_FIX)
TOKEN_DEF(FONT)
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(COLUMNS)
TOKEN_DEF(TILE_WIDTH)
TOKEN_DEF(TILE_HEIGHT)
TOKEN_DEF(DEFAULT_WIDTH)
TOKEN_DEF(WIDTHS)
TOKEN_DEF(AUTO_WIDTH)
TOKEN_DEF(SPACE_WIDTH)
TOKEN_DEF(EXPAND_WIDTH)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(SPRITE)
TOKEN_DEF(WIDTHS_FRAME)
TOKEN_DEF(PAINT_WHOLE_CELL)
#ifdef ENABLE_FOXTAIL
TOKEN_DEF(COLOR)
#endif
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
bool BaseFontBitmap::loadBuffer(char *buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(FONTEXT_FIX)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(COLUMNS)
	TOKEN_TABLE(TILE_WIDTH)
	TOKEN_TABLE(TILE_HEIGHT)
	TOKEN_TABLE(DEFAULT_WIDTH)
	TOKEN_TABLE(WIDTHS)
	TOKEN_TABLE(AUTO_WIDTH)
	TOKEN_TABLE(SPACE_WIDTH)
	TOKEN_TABLE(EXPAND_WIDTH)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(WIDTHS_FRAME)
	TOKEN_TABLE(PAINT_WHOLE_CELL)
#ifdef ENABLE_FOXTAIL
	TOKEN_TABLE(COLOR)
#endif
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	if (parser.getCommand(&buffer, commands, &params) != TOKEN_FONT) {
		_gameRef->LOG(0, "'FONT' keyword expected.");
		return STATUS_FAILED;
	}
	buffer = params;

	int widths[300];
	int num = 0, defaultWidth = 8;
	int lastWidth = 0;
	int i;
	int r = 255, g = 255, b = 255;
	bool customTrans = false;
#ifdef ENABLE_FOXTAIL
	int ar = 255, ag = 255, ab = 255;
	bool customAlpha = false;
#endif
	char *surfaceFile = nullptr;
	char *spriteFile = nullptr;

	bool autoWidth = false;
	int spaceWidth = 0;
	int expandWidth = 0;

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {

		switch (cmd) {
		case TOKEN_IMAGE:
			surfaceFile = params;
			break;

		case TOKEN_SPRITE:
			spriteFile = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			customTrans = true;
			break;

#ifdef ENABLE_FOXTAIL
		case TOKEN_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			customAlpha = true;
			break;
#endif

		case TOKEN_WIDTHS:
			parser.scanStr(params, "%D", widths, &num);
			for (i = 0; lastWidth < NUM_CHARACTERS && num > 0; lastWidth++, num--, i++) {
				_widths[lastWidth] = (byte)widths[i];
			}
			break;

		case TOKEN_DEFAULT_WIDTH:
			parser.scanStr(params, "%d", &defaultWidth);
			break;

		case TOKEN_WIDTHS_FRAME:
			parser.scanStr(params, "%d", &_widthsFrame);
			break;

		case TOKEN_COLUMNS:
			parser.scanStr(params, "%d", &_numColumns);
			break;

		case TOKEN_TILE_WIDTH:
			parser.scanStr(params, "%d", &_tileWidth);
			break;

		case TOKEN_TILE_HEIGHT:
			parser.scanStr(params, "%d", &_tileHeight);
			break;

		case TOKEN_AUTO_WIDTH:
			parser.scanStr(params, "%b", &autoWidth);
			break;

		case TOKEN_FONTEXT_FIX:
			parser.scanStr(params, "%b", &_fontextFix);
			break;

		case TOKEN_PAINT_WHOLE_CELL:
			parser.scanStr(params, "%b", &_wholeCell);
			break;

		case TOKEN_SPACE_WIDTH:
			parser.scanStr(params, "%d", &spaceWidth);
			break;

		case TOKEN_EXPAND_WIDTH:
			parser.scanStr(params, "%d", &expandWidth);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		default:
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in FONT definition");
		return STATUS_FAILED;
	}

	if (spriteFile != nullptr) {
		delete _sprite;
		_sprite = new BaseSprite(_gameRef, this);
		if (!_sprite || DID_FAIL(_sprite->loadFile(spriteFile))) {
			delete _sprite;
			_sprite = nullptr;
		}
	}

	if (surfaceFile != nullptr && !_sprite) {
		_subframe = new BaseSubFrame(_gameRef);
		if (customTrans) {
			_subframe->setSurface(surfaceFile, false, r, g, b);
		} else {
			_subframe->setSurface(surfaceFile);
		}
#ifdef ENABLE_FOXTAIL
		if (customAlpha) {
			_subframe->_alpha = BYTETORGBA(ar, ag, ab, 255);
		}
#endif
	}


	if (((_subframe == nullptr || _subframe->_surface == nullptr) && _sprite == nullptr) || _numColumns == 0 || _tileWidth == 0 || _tileHeight == 0) {
		_gameRef->LOG(0, "Incomplete font definition");
		return STATUS_FAILED;
	}

	if (autoWidth) {
		// calculate characters width
		getWidths();

		// do we need to modify widths?
		if (expandWidth != 0) {
			for (i = 0; i < NUM_CHARACTERS; i++) {
				int newWidth = (int)_widths[i] + expandWidth;
				if (newWidth < 0) {
					newWidth = 0;
				}

				_widths[i] = (byte)newWidth;
			}
		}

		// handle space character
		uint32 spaceChar = ' ';
		if (_fontextFix) {
			spaceChar--;
		}

		if (spaceWidth != 0) {
			_widths[spaceChar] = spaceWidth;
		} else {
			if (_widths[spaceChar] == expandWidth || _widths[spaceChar] == 0) {
				_widths[spaceChar] = (_widths[(uint)'m'] + _widths[(uint)'i']) / 2;
			}
		}
	} else {
		for (i = lastWidth; i < NUM_CHARACTERS; i++) {
			_widths[i] = defaultWidth;
		}
	}

#ifdef ENABLE_FOXTAIL
	if (BaseEngine::instance().isFoxTail()) {
		for (i = lastWidth; i < NUM_CHARACTERS; i++) {
			_widths[i]--;
		}
	}
#endif


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontBitmap::persist(BasePersistenceManager *persistMgr) {

	BaseFont::persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_numColumns));

	persistMgr->transferPtr(TMEMBER_PTR(_subframe));
	persistMgr->transferSint32(TMEMBER(_tileHeight));
	persistMgr->transferSint32(TMEMBER(_tileWidth));
	persistMgr->transferPtr(TMEMBER_PTR(_sprite));
	persistMgr->transferSint32(TMEMBER(_widthsFrame));

	if (persistMgr->getIsSaving()) {
		persistMgr->putBytes(_widths, sizeof(_widths));
	} else {
		persistMgr->getBytes(_widths, sizeof(_widths));
	}


	persistMgr->transferBool(TMEMBER(_fontextFix));
	persistMgr->transferBool(TMEMBER(_wholeCell));


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int BaseFontBitmap::getCharWidth(byte index) {
	if (_fontextFix) {
		index--;
	}
	return _widths[index];
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontBitmap::getWidths() {
	BaseSurface *surf = nullptr;

	if (_sprite) {
		if (_widthsFrame >= 0 && _widthsFrame < (int32)_sprite->_frames.getSize()) {
			if (_sprite->_frames[_widthsFrame] && (int32)_sprite->_frames[_widthsFrame]->_subframes.getSize() > 0) {
				surf = _sprite->_frames[_widthsFrame]->_subframes[0]->_surface;
			}
		}
	}
	if (surf == nullptr && _subframe) {
		surf = _subframe->_surface;
	}
	if (!surf || DID_FAIL(surf->startPixelOp())) {
		return STATUS_FAILED;
	}


	for (int i = 0; i < NUM_CHARACTERS; i++) {
		int xxx = (i % _numColumns) * _tileWidth;
		int yyy = (i / _numColumns) * _tileHeight;


		int minCol = -1;
		for (int row = 0; row < _tileHeight; row++) {
			for (int col = _tileWidth - 1; col >= minCol + 1; col--) {
				if (xxx + col < 0 || xxx + col >= surf->getWidth() || yyy + row < 0 || yyy + row >= surf->getHeight()) {
					continue;
				}
				if (!surf->isTransparentAtLite(xxx + col, yyy + row)) {
					//min_col = col;
					minCol = MAX(col, minCol);
					break;
				}
			}
			if (minCol == _tileWidth - 1) {
				break;
			}
		}

		_widths[i] = minCol + 1;
	}
	surf->endPixelOp();
	/*
	_gameRef->LOG(0, "----- %s ------", _filename);
	for(int j=0; j<16; j++)
	{
	_gameRef->LOG(0, "%02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d", _widths[j*16+0], _widths[j*16+1], _widths[j*16+2], _widths[j*16+3], _widths[j*16+4], _widths[j*16+5], _widths[j*16+6], _widths[j*16+7], _widths[j*16+8], _widths[j*16+9], _widths[j*16+10], _widths[j*16+11], _widths[j*16+12], _widths[j*16+13], _widths[j*16+14], _widths[j*16+15]);
	}
	*/
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
int BaseFontBitmap::getLetterHeight() {
	return _tileHeight;
}

} // End of namespace Wintermute
