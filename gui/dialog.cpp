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

#include "common/rect.h"

#include "gui/gui-manager.h"
#include "gui/dialog.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

namespace GUI {

/*
 * TODO list
 * - add some sense of the window being "active" (i.e. in front) or not. If it
 *   was inactive and just became active, reset certain vars (like who is focused).
 *   Maybe we should just add lostFocus and receivedFocus methods to Dialog, just
 *   like we have for class Widget?
 * ...
 */

Dialog::Dialog(int x, int y, int w, int h, bool scale)
	: GuiObject(x, y, w, h, scale),
	  _mouseWidget(nullptr), _focusedWidget(nullptr), _dragWidget(nullptr), _tickleWidget(nullptr), _visible(false),
	_backgroundType(GUI::ThemeEngine::kDialogBackgroundDefault), _handlingMouseWheel(false) {
	// Some dialogs like LauncherDialog use internally a fixed size, even though
	// their widgets rely on the layout to be initialized correctly by the theme.
	// Thus we need to catch screen changes here too. If we do not do that, it
	// will for example crash after returning to the launcher when the user
	// started a 640x480 game with a non 1x scaler.
	g_gui.checkScreenChange();

	_mouseUpdatedOnFocus = true;
	_result = -1;
}

Dialog::Dialog(const Common::String &name)
	: GuiObject(name),
	  _mouseWidget(nullptr), _focusedWidget(nullptr), _dragWidget(nullptr), _tickleWidget(nullptr), _visible(false),
	_backgroundType(GUI::ThemeEngine::kDialogBackgroundDefault), _handlingMouseWheel(false) {

	// It may happen that we have 3x scaler in launcher (960xY) and then 640x480
	// game will be forced to 1x. At this stage GUI will not be aware of
	// resolution change, so widgets will be off screen. This forces it to
	// recompute
	//
	// Fixes bug #2892: "HE: When 3x graphics are chosen, F5 crashes game"
	// and bug #2903: "SCUMM: F5 crashes game (640x480)"
	g_gui.checkScreenChange();

	_mouseUpdatedOnFocus = true;
	_result = -1;
}

int Dialog::runModal() {
	// Open up
	open();

	// Start processing events
	g_gui.runLoop();

	// Return the result code
	return _result;
}

void Dialog::open() {
	_result = 0;
	_visible = true;
	g_gui.openDialog(this);

	setDefaultFocusedWidget();
}

void Dialog::close() {
	_visible = false;

	if (_mouseWidget) {
		_mouseWidget->handleMouseLeft(0);
		_mouseWidget = nullptr;
	}
	releaseFocus();
	g_gui.closeTopDialog();
}

void Dialog::reflowLayout() {
	// The screen has changed. That means the screen visual may also have
	// changed, so any cached image may be invalid. The subsequent redraw
	// should be treated as the very first draw.

	if (!_name.empty()) {
		g_gui.xmlEval()->reflowDialogLayout(_name, _firstWidget);
	}

	GuiObject::reflowLayout();

	Widget *w = _firstWidget;
	while (w) {
		w->reflowLayout();
		w = w->_next;
	}
}

void Dialog::lostFocus() {
	_dragWidget = nullptr;

	if (_tickleWidget) {
		_tickleWidget->lostFocus();
	}
}

void Dialog::setFocusWidget(Widget *widget) {
	// The focus will change. Tell the old focused widget (if any)
	// that it lost the focus.
	releaseFocus();

	// Tell the new focused widget (if any) that it just gained the focus.
	if (widget)
		widget->receivedFocus();

	_focusedWidget = widget;
}

void Dialog::setDefaultFocusedWidget() {
	Widget *w = _firstWidget;
	// Search for the first objects that wantsFocus() (if any) and give it the focus
	while (w && !w->wantsFocus()) {
		w = w->_next;
	}

	setFocusWidget(w);
}

void Dialog::releaseFocus() {
	if (_focusedWidget) {
		_focusedWidget->lostFocus();
		_focusedWidget = nullptr;
	}
}

void Dialog::markWidgetsAsDirty() {
	Widget *w = _firstWidget;
	while (w) {
		w->markAsDirty();
		w = w->_next;
	}
}

void Dialog::drawDialog(DrawLayer layerToDraw) {

	if (!isVisible())
		return;

	g_gui.theme()->disableClipRect();
	g_gui.theme()->_layerToDraw = layerToDraw;
	int16 x = _x;
	if (g_gui.useRTL()) {
		x = g_system->getOverlayWidth() - _x - _w;
	}
	g_gui.theme()->drawDialogBackground(Common::Rect(x, _y, x + _w, _y + _h), _backgroundType);

	markWidgetsAsDirty();

#ifdef LAYOUT_DEBUG_DIALOG
	return;
#endif
	drawWidgets();
}

void Dialog::drawWidgets() {

	if (!isVisible())
		return;

	// Draw all children
	Widget *w = _firstWidget;
	while (w) {
		//if (w->_debugVisible)
		w->draw();
		w = w->_next;
	}
}

void Dialog::handleMouseDown(int x, int y, int button, int clickCount) {
	Widget *w;

	w = findWidget(x, y);

	if (w && !(w->getFlags() & WIDGET_IGNORE_DRAG))
		_dragWidget = w;

	// If the click occurred inside a widget which is not the currently
	// focused one, change the focus to that widget.
	if (w && w != _focusedWidget && w->wantsFocus()) {
		setFocusWidget(w);
	}

	if (w)
		w->handleMouseDown(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button, clickCount);
}

void Dialog::handleMouseUp(int x, int y, int button, int clickCount) {
	Widget *w;

	if (_focusedWidget) {
		//w = _focusedWidget;

		// Lose focus on mouseup unless the widget requested to retain the focus
		if (! (_focusedWidget->getFlags() & WIDGET_RETAIN_FOCUS )) {
			releaseFocus();
		}
	}

	if (_dragWidget)
		w = _dragWidget;
	else
		w = findWidget(x, y);

	if (w)
		w->handleMouseUp(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button, clickCount);

	if (_dragWidget) {
		_dragWidget = nullptr;
		// Fake a mouse move to refresh now hovered widget
		handleMouseMoved(x, y, button);
	}
}

void Dialog::handleMouseWheel(int x, int y, int direction) {
	// Guard against recursive call.
	// This can happen as we call handleMouseWheel() on the widget under the mouse,
	// and the default implementation of Widget::handleMouseWheel() is to call it
	// for the widget boss, which can be this dialog.
	if (_handlingMouseWheel)
		return;
	_handlingMouseWheel = true;

	// This may look a bit backwards, but I think it makes more sense for
	// the mouse wheel to primarily affect the widget the mouse is at than
	// the widget that happens to be focused.
	Widget *w = findWidget(x, y);
	if (!w)
		w = _focusedWidget;
	if (w)
		w->handleMouseWheel(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), direction);

	_handlingMouseWheel = false;
}

void Dialog::handleKeyDown(Common::KeyState state) {
	if (_focusedWidget) {
		if (_focusedWidget->handleKeyDown(state))
			return;
	}

	// Hotkey handling

	// Convert keypad Enter to Return key
	if (state.keycode == Common::KEYCODE_KP_ENTER) {
		state.ascii = Common::ASCII_RETURN;
	}
	if (state.ascii != 0) {
		Widget *w = _firstWidget;
		state.ascii = toupper(state.ascii);
		while (w) {
			if (w->_type == kButtonWidget && state.ascii == toupper(((ButtonWidget *)w)->_hotkey)) {
				// The hotkey for widget w was pressed. We fake a mouse click into the
				// button by invoking the appropriate methods.
				w->handleMouseDown(0, 0, 1, 1);
				w->handleMouseUp(0, 0, 1, 1);
				return;
			}
			w = w->_next;
		}
	}

	// ESC closes all dialogs by default
	if (state.keycode == Common::KEYCODE_ESCAPE) {
		setResult(-1);
		close();
	}

	if (state.keycode == Common::KEYCODE_TAB) {
		// TODO: Maybe add Tab behaviours for all widgets too.
		// searches through widgets on screen for tab widget
		Widget *w = _firstWidget;
		while (w) {
			if (w->_type == kTabWidget)
				if (w->handleKeyDown(state))
					return;

			w = w->_next;
		}
	}
}

void Dialog::handleKeyUp(Common::KeyState state) {
	// Focused widget receives keyup events
	if (_focusedWidget)
		_focusedWidget->handleKeyUp(state);
}

void Dialog::handleMouseMoved(int x, int y, int button) {
	Widget *w;

	if (_focusedWidget && !_dragWidget) {
		w = _focusedWidget;
		int wx = w->getAbsX() - _x;
		int wy = w->getAbsY() - _y;

		// We still send mouseEntered/Left messages to the focused item
		// (but to no other items).
		bool mouseInFocusedWidget = (x >= wx && x < wx + w->_w && y >= wy && y < wy + w->_h);
		if (mouseInFocusedWidget && _mouseWidget != w) {
			if (_mouseWidget)
				_mouseWidget->handleMouseLeft(button);
			_mouseWidget = w;
			w->handleMouseEntered(button);
		} else if (!mouseInFocusedWidget && _mouseWidget == w) {
			_mouseWidget = nullptr;
			w->handleMouseLeft(button);
		}

		if (w->getFlags() & WIDGET_TRACK_MOUSE)
			w->handleMouseMoved(x - wx, y - wy, button);
	}

	// We process mouseEntered/Left events if we don't have any
	// currently active dragged widget or if the currently dragged widget
	// does not want to be informed about the mouse mouse events.
	if (!_dragWidget || !(_dragWidget->getFlags() & WIDGET_TRACK_MOUSE))
		w = findWidget(x, y);
	else
		w = _dragWidget;

	if (_mouseWidget != w) {
		if (_mouseWidget)
			_mouseWidget->handleMouseLeft(button);

		// If we have a widget in drag mode we prevent mouseEntered
		// events from being sent to other widgets.
		if (_dragWidget && w != _dragWidget)
			w = nullptr;

		if (w)
			w->handleMouseEntered(button);
		_mouseWidget = w;
	}

	// We only sent mouse move events when the widget requests to be informed about them.
	if (w && (w->getFlags() & WIDGET_TRACK_MOUSE))
		w->handleMouseMoved(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button);
}

void Dialog::handleTickle() {
	// Focused widget receives tickle notifications
	if (_focusedWidget && _focusedWidget->getFlags() & WIDGET_WANT_TICKLE)
		_focusedWidget->handleTickle();

	if (_tickleWidget && _tickleWidget->getFlags() & WIDGET_WANT_TICKLE)
		_tickleWidget->handleTickle();
}

void Dialog::handleOtherEvent(const Common::Event &evt) {
	if (_focusedWidget)
		_focusedWidget->handleOtherEvent(evt);
}

void Dialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCloseCmd:
		close();
		break;
	case kCloseWithResultCmd:
		setResult(data);
		close();
		break;
	default:
		break;
	}
}

/*
 * Determine the widget at location (x,y) if any. Assumes the coordinates are
 * in the local coordinate system, i.e. relative to the top left of the dialog.
 */
Widget *Dialog::findWidget(int x, int y) {
	return Widget::findWidgetInChain(_firstWidget, x, y);
}

Widget *Dialog::findWidget(const char *name) {
	return Widget::findWidgetInChain(_firstWidget, name);
}

Widget *Dialog::findWidget(uint32 type) {
	return Widget::findWidgetInChain(_firstWidget, type);
}

void Dialog::removeWidget(Widget *del) {
	if (del == _mouseWidget || del->containsWidget(_mouseWidget))
		_mouseWidget = nullptr;
	if (del == _focusedWidget || del->containsWidget(_focusedWidget))
		_focusedWidget = nullptr;
	if (del == _dragWidget || del->containsWidget(_dragWidget))
		_dragWidget = nullptr;

	GuiObject::removeWidget(del);
}

} // End of namespace GUI
