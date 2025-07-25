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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"

#include "adl/display.h"

namespace Adl {

Display::~Display() {
	delete[] _textBuf;
}

void Display::createTextBuffer(uint textWidth, uint textHeight) {
	_textWidth = textWidth;
	_textHeight = textHeight;

	_textBuf = new byte[textWidth * textHeight];
	memset(_textBuf, (byte)asciiToNative(' '), textWidth * textHeight);
}

void Display::setMode(Display::Mode mode) {
	_mode = mode;

	if (_mode == Display::kModeText || _mode == Display::kModeMixed)
		renderText();
	if (_mode == Display::kModeGraphics || _mode == Display::kModeMixed)
		renderGraphics();
}

void Display::home() {
	memset(_textBuf, (byte)asciiToNative(' '), _textWidth * _textHeight);
	_cursorPos = 0;
}

void Display::moveCursorForward() {
	++_cursorPos;

	if (_cursorPos >= _textWidth * _textHeight)
		scrollUp();
}

void Display::moveCursorBackward() {
	if (_cursorPos > 0)
		--_cursorPos;
}

void Display::moveCursorTo(const Common::Point &pos) {
	_cursorPos = pos.y * _textWidth + pos.x;

	if (_cursorPos >= _textWidth * _textHeight)
		error("Cursor position (%i, %i) out of bounds", pos.x, pos.y);
}

void Display::printString(const Common::String &str, bool voiceString) {
	for (const auto &c : str)
		printChar(c);

	if (voiceString) {
		sayText(str);
	}

	renderText();
}

void Display::printAsciiString(const Common::String &str, bool voiceString) {
	for (const auto &c : str)
		printChar(asciiToNative(c));

	if (voiceString) {
		sayText(str);
	}

	renderText();
}

void Display::sayText(const Common::String &text, Common::TextToSpeechManager::Action action) const {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled")) {
		ttsMan->say(convertText(text), action);
	}
}

Common::U32String Display::convertText(const Common::String &text) const {
	Common::String result(text);

	bool startsWithDash = result[0] == '\xad';

	for (uint i = 0; i < result.size(); i++) {
		// Convert carriage returns and dashes to spaces
		// Only convert dashes if the line starts with a dash (which is usually the case with the "enter command"
		// prompt), to avoid converting mid-line dashes to spaces and causing odd voicing
		// (i.e. if the dash in "Hi-Res Adventure" is replaced, English TTS pronounces it as "Hi Residential Adventure")
		if (result[i] == '\x8d' || (startsWithDash && result[i] == '\xad')) {
			result[i] = '\xa0';
		}

		result[i] &= 0x7f;
	}

	return Common::U32String(result, Common::CodePage::kASCII);
}

void Display::setCharAtCursor(byte c) {
	_textBuf[_cursorPos] = c;
}

void Display::scrollUp() {
	memmove(_textBuf, _textBuf + _textWidth, (_textHeight - 1) * _textWidth);
	memset(_textBuf + (_textHeight - 1) * _textWidth, asciiToNative(' '), _textWidth);
	if (_cursorPos >= _textWidth)
		_cursorPos -= _textWidth;
}

} // End of namespace Adl
