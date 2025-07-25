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

#ifndef LURE_LURE_H
#define LURE_LURE_H

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "common/rect.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/util.h"
#include "common/random.h"

#include "lure/disk.h"
#include "lure/res.h"
#include "lure/screen.h"
#include "lure/events.h"
#include "lure/menu.h"
#include "lure/strings.h"
#include "lure/room.h"
#include "lure/fights.h"
#include "lure/detection.h"

/**
 * This is the namespace of the Lure engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Lure of the Temptress
 */
namespace Lure {

#define RandomNumberGen LureEngine::getReference().rnd()

enum LureLanguage {
	LANG_IT_ITA = 10,
	LANG_FR_FRA = 6,
	LANG_DE_DEU = 7,
	LANG_ES_ESP = 17,
	LANG_EN_ANY = 3,
	LANG_RU_RUS = 5,
	LANG_EN_KONAMI = 4,
	LANG_UNKNOWN = -1
};

enum LUREActions {
	kActionNone,
	kActionSaveGame,
	kActionRestoreGame,
	kActionRestartGame,
	kActionQuitGame,
	kActionEscape,
	kActionFightMoveLeft,
	kActionFightMoveRight,
	kActionFightCursorLeftTop,
	kActionFightCursorLeftMiddle,
	kActionFightCursorLeftBottom,
	kActionFightCursorRightTop,
	kActionFightCursorRightMiddle,
	kActionFightCursorRightBottom,
	kActionIndexNext,
	kActionIndexPrevious,
	kActionIndexSelect,
	kActionYes,
	kActionNo
};

struct LureGameDescription;

class LureEngine : public Engine {
private:
	bool _initialized;
	int _gameToLoad;
	uint8 _saveVersion;
	Disk *_disk;
	Resources *_resources;
	Screen *_screen;
	Mouse *_mouse;
	Events *_events;
	Menu *_menu;
	StringData *_strings;
	Room *_room;
	FightsManager *_fights;
	Common::RandomSource _rnd;

	const char *generateSaveName(int slotNumber);

	const LureGameDescription *_gameDescription;

public:
	LureEngine(OSystem *system, const LureGameDescription *gameDesc);
	~LureEngine() override;
	static LureEngine &getReference();
	bool _saveLoadAllowed;

	// Engine APIs
	Common::Error init();
	Common::Error go();
	Common::Error run() override {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}
	bool hasFeature(EngineFeature f) const override;
	void syncSoundSettings() override;
	void pauseEngineIntern(bool pause) override;

	Disk &disk() { return *_disk; }

	Common::RandomSource &rnd() { return _rnd; }
	int gameToLoad() { return _gameToLoad; }
	bool loadGame(uint8 slotNumber);
	bool saveGame(uint8 slotNumber, Common::String &caption);
	Common::String *detectSave(int slotNumber);
	uint8 saveVersion() { return _saveVersion; }

	uint32 getFeatures() const;
	LureLanguage getLureLanguage() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	bool isEGA() const { return (getFeatures() & GF_EGA) != 0; }
	bool isKonami() const { return (getFeatures() & GF_KONAMI) != 0; }

	Common::Error loadGameState(int slot) override {
		return loadGame(slot) ? Common::kNoError : Common::kReadingFailed;
	}
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override {
		Common::String s(desc);
		return saveGame(slot, s) ? Common::kNoError : Common::kReadingFailed;
	}
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return _saveLoadAllowed && !Fights.isFighting();
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return _saveLoadAllowed && !Fights.isFighting();
	}
};

Common::String getSaveName(Common::InSaveFile *in);

} // End of namespace Lure

#endif
