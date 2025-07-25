/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_NAVARROW_H
#define BURIED_NAVARROW_H

#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

struct LocationStaticData;

class NavArrowWindow : public Window {
public:
	NavArrowWindow(BuriedEngine *vm, Window *parent);
	~NavArrowWindow();

	bool updateArrow(int button, int newStatus);
	bool updateAllArrows(int up, int left, int right, int down, int forward);
	bool updateAllArrows(const LocationStaticData &locationStaticData);

	void onPaint();
	void onEnable(bool enable);
	void onLButtonDown(const Common::Point &point, uint flags);
	void onActionEnd(const Common::CustomEventType &action, uint flags);

	enum {
		BUTTON_DISABLED = 0,
		BUTTON_ENABLED = 1,
		BUTTON_SELECTED = 2
	};

	enum {
		NAV_BUTTON_UP = 0,
		NAV_BUTTON_LEFT = 1,
		NAV_BUTTON_RIGHT = 2,
		NAV_BUTTON_DOWN = 3,
		NAV_BUTTON_FORWARD = 4
	};

private:
	static const int NUM_ARROWS = 5;
	static const int NUM_ARROW_BITMAPS = 3;

	Graphics::Surface *_background;
	int _arrowBitmaps[NUM_ARROWS][NUM_ARROW_BITMAPS];
	byte _arrowStatus[NUM_ARROWS];

	bool drawArrow(int xDst, int yDst, int arrow);
	bool rebuildArrows();

};

} // End of namespace Buried

#endif
