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


#include "audio/midiparser.h"
#include "common/textconsole.h"

namespace Scumm {

//////////////////////////////////////////////////
//
// The Standard MIDI File version of MidiParser
//
//////////////////////////////////////////////////

class MidiParser_RO : public MidiParser {
protected:
	int _markerCount = 0;     // Number of markers encountered in stream so far
	int _lastMarkerCount = 0; // Cache markers until parsed event is actually consumed

	void parseNextEvent (EventInfo &info) override;

public:
	bool loadMusic (byte *data, uint32 size) override;
	uint32 getTick() override { return (uint32) _markerCount * _ppqn / 2; }
};



//////////////////////////////////////////////////
//
// MidiParser_RO implementation
//
//////////////////////////////////////////////////

void MidiParser_RO::parseNextEvent (EventInfo &info) {
	_markerCount += _lastMarkerCount;
	_lastMarkerCount = 0;

	byte *playPos = _position._subtracks[0]._playPos;

	info.delta = 0;
	do {
		info.start = playPos;
		info.event = *(playPos++);
		if (info.command() == 0xA) {
			++_lastMarkerCount;
			info.event = 0xF0;
		} else if (info.event == 0xF0 || info.event == 0xF1) {
			byte delay = *(playPos++);
			info.delta += delay;
			if (info.event == 0xF1) {
				// This event is, as far as we have been able
				// to determine, only used in one single song
				// in EGA Loom. It seems to be meant for adding
				// values greater than 255 to info.delta. See
				// bug #2650.
				info.delta += 256;
			}
			continue;
		}
		break;
	} while (true);

	_position._subtracks[0]._playPos = playPos;

	// Seems to indicate EOT
	if (info.event == 0) {
		info.event = 0xFF;
		info.ext.type = 0x2F;
		info.length = 0;
		info.ext.data = nullptr;
		return;
	}

	if (info.event < 0x80)
		return;

	_position._subtracks[0]._runningStatus = info.event;
	switch (info.command()) {
	case 0xC:
		info.basic.param1 = *(playPos++);
		info.basic.param2 = 0;
		break;

	case 0x8: case 0x9: case 0xB:
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		if (info.command() == 0x9 && info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xF: // Marker and EOT messages
		info.length = 0;
		info.ext.data = nullptr;
		if (info.event == 0xFF) {
			_autoLoop = true;
			info.ext.type = 0x2F;
		} else {
			info.ext.type = 0x7F; // Bogus META
		}
		info.event = 0xFF;
		break;

	default:
		break;
	}

	_position._subtracks[0]._playPos = playPos;
}

bool MidiParser_RO::loadMusic (byte *data, uint32 size) {
	unloadMusic();
	byte *pos = data;

	if (memcmp (pos, "RO", 2)) {
		error("'RO' header expected but found '%c%c' instead", pos[0], pos[1]);
		return false;
	}

	_numTracks = 1;
	_numSubtracks[0] = 1;
	_autoLoop = false;
	_ppqn = 120;
	_tracks[0][0] = pos + 2;
	_markerCount = _lastMarkerCount = 0;

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	resetTracking();
	setTempo (500000);
	setTrack (0);
	return true;
}

MidiParser *MidiParser_createRO() { return new MidiParser_RO; }

} // End of namespace Scumm
