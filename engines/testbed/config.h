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

#ifndef TESTBED_CONFIG_H
#define TESTBED_CONFIG_H


#include "common/array.h"
#include "common/formats/ini-file.h"
#include "common/str-array.h"
#include "common/tokenizer.h"

#include "gui/widgets/list.h"
#include "gui/dialog.h"
#include "gui/ThemeEngine.h"

#include "testbed/testsuite.h"

namespace Testbed {

enum {
	kTestbedQuitCmd = 'Quit',
	kTestbedSelectAll = 'sAll',
	kTestbedDeselectAll = 'dAll'
};



class TestbedConfigManager {
public:
	TestbedConfigManager(Common::Array<Testsuite *> &tList, const Common::String &fName) : _testsuiteList(tList), _configFileName(fName) {}
	~TestbedConfigManager() {}
	void selectTestsuites();
	void setConfigFile(const Common::String &fName) { _configFileName = fName; }
	Common::SeekableReadStream *getConfigReadStream() const;
	Common::WriteStream *getConfigWriteStream() const;
	void writeTestbedConfigToStream(Common::WriteStream *ws);
	Testsuite *getTestsuiteByName(const Common::String &name);
	bool stringToBool(const Common::String &str) { return str.equalsIgnoreCase("true") ? true : false; }
	Common::String boolToString(bool val) { return val ? "true" : "false"; }
	int getNumSuitesEnabled();

private:
	Common::Array<Testsuite *> &_testsuiteList;
	Common::String	_configFileName;
	Common::INIFile	_configFileInterface;
	void parseConfigFile();
};

class TestbedOptionsDialog : public GUI::Dialog {
public:
	TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan);
	~TestbedOptionsDialog() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void reflowLayout() override;

private:
	GUI::ButtonWidget *_selectButton;
	GUI::ButtonWidget *_runTestButton;
	GUI::ButtonWidget *_quitButton;
	GUI::StaticTextWidget *_messageText;
	GUI::ScrollContainerWidget *_testContainerDisplay;
	Common::Array<GUI::CheckboxWidget *> _testSuiteCheckboxArray;

	Common::Array<Testsuite *> _testSuiteArray;
	Common::U32StringArray _testSuiteDescArray;

	TestbedConfigManager *_testbedConfMan;
};

class TestbedInteractionDialog : public GUI::Dialog {
public:
	TestbedInteractionDialog(uint x, uint y, uint w, uint h) : GUI::Dialog(x, y, w, h, true), _xOffset(0), _yOffset(0) {}
	~TestbedInteractionDialog() override {}
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void addButton(uint w, uint h, const Common::String &name, uint32 cmd, uint xOffset = 0, uint yPadding = 8);
	void addButtonXY(uint x, uint y, uint w, uint h, const Common::String &name, uint32 cmd);
	void addText(uint w, uint h, const Common::String &text, Graphics::TextAlign textAlign, uint xOffset, uint yPadding = 8);
	void addList(uint x, uint y, uint w, uint h, const Common::Array<Common::U32String> &strArray, uint yPadding = 8);
protected:
	Common::Array<GUI::ButtonWidget *> _buttonArray;
	uint _xOffset;
	uint _yOffset;

};

} // End of namespace Testbed

#endif // TESTBED_CONFIG_H
