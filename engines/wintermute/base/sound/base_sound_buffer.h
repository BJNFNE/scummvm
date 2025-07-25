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

#ifndef WINTERMUTE_BASE_SOUNDBUFFER_H
#define WINTERMUTE_BASE_SOUNDBUFFER_H


#include "engines/wintermute/base/base.h"
#include "audio/mixer.h"
#include "common/stream.h"

namespace Audio {
class SeekableAudioStream;
class SoundHandle;
}

namespace Wintermute {

class BaseSoundBuffer : public BaseClass {
public:

	BaseSoundBuffer(BaseGame *inGame);
	~BaseSoundBuffer() override;

	bool pause();
	bool play(bool looping = false, uint32 startSample = 0);
	bool resume();
	bool stop();
	bool isPlaying();

	void setLooping(bool looping);

	uint32 getPosition();
	bool setPosition(uint32 pos);
	uint32 getLength();

	bool setLoopStart(uint32 pos);
	uint32 getLoopStart() const {
		return _loopStart;
	}

	bool setPan(float pan);
	bool setPrivateVolume(int colume);
	bool setVolume(int colume);
	void updateVolume();

	void setType(Audio::Mixer::SoundType Type);
	Audio::Mixer::SoundType getType() const;

	bool loadFromFile(const Common::String &filename, bool forceReload = false);
	void setStreaming(bool streamed, uint32 numBlocks = 0, uint32 blockSize = 0);
	bool applyFX(TSFXType type, float param1, float param2, float param3, float param4);
	int32 getPrivateVolume() const;
	void setFreezePaused(bool freezePaused);
	bool isFreezePaused() const;
	bool isLooping() const;
	//HSTREAM _stream;
	//HSYNC _sync;

private:
	Audio::Mixer::SoundType _type;
	Audio::SeekableAudioStream *_stream;
	Audio::SoundHandle *_handle;
	bool _freezePaused;
	bool _looping;
	int32 _privateVolume;
	uint32 _loopStart;
	uint32 _startPos;
	Common::String _filename;
	bool _streamed;
	int32 _volume;
	int8 _pan;
};

} // End of namespace Wintermute

#endif
