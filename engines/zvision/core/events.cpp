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

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/rational.h"
#include "engines/util.h"
#include "zvision/detection.h"
#include "zvision/zvision.h"
#include "zvision/core/console.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/menu.h"
#include "zvision/sound/zork_raw.h"
#include "zvision/text/string_manager.h"
#include "zvision/text/subtitle_manager.h"

namespace ZVision {

void ZVision::pushKeyToCheatBuf(uint8 key) {
	for (int i = 0; i < KEYBUF_SIZE - 1; i++)
		_cheatBuffer[i] = _cheatBuffer[i + 1];
	_cheatBuffer[KEYBUF_SIZE - 1] = key;
}

bool ZVision::checkCode(const char *code) {
	int codeLen = strlen(code);
	if (codeLen > KEYBUF_SIZE)
		return false;
	for (int i = 0; i < codeLen; i++)
		if (code[i] != _cheatBuffer[KEYBUF_SIZE - codeLen + i] && code[i] != '?')
			return false;
	return true;
}

uint8 ZVision::getBufferedKey(uint8 pos) {
	if (pos >= KEYBUF_SIZE)
		return 0;
	else
		return _cheatBuffer[KEYBUF_SIZE - pos - 1];
}

void ZVision::cheatCodes(uint8 key) {
	Location loc = _scriptManager->getCurrentLocation();
	// Do not process cheat codes while in the game menus
	if (loc.world == 'g' && loc.room == 'j')
		return;

	pushKeyToCheatBuf(key);

	if (getGameId() == GID_GRANDINQUISITOR) {
		if (checkCode("IMNOTDEAF")) {
			// Unknown cheat
			_subtitleManager->showDebugMsg(Common::String::format("IMNOTDEAF cheat or debug, not implemented"));
		}

		if (checkCode("3100OPB")) {
			_subtitleManager->showDebugMsg(Common::String::format("Current location: %c%c%c%c",
			                               _scriptManager->getStateValue(StateKey_World),
			                               _scriptManager->getStateValue(StateKey_Room),
			                               _scriptManager->getStateValue(StateKey_Node),
			                               _scriptManager->getStateValue(StateKey_View)));
		}

		if (checkCode("KILLMENOW")) {
			_scriptManager->changeLocation('g', 'j', 'd', 'e', 0);
			_scriptManager->setStateValue(2201, 35);
		}

		if (checkCode("MIKESPANTS")) {
			_scriptManager->changeLocation('g', 'j', 't', 'm', 0);
		}

		// There are 3 more cheats in script files:
		// - "WHOAMI": gjcr.scr
		// - "HUISOK": hp1e.scr
		// - "EAT ME": uh1f.scr
	} else if (getGameId() == GID_NEMESIS) {
		if (checkCode("CHLOE")) {
			_scriptManager->changeLocation('t', 'm', '2', 'g', 0);
			_scriptManager->setStateValue(224, 1);
		}

		if (checkCode("77MASSAVE")) {
			_subtitleManager->showDebugMsg(Common::String::format("Current location: %c%c%c%c",
			                               _scriptManager->getStateValue(StateKey_World),
			                               _scriptManager->getStateValue(StateKey_Room),
			                               _scriptManager->getStateValue(StateKey_Node),
			                               _scriptManager->getStateValue(StateKey_View)));
		}

		if (checkCode("IDKFA")) {
			_scriptManager->changeLocation('t', 'w', '3', 'f', 0);
			_scriptManager->setStateValue(249, 1);
		}

		if (checkCode("309NEWDORMA")) {
			_scriptManager->changeLocation('g', 'j', 'g', 'j', 0);
		}

		if (checkCode("HELLOSAILOR")) {
			Audio::AudioStream *soundStream;
			if (loc == "vb10") {
				soundStream = makeRawZorkStream("v000hpta.raw", this);
			} else {
				soundStream = makeRawZorkStream("v000hnta.raw", this);
			}
			Audio::SoundHandle handle;
			_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream);
		}
	}

	if (checkCode("FRAME")) {
		Common::String fpsStr = Common::String::format("FPS: %d", getFPS());
		_subtitleManager->showDebugMsg(fpsStr);
	}

	if (checkCode("COMPUTERARCH"))
		_subtitleManager->showDebugMsg("COMPUTERARCH: var-viewer not implemented");

	// This cheat essentially toggles the GOxxxx cheat below
	if (checkCode("XYZZY"))
		_scriptManager->setStateValue(StateKey_DebugCheats, 1 - _scriptManager->getStateValue(StateKey_DebugCheats));

	if (_scriptManager->getStateValue(StateKey_DebugCheats) == 1)
		if (checkCode("GO????"))
			_scriptManager->changeLocation(getBufferedKey(3),
			                               getBufferedKey(2),
			                               getBufferedKey(1),
			                               getBufferedKey(0), 0);

	// Show the Venus screen when "?" or "/" is pressed while inside the temple world
	if (_scriptManager->getStateValue(StateKey_VenusEnable) == 1)
		if (getBufferedKey(0) == 0xBF && _scriptManager->getStateValue(StateKey_World) == 't')
			_scriptManager->changeLocation('g', 'j', 'h', 'e', 0);
}

void ZVision::processEvents() {
	while (_eventMan->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_cursorManager->cursorDown(true);
			_menu->onMouseDown(_event.mouse);
			if (!_menu->inMenu() || !_widescreen) {
				_scriptManager->setStateValue(StateKey_LMouse, 1);
				_scriptManager->addEvent(_event);
			}
			break;

		case Common::EVENT_LBUTTONUP:
			_cursorManager->cursorDown(false);
			_menu->onMouseUp(_event.mouse);
			if (!_menu->inMenu() || !_widescreen) {
				_scriptManager->setStateValue(StateKey_LMouse, 0);
				_scriptManager->addEvent(_event);
			}
			break;

		case Common::EVENT_RBUTTONDOWN:
			_cursorManager->cursorDown(true);
			if (!_menu->inMenu() || !_widescreen) {
				_scriptManager->setStateValue(StateKey_RMouse, 1);
				if (getGameId() == GID_NEMESIS)
					_scriptManager->inventoryCycle();
			}
			break;

		case Common::EVENT_RBUTTONUP:
			_cursorManager->cursorDown(false);
			if (!_menu->inMenu() || !_widescreen)
				_scriptManager->setStateValue(StateKey_RMouse, 0);
			break;

		case Common::EVENT_MOUSEMOVE:
			onMouseMove(_event.mouse);
			break;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch ((ZVisionAction)_event.customType) {
			case kZVisionActionLeft:
			case kZVisionActionRight:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::PANORAMA)
					_keyboardVelocity = (_event.customType == kZVisionActionLeft ?
					                     -_scriptManager->getStateValue(StateKey_KbdRotateSpeed) :
					                     _scriptManager->getStateValue(StateKey_KbdRotateSpeed)) * 2;
				break;

			case kZVisionActionUp:
			case kZVisionActionDown:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::TILT)
					_keyboardVelocity = (_event.customType == kZVisionActionUp ?
					                     -_scriptManager->getStateValue(StateKey_KbdRotateSpeed) :
					                     _scriptManager->getStateValue(StateKey_KbdRotateSpeed)) * 2;
				break;

			case kZVisionActionSave:
				if (_menu->getEnable(kMainMenuSave))
					_scriptManager->changeLocation('g', 'j', 's', 'e', 0);
				break;

			case kZVisionActionRestore:
				if (_menu->getEnable(kMainMenuLoad))
					_scriptManager->changeLocation('g', 'j', 'r', 'e', 0);
				break;

			case kZVisionActionPreferences:
				if (_menu->getEnable(kMainMenuPrefs))
					_scriptManager->changeLocation('g', 'j', 'p', 'e', 0);
				break;

			case kZVisionActionQuit:
				if (_menu->getEnable(kMainMenuExit)) {
					if (ConfMan.hasKey("confirm_exit") && ConfMan.getBool("confirm_exit"))
						quit(true);
					else
						quit(false);
				}
				break;

			case kZVisionActionShowFPS: {
				Common::String fpsStr = Common::String::format("FPS: %d", getFPS());
				_subtitleManager->showDebugMsg(fpsStr);
			}
			break;
			default:
				break;
			}
			break;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			switch ((ZVisionAction)_event.customType) {
			case kZVisionActionLeft:
			case kZVisionActionRight:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::PANORAMA)
					_keyboardVelocity = 0;
				break;
			case kZVisionActionUp:
			case kZVisionActionDown:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::TILT)
					_keyboardVelocity = 0;
				break;
			default:
				break;
			}
			break;

		case Common::EVENT_KEYDOWN: {
			uint8 vkKey = getZvisionKey(_event.kbd.keycode);
			_scriptManager->setStateValue(StateKey_KeyPress, vkKey);
			_scriptManager->addEvent(_event);
			cheatCodes(vkKey);
		}
		break;
		case Common::EVENT_KEYUP:
			_scriptManager->addEvent(_event);
			break;
		default:
			break;
		}
	}
}

void ZVision::onMouseMove(const Common::Point &pos) {
	debugC(6, kDebugEvent, "ZVision::onMouseMove()");
	_menu->onMouseMove(pos);
	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));
	Common::Rect workingArea = _renderManager->getWorkingArea();
	bool cursorWasChanged = false;

	// Graph of the function governing rotation velocity:
	//
	//                                    |---------------- working window ------------------|
	//               ^                    |---------|
	//               |                          |
	// +Max velocity |                        rotation screen edge offset
	//               |                                                                      /|
	//               |                                                                     / |
	//               |                                                                    /  |
	//               |                                                                   /   |
	//               |                                                                  /    |
	//               |                                                                 /     |
	//               |                                                                /      |
	//               |                                                               /       |
	//               |                                                              /        |
	// Zero velocity |______________________________ ______________________________/_________|__________________________>
	//               | Position ->        |         /
	//               |                    |        /
	//               |                    |       /
	//               |                    |      /
	//               |                    |     /
	//               |                    |    /
	//               |                    |   /
	//               |                    |  /
	//               |                    | /
	// -Max velocity |                    |/
	//               |
	//               |
	//               ^

	// Clip the horizontal mouse position to the working window
	debugC(6, kDebugEvent, "Mouse pos.x, %d, clipping with %d+1, %d+1", pos.x, workingArea.left, workingArea.right);
	Common::Point clippedPos = pos;
	clippedPos.x = CLIP<int16>(pos.x, workingArea.left + 1, workingArea.right - 1);
	if (workingArea.contains(clippedPos) && !_menu->inMenu()) {
		cursorWasChanged = _scriptManager->onMouseMove(clippedPos, imageCoord);
		RenderTable::RenderState renderState = _renderManager->getRenderTable()->getRenderState();
		switch (renderState) {
		case RenderTable::PANORAMA:
			if (clippedPos.x >= workingArea.left && clippedPos.x < workingArea.left + ROTATION_SCREEN_EDGE_OFFSET) {
				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 25;
				_mouseVelocity  = MIN(((Common::Rational(mspeed, ROTATION_SCREEN_EDGE_OFFSET) * (clippedPos.x - workingArea.left)) - mspeed).toInt(), -1);
				_cursorManager->changeCursor(CursorIndex_Left);
				cursorWasChanged = true;
			} else if (clippedPos.x <= workingArea.right && clippedPos.x > workingArea.right - ROTATION_SCREEN_EDGE_OFFSET) {
				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 25;
				_mouseVelocity  = MAX((Common::Rational(mspeed, ROTATION_SCREEN_EDGE_OFFSET) * (clippedPos.x - workingArea.right + ROTATION_SCREEN_EDGE_OFFSET)).toInt(), 1);
				_cursorManager->changeCursor(CursorIndex_Right);
				cursorWasChanged = true;
			} else
				_mouseVelocity = 0;
			break;
		case RenderTable::TILT:
			if (clippedPos.y >= workingArea.top && clippedPos.y < workingArea.top + ROTATION_SCREEN_EDGE_OFFSET) {
				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 25;
				_mouseVelocity  = MIN(((Common::Rational(mspeed, ROTATION_SCREEN_EDGE_OFFSET) * (pos.y - workingArea.top)) - mspeed).toInt(), -1);
				_cursorManager->changeCursor(CursorIndex_UpArr);
				cursorWasChanged = true;
			} else if (clippedPos.y <= workingArea.bottom && clippedPos.y > workingArea.bottom - ROTATION_SCREEN_EDGE_OFFSET) {
				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 25;
				_mouseVelocity = MAX((Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.y - workingArea.bottom + ROTATION_SCREEN_EDGE_OFFSET)).toInt(), 1);
				_cursorManager->changeCursor(CursorIndex_DownArr);
				cursorWasChanged = true;
			} else
				_mouseVelocity = 0;
			break;
		case RenderTable::FLAT:
		default:
			_mouseVelocity = 0;
			break;
		}
	} else
		_mouseVelocity = 0;
	if (!cursorWasChanged)
		_cursorManager->changeCursor(CursorIndex_Idle);
}

uint8 ZVision::getZvisionKey(Common::KeyCode scummKeyCode) {
	if (scummKeyCode >= Common::KEYCODE_a && scummKeyCode <= Common::KEYCODE_z)
		return 0x41 + scummKeyCode - Common::KEYCODE_a;
	if (scummKeyCode >= Common::KEYCODE_0 && scummKeyCode <= Common::KEYCODE_9)
		return 0x30 + scummKeyCode - Common::KEYCODE_0;
	if (scummKeyCode >= Common::KEYCODE_F1 && scummKeyCode <= Common::KEYCODE_F15)
		return 0x70 + scummKeyCode - Common::KEYCODE_F1;
	if (scummKeyCode >= Common::KEYCODE_KP0 && scummKeyCode <= Common::KEYCODE_KP9)
		return 0x60 + scummKeyCode - Common::KEYCODE_KP0;

	switch (scummKeyCode) {
	case Common::KEYCODE_BACKSPACE:
		return 0x8;
	case Common::KEYCODE_TAB:
		return 0x9;
	case Common::KEYCODE_CLEAR:
		return 0xC;
	case Common::KEYCODE_RETURN:
		return 0xD;
	case Common::KEYCODE_CAPSLOCK:
		return 0x14;
	case Common::KEYCODE_ESCAPE:
		return 0x1B;
	case Common::KEYCODE_SPACE:
		return 0x20;
	case Common::KEYCODE_PAGEUP:
		return 0x21;
	case Common::KEYCODE_PAGEDOWN:
		return 0x22;
	case Common::KEYCODE_END:
		return 0x23;
	case Common::KEYCODE_HOME:
		return 0x24;
	case Common::KEYCODE_LEFT:
		return 0x25;
	case Common::KEYCODE_UP:
		return 0x26;
	case Common::KEYCODE_RIGHT:
		return 0x27;
	case Common::KEYCODE_DOWN:
		return 0x28;
	case Common::KEYCODE_PRINT:
		return 0x2A;
	case Common::KEYCODE_INSERT:
		return 0x2D;
	case Common::KEYCODE_DELETE:
		return 0x2E;
	case Common::KEYCODE_HELP:
		return 0x2F;
	case Common::KEYCODE_KP_MULTIPLY:
		return 0x6A;
	case Common::KEYCODE_KP_PLUS:
		return 0x6B;
	case Common::KEYCODE_KP_MINUS:
		return 0x6D;
	case Common::KEYCODE_KP_PERIOD:
		return 0x6E;
	case Common::KEYCODE_KP_DIVIDE:
		return 0x6F;
	case Common::KEYCODE_NUMLOCK:
		return 0x90;
	case Common::KEYCODE_SCROLLOCK:
		return 0x91;
	case Common::KEYCODE_LSHIFT:
		return 0xA0;
	case Common::KEYCODE_RSHIFT:
		return 0xA1;
	case Common::KEYCODE_LCTRL:
		return 0xA2;
	case Common::KEYCODE_RCTRL:
		return 0xA3;
	case Common::KEYCODE_MENU:
		return 0xA5;
	case Common::KEYCODE_LEFTBRACKET:
		return 0xDB;
	case Common::KEYCODE_RIGHTBRACKET:
		return 0xDD;
	case Common::KEYCODE_SEMICOLON:
		return 0xBA;
	case Common::KEYCODE_BACKSLASH:
		return 0xDC;
	case Common::KEYCODE_QUOTE:
		return 0xDE;
	case Common::KEYCODE_SLASH:
		return 0xBF;
	case Common::KEYCODE_TILDE:
		return 0xC0;
	case Common::KEYCODE_COMMA:
		return 0xBC;
	case Common::KEYCODE_PERIOD:
		return 0xBE;
	case Common::KEYCODE_MINUS:
		return 0xBD;
	case Common::KEYCODE_PLUS:
		return 0xBB;
	default:
		return 0;
	}

	return 0;
}

bool ZVision::quit(bool askFirst, bool streaming) {
	debugC(1, kDebugEvent, "ZVision::quit()");
	if (askFirst)
		if (!_subtitleManager->askQuestion(_stringManager->getTextLine(StringManager::ZVISION_STR_EXITPROMT), streaming, true)) {
			debugC(1, kDebugEvent, "~ZVision::quit()");
			return false;
		}
	//quitGame();
	_breakMainLoop = true;
	debugC(1, kDebugEvent, "~ZVision::quit()");
	return true;
}

} // End of namespace ZVision
