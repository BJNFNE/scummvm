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


#include "ultima/nuvie/core/nuvie_defs.h"

#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_text.h"

#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/gui/gui_yes_no_dialog.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

GUI_YesNoDialog::GUI_YesNoDialog(GUI *gui, int x, int y, int w, int h, const char *msg,
		CallBack *yesCallback, CallBack *noCallback)
	: GUI_Dialog(x, y, w, h, 244, 216, 131, GUI_DIALOG_MOVABLE), b_index_num(-1),
	  yes_callback_object(yesCallback), no_callback_object(noCallback) {
	GUI_Widget *widget;

	yes_button = new GUI_Button(this, 100, 50, 40, 18, "Yes", gui->get_font(), BUTTON_TEXTALIGN_CENTER, false, this, false);
	AddWidget(yes_button);
	button_index[0] = yes_button;

	no_button = new GUI_Button(this, 30, 50, 40, 18, "No", gui->get_font(), BUTTON_TEXTALIGN_CENTER, false, this, false);
	AddWidget(no_button);
	button_index[1] = no_button;

	widget = new GUI_Text(10, 25, 0, 0, 0, msg, gui->get_font());
	AddWidget(widget);
}


GUI_YesNoDialog::~GUI_YesNoDialog() {
}

GUI_status GUI_YesNoDialog::KeyDown(const Common::KeyState &key) {
	if (key.keycode == Common::KEYCODE_y)
		return (GUI_status)yes_callback_object->callback(YESNODIALOG_CB_YES, nullptr);

	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case EAST_KEY:
	case WEST_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num == 0)
			b_index_num = 1;
		else
			b_index_num = 0;
		button_index[b_index_num]->set_highlighted(true);
		return GUI_YUM;
	case DO_ACTION_KEY:
		if (b_index_num != -1)
			return button_index[b_index_num]->Activate_button();
		break;
	default:
		break;
	}

	return (GUI_status)no_callback_object->callback(YESNODIALOG_CB_NO, nullptr, this);
}

GUI_status GUI_YesNoDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (caller == yes_button)
		return (GUI_status)yes_callback_object->callback(YESNODIALOG_CB_YES, nullptr);

	if (caller == no_button)
		return (GUI_status)no_callback_object->callback(YESNODIALOG_CB_NO, nullptr, this);

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
