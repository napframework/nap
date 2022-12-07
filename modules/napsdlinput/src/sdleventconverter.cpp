/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sdleventconverter.h"
#include "inputevent.h"

// External Includes
#include <mathutils.h>
#include <SDL_joystick.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Input mappings
	//////////////////////////////////////////////////////////////////////////

	/** 
	 * Mapping used to translate from SDL KeyCode to nap KeyCodes
	 */
	const static std::unordered_map<SDL_Keycode, nap::EKeyCode> SDLToKeyCodeMapping =
	{
		std::make_pair(SDLK_RETURN,					nap::EKeyCode::KEY_RETURN),
		std::make_pair(SDLK_ESCAPE,					nap::EKeyCode::KEY_ESCAPE),
		std::make_pair(SDLK_BACKSPACE,				nap::EKeyCode::KEY_BACKSPACE),
		std::make_pair(SDLK_TAB,					nap::EKeyCode::KEY_TAB),
		std::make_pair(SDLK_SPACE,					nap::EKeyCode::KEY_SPACE),
		std::make_pair(SDLK_EXCLAIM,				nap::EKeyCode::KEY_EXCLAIM),
		std::make_pair(SDLK_QUOTEDBL,				nap::EKeyCode::KEY_QUOTEDBL),
		std::make_pair(SDLK_HASH,					nap::EKeyCode::KEY_HASH),
		std::make_pair(SDLK_PERCENT,				nap::EKeyCode::KEY_PERCENT),
		std::make_pair(SDLK_DOLLAR,					nap::EKeyCode::KEY_DOLLAR),
		std::make_pair(SDLK_AMPERSAND,				nap::EKeyCode::KEY_AMPERSAND),
		std::make_pair(SDLK_QUOTE,					nap::EKeyCode::KEY_QUOTE),
		std::make_pair(SDLK_LEFTPAREN,				nap::EKeyCode::KEY_LEFTPAREN),
		std::make_pair(SDLK_RIGHTPAREN,				nap::EKeyCode::KEY_RIGHTPAREN),
		std::make_pair(SDLK_ASTERISK,				nap::EKeyCode::KEY_ASTERISK),
		std::make_pair(SDLK_PLUS,					nap::EKeyCode::KEY_PLUS),
		std::make_pair(SDLK_COMMA,					nap::EKeyCode::KEY_COMMA),
		std::make_pair(SDLK_MINUS,					nap::EKeyCode::KEY_MINUS),
		std::make_pair(SDLK_PERIOD,					nap::EKeyCode::KEY_PERIOD),
		std::make_pair(SDLK_SLASH,					nap::EKeyCode::KEY_SLASH),
		std::make_pair(SDLK_0,						nap::EKeyCode::KEY_0),
		std::make_pair(SDLK_1,						nap::EKeyCode::KEY_1),
		std::make_pair(SDLK_2,						nap::EKeyCode::KEY_2),
		std::make_pair(SDLK_3,						nap::EKeyCode::KEY_3),
		std::make_pair(SDLK_4,						nap::EKeyCode::KEY_4),
		std::make_pair(SDLK_5,						nap::EKeyCode::KEY_5),
		std::make_pair(SDLK_6,						nap::EKeyCode::KEY_6),
		std::make_pair(SDLK_7,						nap::EKeyCode::KEY_7),
		std::make_pair(SDLK_8,						nap::EKeyCode::KEY_8),
		std::make_pair(SDLK_9,						nap::EKeyCode::KEY_9),
		std::make_pair(SDLK_COLON,					nap::EKeyCode::KEY_COLON),
		std::make_pair(SDLK_SEMICOLON,				nap::EKeyCode::KEY_SEMICOLON),
		std::make_pair(SDLK_LESS,					nap::EKeyCode::KEY_LESS),
		std::make_pair(SDLK_EQUALS,					nap::EKeyCode::KEY_EQUALS),
		std::make_pair(SDLK_GREATER,				nap::EKeyCode::KEY_GREATER),
		std::make_pair(SDLK_QUESTION,				nap::EKeyCode::KEY_QUESTION),
		std::make_pair(SDLK_AT,						nap::EKeyCode::KEY_AT),
		std::make_pair(SDLK_LEFTBRACKET,			nap::EKeyCode::KEY_LEFTBRACKET),
		std::make_pair(SDLK_BACKSLASH,				nap::EKeyCode::KEY_BACKSLASH),
		std::make_pair(SDLK_RIGHTBRACKET,			nap::EKeyCode::KEY_RIGHTBRACKET),
		std::make_pair(SDLK_CARET,					nap::EKeyCode::KEY_CARET),
		std::make_pair(SDLK_UNDERSCORE,				nap::EKeyCode::KEY_UNDERSCORE),
		std::make_pair(SDLK_BACKQUOTE,				nap::EKeyCode::KEY_BACKQUOTE),
		std::make_pair(SDLK_a,						nap::EKeyCode::KEY_a),
		std::make_pair(SDLK_b,						nap::EKeyCode::KEY_b),
		std::make_pair(SDLK_c,						nap::EKeyCode::KEY_c),
		std::make_pair(SDLK_d,						nap::EKeyCode::KEY_d),
		std::make_pair(SDLK_e,						nap::EKeyCode::KEY_e),
		std::make_pair(SDLK_f,						nap::EKeyCode::KEY_f),
		std::make_pair(SDLK_g,						nap::EKeyCode::KEY_g),
		std::make_pair(SDLK_h,						nap::EKeyCode::KEY_h),
		std::make_pair(SDLK_i,						nap::EKeyCode::KEY_i),
		std::make_pair(SDLK_j,						nap::EKeyCode::KEY_j),
		std::make_pair(SDLK_k,						nap::EKeyCode::KEY_k),
		std::make_pair(SDLK_l,						nap::EKeyCode::KEY_l),
		std::make_pair(SDLK_m,						nap::EKeyCode::KEY_m),
		std::make_pair(SDLK_n,						nap::EKeyCode::KEY_n),
		std::make_pair(SDLK_o,						nap::EKeyCode::KEY_o),
		std::make_pair(SDLK_p,						nap::EKeyCode::KEY_p),
		std::make_pair(SDLK_q,						nap::EKeyCode::KEY_q),
		std::make_pair(SDLK_r,						nap::EKeyCode::KEY_r),
		std::make_pair(SDLK_s,						nap::EKeyCode::KEY_s),
		std::make_pair(SDLK_t,						nap::EKeyCode::KEY_t),
		std::make_pair(SDLK_u,						nap::EKeyCode::KEY_u),
		std::make_pair(SDLK_v,						nap::EKeyCode::KEY_v),
		std::make_pair(SDLK_w,						nap::EKeyCode::KEY_w),
		std::make_pair(SDLK_x,						nap::EKeyCode::KEY_x),
		std::make_pair(SDLK_y,						nap::EKeyCode::KEY_y),
		std::make_pair(SDLK_z,						nap::EKeyCode::KEY_z),
		std::make_pair(SDLK_CAPSLOCK,				nap::EKeyCode::KEY_CAPSLOCK),
		std::make_pair(SDLK_F1,						nap::EKeyCode::KEY_F1),
		std::make_pair(SDLK_F2,						nap::EKeyCode::KEY_F2),
		std::make_pair(SDLK_F3,						nap::EKeyCode::KEY_F3),
		std::make_pair(SDLK_F4,						nap::EKeyCode::KEY_F4),
		std::make_pair(SDLK_F5,						nap::EKeyCode::KEY_F5),
		std::make_pair(SDLK_F6,						nap::EKeyCode::KEY_F6),
		std::make_pair(SDLK_F7,						nap::EKeyCode::KEY_F7),
		std::make_pair(SDLK_F8,						nap::EKeyCode::KEY_F8),
		std::make_pair(SDLK_F9,						nap::EKeyCode::KEY_F9),
		std::make_pair(SDLK_F10,					nap::EKeyCode::KEY_F10),
		std::make_pair(SDLK_F11,					nap::EKeyCode::KEY_F11),
		std::make_pair(SDLK_F12,					nap::EKeyCode::KEY_F12),
		std::make_pair(SDLK_PRINTSCREEN,			nap::EKeyCode::KEY_PRINTSCREEN),
		std::make_pair(SDLK_SCROLLLOCK,				nap::EKeyCode::KEY_SCROLLLOCK),
		std::make_pair(SDLK_PAUSE,					nap::EKeyCode::KEY_PAUSE),
		std::make_pair(SDLK_INSERT,					nap::EKeyCode::KEY_INSERT),
		std::make_pair(SDLK_HOME,					nap::EKeyCode::KEY_HOME),
		std::make_pair(SDLK_PAGEUP,					nap::EKeyCode::KEY_PAGEUP),
		std::make_pair(SDLK_DELETE,					nap::EKeyCode::KEY_DELETE),
		std::make_pair(SDLK_END,					nap::EKeyCode::KEY_END),
		std::make_pair(SDLK_PAGEDOWN,				nap::EKeyCode::KEY_PAGEDOWN),
		std::make_pair(SDLK_RIGHT,					nap::EKeyCode::KEY_RIGHT),
		std::make_pair(SDLK_LEFT,					nap::EKeyCode::KEY_LEFT),
		std::make_pair(SDLK_DOWN,					nap::EKeyCode::KEY_DOWN),
		std::make_pair(SDLK_UP,						nap::EKeyCode::KEY_UP),
		std::make_pair(SDLK_NUMLOCKCLEAR,			nap::EKeyCode::KEY_NUMLOCKCLEAR),
		std::make_pair(SDLK_KP_DIVIDE,				nap::EKeyCode::KEY_KP_DIVIDE),
		std::make_pair(SDLK_KP_MULTIPLY,			nap::EKeyCode::KEY_KP_MULTIPLY),
		std::make_pair(SDLK_KP_MINUS,				nap::EKeyCode::KEY_KP_MINUS),
		std::make_pair(SDLK_KP_PLUS,				nap::EKeyCode::KEY_KP_PLUS),
		std::make_pair(SDLK_KP_ENTER,				nap::EKeyCode::KEY_KP_ENTER),
		std::make_pair(SDLK_KP_1,					nap::EKeyCode::KEY_KP_1),
		std::make_pair(SDLK_KP_2,					nap::EKeyCode::KEY_KP_2),
		std::make_pair(SDLK_KP_3,					nap::EKeyCode::KEY_KP_3),
		std::make_pair(SDLK_KP_4,					nap::EKeyCode::KEY_KP_4),
		std::make_pair(SDLK_KP_5,					nap::EKeyCode::KEY_KP_5),
		std::make_pair(SDLK_KP_6,					nap::EKeyCode::KEY_KP_6),
		std::make_pair(SDLK_KP_7,					nap::EKeyCode::KEY_KP_7),
		std::make_pair(SDLK_KP_8,					nap::EKeyCode::KEY_KP_8),
		std::make_pair(SDLK_KP_9,					nap::EKeyCode::KEY_KP_9),
		std::make_pair(SDLK_KP_0,					nap::EKeyCode::KEY_KP_0),
		std::make_pair(SDLK_KP_PERIOD,				nap::EKeyCode::KEY_KP_PERIOD),
		std::make_pair(SDLK_APPLICATION,			nap::EKeyCode::KEY_APPLICATION),
		std::make_pair(SDLK_POWER,					nap::EKeyCode::KEY_POWER),
		std::make_pair(SDLK_KP_EQUALS,				nap::EKeyCode::KEY_KP_EQUALS),
		std::make_pair(SDLK_F13,					nap::EKeyCode::KEY_F13),
		std::make_pair(SDLK_F14,					nap::EKeyCode::KEY_F14),
		std::make_pair(SDLK_F15,					nap::EKeyCode::KEY_F15),
		std::make_pair(SDLK_F16,					nap::EKeyCode::KEY_F16),
		std::make_pair(SDLK_F17,					nap::EKeyCode::KEY_F17),
		std::make_pair(SDLK_F18,					nap::EKeyCode::KEY_F18),
		std::make_pair(SDLK_F19,					nap::EKeyCode::KEY_F19),
		std::make_pair(SDLK_F20,					nap::EKeyCode::KEY_F20),
		std::make_pair(SDLK_F21,					nap::EKeyCode::KEY_F21),
		std::make_pair(SDLK_F22,					nap::EKeyCode::KEY_F22),
		std::make_pair(SDLK_F23,					nap::EKeyCode::KEY_F23),
		std::make_pair(SDLK_F24,					nap::EKeyCode::KEY_F24),
		std::make_pair(SDLK_EXECUTE,				nap::EKeyCode::KEY_EXECUTE),
		std::make_pair(SDLK_HELP,					nap::EKeyCode::KEY_HELP),
		std::make_pair(SDLK_MENU,					nap::EKeyCode::KEY_MENU),
		std::make_pair(SDLK_SELECT,					nap::EKeyCode::KEY_SELECT),
		std::make_pair(SDLK_STOP,					nap::EKeyCode::KEY_STOP),
		std::make_pair(SDLK_AGAIN,					nap::EKeyCode::KEY_AGAIN),
		std::make_pair(SDLK_UNDO,					nap::EKeyCode::KEY_UNDO),
		std::make_pair(SDLK_CUT,					nap::EKeyCode::KEY_CUT),
		std::make_pair(SDLK_COPY,					nap::EKeyCode::KEY_COPY),
		std::make_pair(SDLK_PASTE,					nap::EKeyCode::KEY_PASTE),
		std::make_pair(SDLK_FIND,					nap::EKeyCode::KEY_FIND),
		std::make_pair(SDLK_MUTE,					nap::EKeyCode::KEY_MUTE),
		std::make_pair(SDLK_VOLUMEUP,				nap::EKeyCode::KEY_VOLUMEUP),
		std::make_pair(SDLK_VOLUMEDOWN,				nap::EKeyCode::KEY_VOLUMEDOWN),
		std::make_pair(SDLK_KP_COMMA,				nap::EKeyCode::KEY_KP_COMMA),
		std::make_pair(SDLK_KP_EQUALSAS400,			nap::EKeyCode::KEY_KP_EQUALSAS400),
		std::make_pair(SDLK_ALTERASE,				nap::EKeyCode::KEY_ALTERASE),
		std::make_pair(SDLK_SYSREQ,					nap::EKeyCode::KEY_SYSREQ),
		std::make_pair(SDLK_CANCEL,					nap::EKeyCode::KEY_CANCEL),
		std::make_pair(SDLK_CLEAR,					nap::EKeyCode::KEY_CLEAR),
		std::make_pair(SDLK_PRIOR,					nap::EKeyCode::KEY_PRIOR),
		std::make_pair(SDLK_RETURN2,				nap::EKeyCode::KEY_RETURN2),
		std::make_pair(SDLK_SEPARATOR,				nap::EKeyCode::KEY_SEPARATOR),
		std::make_pair(SDLK_OUT,					nap::EKeyCode::KEY_OUT),
		std::make_pair(SDLK_OPER,					nap::EKeyCode::KEY_OPER),
		std::make_pair(SDLK_CLEARAGAIN,				nap::EKeyCode::KEY_CLEARAGAIN),
		std::make_pair(SDLK_CRSEL,					nap::EKeyCode::KEY_CRSEL),
		std::make_pair(SDLK_EXSEL,					nap::EKeyCode::KEY_EXSEL),
		std::make_pair(SDLK_KP_00,					nap::EKeyCode::KEY_KP_00),
		std::make_pair(SDLK_KP_000,					nap::EKeyCode::KEY_KP_000),
		std::make_pair(SDLK_THOUSANDSSEPARATOR,		nap::EKeyCode::KEY_THOUSANDSSEPARATOR),
		std::make_pair(SDLK_DECIMALSEPARATOR,		nap::EKeyCode::KEY_DECIMALSEPARATOR),
		std::make_pair(SDLK_CURRENCYUNIT,			nap::EKeyCode::KEY_CURRENCYUNIT),
		std::make_pair(SDLK_CURRENCYSUBUNIT,		nap::EKeyCode::KEY_CURRENCYSUBUNIT),
		std::make_pair(SDLK_KP_LEFTPAREN,			nap::EKeyCode::KEY_KP_LEFTPAREN),
		std::make_pair(SDLK_KP_RIGHTPAREN,			nap::EKeyCode::KEY_KP_RIGHTPAREN),
		std::make_pair(SDLK_KP_LEFTBRACE,			nap::EKeyCode::KEY_KP_LEFTBRACE),
		std::make_pair(SDLK_KP_RIGHTBRACE,			nap::EKeyCode::KEY_KP_RIGHTBRACE),
		std::make_pair(SDLK_KP_TAB,					nap::EKeyCode::KEY_KP_TAB),
		std::make_pair(SDLK_KP_BACKSPACE,			nap::EKeyCode::KEY_KP_BACKSPACE),
		std::make_pair(SDLK_KP_A,					nap::EKeyCode::KEY_KP_A),
		std::make_pair(SDLK_KP_B,					nap::EKeyCode::KEY_KP_B),
		std::make_pair(SDLK_KP_C,					nap::EKeyCode::KEY_KP_C),
		std::make_pair(SDLK_KP_D,					nap::EKeyCode::KEY_KP_D),
		std::make_pair(SDLK_KP_E,					nap::EKeyCode::KEY_KP_E),
		std::make_pair(SDLK_KP_F,					nap::EKeyCode::KEY_KP_F),
		std::make_pair(SDLK_KP_XOR,					nap::EKeyCode::KEY_KP_XOR),
		std::make_pair(SDLK_KP_POWER,				nap::EKeyCode::KEY_KP_POWER),
		std::make_pair(SDLK_KP_PERCENT,				nap::EKeyCode::KEY_KP_PERCENT),
		std::make_pair(SDLK_KP_LESS,				nap::EKeyCode::KEY_KP_LESS),
		std::make_pair(SDLK_KP_GREATER,				nap::EKeyCode::KEY_KP_GREATER),
		std::make_pair(SDLK_KP_AMPERSAND,			nap::EKeyCode::KEY_KP_AMPERSAND),
		std::make_pair(SDLK_KP_DBLAMPERSAND,		nap::EKeyCode::KEY_KP_DBLAMPERSAND),
		std::make_pair(SDLK_KP_VERTICALBAR,			nap::EKeyCode::KEY_KP_VERTICALBAR),
		std::make_pair(SDLK_KP_DBLVERTICALBAR,		nap::EKeyCode::KEY_KP_DBLVERTICALBAR),
		std::make_pair(SDLK_KP_COLON,				nap::EKeyCode::KEY_KP_COLON),
		std::make_pair(SDLK_KP_HASH,				nap::EKeyCode::KEY_KP_HASH),
		std::make_pair(SDLK_KP_SPACE,				nap::EKeyCode::KEY_KP_SPACE),
		std::make_pair(SDLK_KP_AT,					nap::EKeyCode::KEY_KP_AT),
		std::make_pair(SDLK_KP_EXCLAM,				nap::EKeyCode::KEY_KP_EXCLAM),
		std::make_pair(SDLK_KP_MEMSTORE,			nap::EKeyCode::KEY_KP_MEMSTORE),
		std::make_pair(SDLK_KP_MEMRECALL,			nap::EKeyCode::KEY_KP_MEMRECALL),
		std::make_pair(SDLK_KP_MEMCLEAR,			nap::EKeyCode::KEY_KP_MEMCLEAR),
		std::make_pair(SDLK_KP_MEMADD,				nap::EKeyCode::KEY_KP_MEMADD),
		std::make_pair(SDLK_KP_MEMSUBTRACT,			nap::EKeyCode::KEY_KP_MEMSUBTRACT),
		std::make_pair(SDLK_KP_MEMMULTIPLY,			nap::EKeyCode::KEY_KP_MEMMULTIPLY),
		std::make_pair(SDLK_KP_MEMDIVIDE,			nap::EKeyCode::KEY_KP_MEMDIVIDE),
		std::make_pair(SDLK_KP_PLUSMINUS,			nap::EKeyCode::KEY_KP_PLUSMINUS),
		std::make_pair(SDLK_KP_CLEAR,				nap::EKeyCode::KEY_KP_CLEAR),
		std::make_pair(SDLK_KP_CLEARENTRY,			nap::EKeyCode::KEY_KP_CLEARENTRY),
		std::make_pair(SDLK_KP_BINARY,				nap::EKeyCode::KEY_KP_BINARY),
		std::make_pair(SDLK_KP_OCTAL,				nap::EKeyCode::KEY_KP_OCTAL),
		std::make_pair(SDLK_KP_DECIMAL,				nap::EKeyCode::KEY_KP_DECIMAL),
		std::make_pair(SDLK_KP_HEXADECIMAL,			nap::EKeyCode::KEY_KP_HEXADECIMAL),
		std::make_pair(SDLK_LCTRL,					nap::EKeyCode::KEY_LCTRL),
		std::make_pair(SDLK_LSHIFT,					nap::EKeyCode::KEY_LSHIFT),
		std::make_pair(SDLK_LALT,					nap::EKeyCode::KEY_LALT),
		std::make_pair(SDLK_LGUI,					nap::EKeyCode::KEY_LGUI),
		std::make_pair(SDLK_RCTRL,					nap::EKeyCode::KEY_RCTRL),
		std::make_pair(SDLK_RSHIFT,					nap::EKeyCode::KEY_RSHIFT),
		std::make_pair(SDLK_RALT,					nap::EKeyCode::KEY_RALT),
		std::make_pair(SDLK_RGUI,					nap::EKeyCode::KEY_RGUI),
		std::make_pair(SDLK_MODE,					nap::EKeyCode::KEY_MODE),
		std::make_pair(SDLK_AUDIONEXT,				nap::EKeyCode::KEY_AUDIONEXT),
		std::make_pair(SDLK_AUDIOPREV,				nap::EKeyCode::KEY_AUDIOPREV),
		std::make_pair(SDLK_AUDIOSTOP,				nap::EKeyCode::KEY_AUDIOSTOP),
		std::make_pair(SDLK_AUDIOPLAY,				nap::EKeyCode::KEY_AUDIOPLAY),
		std::make_pair(SDLK_AUDIOMUTE,				nap::EKeyCode::KEY_AUDIOMUTE),
		std::make_pair(SDLK_MEDIASELECT,			nap::EKeyCode::KEY_MEDIASELECT),
		std::make_pair(SDLK_WWW,					nap::EKeyCode::KEY_WWW),
		std::make_pair(SDLK_MAIL,					nap::EKeyCode::KEY_MAIL),
		std::make_pair(SDLK_CALCULATOR,				nap::EKeyCode::KEY_CALCULATOR),
		std::make_pair(SDLK_COMPUTER,				nap::EKeyCode::KEY_COMPUTER),
		std::make_pair(SDLK_AC_SEARCH,				nap::EKeyCode::KEY_AC_SEARCH),
		std::make_pair(SDLK_AC_HOME,				nap::EKeyCode::KEY_AC_HOME),
		std::make_pair(SDLK_AC_BACK,				nap::EKeyCode::KEY_AC_BACK),
		std::make_pair(SDLK_AC_FORWARD,				nap::EKeyCode::KEY_AC_FORWARD),
		std::make_pair(SDLK_AC_STOP,				nap::EKeyCode::KEY_AC_STOP),
		std::make_pair(SDLK_AC_REFRESH,				nap::EKeyCode::KEY_AC_REFRESH),
		std::make_pair(SDLK_AC_BOOKMARKS,			nap::EKeyCode::KEY_AC_BOOKMARKS),
		std::make_pair(SDLK_BRIGHTNESSDOWN,			nap::EKeyCode::KEY_BRIGHTNESSDOWN),
		std::make_pair(SDLK_BRIGHTNESSUP,			nap::EKeyCode::KEY_BRIGHTNESSUP),
		std::make_pair(SDLK_DISPLAYSWITCH,			nap::EKeyCode::KEY_DISPLAYSWITCH),
		std::make_pair(SDLK_KBDILLUMTOGGLE,			nap::EKeyCode::KEY_KBDILLUMTOGGLE),
		std::make_pair(SDLK_KBDILLUMDOWN,			nap::EKeyCode::KEY_KBDILLUMDOWN),
		std::make_pair(SDLK_KBDILLUMUP,				nap::EKeyCode::KEY_KBDILLUMUP),
		std::make_pair(SDLK_EJECT,					nap::EKeyCode::KEY_EJECT),
		std::make_pair(SDLK_SLEEP,					nap::EKeyCode::KEY_SLEEP)
	};

	// Binds a specific sdl mouse event to a pointer event type
	static std::unordered_map<Uint32, rtti::TypeInfo> SDLToWindowMapping =
	{
		std::make_pair(SDL_WINDOWEVENT_SHOWN,			RTTI_OF(nap::WindowShownEvent)),
		std::make_pair(SDL_WINDOWEVENT_HIDDEN,			RTTI_OF(nap::WindowHiddenEvent)),
		std::make_pair(SDL_WINDOWEVENT_MINIMIZED,		RTTI_OF(nap::WindowMinimizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_MAXIMIZED,		RTTI_OF(nap::WindowMaximizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_RESTORED,		RTTI_OF(nap::WindowRestoredEvent)),
		std::make_pair(SDL_WINDOWEVENT_ENTER,			RTTI_OF(nap::WindowEnterEvent)),
		std::make_pair(SDL_WINDOWEVENT_LEAVE,			RTTI_OF(nap::WindowLeaveEvent)),
		std::make_pair(SDL_WINDOWEVENT_FOCUS_GAINED,	RTTI_OF(nap::WindowFocusGainedEvent)),
		std::make_pair(SDL_WINDOWEVENT_FOCUS_LOST,		RTTI_OF(nap::WindowFocusLostEvent)),
		std::make_pair(SDL_WINDOWEVENT_CLOSE,			RTTI_OF(nap::WindowCloseEvent)),
		std::make_pair(SDL_WINDOWEVENT_RESIZED,			RTTI_OF(nap::WindowResizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_MOVED,			RTTI_OF(nap::WindowMovedEvent)),
		std::make_pair(SDL_WINDOWEVENT_EXPOSED,			RTTI_OF(nap::WindowExposedEvent)),
		std::make_pair(SDL_WINDOWEVENT_SIZE_CHANGED,	RTTI_OF(nap::WindowResizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_TAKE_FOCUS,		RTTI_OF(nap::WindowTakeFocusEvent))
	};


	// Binds a specific sdl mouse event to a pointer event type
	const static std::unordered_map<Uint32, rtti::TypeInfo> SDLToMouseMapping =
	{
		std::make_pair(SDL_MOUSEBUTTONDOWN, RTTI_OF(nap::PointerPressEvent)),
		std::make_pair(SDL_MOUSEBUTTONUP,	RTTI_OF(nap::PointerReleaseEvent)),
		std::make_pair(SDL_MOUSEMOTION,		RTTI_OF(nap::PointerMoveEvent)),
		std::make_pair(SDL_MOUSEWHEEL,		RTTI_OF(nap::MouseWheelEvent))
	};


	// Binds a specific sdl touch finger event to a touch event type
	const static std::unordered_map<Uint32, rtti::TypeInfo> SDLToTouchMapping =
	{
		std::make_pair(SDL_FINGERDOWN,		RTTI_OF(nap::TouchPressEvent)),
		std::make_pair(SDL_FINGERUP,		RTTI_OF(nap::TouchReleaseEvent)),
		std::make_pair(SDL_FINGERMOTION,	RTTI_OF(nap::TouchMoveEvent))
	};


	// Binds a specific sdl key event to a pointer event type
	const static std::unordered_map<Uint32, rtti::TypeInfo> SDLToKeyMapping = 
	{
		std::make_pair(SDL_KEYDOWN,		RTTI_OF(nap::KeyPressEvent)),
		std::make_pair(SDL_KEYUP,		RTTI_OF(nap::KeyReleaseEvent)),
		std::make_pair(SDL_TEXTINPUT,	RTTI_OF(nap::TextInputEvent))
	};

	// Binds specific sdl joystick and controller events to a nap controller event type
	const static std::unordered_map<Uint32, rtti::TypeInfo> SDLToControllerMapping =
	{
		std::make_pair(SDL_CONTROLLERBUTTONDOWN,	RTTI_OF(nap::ControllerButtonPressEvent)),
		std::make_pair(SDL_CONTROLLERBUTTONUP,		RTTI_OF(nap::ControllerButtonReleaseEvent)),
		std::make_pair(SDL_CONTROLLERAXISMOTION,	RTTI_OF(nap::ControllerAxisEvent)),
		std::make_pair(SDL_JOYAXISMOTION,			RTTI_OF(nap::ControllerAxisEvent)),
		std::make_pair(SDL_JOYBUTTONDOWN,			RTTI_OF(nap::ControllerButtonPressEvent)),
		std::make_pair(SDL_JOYBUTTONUP,				RTTI_OF(nap::ControllerButtonReleaseEvent)),
		std::make_pair(SDL_JOYDEVICEREMOVED,		RTTI_OF(nap::ControllerConnectionEvent)),
		std::make_pair(SDL_JOYDEVICEADDED,			RTTI_OF(nap::ControllerConnectionEvent))
	};


	//////////////////////////////////////////////////////////////////////////
	// Helper Functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Helper function to convert an SDL KeyCode to nap KeyCode
	 */
	static nap::EKeyCode toNapKeyCode(SDL_Keycode key)
	{
		auto pos = SDLToKeyCodeMapping.find(key);
		if (pos == SDLToKeyCodeMapping.end())
		{
			return nap::EKeyCode::KEY_UNKNOWN;
		}
		return pos->second;
	}


	/**
	 * Helper function to convert an SDL mouse button to nap MouseButton
	 */
	static nap::EMouseButton toNapMouseButton(uint8_t button)
	{
		switch (button)
		{
			case SDL_BUTTON_LEFT:
				return EMouseButton::LEFT;
			case SDL_BUTTON_MIDDLE:
				return EMouseButton::MIDDLE;
			case SDL_BUTTON_RIGHT:
				return EMouseButton::RIGHT;
		}

		return EMouseButton::UNKNOWN;
	}

	static nap::EControllerAxis toNapAxis(uint8_t axis)
	{
		switch (axis)
		{
		case SDL_CONTROLLER_AXIS_LEFTX:
			return EControllerAxis::LEFT_X;
		case SDL_CONTROLLER_AXIS_LEFTY:
			return EControllerAxis::LEFT_Y;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			return EControllerAxis::RIGHT_X;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			return EControllerAxis::RIGHT_Y;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			return EControllerAxis::TRIGGER_LEFT;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			return EControllerAxis::TRIGGER_RIGHT;
		}
		return EControllerAxis::UNKNOWN;
	}

	static nap::EControllerButton toNapButton(uint8_t button)
	{
		switch (button)
		{
		case SDL_CONTROLLER_BUTTON_A:
			return EControllerButton::A;
		case SDL_CONTROLLER_BUTTON_B:
			return EControllerButton::B;
		case SDL_CONTROLLER_BUTTON_X:
			return EControllerButton::X;
		case SDL_CONTROLLER_BUTTON_Y:
			return EControllerButton::Y;
		case SDL_CONTROLLER_BUTTON_BACK:
			return EControllerButton::BACK;
		case SDL_CONTROLLER_BUTTON_START:
			return EControllerButton::START;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			return EControllerButton::LEFT_STICK;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			return EControllerButton::RIGHT_STICK;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			return EControllerButton::LEFT_SHOULDER;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			return EControllerButton::RIGHT_SHOULDER;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return EControllerButton::DPAD_UP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return EControllerButton::DPAD_DOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return EControllerButton::DPAD_RIGHT;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return EControllerButton::DPAD_LEFT;
		}
		return EControllerButton::UNKNOWN;
	}


	//////////////////////////////////////////////////////////////////////////
	// Static SDL translate functions
	//////////////////////////////////////////////////////////////////////////

	static nap::InputEvent* translateSDLMouseEvent(SDL_Event& sdlEvent, uint32 sdlType, const rtti::TypeInfo& eventType) 
	{
		// Get window
		int window_id = static_cast<int>(sdlEvent.window.windowID);
		SDL_Window* window = SDL_GetWindowFromID(window_id);
		if (window == nullptr)
			return nullptr;

		InputEvent* mouse_event = nullptr;
		switch (sdlType)
		{
		case SDL_MOUSEWHEEL:
		{
			int dix = static_cast<int>(sdlEvent.wheel.x);
			int diy = static_cast<int>(sdlEvent.wheel.y);
			mouse_event = eventType.create<InputEvent>({ dix, diy, window_id });
			break;
		}
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
		{
			// Get position
			int sx, sy;
			SDL_GetWindowSize(window, &sx, &sy);
			int px = sdlEvent.motion.x;
			int py = sy - 1 - sdlEvent.motion.y;
			mouse_event = eventType.create<InputEvent>({ px, py, toNapMouseButton(sdlEvent.button.button), window_id, 0 });
			break;
		}
		case SDL_MOUSEMOTION:
		{
			// Get position
			int sx, sy;
			SDL_GetWindowSize(window, &sx, &sy);
			int px = static_cast<int>(sdlEvent.motion.x);
			int py = sy - 1 - static_cast<int>(sdlEvent.motion.y);
			int rx = static_cast<int>(sdlEvent.motion.xrel);
			int ry = static_cast<int>(-sdlEvent.motion.yrel);
			mouse_event = eventType.create<InputEvent>({ rx, ry, px, py, window_id, 0 });
			break;
		}
		default:
			assert(false);
			break;
		}
		return mouse_event;
	}


	static nap::InputEvent* translateSDLTouchEvent(SDL_Event& sdlEvent, uint32 sdlType, const rtti::TypeInfo& eventType)
	{
		// Get window
		int window_id = static_cast<int>(sdlEvent.tfinger.windowID);
		SDL_Window* window = SDL_GetWindowFromID(window_id);
		if (window == nullptr)
			return nullptr;

		int sx, sy;
		SDL_GetWindowSize(window, &sx, &sy);
		int px = static_cast<int>((sx - 1) * sdlEvent.tfinger.x);
		int py = static_cast<int>((sy - 1) * (1.0f - sdlEvent.tfinger.y));

		nap::int64 fid = static_cast<nap::int64>(sdlEvent.tfinger.fingerId);
		nap::int64 tid = static_cast<nap::int64>(sdlEvent.tfinger.touchId);

		InputEvent* touch_event = nullptr;
		switch (sdlType)
		{
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
			{
				touch_event = eventType.create<InputEvent>(
					{
						fid, tid,
						px, py,
						float(sdlEvent.tfinger.pressure),
						window_id
					});
				break;
			}
			case SDL_FINGERMOTION:
			{
				int dx = static_cast<int>(sx * sdlEvent.tfinger.dx);
				int dy = static_cast<int>(sy * -sdlEvent.tfinger.dy);
				touch_event = eventType.create<InputEvent>(
					{
						fid, tid,
						px, py,
						sdlEvent.tfinger.pressure,
						dx, dy,
						window_id
					});
				break;
			}
			default:
			{
				assert(false);
				break;
			}
		}

		return touch_event;
	}


	nap::InputEvent* SDLEventConverter::translateSDLControllerEvent(SDL_Event& sdlEvent, uint32 sdlType, const rtti::TypeInfo& eventType)
	{
		InputEvent* controller_event = nullptr;
		switch (sdlType)
		{
		case SDL_CONTROLLERAXISMOTION:
		{
			int id = mService.getControllerNumber(sdlEvent.caxis.which);
			nap::EControllerAxis axis = toNapAxis(sdlEvent.caxis.axis);
			double v = math::fit<double>((double)(sdlEvent.caxis.value),
				(double)(SDL_JOYSTICK_AXIS_MIN),
				(double)(SDL_JOYSTICK_AXIS_MAX), -1.0, 1.0);
			controller_event = eventType.create<InputEvent>({ id, axis, static_cast<int>(axis), v });
			break;
		}
		case SDL_JOYAXISMOTION:
		{
			// Don't translate a joystick axis event when the joystick is a controller
			// SDL generates 2 events for the same device if the device is both a joystick and controller
			// NAP uses only 1 class for both types of events: ControllerEvent
			if(mService.isGameController(sdlEvent.jaxis.which))
				break;

			int id = mService.getControllerNumber(sdlEvent.jaxis.which);
			int axisID = static_cast<int>(sdlEvent.jaxis.axis);
			double v = math::fit<double>((double)(sdlEvent.jaxis.value),
				(double)(SDL_JOYSTICK_AXIS_MIN), 
				(double)(SDL_JOYSTICK_AXIS_MAX), -1.0, 1.0);
			controller_event = eventType.create<InputEvent>({ id, EControllerAxis::UNKNOWN, static_cast<int>(axisID), v });
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			int id = mService.getControllerNumber(sdlEvent.cbutton.which);
			nap::EControllerButton btn = toNapButton(sdlEvent.cbutton.button);
			controller_event = eventType.create<InputEvent>({ id, btn, static_cast<int>(btn) });
			break;
		}
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			// Don't translate a joystick button event when the joystick is a controller
			// SDL generates 2 events for the same device if the device is both a joystick and controller
			// NAP uses only 1 class for both types of events: 
			if (mService.isGameController(sdlEvent.jbutton.which))
				break;

			int id = mService.getControllerNumber(sdlEvent.jbutton.which);
			int btn_id = static_cast<int>(sdlEvent.jbutton.button);
			controller_event = eventType.create<InputEvent>({ id, nap::EControllerButton::UNKNOWN, btn_id });
			break;
		}
		case SDL_JOYDEVICEREMOVED:
		{
			int id = mService.getControllerNumber(sdlEvent.jdevice.which);
			controller_event = eventType.create<InputEvent>({ id, false });
			break;
		}
		case SDL_JOYDEVICEADDED:
		{
			int id = static_cast<int>(sdlEvent.jdevice.which);
			controller_event = eventType.create<InputEvent>({ id, true });
			break;
		}
		default:
			assert(false);
			break;
		}
		return controller_event;
	}


	static nap::InputEvent* translateSDLKeyEvent(SDL_Event& sdlEvent, uint32 sdlType, const rtti::TypeInfo& eventType)
	{
		// Get window
		int window_id = static_cast<int>(sdlEvent.window.windowID);
		SDL_Window* window = SDL_GetWindowFromID(window_id);
		if (window == nullptr)
			return nullptr;

		InputEvent* key_event = nullptr;
		switch (sdlType)
		{
		case SDL_TEXTINPUT:
		{
			key_event = eventType.create<InputEvent>({ std::string(sdlEvent.text.text), window_id });
			break;
		}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			key_event = eventType.create<InputEvent>({ toNapKeyCode(sdlEvent.key.keysym.sym), window_id });
			break;
		}
		default:
			break;
		}

		return key_event;
	}


	//////////////////////////////////////////////////////////////////////////
	// SDLConverter functions
	//////////////////////////////////////////////////////////////////////////

	nap::InputEventPtr SDLEventConverter::translateMouseEvent(SDL_Event& sdlEvent)
	{
		// If it's a pointer event it generally has a button except for a move operation
		auto mouse_it = SDLToMouseMapping.find(sdlEvent.type);
		if (mouse_it == SDLToMouseMapping.end())
			return nullptr;

		InputEvent* mouse_event = translateSDLMouseEvent(sdlEvent, mouse_it->first, mouse_it->second);
		return InputEventPtr(mouse_event);
	}



	nap::InputEventPtr SDLEventConverter::translateTouchEvent(SDL_Event& sdlEvent)
	{
		auto touch_it = SDLToTouchMapping.find(sdlEvent.type);
		if (touch_it == SDLToTouchMapping.end())
			return nullptr;

		InputEvent* touch_event = translateSDLTouchEvent(sdlEvent, touch_it->first, touch_it->second);
		return InputEventPtr(touch_event);
	}


	nap::InputEventPtr SDLEventConverter::translateControllerEvent(SDL_Event& sdlEvent)
	{
		// If it's a controller event, create, map and return
		auto control_it = SDLToControllerMapping.find(sdlEvent.type);
		if (control_it == SDLToControllerMapping.end())
			return nullptr;

		InputEvent* control_event = translateSDLControllerEvent(sdlEvent, control_it->first, control_it->second);
		return InputEventPtr(control_event);
	}


	// Translates the SDL event in to a NAP input event
	nap::InputEventPtr SDLEventConverter::translateInputEvent(SDL_Event& sdlEvent)
	{
		// If it's a key event, create, map and return
		auto key_it = SDLToKeyMapping.find(sdlEvent.type);
		if (key_it != SDLToKeyMapping.end())
		{
			InputEvent* key_event = translateSDLKeyEvent(sdlEvent, key_it->first, key_it->second);
			return InputEventPtr(key_event);
		}

		// If it's a pointer event it generally has a button except for a move operation
		auto mouse_it = SDLToMouseMapping.find(sdlEvent.type);
		if (mouse_it != SDLToMouseMapping.end())
		{
			InputEvent* mouse_event = translateSDLMouseEvent(sdlEvent, mouse_it->first, mouse_it->second);
			return InputEventPtr(mouse_event);
		}

		// If it's a controller event, create, map and return
		auto control_it = SDLToControllerMapping.find(sdlEvent.type);
		if (control_it != SDLToControllerMapping.end())
		{
			InputEvent* control_event = translateSDLControllerEvent(sdlEvent, control_it->first, control_it->second);
			return InputEventPtr(control_event);
		}

		// SDL event could not be mapped to a valid nap input event
		return nullptr;
	}


	bool SDLEventConverter::isKeyEvent(SDL_Event& sdlEvent) const
	{
		return SDLToKeyMapping.find(sdlEvent.type) != SDLToKeyMapping.end();
	}


	nap::InputEventPtr SDLEventConverter::translateKeyEvent(SDL_Event& sdlEvent)
	{
		// If it's a key event, create, map and return
		auto key_it = SDLToKeyMapping.find(sdlEvent.type);
		if (key_it == SDLToKeyMapping.end())
			return nullptr;

		InputEvent* key_event = translateSDLKeyEvent(sdlEvent, key_it->first, key_it->second);
		return InputEventPtr(key_event);
	}


	bool SDLEventConverter::isMouseEvent(SDL_Event& sdlEvent) const
	{
		return SDLToMouseMapping.find(sdlEvent.type) != SDLToMouseMapping.end();
	}



	bool SDLEventConverter::isTouchEvent(SDL_Event& sdlEvent) const
	{
		return SDLToTouchMapping.find(sdlEvent.type) != SDLToTouchMapping.end();
	}


	bool SDLEventConverter::isInputEvent(SDL_Event& sdlEvent) const
	{
		return isKeyEvent(sdlEvent)
			|| isMouseEvent(sdlEvent)
			|| isTouchEvent(sdlEvent)
			|| isControllerEvent(sdlEvent);
	}


	bool SDLEventConverter::isControllerEvent(SDL_Event& sdlEvent) const
	{
		return SDLToControllerMapping.find(sdlEvent.type) != SDLToControllerMapping.end();
	}


	nap::WindowEventPtr SDLEventConverter::translateWindowEvent(SDL_Event& sdlEvent)
	{
		// Get the binding and create correct event
		// If the event can't be located there's no valid event mapping 
		auto window_it = SDLToWindowMapping.find(sdlEvent.window.event);
		assert(window_it != SDLToWindowMapping.end());

		int window_id = static_cast<int>(sdlEvent.window.windowID);

		// When destroying a window (for example, during real time editing), we still get events for the destroyed window, after it has already been destroyed
		// We deal with this by checking if the window ID is still known by SDL itself; if not, we ignore the event.
		if (SDL_GetWindowFromID(window_id) == nullptr)
		{
			return nullptr;
		}

		// If it's one of the two parameterized constructors, add the arguments
		rtti::TypeInfo event_type = window_it->second;
		if (event_type.is_derived_from(RTTI_OF(nap::ParameterizedWindowEvent)))
		{
			int d1 = static_cast<int>(sdlEvent.window.data1);
			int d2 = static_cast<int>(sdlEvent.window.data2);
			return WindowEventPtr(event_type.create<WindowEvent>({ d1, d2, window_id }));
		}

		// Create and return correct window event
		return WindowEventPtr(event_type.create<nap::WindowEvent>({ window_id }));
	}


	bool SDLEventConverter::isWindowEvent(SDL_Event& sdlEvent) const
	{
		return sdlEvent.type == SDL_WINDOWEVENT &&
			SDLToWindowMapping.find(sdlEvent.window.event) != SDLToWindowMapping.end();
	}
}
