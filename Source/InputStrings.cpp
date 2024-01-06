#include "Input.h"


std::string InputWatchdog::getNameForCustomKey(CustomKey key)
{
	switch (key) {
	case CustomKey::SKIP_PUZZLE:
		return "Skip Puzzle";
	case CustomKey::SLEEP:
		return "Sleep (Warp)";
	default:
		return "UNKNOWN KEY";
	};

	return std::string();
}

std::string InputWatchdog::getNameForInputButton(InputButton button)
{
	switch (button)
	{
	case InputButton::KEY_A:
		return "A";
	case InputButton::KEY_B:
		return "B";
	case InputButton::KEY_C:
		return "C";
	case InputButton::KEY_D:
		return "D";
	case InputButton::KEY_E:
		return "E";
	case InputButton::KEY_F:
		return "F";
	case InputButton::KEY_G:
		return "G";
	case InputButton::KEY_H:
		return "H";
	case InputButton::KEY_I:
		return "I";
	case InputButton::KEY_J:
		return "J";
	case InputButton::KEY_K:
		return "K";
	case InputButton::KEY_L:
		return "L";
	case InputButton::KEY_M:
		return "M";
	case InputButton::KEY_N:
		return "N";
	case InputButton::KEY_O:
		return "O";
	case InputButton::KEY_P:
		return "P";
	case InputButton::KEY_Q:
		return "Q";
	case InputButton::KEY_R:
		return "R";
	case InputButton::KEY_S:
		return "S";
	case InputButton::KEY_T:
		return "T";
	case InputButton::KEY_U:
		return "U";
	case InputButton::KEY_V:
		return "V";
	case InputButton::KEY_W:
		return "W";
	case InputButton::KEY_X:
		return "X";
	case InputButton::KEY_Y:
		return "Y";
	case InputButton::KEY_Z:
		return "Z";
	case InputButton::KEY_0:
		return "0";
	case InputButton::KEY_1:
		return "1";
	case InputButton::KEY_2:
		return "2";
	case InputButton::KEY_3:
		return "3";
	case InputButton::KEY_4:
		return "4";
	case InputButton::KEY_5:
		return "5";
	case InputButton::KEY_6:
		return "6";
	case InputButton::KEY_7:
		return "7";
	case InputButton::KEY_8:
		return "8";
	case InputButton::KEY_9:
		return "9";
	case InputButton::KEY_F1:
		return "F1";
	case InputButton::KEY_F2:
		return "F2";
	case InputButton::KEY_F3:
		return "F3";
	case InputButton::KEY_F4:
		return "F4";
	case InputButton::KEY_F5:
		return "F5";
	case InputButton::KEY_F6:
		return "F6";
	case InputButton::KEY_F7:
		return "F7";
	case InputButton::KEY_F8:
		return "F8";
	case InputButton::KEY_F9:
		return "F9";
	case InputButton::KEY_F10:
		return "F10";
	case InputButton::KEY_F11:
		return "F11";
	case InputButton::KEY_F12:
		return "F12";
	case InputButton::KEY_SHIFT:
		return "Shift";
	case InputButton::KEY_CTRL:
		return "Control";
	case InputButton::KEY_ALT:
		return "Alt";
	case InputButton::MODIFIER_HELD:
		return "<any modifier>";
	case InputButton::KEY_BACKTICK:
		return "`";
	case InputButton::KEY_MINUS:
		return "-";
	case InputButton::KEY_EQUALS:
		return "=";
	case InputButton::KEY_TAB:
		return "Tab";
	case InputButton::KEY_LEFTBRACKET:
		return "[";
	case InputButton::KEY_RIGHTBRACKET:
		return "]";
	case InputButton::KEY_BACKSLASH:
		return "\\";
	case InputButton::KEY_SEMICOLON:
		return ";";
	case InputButton::KEY_QUOTE:
		return "'";
	case InputButton::KEY_ARROW_UP:
		return "Up Arrow";
	case InputButton::KEY_ARROW_DOWN:
		return "Down Arrow";
	case InputButton::KEY_ARROW_LEFT:
		return "Left Arrow";
	case InputButton::KEY_ARROW_RIGHT:
		return "Right Arrow";
	case InputButton::KEY_ESCAPE:
		return "Escape";
	case InputButton::KEY_SPACEBAR:
		return "Spacebar";
	case InputButton::KEY_RETURN:
		return "Return";
	case InputButton::KEY_BACKSPACE:
		return "Backspace";
	case InputButton::KEY_DELETE:
		return "Delete";
	case InputButton::KEY_PAGE_UP:
		return "Page Up";
	case InputButton::KEY_PAGE_DOWN:
		return "Page Down";
	case InputButton::KEY_INSERT:
		return "Insert";
	case InputButton::KEY_PAUSE:
		return "Pause";
	case InputButton::KEY_PRINT_SCREEN:
		return "Print Screen";
	case InputButton::KEY_HOME:
		return "Home";
	case InputButton::KEY_END:
		return "End";
	case InputButton::KEY_NUMPAD_0:
		return "Numpad 0";
	case InputButton::KEY_NUMPAD_1:
		return "Numpad 1";
	case InputButton::KEY_NUMPAD_2:
		return "Numpad 2";
	case InputButton::KEY_NUMPAD_3:
		return "Numpad 3";
	case InputButton::KEY_NUMPAD_4:
		return "Numpad 4";
	case InputButton::KEY_NUMPAD_5:
		return "Numpad 5";
	case InputButton::KEY_NUMPAD_6:
		return "Numpad 6";
	case InputButton::KEY_NUMPAD_7:
		return "Numpad 7";
	case InputButton::KEY_NUMPAD_8:
		return "Numpad 8";
	case InputButton::KEY_NUMPAD_9:
		return "Numpad 9";
	case InputButton::KEY_NUMPAD_PLUS:
		return "Numpad +";
	case InputButton::KEY_NUMPAD_MINUS:
		return "Numpad -";
	case InputButton::KEY_NUMPAD_MULTIPLY:
		return "Numpad *";
	case InputButton::KEY_NUMPAD_DIVIDE:
		return "Numpad /";
	case InputButton::MOUSE_BUTTON_LEFT:
		return "LMB";
	case InputButton::MOUSE_BUTTON_MIDDLE:
		return "MMB";
	case InputButton::MOUSE_BUTTON_RIGHT:
		return "RMB";
	case InputButton::MOUSE_BUTTON_4:
		return "Mouse 4";
	case InputButton::MOUSE_BUTTON_5:
		return "Mouse 5";
	case InputButton::CONTROLLER_FACEBUTTON_DOWN:
		return "Face Down";
	case InputButton::CONTROLLER_FACEBUTTON_RIGHT:
		return "Face Right";
	case InputButton::CONTROLLER_FACEBUTTON_LEFT:
		return "Face Left";
	case InputButton::CONTROLLER_FACEBUTTON_UP:
		return "Face Up";
	case InputButton::CONTROLLER_SPECIAL_LEFT:
		return "Select";
	case InputButton::CONTROLLER_SPECIAL_RIGHT:
		return "Start";
	case InputButton::CONTROLLER_LSTICK_UP:
		return "LStick Up";
	case InputButton::CONTROLLER_LSTICK_DOWN:
		return "LStick Down";
	case InputButton::CONTROLLER_LSTICK_LEFT:
		return "LStick Left";
	case InputButton::CONTROLLER_LSTICK_RIGHT:
		return "LStick Right";
	case InputButton::CONTROLLER_LSTICK_CLICK:
		return "LStick Press";
	case InputButton::CONTROLLER_RSTICK_UP:
		return "RStick Up";
	case InputButton::CONTROLLER_RSTICK_DOWN:
		return "RStick Down";
	case InputButton::CONTROLLER_RSTICK_LEFT:
		return "RStick Left";
	case InputButton::CONTROLLER_RSTICK_RIGHT:
		return "RStick Right";
	case InputButton::CONTROLLER_RSTICK_CLICK:
		return "RStick Click";
	case InputButton::CONTROLLER_DPAD_UP:
		return "D-pad Up";
	case InputButton::CONTROLLER_DPAD_DOWN:
		return "D-pad Down";
	case InputButton::CONTROLLER_DPAD_LEFT:
		return "D-pad Left";
	case InputButton::CONTROLLER_DPAD_RIGHT:
		return "D-pad Right";
	case InputButton::CONTROLLER_SHOULDER_LEFT:
		return "Left Shoulder";
	case InputButton::CONTROLLER_SHOULDER_RIGHT:
		return "Right Shoulder";
	case InputButton::CONTROLLER_TRIGGER_LEFT:
		return "Left Trigger";
	case InputButton::CONTROLLER_TRIGGER_RIGHT:
		return "Right Trigger";
	case InputButton::NONE:
		return "NO KEY";
	default:
		return "UNKNOWN BUTTON";
	}
}