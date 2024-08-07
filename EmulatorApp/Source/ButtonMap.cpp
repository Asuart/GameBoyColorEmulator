#include "ButtonMap.h"

std::string GLFWKeyToString(uint32_t key) {
	switch (key) {
	case GLFW_KEY_SPACE: return "SPACE";
	case GLFW_KEY_APOSTROPHE: return "'";
	case GLFW_KEY_COMMA: return ",";
	case GLFW_KEY_MINUS: return "-";
	case GLFW_KEY_PERIOD: return ".";
	case GLFW_KEY_SLASH: return "/";
	case GLFW_KEY_0: return "0";
	case GLFW_KEY_1: return "1";
	case GLFW_KEY_2: return "2";
	case GLFW_KEY_3: return "3";
	case GLFW_KEY_4: return "4";
	case GLFW_KEY_5: return "5";
	case GLFW_KEY_6: return "6";
	case GLFW_KEY_7: return "7";
	case GLFW_KEY_8: return "8";
	case GLFW_KEY_9: return "9";
	case GLFW_KEY_SEMICOLON: return ";";
	case GLFW_KEY_EQUAL: return "=";
	case GLFW_KEY_A: return "A";
	case GLFW_KEY_B: return "B";
	case GLFW_KEY_C: return "C";
	case GLFW_KEY_D: return "D";
	case GLFW_KEY_E: return "E";
	case GLFW_KEY_F: return "F";
	case GLFW_KEY_G: return "G";
	case GLFW_KEY_H: return "H";
	case GLFW_KEY_I: return "I";
	case GLFW_KEY_J: return "J";
	case GLFW_KEY_K: return "K";
	case GLFW_KEY_L: return "L";
	case GLFW_KEY_M: return "M";
	case GLFW_KEY_N: return "N";
	case GLFW_KEY_O: return "O";
	case GLFW_KEY_P: return "P";
	case GLFW_KEY_Q: return "Q";
	case GLFW_KEY_R: return "R";
	case GLFW_KEY_S: return "S";
	case GLFW_KEY_T: return "T";
	case GLFW_KEY_U: return "U";
	case GLFW_KEY_V: return "V";
	case GLFW_KEY_W: return "W";
	case GLFW_KEY_X: return "X";
	case GLFW_KEY_Y: return "Y";
	case GLFW_KEY_Z: return "Z";
	case GLFW_KEY_LEFT_BRACKET: return "[";
	case GLFW_KEY_BACKSLASH: return "\\";
	case GLFW_KEY_RIGHT_BRACKET: return "]";
	case GLFW_KEY_GRAVE_ACCENT: return "`";
	case GLFW_KEY_ESCAPE: return "ESCAPE";
	case GLFW_KEY_ENTER: return "ENTER";
	case GLFW_KEY_TAB: return "TAB";
	case GLFW_KEY_RIGHT: return "RIGHT";
	case GLFW_KEY_LEFT: return "LEFT";
	case GLFW_KEY_DOWN: return "DOWN";
	case GLFW_KEY_UP: return "UP";
	case GLFW_KEY_F1: return "F1";
	case GLFW_KEY_F2: return "F2";
	case GLFW_KEY_F3: return "F3";
	case GLFW_KEY_F4: return "F4";
	case GLFW_KEY_F5: return "F5";
	case GLFW_KEY_F6: return "F6";
	case GLFW_KEY_F7: return "F7";
	case GLFW_KEY_F8: return "F8";
	case GLFW_KEY_F9: return "F9";
	case GLFW_KEY_F10: return "F10";
	case GLFW_KEY_F11: return "F11";
	case GLFW_KEY_F12: return "F12";
	default: return "???";
	}
}