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
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/interface.h"
#include "saga/render.h"
#include "saga/scene.h"
#include "saga/script.h"
#include "saga/isomap.h"

#include "common/events.h"
#include "common/system.h"

namespace Saga {

int SagaEngine::processInput() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		// Scale down mouse coordinates for the Japanese version which runs in double resolution internally.
		if ((event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN ||
			event.type == Common::EVENT_WHEELUP || event.type == Common::EVENT_WHEELDOWN) && getLanguage() == Common::JA_JPN) {
				event.mouse.x >>= 1;
				event.mouse.y >>= 1;
		}

		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kActionPause:
				_render->toggleFlag(RF_RENDERPAUSE);
				break;
			case kActionAbortSpeech:
				_actor->abortSpeech();
				break;
			case kActionBossKey:
				_interface->keyBoss();
				break;
			case kActionShowDialogue:
				_interface->draw();
				break;
			case kActionOptions:
				if (_interface->getSaveReminderState() > 0)
					_interface->setMode(kPanelOption);
				break;
			default:
				_interface->processAscii(event.kbd, event.customType);
				break;
			};

			break;
		case Common::EVENT_KEYDOWN:
			if (_interface->_textInput || _interface->_statusTextInput) {
				_interface->processAscii(event.kbd, event.customType);
				return SUCCESS;
			}

#ifdef SAGA_DEBUG
			switch (event.kbd.keycode) {
#if 0
			case Common::KEYCODE_KP_MINUS:
			case Common::KEYCODE_KP_PLUS:
			case Common::KEYCODE_UP:
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_LEFT:
				if (_vm->_scene->getFlags() & kSceneFlagISO) {
					_vm->_isoMap->_viewDiff += (event.kbd.keycode == Common::KEYCODE_KP_PLUS) - (event.kbd.keycode == Common::KEYCODE_KP_MINUS);
					_vm->_isoMap->_viewScroll.y += (_vm->_isoMap->_viewDiff * (event.kbd.keycode == Common::KEYCODE_DOWN) - _vm->_isoMap->_viewDiff * (event.kbd.keycode == Common::KEYCODE_UP));
					_vm->_isoMap->_viewScroll.x += (_vm->_isoMap->_viewDiff * (event.kbd.keycode == Common::KEYCODE_RIGHT) - _vm->_isoMap->_viewDiff * (event.kbd.keycode == Common::KEYCODE_LEFT));
				}
				break;
#endif

			case Common::KEYCODE_F1:
				_render->toggleFlag(RF_SHOW_FPS);
				_actor->_handleActionDiv = (_actor->_handleActionDiv == 15) ? 50 : 15;
				break;
			case Common::KEYCODE_F2:
				_render->toggleFlag(RF_PALETTE_TEST);
				break;
			case Common::KEYCODE_F3:
				_render->toggleFlag(RF_TEXT_TEST);
				break;
			case Common::KEYCODE_F4:
				_render->toggleFlag(RF_OBJECTMAP_TEST);
				break;
			case Common::KEYCODE_F6:
				_render->toggleFlag(RF_ACTOR_PATH_TEST);
				break;
			case Common::KEYCODE_F7:
				//_actor->frameTest();
				break;
			case Common::KEYCODE_F8:
				break;
			default:
				_interface->processAscii(event.kbd, event.customType);
				break;
			}
#else
			_interface->processAscii(event.kbd, event.customType);
#endif
			break;
		case Common::EVENT_LBUTTONUP:
			_leftMouseButtonPressed = false;
			break;
		case Common::EVENT_RBUTTONUP:
			_rightMouseButtonPressed = false;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftMouseButtonPressed = true;
			_interface->update(event.mouse, UPDATE_LEFTBUTTONCLICK);
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightMouseButtonPressed = true;
			_interface->update(event.mouse, UPDATE_RIGHTBUTTONCLICK);
			break;
		case Common::EVENT_WHEELUP:
			_interface->update(event.mouse, UPDATE_WHEELUP);
			break;
		case Common::EVENT_WHEELDOWN:
			_interface->update(event.mouse, UPDATE_WHEELDOWN);
			break;
		case Common::EVENT_MOUSEMOVE:
			break;
		default:
			break;
		}

		Common::Keymapper *keymapper = SagaEngine::getEventManager()->getKeymapper();
		if (_interface->_textInput || _interface->_statusTextInput) {
			keymapper->getKeymap("game-shortcuts")->setEnabled(false);
			keymapper->getKeymap("save-panel")->setEnabled(false);
		} else {
			keymapper->getKeymap("game-shortcuts")->setEnabled(true);
			keymapper->getKeymap("save-panel")->setEnabled(true);
		}

		enableKeyMap(_interface->getMode());
	}

	return SUCCESS;
}

Point SagaEngine::mousePos() const {
	Common::Point pos = _eventMan->getMousePos();
	// Scale down mouse coordinates for the Japanese version which runs in double resolution internally.
	if (getLanguage() == Common::JA_JPN) {
		pos.x >>= 1;
		pos.y >>= 1;
	}

	return pos;
}

} // End of namespace Saga
