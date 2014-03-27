#include <orbitersdk.h>
#include "common.h"

bool GetKey(WPARAM w, DWORD & key)
{
	bool found = true;
	switch(w)
	{
	case VK_ESCAPE:
		key=OAPI_KEY_ESCAPE;
		break;
	case 0x31:
		key=OAPI_KEY_1;
		break;
	case 0x32:
		key=OAPI_KEY_2;
		break;
	case 0x33:
		key=OAPI_KEY_3;
		break;
	case 0x34:
		key=OAPI_KEY_4;
		break;
	case 0x35:
		key=OAPI_KEY_5;
		break;
	case 0x36:
		key=OAPI_KEY_6;
		break;
	case 0x37:
		key=OAPI_KEY_7;
		break;
	case 0x38:
		key=OAPI_KEY_8;
		break;
	case 0x39:
		key=OAPI_KEY_9;
		break;
	case 0x30:
		key=OAPI_KEY_0;
		break;
	case 0xBD:
		key=OAPI_KEY_MINUS;
		break;
	case 0xBB:
		key=OAPI_KEY_EQUALS;
		break;
	case 0x08:
		key=OAPI_KEY_BACK;
		break;
	// VK_TAB (skipping the bypasser)
	case 0x09:
		found = false;
		key=OAPI_KEY_TAB;
		break;
	case 0x51:
		key=OAPI_KEY_Q;
		break;
	case 0x57:
		key=OAPI_KEY_W;
		break;
	case 0x45:
		key=OAPI_KEY_E;
		break;
	case 0x52:
		key=OAPI_KEY_R;
		break;
	case 0x54:
		key=OAPI_KEY_T;
		break;
	case 0x59:
		key=OAPI_KEY_Y;
		break;
	case 0x55:
		key=OAPI_KEY_U;
		break;
	case 0x49:
		key=OAPI_KEY_I;
		break;
	case 0x4F:
		key=OAPI_KEY_O;
		break;
	case 0x50:
		key=OAPI_KEY_P;
		break;
	case 0xDB:
		key=OAPI_KEY_LBRACKET;
		break;
	case 0xDD:
		key=OAPI_KEY_RBRACKET;
		break;
	case 0xD:
		key=OAPI_KEY_RETURN;
		break;
	case 0xA2:
		key=OAPI_KEY_LCONTROL;
		break;
	case 0x41:
		key=OAPI_KEY_A;
		break;
	case 0x53:
		key=OAPI_KEY_S;
		break;
	case 0x44:
		key=OAPI_KEY_D;
		break;
	case 0x46:
		key=OAPI_KEY_F;
		break;
	case 0x47:
		key=OAPI_KEY_G;
		break;
	case 0x48:
		key=OAPI_KEY_H;
		break;
	case 0x4A:
		key=OAPI_KEY_J;
		break;
	case 0x4B:
		key=OAPI_KEY_K;
		break;
	case 0x4C:
		key=OAPI_KEY_L;
		break;
	case 0xBA:
		key=OAPI_KEY_SEMICOLON;
		break;
	case 0xDE:
		key=OAPI_KEY_APOSTROPHE;
		break;
	case 0xA0:
		key=OAPI_KEY_LSHIFT;
		break;
	case 0xDC:
		key=OAPI_KEY_BACKSLASH;
		break;
	case 0x5A:
		key=OAPI_KEY_Z;
		break;
	case 0x58:
		key=OAPI_KEY_X;
		break;
	case 0x43:
		key=OAPI_KEY_C;
		break;
	case 0x56:
		key=OAPI_KEY_V;
		break;
	case 0x42:
		key=OAPI_KEY_B;
		break;
	case 0x4E:
		key=OAPI_KEY_N;
		break;
	case 0x4D:
		key=OAPI_KEY_M;
		break;
	case 0xBC:
		key=OAPI_KEY_COMMA;
		break;
	case 0xBE:
		key=OAPI_KEY_PERIOD;
		break;
	case 0xBF:
		key=OAPI_KEY_SLASH;
		break;
	case 0xA1:
		key=OAPI_KEY_RSHIFT;
		break;
	case 0x6A:
		key=OAPI_KEY_MULTIPLY;
		break;
	case 0xA4:
		key=OAPI_KEY_LALT;
		break;
	case 0x20:
		key=OAPI_KEY_SPACE;
		break;
	case 0x14:
		key=OAPI_KEY_CAPITAL;
		break;
	case 0x70:
		key=OAPI_KEY_F1;
		break;
	case 0x71:
		key=OAPI_KEY_F2;
		break;
	case 0x72:
	    key=OAPI_KEY_F3;
		break;
	case 0x73:
		key=OAPI_KEY_F4;
		break;
	case 0x74:
		key=OAPI_KEY_F5;
		break;
	case 0x75:
		key=OAPI_KEY_F6;
		break;
	case 0x76:
		key=OAPI_KEY_F7;
		break;
	case 0x77:
		key=OAPI_KEY_F8;
	case 0x78:
		key=OAPI_KEY_F9;
		break;
	case 0x79:
		key=OAPI_KEY_F10;
		break;
	case 0x67:
		key=OAPI_KEY_NUMPAD7;
		break;
	case 0x68:
		key=OAPI_KEY_NUMPAD8;
		break;
	case 0x69:
		key=OAPI_KEY_NUMPAD9;
		break;
	case 0x6D:
		key=OAPI_KEY_SUBTRACT;
		break;
	case 0x64:
		key=OAPI_KEY_NUMPAD4;
		break;
	case 0x65:
		key=OAPI_KEY_NUMPAD5;
		break;
	case 0x66:
		key=OAPI_KEY_NUMPAD6;
		break;
	case 0x6B:
		key=OAPI_KEY_ADD;
		break;
	case 0x60:
		key=OAPI_KEY_NUMPAD0;
		break;
	case 0x61:
		key=OAPI_KEY_NUMPAD1;
		break;
	case 0x62:
		key=OAPI_KEY_NUMPAD2;
		break;
	case 0x63:
		key=OAPI_KEY_NUMPAD3;
		break;
	case 0x6E:
		key=OAPI_KEY_DECIMAL;
		break;
	case 0x7A:
		key=OAPI_KEY_F11;
		break;
	case 0x7B:
		key=OAPI_KEY_F12;
		break;
	case 0xA3:
		key=OAPI_KEY_RCONTROL;
		break;
	case 0x6F:
		key=OAPI_KEY_DIVIDE;
		break;
	case 0xA5:
		key=OAPI_KEY_RALT;
		break;
	case 0x24:
		key=OAPI_KEY_HOME;
		break;
	case 0x26:
		key=OAPI_KEY_UP;
		break;
	case 0x21:
		key=OAPI_KEY_PRIOR;
		break;
	case 0x25:
		key=OAPI_KEY_LEFT;
		break;
	case 0x27:
		key=OAPI_KEY_RIGHT;
		break;
	case 0x23:
		key=OAPI_KEY_END;
		break;
	case 0x28:
		key=OAPI_KEY_DOWN;
		break;
	case 0x22:
		key=OAPI_KEY_NEXT;
		break;
	case 0x2D:
		key=OAPI_KEY_INSERT;
		break;
	case 0x2E:
		key=OAPI_KEY_DELETE;
		break;

	default:
		return false;

	}
	return found;
}