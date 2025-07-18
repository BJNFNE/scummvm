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

// MIDI music class

#include "made/music.h"
#include "made/redreader.h"
#include "made/resource.h"

#include "audio/adlib_ms.h"
#include "audio/midiparser.h"
#include "audio/miles.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/stream.h"

namespace Made {

const uint8 DOSMusicPlayer::MT32_GOODBYE_MSG[] = { 0x52, 0x65, 0x74, 0x75, 0x72, 0x6E, 0x20, 0x54, 0x6F, 0x20, 0x5A, 0x6F, 0x72, 0x6B, 0x20, 0x53, 0x6F, 0x6F, 0x6E, 0x21 };

DOSMusicPlayer::DOSMusicPlayer(MadeEngine *vm, bool milesAudio) : _vm(vm), _parser(nullptr) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	_driverType = MidiDriver::getMusicType(dev);
	if (_driverType == MT_GM && ConfMan.getBool("native_mt32"))
		_driverType = MT_MT32;

	Common::SeekableReadStream *adLibInstrumentStream = nullptr;
	switch (_driverType) {
	case MT_ADLIB:
		if (milesAudio) {
			if (Common::File::exists("rtzcd.red")) {
				// Installing Return to Zork produces both a SAMPLE.AD and
				// a SAMPLE.OPL file, but they are identical. The resource
				// file appears to only contain SAMPLE.AD.
				adLibInstrumentStream = RedReader::loadFromRed("rtzcd.red", "SAMPLE.AD");
			}
			_driver = Audio::MidiDriver_Miles_AdLib_create("SAMPLE.AD", "SAMPLE.OPL", adLibInstrumentStream);
			delete adLibInstrumentStream;
		} else {
			_driver = new MidiDriver_ADLIB_MADE(OPL::Config::kOpl2);
		}
		break;
	case MT_GM:
	case MT_MT32:
		if (milesAudio) {
			_driver = Audio::MidiDriver_Miles_MIDI_create(MT_MT32, "");
		} else {
			_driver = new MidiDriver_MT32GM(MT_MT32);
		}
		break;
	default:
		_driver = new MidiDriver_NULL_Multisource();
		break;
	}

	if (_driver) {
		_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
		if (_driver->open() != 0)
			error("Failed to open MIDI driver.");

		_driver->setTimerCallback(this, &timerCallback);
	}

	syncSoundSettings();
}

DOSMusicPlayer::~DOSMusicPlayer() {
	if (_parser) {
		_parser->stopPlaying();
		delete _parser;
	}
	if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
		delete _driver;
	}
}

void DOSMusicPlayer::close() {
	if (_parser)
		_parser->stopPlaying();

	if (_vm->getGameID() == GID_RTZ && _vm->getPlatform() == Common::kPlatformDOS && _driver) {
		MidiDriver_MT32GM *mt32Driver = dynamic_cast<MidiDriver_MT32GM *>(_driver);
		if (mt32Driver)
			mt32Driver->sysExMT32(MT32_GOODBYE_MSG, MidiDriver_MT32GM::MT32_DISPLAY_NUM_CHARS,
				MidiDriver_MT32GM::MT32_DISPLAY_MEMORY_ADDRESS, false, false);
	}
}

bool DOSMusicPlayer::load(int16 musicNum) {
	GenericResource *xmidi = _vm->_res->getXmidi(musicNum);
	if (xmidi) {
		_vm->_res->freeResource(xmidi);
		return true;
	}
	return false;
}

void DOSMusicPlayer::play(int16 musicNum) {
	if (_vm->getGameID() == GID_RTZ) {
		if (musicNum > 0) {
			_musicRes = _vm->_res->getXmidi(musicNum);
			if (_musicRes)
				playXMIDI(_musicRes);
		}
	} else {
		// HACK: music number 2 in LGOP2 is file MT32SET.TON, which
		// is used to set the MT32 instruments. This is not loaded
		// correctly and the game freezes, and since we don't support
		// MT32 music yet, we ignore it here
		// FIXME: Remove this hack and handle this file properly
		if (_vm->getGameID() == GID_LGOP2 && musicNum == 2)
			return;
		if (musicNum > 0) {
			_musicRes = _vm->_res->getMidi(musicNum);
			if (_musicRes)
				playSMF(_musicRes);
		}
	}
}

void DOSMusicPlayer::playXMIDI(GenericResource *midiResource) {
	if (_parser) {
		_parser->unloadMusic();
	} else {
		_parser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);

		_parser->setMidiDriver(_driver);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);
	}

	// Load XMID resource data

	_parser->loadMusic(midiResource->getData(), midiResource->getSize());
}

void DOSMusicPlayer::playSMF(GenericResource *midiResource) {
	if (_parser) {
		_parser->unloadMusic();
	} else {
		_parser = MidiParser::createParser_SMF(0);

		_parser->setMidiDriver(_driver);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	}

	// Load MIDI resource data

	_parser->loadMusic(midiResource->getData(), midiResource->getSize());
}

void DOSMusicPlayer::pause() {
	if (_parser)
		_parser->pausePlaying();
}

void DOSMusicPlayer::resume() {
	if (_parser)
		_parser->resumePlaying();
}

void DOSMusicPlayer::stop() {
	if (_parser)
		_parser->stopPlaying();

	if (_musicRes) {
		_vm->_res->freeResource(_musicRes);
		_musicRes = nullptr;
	}
}

bool DOSMusicPlayer::isPlaying() {
	return _parser ? _parser->isPlaying() : false;
}

void DOSMusicPlayer::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

void DOSMusicPlayer::onTimer() {
	if (_parser)
		_parser->onTimer();
}

void DOSMusicPlayer::timerCallback(void *data) {
	((DOSMusicPlayer *)data)->onTimer();
}

MidiDriver_ADLIB_MADE::MidiDriver_ADLIB_MADE(OPL::Config::OplType oplType) : MidiDriver_ADLIB_Multisource(oplType) {
	_modulationDepth = MODULATION_DEPTH_LOW;
	_vibratoDepth = VIBRATO_DEPTH_LOW;
	_defaultChannelVolume = 0x7F;
}

} // End of namespace Made
