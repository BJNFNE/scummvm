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

#include "image/codecs/cinepak.h"
#include "image/codecs/cinepak_tables.h"
#include "image/codecs/dither.h"

#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/surface.h"

// Code here partially based off of ffmpeg ;)

namespace Image {

namespace {

inline void convertYUVToRGB(const byte *clipTable, byte y, int8 u, int8 v, byte &r, byte &g, byte &b) {
	r = clipTable[y + (v * 2)];
	g = clipTable[y - (u >> 1) - v];
	b = clipTable[y + (u * 2)];
}

inline uint32 convertYUVToColor(const byte *clipTable, const Graphics::PixelFormat &format, byte y, int8 u, int8 v) {
	byte r, g, b;
	convertYUVToRGB(clipTable, y, u, v, r, g, b);
	return format.RGBToColor(r, g, b);
}

inline uint16 createDitherTableIndex(const byte *clipTable, byte y, int8 u, int8 v) {
	byte r, g, b;
	convertYUVToRGB(clipTable, y, u, v, r, g, b);
	return ((r & 0xF8) << 6) |
	       ((g & 0xF8) << 1) |
	       ((b & 0xF0) >> 4);
}

/**
 * The default codebook converter for 24bpp: RGB output.
 */
struct CodebookConverterRGB {
	template<typename PixelInt>
	static inline void decodeBlock1(byte codebookIndex, const CinepakStrip &strip, PixelInt *dst, size_t dstPitch, const byte *clipTable, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook = strip.v1_codebook[codebookIndex];
		const int8 u = codebook.u, v = codebook.v;

		const PixelInt rgb0 = convertYUVToColor(clipTable, format, codebook.y[0], u, v);
		const PixelInt rgb1 = convertYUVToColor(clipTable, format, codebook.y[1], u, v);

		dst[0] = dst[1] = rgb0;
		dst[2] = dst[3] = rgb1;
		dst = (PixelInt *)((uint8 *)dst + dstPitch);

		dst[0] = dst[1] = rgb0;
		dst[2] = dst[3] = rgb1;
		dst = (PixelInt *)((uint8 *)dst + dstPitch);

		const PixelInt rgb2 = convertYUVToColor(clipTable, format, codebook.y[2], u, v);
		const PixelInt rgb3 = convertYUVToColor(clipTable, format, codebook.y[3], u, v);

		dst[0] = dst[1] = rgb2;
		dst[2] = dst[3] = rgb3;
		dst = (PixelInt *)((uint8 *)dst + dstPitch);

		dst[0] = dst[1] = rgb2;
		dst[2] = dst[3] = rgb3;
		dst = (PixelInt *)((uint8 *)dst + dstPitch);
	}

	template<typename PixelInt>
	static inline void decodeBlock4(const byte (&codebookIndex)[4], const CinepakStrip &strip, PixelInt *dst, size_t dstPitch, const byte *clipTable, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook1 = strip.v4_codebook[codebookIndex[0]];
		const CinepakCodebook &codebook2 = strip.v4_codebook[codebookIndex[1]];

		dst[0] = convertYUVToColor(clipTable, format, codebook1.y[0], codebook1.u, codebook1.v);
		dst[1] = convertYUVToColor(clipTable, format, codebook1.y[1], codebook1.u, codebook1.v);
		dst[2] = convertYUVToColor(clipTable, format, codebook2.y[0], codebook2.u, codebook2.v);
		dst[3] = convertYUVToColor(clipTable, format, codebook2.y[1], codebook2.u, codebook2.v);
		dst = (PixelInt *)((uint8 *)dst + dstPitch);

		dst[0] = convertYUVToColor(clipTable, format, codebook1.y[2], codebook1.u, codebook1.v);
		dst[1] = convertYUVToColor(clipTable, format, codebook1.y[3], codebook1.u, codebook1.v);
		dst[2] = convertYUVToColor(clipTable, format, codebook2.y[2], codebook2.u, codebook2.v);
		dst[3] = convertYUVToColor(clipTable, format, codebook2.y[3], codebook2.u, codebook2.v);
		dst = (PixelInt *)((uint8 *)dst + dstPitch);

		const CinepakCodebook &codebook3 = strip.v4_codebook[codebookIndex[2]];
		const CinepakCodebook &codebook4 = strip.v4_codebook[codebookIndex[3]];

		dst[0] = convertYUVToColor(clipTable, format, codebook3.y[0], codebook3.u, codebook3.v);
		dst[1] = convertYUVToColor(clipTable, format, codebook3.y[1], codebook3.u, codebook3.v);
		dst[2] = convertYUVToColor(clipTable, format, codebook4.y[0], codebook4.u, codebook4.v);
		dst[3] = convertYUVToColor(clipTable, format, codebook4.y[1], codebook4.u, codebook4.v);
		dst = (PixelInt *)((uint8 *)dst + dstPitch);

		dst[0] = convertYUVToColor(clipTable, format, codebook3.y[2], codebook3.u, codebook3.v);
		dst[1] = convertYUVToColor(clipTable, format, codebook3.y[3], codebook3.u, codebook3.v);
		dst[2] = convertYUVToColor(clipTable, format, codebook4.y[2], codebook4.u, codebook4.v);
		dst[3] = convertYUVToColor(clipTable, format, codebook4.y[3], codebook4.u, codebook4.v);
		dst = (PixelInt *)((uint8 *)dst + dstPitch);
	}
};

/**
 * The default codebook converter for 8bpp: palettized output.
 */
struct CodebookConverterPalette {
	static inline void decodeBlock1(byte codebookIndex, const CinepakStrip &strip, byte *dst, size_t dstPitch, const byte *clipTable, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook = strip.v1_codebook[codebookIndex];

		byte y0 = codebook.y[0], y1 = codebook.y[1];
		byte y2 = codebook.y[2], y3 = codebook.y[3];

		dst[0] = dst[1] = y0;
		dst[2] = dst[3] = y1;
		dst += dstPitch;

		dst[0] = dst[1] = y0;
		dst[2] = dst[3] = y1;
		dst += dstPitch;

		dst[0] = dst[1] = y2;
		dst[2] = dst[3] = y3;
		dst += dstPitch;

		dst[0] = dst[1] = y2;
		dst[2] = dst[3] = y3;
		dst += dstPitch;
	}

	static inline void decodeBlock4(const byte (&codebookIndex)[4], const CinepakStrip &strip, byte *dst, size_t dstPitch, const byte *clipTable, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook1 = strip.v4_codebook[codebookIndex[0]];
		const CinepakCodebook &codebook2 = strip.v4_codebook[codebookIndex[1]];

		dst[0] = codebook1.y[0];
		dst[1] = codebook1.y[1];
		dst[2] = codebook2.y[0];
		dst[3] = codebook2.y[1];
		dst += dstPitch;

		dst[0] = codebook1.y[2];
		dst[1] = codebook1.y[3];
		dst[2] = codebook2.y[2];
		dst[3] = codebook2.y[3];
		dst += dstPitch;

		const CinepakCodebook &codebook3 = strip.v4_codebook[codebookIndex[2]];
		const CinepakCodebook &codebook4 = strip.v4_codebook[codebookIndex[3]];

		dst[0] = codebook3.y[0];
		dst[1] = codebook3.y[1];
		dst[2] = codebook4.y[0];
		dst[3] = codebook4.y[1];
		dst += dstPitch;

		dst[0] = codebook3.y[2];
		dst[1] = codebook3.y[3];
		dst[2] = codebook4.y[2];
		dst[3] = codebook4.y[3];
		dst += dstPitch;
	}
};

/**
 * Codebook converter that dithers in QT-style or VFW-style
 */
struct CodebookConverterDithered {
	static inline void decodeBlock1(byte codebookIndex, const CinepakStrip &strip, byte *dst, size_t dstPitch, const byte *clipTable, const Graphics::PixelFormat &format) {
		const uint32 *colorPtr = (const uint32 *)(strip.v1_dither + codebookIndex);

		*((uint32 *)dst) = colorPtr[0];
		dst += dstPitch;

		*((uint32 *)dst) = colorPtr[256];
		dst += dstPitch;

		*((uint32 *)dst) = colorPtr[512];
		dst += dstPitch;

		*((uint32 *)dst) = colorPtr[768];
		dst += dstPitch;
	}

	static inline void decodeBlock4(const byte (&codebookIndex)[4], const CinepakStrip &strip, byte *dst, size_t dstPitch, const byte *clipTable, const Graphics::PixelFormat &format) {
		const uint16 *colorPtr1 = (const uint16 *)(strip.v4_dither + codebookIndex[0]);
		const uint16 *colorPtr2 = (const uint16 *)(strip.v4_dither + codebookIndex[1]);

		*((uint16 *)(dst + 0)) = colorPtr1[0];
		*((uint16 *)(dst + 2)) = colorPtr2[512];
		dst += dstPitch;

		*((uint16 *)(dst + 0)) = colorPtr1[1];
		*((uint16 *)(dst + 2)) = colorPtr2[513];
		dst += dstPitch;

		const uint16 *colorPtr3 = (const uint16 *)(strip.v4_dither + codebookIndex[2]);
		const uint16 *colorPtr4 = (const uint16 *)(strip.v4_dither + codebookIndex[3]);

		*((uint16 *)(dst + 0)) = colorPtr3[1024];
		*((uint16 *)(dst + 2)) = colorPtr4[1536];
		dst += dstPitch;

		*((uint16 *)(dst + 0)) = colorPtr3[1025];
		*((uint16 *)(dst + 2)) = colorPtr4[1537];
		dst += dstPitch;
	}
};

template<typename PixelInt, typename CodebookConverter>
void decodeVectorsTmpl(CinepakFrame &frame, const byte *clipTable, Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	uint32 flag = 0, mask = 0;
	int32 startPos = stream.pos();
	PixelInt *dst;

	for (uint16 y = frame.strips[strip].rect.top; y < frame.strips[strip].rect.bottom; y += 4) {
		dst = (PixelInt *)frame.surface->getBasePtr(frame.strips[strip].rect.left, + y);

		for (uint16 x = frame.strips[strip].rect.left; x < frame.strips[strip].rect.right; x += 4) {
			if ((chunkID & 0x01) && !(mask >>= 1)) {
				if ((stream.pos() - startPos + 4) > (int32)chunkSize)
					return;

				flag  = stream.readUint32BE();
				mask  = 0x80000000;
			}

			if (!(chunkID & 0x01) || (flag & mask)) {
				if (!(chunkID & 0x02) && !(mask >>= 1)) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					flag  = stream.readUint32BE();
					mask  = 0x80000000;
				}

				if ((chunkID & 0x02) || (~flag & mask)) {
					if ((stream.pos() - startPos + 1) > (int32)chunkSize)
						return;

					// Get the codebook
					byte codebook = stream.readByte();
					CodebookConverter::decodeBlock1(codebook, frame.strips[strip], dst, frame.surface->pitch, clipTable, frame.surface->format);
				} else if (flag & mask) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					byte codebook[4];
					stream.read(codebook, 4);
					CodebookConverter::decodeBlock4(codebook, frame.strips[strip], dst, frame.surface->pitch, clipTable, frame.surface->format);
				}
			}

			dst += 4;
		}
	}
}

} // End of anonymous namespace

CinepakDecoder::CinepakDecoder(int bitsPerPixel) : Codec(), _bitsPerPixel(bitsPerPixel), _ditherPalette(0) {
	_curFrame.surface = 0;
	_curFrame.strips = 0;
	_y = 0;
	_colorMap = 0;
	_ditherType = kDitherTypeUnknown;

	if (bitsPerPixel == 8) {
		_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	} else {
		_pixelFormat = getDefaultYUVFormat();
	}

	// Create a lookup for the clip function
	// This dramatically improves the performance of the color conversion
	_clipTableBuf = new byte[1024];

	for (uint i = 0; i < 1024; i++) {
		if (i <= 512)
			_clipTableBuf[i] = 0;
		else if (i >= 768)
			_clipTableBuf[i] = 255;
		else
			_clipTableBuf[i] = i - 512;
	}

	_clipTable = _clipTableBuf + 512;
}

CinepakDecoder::~CinepakDecoder() {
	if (_curFrame.surface) {
		_curFrame.surface->free();
		delete _curFrame.surface;
	}

	delete[] _curFrame.strips;
	delete[] _clipTableBuf;

	delete[] _colorMap;
}

const Graphics::Surface *CinepakDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	_curFrame.flags = stream.readByte();
	_curFrame.length = (stream.readByte() << 16);
	_curFrame.length |= stream.readUint16BE();
	_curFrame.width = stream.readUint16BE();
	_curFrame.height = stream.readUint16BE();
	_curFrame.stripCount = stream.readUint16BE();

	if (!_curFrame.strips) {
		_curFrame.strips = new CinepakStrip[_curFrame.stripCount];
		for (uint16 i = 0; i < _curFrame.stripCount; i++) {
			initializeCodebook(i, 1);
			initializeCodebook(i, 4);
		}
	}

	debug(4, "Cinepak Frame: Width = %d, Height = %d, Strip Count = %d", _curFrame.width, _curFrame.height, _curFrame.stripCount);

	// Borrowed from FFMPEG. This should cut out the extra data Cinepak for Sega has (which is useless).
	// The theory behind this is that this is here to confuse standard Cinepak decoders. But, we won't let that happen! ;)
	if (_curFrame.length != (uint32)stream.size()) {
		if (stream.readUint16BE() == 0xFE00)
			stream.readUint32BE();
		else if ((stream.size() % _curFrame.length) == 0)
			stream.seek(-2, SEEK_CUR);
	}

	if (!_curFrame.surface) {
		_curFrame.surface = new Graphics::Surface();
		_curFrame.surface->create(_curFrame.width, _curFrame.height, _pixelFormat);
	}

	_y = 0;

	for (uint16 i = 0; i < _curFrame.stripCount; i++) {
		if (i > 0 && !(_curFrame.flags & 1)) { // Use codebooks from last strip

			for (uint16 j = 0; j < 256; j++) {
				_curFrame.strips[i].v1_codebook[j] = _curFrame.strips[i - 1].v1_codebook[j];
				_curFrame.strips[i].v4_codebook[j] = _curFrame.strips[i - 1].v4_codebook[j];
			}

			// Copy the dither tables
			memcpy(_curFrame.strips[i].v1_dither, _curFrame.strips[i - 1].v1_dither, 256 * 4 * 4 * sizeof(uint32));
			memcpy(_curFrame.strips[i].v4_dither, _curFrame.strips[i - 1].v4_dither, 256 * 4 * 4 * sizeof(uint32));
		}

		_curFrame.strips[i].id = stream.readUint16BE();
		_curFrame.strips[i].length = stream.readUint16BE() - 12; // Subtract the 12 byte header
		_curFrame.strips[i].rect.top = _y; stream.readUint16BE(); // Ignore, substitute with our own.
		_curFrame.strips[i].rect.left = 0; stream.readUint16BE(); // Ignore, substitute with our own
		_curFrame.strips[i].rect.bottom = _y + stream.readUint16BE();
		_curFrame.strips[i].rect.right = _curFrame.width; stream.readUint16BE(); // Ignore, substitute with our own

		// Sanity check. Because Cinepak is based on 4x4 blocks, the width and height of each strip needs to be divisible by 4.
		assert(!(_curFrame.strips[i].rect.width() % 4) && !(_curFrame.strips[i].rect.height() % 4));

		uint32 pos = stream.pos();

		while ((uint32)stream.pos() < (pos + _curFrame.strips[i].length) && !stream.eos()) {
			byte chunkID = stream.readByte();

			if (stream.eos())
				break;

			// Chunk Size is 24-bit, ignore the first 4 bytes
			uint32 chunkSize = stream.readByte() << 16;
			chunkSize += stream.readUint16BE() - 4;

			int32 startPos = stream.pos();

			switch (chunkID) {
			case 0x20:
			case 0x21:
			case 0x24:
			case 0x25:
				loadCodebook(stream, i, 4, chunkID, chunkSize);
				break;
			case 0x22:
			case 0x23:
			case 0x26:
			case 0x27:
				loadCodebook(stream, i, 1, chunkID, chunkSize);
				break;
			case 0x30:
			case 0x31:
			case 0x32:
				if (_ditherPalette.size() > 0)
					ditherVectors(stream, i, chunkID, chunkSize);
				else if (_bitsPerPixel == 8)
					decodeVectors8(stream, i, chunkID, chunkSize);
				else
					decodeVectors24(stream, i, chunkID, chunkSize);
				break;
			default:
				warning("Unknown Cinepak chunk ID %02x", chunkID);
				return _curFrame.surface;
			}

			if (stream.pos() != startPos + (int32)chunkSize)
				stream.seek(startPos + chunkSize);
		}

		_y = _curFrame.strips[i].rect.bottom;
	}

	return _curFrame.surface;
}

void CinepakDecoder::initializeCodebook(uint16 strip, byte codebookType) {
	CinepakCodebook *codebook = (codebookType == 1) ? _curFrame.strips[strip].v1_codebook : _curFrame.strips[strip].v4_codebook;

	for (uint16 i = 0; i < 256; i++) {
		memset(codebook[i].y, 0, 4);
		codebook[i].u = 0;
		codebook[i].v = 0;

		if (_ditherType == kDitherTypeQT)
			ditherCodebookQT(strip, codebookType, i);
		else if (_ditherType == kDitherTypeVFW)
			ditherCodebookVFW(strip, codebookType, i);
	}
}

void CinepakDecoder::loadCodebook(Common::SeekableReadStream &stream, uint16 strip, byte codebookType, byte chunkID, uint32 chunkSize) {
	CinepakCodebook *codebook = (codebookType == 1) ? _curFrame.strips[strip].v1_codebook : _curFrame.strips[strip].v4_codebook;

	int32 startPos = stream.pos();
	uint32 flag = 0, mask = 0;

	for (uint16 i = 0; i < 256; i++) {
		if ((chunkID & 0x01) && !(mask >>= 1)) {
			if ((stream.pos() - startPos + 4) > (int32)chunkSize)
				break;

			flag  = stream.readUint32BE();
			mask  = 0x80000000;
		}

		if (!(chunkID & 0x01) || (flag & mask)) {
			byte n = (chunkID & 0x04) ? 4 : 6;
			if ((stream.pos() - startPos + n) > (int32)chunkSize)
				break;

			stream.read(codebook[i].y, 4);

			if (n == 6) {
				codebook[i].u = stream.readSByte();
				codebook[i].v = stream.readSByte();
			} else {
				// This codebook type indicates either greyscale or
				// palettized video. For greyscale, default us to
				// 0 for both u and v.
				codebook[i].u = 0;
				codebook[i].v = 0;
			}

			// Dither the codebook if we're dithering for QuickTime
			if (_ditherType == kDitherTypeQT)
				ditherCodebookQT(strip, codebookType, i);
			else if (_ditherType == kDitherTypeVFW)
				ditherCodebookVFW(strip, codebookType, i);
		}
	}
}

void CinepakDecoder::ditherCodebookQT(uint16 strip, byte codebookType, uint16 codebookIndex) {
	if (codebookType == 1) {
		const CinepakCodebook &codebook = _curFrame.strips[strip].v1_codebook[codebookIndex];
		byte *output = (byte *)(_curFrame.strips[strip].v1_dither + codebookIndex);

		byte *ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[0], codebook.u, codebook.v);
		output[0x000] = ditherEntry[0x0000];
		output[0x001] = ditherEntry[0x4000];
		output[0x400] = ditherEntry[0xC000];
		output[0x401] = ditherEntry[0x0000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[1], codebook.u, codebook.v);
		output[0x002] = ditherEntry[0x8000];
		output[0x003] = ditherEntry[0xC000];
		output[0x402] = ditherEntry[0x4000];
		output[0x403] = ditherEntry[0x8000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[2], codebook.u, codebook.v);
		output[0x800] = ditherEntry[0x4000];
		output[0x801] = ditherEntry[0x8000];
		output[0xC00] = ditherEntry[0x8000];
		output[0xC01] = ditherEntry[0xC000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[3], codebook.u, codebook.v);
		output[0x802] = ditherEntry[0xC000];
		output[0x803] = ditherEntry[0x0000];
		output[0xC02] = ditherEntry[0x0000];
		output[0xC03] = ditherEntry[0x4000];
	} else {
		const CinepakCodebook &codebook = _curFrame.strips[strip].v4_codebook[codebookIndex];
		byte *output = (byte *)(_curFrame.strips[strip].v4_dither + codebookIndex);

		byte *ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[0], codebook.u, codebook.v);
		output[0x000] = ditherEntry[0x0000];
		output[0x400] = ditherEntry[0x8000];
		output[0x800] = ditherEntry[0x4000];
		output[0xC00] = ditherEntry[0xC000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[1], codebook.u, codebook.v);
		output[0x001] = ditherEntry[0x4000];
		output[0x401] = ditherEntry[0xC000];
		output[0x801] = ditherEntry[0x8000];
		output[0xC01] = ditherEntry[0x0000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[2], codebook.u, codebook.v);
		output[0x002] = ditherEntry[0xC000];
		output[0x402] = ditherEntry[0x4000];
		output[0x802] = ditherEntry[0x8000];
		output[0xC02] = ditherEntry[0x0000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[3], codebook.u, codebook.v);
		output[0x003] = ditherEntry[0x0000];
		output[0x403] = ditherEntry[0x8000];
		output[0x803] = ditherEntry[0xC000];
		output[0xC03] = ditherEntry[0x4000];
	}
}

static inline byte getRGBLookupEntry(const byte *colorMap, uint16 index) {
	return colorMap[CLIP<int>(index, 0, 1023)];
}

void CinepakDecoder::ditherCodebookVFW(uint16 strip, byte codebookType, uint16 codebookIndex) {
	if (codebookType == 1) {
		const CinepakCodebook &codebook = _curFrame.strips[strip].v1_codebook[codebookIndex];
		byte *output = (byte *)(_curFrame.strips[strip].v1_dither + codebookIndex);

		int uLookup = (byte)codebook.u * 2;
		int vLookup = (byte)codebook.v * 2;
		uint32 uv1 = s_uLookup[uLookup] | s_vLookup[vLookup];
		uint32 uv2 = s_uLookup[uLookup + 1] | s_vLookup[vLookup + 1];

		int yLookup1 = codebook.y[0] * 2;
		int yLookup2 = codebook.y[1] * 2;
		int yLookup3 = codebook.y[2] * 2;
		int yLookup4 = codebook.y[3] * 2;

		uint32 pixelGroup1 = uv2 | s_yLookup[yLookup1 + 1];
		uint32 pixelGroup2 = uv1 | s_yLookup[yLookup2];
		uint32 pixelGroup3 = uv1 | s_yLookup[yLookup1];
		uint32 pixelGroup4 = uv2 | s_yLookup[yLookup2 + 1];
		uint32 pixelGroup5 = uv2 | s_yLookup[yLookup3 + 1];
		uint32 pixelGroup6 = uv1 | s_yLookup[yLookup3];
		uint32 pixelGroup7 = uv1 | s_yLookup[yLookup4];
		uint32 pixelGroup8 = uv2 | s_yLookup[yLookup4 + 1];

		output[0x000] = getRGBLookupEntry(_colorMap, pixelGroup1 & 0xFFFF);
		output[0x001] = getRGBLookupEntry(_colorMap, pixelGroup1 >> 16);
		output[0x002] = getRGBLookupEntry(_colorMap, pixelGroup2 & 0xFFFF);
		output[0x003] = getRGBLookupEntry(_colorMap, pixelGroup2 >> 16);
		output[0x400] = getRGBLookupEntry(_colorMap, pixelGroup3 & 0xFFFF);
		output[0x401] = getRGBLookupEntry(_colorMap, pixelGroup3 >> 16);
		output[0x402] = getRGBLookupEntry(_colorMap, pixelGroup4 & 0xFFFF);
		output[0x403] = getRGBLookupEntry(_colorMap, pixelGroup4 >> 16);
		output[0x800] = getRGBLookupEntry(_colorMap, pixelGroup5 >> 16);
		output[0x801] = getRGBLookupEntry(_colorMap, pixelGroup6 & 0xFFFF);
		output[0x802] = getRGBLookupEntry(_colorMap, pixelGroup7 >> 16);
		output[0x803] = getRGBLookupEntry(_colorMap, pixelGroup8 & 0xFFFF);
		output[0xC00] = getRGBLookupEntry(_colorMap, pixelGroup6 >> 16);
		output[0xC01] = getRGBLookupEntry(_colorMap, pixelGroup5 & 0xFFFF);
		output[0xC02] = getRGBLookupEntry(_colorMap, pixelGroup8 >> 16);
		output[0xC03] = getRGBLookupEntry(_colorMap, pixelGroup7 & 0xFFFF);
	} else {
		const CinepakCodebook &codebook = _curFrame.strips[strip].v4_codebook[codebookIndex];
		byte *output = (byte *)(_curFrame.strips[strip].v4_dither + codebookIndex);

		int uLookup = (byte)codebook.u * 2;
		int vLookup = (byte)codebook.v * 2;
		uint32 uv1 = s_uLookup[uLookup] | s_vLookup[vLookup];
		uint32 uv2 = s_uLookup[uLookup + 1] | s_vLookup[vLookup + 1];

		int yLookup1 = codebook.y[0] * 2;
		int yLookup2 = codebook.y[1] * 2;
		int yLookup3 = codebook.y[2] * 2;
		int yLookup4 = codebook.y[3] * 2;

		uint32 pixelGroup1 = uv2 | s_yLookup[yLookup1 + 1];
		uint32 pixelGroup2 = uv2 | s_yLookup[yLookup2 + 1];
		uint32 pixelGroup3 = uv1 | s_yLookup[yLookup3];
		uint32 pixelGroup4 = uv1 | s_yLookup[yLookup4];
		uint32 pixelGroup5 = uv1 | s_yLookup[yLookup1];
		uint32 pixelGroup6 = uv1 | s_yLookup[yLookup2];
		uint32 pixelGroup7 = uv2 | s_yLookup[yLookup3 + 1];
		uint32 pixelGroup8 = uv2 | s_yLookup[yLookup4 + 1];

		output[0x000] = getRGBLookupEntry(_colorMap, pixelGroup1 & 0xFFFF);
		output[0x001] = getRGBLookupEntry(_colorMap, pixelGroup2 >> 16);
		output[0x400] = getRGBLookupEntry(_colorMap, pixelGroup5 & 0xFFFF);
		output[0x401] = getRGBLookupEntry(_colorMap, pixelGroup6 >> 16);
		output[0x002] = getRGBLookupEntry(_colorMap, pixelGroup3 & 0xFFFF);
		output[0x003] = getRGBLookupEntry(_colorMap, pixelGroup4 >> 16);
		output[0x402] = getRGBLookupEntry(_colorMap, pixelGroup7 & 0xFFFF);
		output[0x403] = getRGBLookupEntry(_colorMap, pixelGroup8 >> 16);
		output[0x800] = getRGBLookupEntry(_colorMap, pixelGroup1 >> 16);
		output[0x801] = getRGBLookupEntry(_colorMap, pixelGroup6 & 0xFFFF);
		output[0xC00] = getRGBLookupEntry(_colorMap, pixelGroup5 >> 16);
		output[0xC01] = getRGBLookupEntry(_colorMap, pixelGroup2 & 0xFFFF);
		output[0x802] = getRGBLookupEntry(_colorMap, pixelGroup3 >> 16);
		output[0x803] = getRGBLookupEntry(_colorMap, pixelGroup8 & 0xFFFF);
		output[0xC02] = getRGBLookupEntry(_colorMap, pixelGroup7 >> 16);
		output[0xC03] = getRGBLookupEntry(_colorMap, pixelGroup4 & 0xFFFF);
	}
}

void CinepakDecoder::decodeVectors8(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	decodeVectorsTmpl<byte, CodebookConverterPalette>(_curFrame, _clipTable, stream, strip, chunkID, chunkSize);
}

void CinepakDecoder::decodeVectors24(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	if (_curFrame.surface->format.bytesPerPixel == 2) {
		decodeVectorsTmpl<uint16, CodebookConverterRGB>(_curFrame, _clipTable, stream, strip, chunkID, chunkSize);
	} else if (_curFrame.surface->format.bytesPerPixel == 4) {
		decodeVectorsTmpl<uint32, CodebookConverterRGB>(_curFrame, _clipTable, stream, strip, chunkID, chunkSize);
	}
}

bool CinepakDecoder::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (_bitsPerPixel == 8)
		return format.isCLUT8();

	if (format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
		return false;

	_pixelFormat = format;
	return true;
}

bool CinepakDecoder::canDither(DitherType type) const {
	return (type == kDitherTypeVFW || type == kDitherTypeQT) && _bitsPerPixel == 24;
}

void CinepakDecoder::setDither(DitherType type, const byte *palette) {
	assert(canDither(type));

	delete[] _colorMap;

	_ditherPalette.resize(256, false);
	_ditherPalette.set(palette, 0, 256);

	_dirtyPalette = true;
	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	_ditherType = type;

	if (type == kDitherTypeVFW) {
		_colorMap = new byte[1024];

		for (int i = 0; i < 1024; i++)
			_colorMap[i] = findNearestRGB(s_defaultPaletteLookup[i]);
	} else {
		// Generate QuickTime dither table
		// 4 blocks of 0x4000 bytes (RGB554 lookup)
		_colorMap = DitherCodec::createQuickTimeDitherTable(palette, 256);
	}
}

byte CinepakDecoder::findNearestRGB(int index) const {
	byte r = s_defaultPalette[index * 3];
	byte g = s_defaultPalette[index * 3 + 1];
	byte b = s_defaultPalette[index * 3 + 2];

	return _ditherPalette.findBestColor(r, g, b, Graphics::kColorDistanceEuclidean);
}

void CinepakDecoder::ditherVectors(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	decodeVectorsTmpl<byte, CodebookConverterDithered>(_curFrame, _clipTable, stream, strip, chunkID, chunkSize);
}

} // End of namespace Image
