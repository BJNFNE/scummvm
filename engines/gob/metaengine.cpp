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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "gob/gameidtotype.h"
#include "gob/gob.h"
#include "gob/obsolete.h"

// For struct GOBGameDescription.
#include "gob/detection/detection.h"

class GobMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "gob";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine) override {
		Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
		return AdvancedMetaEngine::createInstance(syst, engine);
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

Common::KeymapArray GobMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Gob;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "gob-main", "Gob main");
	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setKeyEvent(KeyState(KEYCODE_MENU));
	act->addDefaultInputMapping("CTRL+M");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause Game"));
	act->setCustomEngineActionEvent(KEYCODE_p);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("PAUSE");
	engineKeyMap->addAction(act);

	act = new Action("DEBUGGER", _("Open Debugger"));
	act->setCustomEngineActionEvent(KEYCODE_d);
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("DEBUGGER");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

Common::Error GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Gob::GOBGameDescription *gd = (const Gob::GOBGameDescription *)desc;
	*engine = new Gob::GobEngine(syst);
	((Gob::GobEngine *)*engine)->initGame(gd);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

GameType GobEngine::getGameType(const char *gameId) const {
	const GameIdToType *gameInfo = gameIdToType;

	while (gameInfo->gameId != nullptr) {
		if (!strcmp(gameId, gameInfo->gameId))
			return gameInfo->gameType;
		gameInfo++;
	}

	error("Unknown game ID: %s", gameId);
}

void GobEngine::initGame(const GOBGameDescription *gd) {
	if (gd->startTotBase == nullptr)
		_startTot = "intro.tot";
	else
		_startTot = gd->startTotBase;

	if (gd->startStkBase == nullptr)
		_startStk = "intro.stk";
	else
		_startStk = gd->startStkBase;

	_demoIndex = gd->demoIndex;

	_gameType = getGameType(gd->desc.gameId);
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;

	_enableAdibou2FreeBananasWorkaround = gd->desc.flags & GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND;
	_enableAdibou2FlowersInfiniteLoopWorkaround = gd->desc.flags & GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND;
}

} // End of namespace Gob
