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

#include "common/translation.h"

#include "dev7/metaengine.h"
#include "dev7/detection.h"
#include "dev7/dev7.h"

namespace Dev7 {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Dev7

const char *Dev7MetaEngine::getName() const {
	return "dev7";
}

const ADExtraGuiOptionsMap *Dev7MetaEngine::getAdvancedExtraGuiOptions() const {
	return Dev7::optionsList;
}

Common::Error Dev7MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Dev7::Dev7Engine(syst, desc);
	return Common::kNoError;
}

bool Dev7MetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(DEV7)
REGISTER_PLUGIN_DYNAMIC(DEV7, PLUGIN_TYPE_ENGINE, Dev7MetaEngine);
#else
REGISTER_PLUGIN_STATIC(DEV7, PLUGIN_TYPE_ENGINE, Dev7MetaEngine);
#endif
