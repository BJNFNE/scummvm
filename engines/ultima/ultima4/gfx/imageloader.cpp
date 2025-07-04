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

#include "ultima/ultima4/gfx/imageloader.h"

#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Ultima {
namespace Ultima4 {

/*-------------------------------------------------------------------*/
U4ImageDecoder::U4ImageDecoder(int width, int height, int bpp) : _palette(0) {
	_width = width;
	_height = height;
	_bpp = bpp;
	_surface = nullptr;
}

U4ImageDecoder::~U4ImageDecoder() {
	destroy();
}

void U4ImageDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	_palette.clear();
}

void U4ImageDecoder::setFromRawData(const byte *rawData) {
	int x, y;

	switch (_bpp) {
	case 32:
	case 24:
	case 8:
		_surface->copyRectToSurface(rawData, (_width * _bpp) / 8, 0, 0, _width, _height);
		break;

	case 4:
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x += 2) {
				_surface->setPixel(x, y, rawData[(y * _width + x) / 2] >> 4);
				_surface->setPixel(x + 1, y, rawData[(y * _width + x) / 2] & 0x0f);
			}
		}
		break;

	case 1:
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x += 8) {
				_surface->setPixel(x + 0, y, (rawData[(y * _width + x) / 8] >> 7) & 0x01);
				_surface->setPixel(x + 1, y, (rawData[(y * _width + x) / 8] >> 6) & 0x01);
				_surface->setPixel(x + 2, y, (rawData[(y * _width + x) / 8] >> 5) & 0x01);
				_surface->setPixel(x + 3, y, (rawData[(y * _width + x) / 8] >> 4) & 0x01);
				_surface->setPixel(x + 4, y, (rawData[(y * _width + x) / 8] >> 3) & 0x01);
				_surface->setPixel(x + 5, y, (rawData[(y * _width + x) / 8] >> 2) & 0x01);
				_surface->setPixel(x + 6, y, (rawData[(y * _width + x) / 8] >> 1) & 0x01);
				_surface->setPixel(x + 7, y, (rawData[(y * _width + x) / 8] >> 0) & 0x01);
			}
		}
		break;

	default:
		error("invalid bits-per-pixel (bpp): %d", _bpp);
	}
}

Graphics::PixelFormat U4ImageDecoder::getPixelFormatForBpp() const {
	switch (_bpp) {
	case 1:
	case 4:
	case 8:
		return Graphics::PixelFormat::createFormatCLUT8();
	case 24:
		return Graphics::PixelFormat::createFormatRGB24();
	case 32:
		return Graphics::PixelFormat::createFormatRGBA32();
	default:
		error("invalid bits-per-pixel (bpp): %d", _bpp);
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
