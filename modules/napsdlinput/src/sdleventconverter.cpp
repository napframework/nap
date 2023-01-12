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

	// SDL KeyCode to nap KeyCodes
	using SDLKeyCodeMap = std::unordered_map<SDL_Keycode, nap::EKeyCode>;
	static const SDLKeyCodeMap& getSDLKeyCodeMap()
	{
		static const SDLKeyCodeMap key_code_map =
		{
			{ SDLK_RETURN,					nap::EKeyCode::KEY_RETURN },
			{ SDLK_ESCAPE,					nap::EKeyCode::KEY_ESCAPE },
			{ SDLK_BACKSPACE,				nap::EKeyCode::KEY_BACKSPACE },
			{ SDLK_TAB,						nap::EKeyCode::KEY_TAB },
			{ SDLK_SPACE,					nap::EKeyCode::KEY_SPACE },
			{ SDLK_EXCLAIM,					nap::EKeyCode::KEY_EXCLAIM },
			{ SDLK_QUOTEDBL,				nap::EKeyCode::KEY_QUOTEDBL },
			{ SDLK_HASH,					nap::EKeyCode::KEY_HASH },
			{ SDLK_PERCENT,					nap::EKeyCode::KEY_PERCENT },
			{ SDLK_DOLLAR,					nap::EKeyCode::KEY_DOLLAR },
			{ SDLK_AMPERSAND,				nap::EKeyCode::KEY_AMPERSAND },
			{ SDLK_QUOTE,					nap::EKeyCode::KEY_QUOTE },
			{ SDLK_LEFTPAREN,				nap::EKeyCode::KEY_LEFTPAREN },
			{ SDLK_RIGHTPAREN,				nap::EKeyCode::KEY_RIGHTPAREN },
			{ SDLK_ASTERISK,				nap::EKeyCode::KEY_ASTERISK },
			{ SDLK_PLUS,					nap::EKeyCode::KEY_PLUS },
			{ SDLK_COMMA,					nap::EKeyCode::KEY_COMMA },
			{ SDLK_MINUS,					nap::EKeyCode::KEY_MINUS },
			{ SDLK_PERIOD,					nap::EKeyCode::KEY_PERIOD },
			{ SDLK_SLASH,					nap::EKeyCode::KEY_SLASH },
			{ SDLK_0,						nap::EKeyCode::KEY_0 },
			{ SDLK_1,						nap::EKeyCode::KEY_1 },
			{ SDLK_2,						nap::EKeyCode::KEY_2 },
			{ SDLK_3,						nap::EKeyCode::KEY_3 },
			{ SDLK_4,						nap::EKeyCode::KEY_4 },
			{ SDLK_5,						nap::EKeyCode::KEY_5 },
			{ SDLK_6,						nap::EKeyCode::KEY_6 },
			{ SDLK_7,						nap::EKeyCode::KEY_7 },
			{ SDLK_8,						nap::EKeyCode::KEY_8 },
			{ SDLK_9,						nap::EKeyCode::KEY_9 },
			{ SDLK_COLON,					nap::EKeyCode::KEY_COLON },
			{ SDLK_SEMICOLON,				nap::EKeyCode::KEY_SEMICOLON },
			{ SDLK_LESS,					nap::EKeyCode::KEY_LESS },
			{ SDLK_EQUALS,					nap::EKeyCode::KEY_EQUALS },
			{ SDLK_GREATER,					nap::EKeyCode::KEY_GREATER },
			{ SDLK_QUESTION,				nap::EKeyCode::KEY_QUESTION },
			{ SDLK_AT,						nap::EKeyCode::KEY_AT },
			{ SDLK_LEFTBRACKET,				nap::EKeyCode::KEY_LEFTBRACKET },
			{ SDLK_BACKSLASH,				nap::EKeyCode::KEY_BACKSLASH },
			{ SDLK_RIGHTBRACKET,			nap::EKeyCode::KEY_RIGHTBRACKET },
			{ SDLK_CARET,					nap::EKeyCode::KEY_CARET },
			{ SDLK_UNDERSCORE,				nap::EKeyCode::KEY_UNDERSCORE },
			{ SDLK_BACKQUOTE,				nap::EKeyCode::KEY_BACKQUOTE },
			{ SDLK_a,						nap::EKeyCode::KEY_a },
			{ SDLK_b,						nap::EKeyCode::KEY_b },
			{ SDLK_c,						nap::EKeyCode::KEY_c },
			{ SDLK_d,						nap::EKeyCode::KEY_d },
			{ SDLK_e,						nap::EKeyCode::KEY_e },
			{ SDLK_f,						nap::EKeyCode::KEY_f },
			{ SDLK_g,						nap::EKeyCode::KEY_g },
			{ SDLK_h,						nap::EKeyCode::KEY_h },
			{ SDLK_i,						nap::EKeyCode::KEY_i },
			{ SDLK_j,						nap::EKeyCode::KEY_j },
			{ SDLK_k,						nap::EKeyCode::KEY_k },
			{ SDLK_l,						nap::EKeyCode::KEY_l },
			{ SDLK_m,						nap::EKeyCode::KEY_m },
			{ SDLK_n,						nap::EKeyCode::KEY_n },
			{ SDLK_o,						nap::EKeyCode::KEY_o },
			{ SDLK_p,						nap::EKeyCode::KEY_p },
			{ SDLK_q,						nap::EKeyCode::KEY_q },
			{ SDLK_r,						nap::EKeyCode::KEY_r },
			{ SDLK_s,						nap::EKeyCode::KEY_s },
			{ SDLK_t,						nap::EKeyCode::KEY_t },
			{ SDLK_u,						nap::EKeyCode::KEY_u },
			{ SDLK_v,						nap::EKeyCode::KEY_v },
			{ SDLK_w,						nap::EKeyCode::KEY_w },
			{ SDLK_x,						nap::EKeyCode::KEY_x },
			{ SDLK_y,						nap::EKeyCode::KEY_y },
			{ SDLK_z,						nap::EKeyCode::KEY_z },
			{ SDLK_CAPSLOCK,				nap::EKeyCode::KEY_CAPSLOCK },
			{ SDLK_F1,						nap::EKeyCode::KEY_F1 },
			{ SDLK_F2,						nap::EKeyCode::KEY_F2 },
			{ SDLK_F3,						nap::EKeyCode::KEY_F3 },
			{ SDLK_F4,						nap::EKeyCode::KEY_F4 },
			{ SDLK_F5,						nap::EKeyCode::KEY_F5 },
			{ SDLK_F6,						nap::EKeyCode::KEY_F6 },
			{ SDLK_F7,						nap::EKeyCode::KEY_F7 },
			{ SDLK_F8,						nap::EKeyCode::KEY_F8 },
			{ SDLK_F9,						nap::EKeyCode::KEY_F9 },
			{ SDLK_F10,						nap::EKeyCode::KEY_F10 },
			{ SDLK_F11,						nap::EKeyCode::KEY_F11 },
			{ SDLK_F12,						nap::EKeyCode::KEY_F12 },
			{ SDLK_PRINTSCREEN,				nap::EKeyCode::KEY_PRINTSCREEN },
			{ SDLK_SCROLLLOCK,				nap::EKeyCode::KEY_SCROLLLOCK },
			{ SDLK_PAUSE,					nap::EKeyCode::KEY_PAUSE },
			{ SDLK_INSERT,					nap::EKeyCode::KEY_INSERT },
			{ SDLK_HOME,					nap::EKeyCode::KEY_HOME },
			{ SDLK_PAGEUP,					nap::EKeyCode::KEY_PAGEUP },
			{ SDLK_DELETE,					nap::EKeyCode::KEY_DELETE },
			{ SDLK_END,						nap::EKeyCode::KEY_END },
			{ SDLK_PAGEDOWN,				nap::EKeyCode::KEY_PAGEDOWN },
			{ SDLK_RIGHT,					nap::EKeyCode::KEY_RIGHT },
			{ SDLK_LEFT,					nap::EKeyCode::KEY_LEFT },
			{ SDLK_DOWN,					nap::EKeyCode::KEY_DOWN },
			{ SDLK_UP,						nap::EKeyCode::KEY_UP },
			{ SDLK_NUMLOCKCLEAR,			nap::EKeyCode::KEY_NUMLOCKCLEAR },
			{ SDLK_KP_DIVIDE,				nap::EKeyCode::KEY_KP_DIVIDE },
			{ SDLK_KP_MULTIPLY,				nap::EKeyCode::KEY_KP_MULTIPLY },
			{ SDLK_KP_MINUS,				nap::EKeyCode::KEY_KP_MINUS },
			{ SDLK_KP_PLUS,					nap::EKeyCode::KEY_KP_PLUS },
			{ SDLK_KP_ENTER,				nap::EKeyCode::KEY_KP_ENTER },
			{ SDLK_KP_1,					nap::EKeyCode::KEY_KP_1 },
			{ SDLK_KP_2,					nap::EKeyCode::KEY_KP_2 },
			{ SDLK_KP_3,					nap::EKeyCode::KEY_KP_3 },
			{ SDLK_KP_4,					nap::EKeyCode::KEY_KP_4 },
			{ SDLK_KP_5,					nap::EKeyCode::KEY_KP_5 },
			{ SDLK_KP_6,					nap::EKeyCode::KEY_KP_6 },
			{ SDLK_KP_7,					nap::EKeyCode::KEY_KP_7 },
			{ SDLK_KP_8,					nap::EKeyCode::KEY_KP_8 },
			{ SDLK_KP_9,					nap::EKeyCode::KEY_KP_9 },
			{ SDLK_KP_0,					nap::EKeyCode::KEY_KP_0 },
			{ SDLK_KP_PERIOD,				nap::EKeyCode::KEY_KP_PERIOD },
			{ SDLK_APPLICATION,				nap::EKeyCode::KEY_APPLICATION },
			{ SDLK_POWER,					nap::EKeyCode::KEY_POWER },
			{ SDLK_KP_EQUALS,				nap::EKeyCode::KEY_KP_EQUALS },
			{ SDLK_F13,						nap::EKeyCode::KEY_F13 },
			{ SDLK_F14,						nap::EKeyCode::KEY_F14 },
			{ SDLK_F15,						nap::EKeyCode::KEY_F15 },
			{ SDLK_F16,						nap::EKeyCode::KEY_F16 },
			{ SDLK_F17,						nap::EKeyCode::KEY_F17 },
			{ SDLK_F18,						nap::EKeyCode::KEY_F18 },
			{ SDLK_F19,						nap::EKeyCode::KEY_F19 },
			{ SDLK_F20,						nap::EKeyCode::KEY_F20 },
			{ SDLK_F21,						nap::EKeyCode::KEY_F21 },
			{ SDLK_F22,						nap::EKeyCode::KEY_F22 },
			{ SDLK_F23,						nap::EKeyCode::KEY_F23 },
			{ SDLK_F24,						nap::EKeyCode::KEY_F24 },
			{ SDLK_EXECUTE,					nap::EKeyCode::KEY_EXECUTE },
			{ SDLK_HELP,					nap::EKeyCode::KEY_HELP },
			{ SDLK_MENU,					nap::EKeyCode::KEY_MENU },
			{ SDLK_SELECT,					nap::EKeyCode::KEY_SELECT },
			{ SDLK_STOP,					nap::EKeyCode::KEY_STOP },
			{ SDLK_AGAIN,					nap::EKeyCode::KEY_AGAIN },
			{ SDLK_UNDO,					nap::EKeyCode::KEY_UNDO },
			{ SDLK_CUT,						nap::EKeyCode::KEY_CUT },
			{ SDLK_COPY,					nap::EKeyCode::KEY_COPY },
			{ SDLK_PASTE,					nap::EKeyCode::KEY_PASTE },
			{ SDLK_FIND,					nap::EKeyCode::KEY_FIND },
			{ SDLK_MUTE,					nap::EKeyCode::KEY_MUTE },
			{ SDLK_VOLUMEUP,				nap::EKeyCode::KEY_VOLUMEUP },
			{ SDLK_VOLUMEDOWN,				nap::EKeyCode::KEY_VOLUMEDOWN },
			{ SDLK_KP_COMMA,				nap::EKeyCode::KEY_KP_COMMA },
			{ SDLK_KP_EQUALSAS400,			nap::EKeyCode::KEY_KP_EQUALSAS400 },
			{ SDLK_ALTERASE,				nap::EKeyCode::KEY_ALTERASE },
			{ SDLK_SYSREQ,					nap::EKeyCode::KEY_SYSREQ },
			{ SDLK_CANCEL,					nap::EKeyCode::KEY_CANCEL },
			{ SDLK_CLEAR,					nap::EKeyCode::KEY_CLEAR },
			{ SDLK_PRIOR,					nap::EKeyCode::KEY_PRIOR },
			{ SDLK_RETURN2,					nap::EKeyCode::KEY_RETURN2 },
			{ SDLK_SEPARATOR,				nap::EKeyCode::KEY_SEPARATOR },
			{ SDLK_OUT,						nap::EKeyCode::KEY_OUT },
			{ SDLK_OPER,					nap::EKeyCode::KEY_OPER },
			{ SDLK_CLEARAGAIN,				nap::EKeyCode::KEY_CLEARAGAIN },
			{ SDLK_CRSEL,					nap::EKeyCode::KEY_CRSEL },
			{ SDLK_EXSEL,					nap::EKeyCode::KEY_EXSEL },
			{ SDLK_KP_00,					nap::EKeyCode::KEY_KP_00 },
			{ SDLK_KP_000,					nap::EKeyCode::KEY_KP_000 },
			{ SDLK_THOUSANDSSEPARATOR,		nap::EKeyCode::KEY_THOUSANDSSEPARATOR },
			{ SDLK_DECIMALSEPARATOR,		nap::EKeyCode::KEY_DECIMALSEPARATOR },
			{ SDLK_CURRENCYUNIT,			nap::EKeyCode::KEY_CURRENCYUNIT },
			{ SDLK_CURRENCYSUBUNIT,			nap::EKeyCode::KEY_CURRENCYSUBUNIT },
			{ SDLK_KP_LEFTPAREN,			nap::EKeyCode::KEY_KP_LEFTPAREN },
			{ SDLK_KP_RIGHTPAREN,			nap::EKeyCode::KEY_KP_RIGHTPAREN },
			{ SDLK_KP_LEFTBRACE,			nap::EKeyCode::KEY_KP_LEFTBRACE },
			{ SDLK_KP_RIGHTBRACE,			nap::EKeyCode::KEY_KP_RIGHTBRACE },
			{ SDLK_KP_TAB,					nap::EKeyCode::KEY_KP_TAB },
			{ SDLK_KP_BACKSPACE,			nap::EKeyCode::KEY_KP_BACKSPACE },
			{ SDLK_KP_A,					nap::EKeyCode::KEY_KP_A },
			{ SDLK_KP_B,					nap::EKeyCode::KEY_KP_B },
			{ SDLK_KP_C,					nap::EKeyCode::KEY_KP_C },
			{ SDLK_KP_D,					nap::EKeyCode::KEY_KP_D },
			{ SDLK_KP_E,					nap::EKeyCode::KEY_KP_E },
			{ SDLK_KP_F,					nap::EKeyCode::KEY_KP_F },
			{ SDLK_KP_XOR,					nap::EKeyCode::KEY_KP_XOR },
			{ SDLK_KP_POWER,				nap::EKeyCode::KEY_KP_POWER },
			{ SDLK_KP_PERCENT,				nap::EKeyCode::KEY_KP_PERCENT },
			{ SDLK_KP_LESS,					nap::EKeyCode::KEY_KP_LESS },
			{ SDLK_KP_GREATER,				nap::EKeyCode::KEY_KP_GREATER },
			{ SDLK_KP_AMPERSAND,			nap::EKeyCode::KEY_KP_AMPERSAND },
			{ SDLK_KP_DBLAMPERSAND,			nap::EKeyCode::KEY_KP_DBLAMPERSAND },
			{ SDLK_KP_VERTICALBAR,			nap::EKeyCode::KEY_KP_VERTICALBAR },
			{ SDLK_KP_DBLVERTICALBAR,		nap::EKeyCode::KEY_KP_DBLVERTICALBAR },
			{ SDLK_KP_COLON,				nap::EKeyCode::KEY_KP_COLON },
			{ SDLK_KP_HASH,					nap::EKeyCode::KEY_KP_HASH },
			{ SDLK_KP_SPACE,				nap::EKeyCode::KEY_KP_SPACE },
			{ SDLK_KP_AT,					nap::EKeyCode::KEY_KP_AT },
			{ SDLK_KP_EXCLAM,				nap::EKeyCode::KEY_KP_EXCLAM },
			{ SDLK_KP_MEMSTORE,				nap::EKeyCode::KEY_KP_MEMSTORE },
			{ SDLK_KP_MEMRECALL,			nap::EKeyCode::KEY_KP_MEMRECALL },
			{ SDLK_KP_MEMCLEAR,				nap::EKeyCode::KEY_KP_MEMCLEAR },
			{ SDLK_KP_MEMADD,				nap::EKeyCode::KEY_KP_MEMADD },
			{ SDLK_KP_MEMSUBTRACT,			nap::EKeyCode::KEY_KP_MEMSUBTRACT },
			{ SDLK_KP_MEMMULTIPLY,			nap::EKeyCode::KEY_KP_MEMMULTIPLY },
			{ SDLK_KP_MEMDIVIDE,			nap::EKeyCode::KEY_KP_MEMDIVIDE },
			{ SDLK_KP_PLUSMINUS,			nap::EKeyCode::KEY_KP_PLUSMINUS },
			{ SDLK_KP_CLEAR,				nap::EKeyCode::KEY_KP_CLEAR },
			{ SDLK_KP_CLEARENTRY,			nap::EKeyCode::KEY_KP_CLEARENTRY },
			{ SDLK_KP_BINARY,				nap::EKeyCode::KEY_KP_BINARY },
			{ SDLK_KP_OCTAL,				nap::EKeyCode::KEY_KP_OCTAL },
			{ SDLK_KP_DECIMAL,				nap::EKeyCode::KEY_KP_DECIMAL },
			{ SDLK_KP_HEXADECIMAL,			nap::EKeyCode::KEY_KP_HEXADECIMAL },
			{ SDLK_LCTRL,					nap::EKeyCode::KEY_LCTRL },
			{ SDLK_LSHIFT,					nap::EKeyCode::KEY_LSHIFT },
			{ SDLK_LALT,					nap::EKeyCode::KEY_LALT },
			{ SDLK_LGUI,					nap::EKeyCode::KEY_LGUI },
			{ SDLK_RCTRL,					nap::EKeyCode::KEY_RCTRL },
			{ SDLK_RSHIFT,					nap::EKeyCode::KEY_RSHIFT },
			{ SDLK_RALT,					nap::EKeyCode::KEY_RALT },
			{ SDLK_RGUI,					nap::EKeyCode::KEY_RGUI },
			{ SDLK_MODE,					nap::EKeyCode::KEY_MODE },
			{ SDLK_AUDIONEXT,				nap::EKeyCode::KEY_AUDIONEXT },
			{ SDLK_AUDIOPREV,				nap::EKeyCode::KEY_AUDIOPREV },
			{ SDLK_AUDIOSTOP,				nap::EKeyCode::KEY_AUDIOSTOP },
			{ SDLK_AUDIOPLAY,				nap::EKeyCode::KEY_AUDIOPLAY },
			{ SDLK_AUDIOMUTE,				nap::EKeyCode::KEY_AUDIOMUTE },
			{ SDLK_MEDIASELECT,				nap::EKeyCode::KEY_MEDIASELECT },
			{ SDLK_WWW,						nap::EKeyCode::KEY_WWW },
			{ SDLK_MAIL,					nap::EKeyCode::KEY_MAIL },
			{ SDLK_CALCULATOR,				nap::EKeyCode::KEY_CALCULATOR },
			{ SDLK_COMPUTER,				nap::EKeyCode::KEY_COMPUTER },
			{ SDLK_AC_SEARCH,				nap::EKeyCode::KEY_AC_SEARCH },
			{ SDLK_AC_HOME,					nap::EKeyCode::KEY_AC_HOME },
			{ SDLK_AC_BACK,					nap::EKeyCode::KEY_AC_BACK },
			{ SDLK_AC_FORWARD,				nap::EKeyCode::KEY_AC_FORWARD },
			{ SDLK_AC_STOP,					nap::EKeyCode::KEY_AC_STOP },
			{ SDLK_AC_REFRESH,				nap::EKeyCode::KEY_AC_REFRESH },
			{ SDLK_AC_BOOKMARKS,			nap::EKeyCode::KEY_AC_BOOKMARKS },
			{ SDLK_BRIGHTNESSDOWN,			nap::EKeyCode::KEY_BRIGHTNESSDOWN },
			{ SDLK_BRIGHTNESSUP,			nap::EKeyCode::KEY_BRIGHTNESSUP },
			{ SDLK_DISPLAYSWITCH,			nap::EKeyCode::KEY_DISPLAYSWITCH },
			{ SDLK_KBDILLUMTOGGLE,			nap::EKeyCode::KEY_KBDILLUMTOGGLE },
			{ SDLK_KBDILLUMDOWN,			nap::EKeyCode::KEY_KBDILLUMDOWN },
			{ SDLK_KBDILLUMUP,				nap::EKeyCode::KEY_KBDILLUMUP },
			{ SDLK_EJECT,					nap::EKeyCode::KEY_EJECT },
			{ SDLK_SLEEP,					nap::EKeyCode::KEY_SLEEP }
		};
		return key_code_map;
	}


	// Binds a specific SDL mouse event to a pointer event type
	using SDLWindowMap = std::unordered_map<Uint32, rtti::TypeInfo>;
	static const SDLWindowMap& getSDLWindowMap()
	{
		static const SDLWindowMap sdl_window_map =
		{
			{ SDL_WINDOWEVENT_SHOWN,			RTTI_OF(nap::WindowShownEvent) },
			{ SDL_WINDOWEVENT_HIDDEN,			RTTI_OF(nap::WindowHiddenEvent) },
			{ SDL_WINDOWEVENT_MINIMIZED,		RTTI_OF(nap::WindowMinimizedEvent) },
			{ SDL_WINDOWEVENT_MAXIMIZED,		RTTI_OF(nap::WindowMaximizedEvent) },
			{ SDL_WINDOWEVENT_RESTORED,			RTTI_OF(nap::WindowRestoredEvent) },
			{ SDL_WINDOWEVENT_ENTER,			RTTI_OF(nap::WindowEnterEvent) },
			{ SDL_WINDOWEVENT_LEAVE,			RTTI_OF(nap::WindowLeaveEvent) },
			{ SDL_WINDOWEVENT_FOCUS_GAINED,		RTTI_OF(nap::WindowFocusGainedEvent) },
			{ SDL_WINDOWEVENT_FOCUS_LOST,		RTTI_OF(nap::WindowFocusLostEvent) },
			{ SDL_WINDOWEVENT_CLOSE,			RTTI_OF(nap::WindowCloseEvent) },
			{ SDL_WINDOWEVENT_RESIZED,			RTTI_OF(nap::WindowResizedEvent) },
			{ SDL_WINDOWEVENT_MOVED,			RTTI_OF(nap::WindowMovedEvent) },
			{ SDL_WINDOWEVENT_EXPOSED,			RTTI_OF(nap::WindowExposedEvent) },
			{ SDL_WINDOWEVENT_SIZE_CHANGED,		RTTI_OF(nap::WindowResizedEvent) },
			{ SDL_WINDOWEVENT_TAKE_FOCUS,		RTTI_OF(nap::WindowTakeFocusEvent) }
		};
		return sdl_window_map;
	}


	// Binds a specific sdl mouse event to a pointer event type
	using SDLMouseMap = std::unordered_map<Uint32, rtti::TypeInfo>;
	static const SDLMouseMap& getSDLMouseMap()
	{
		static const std::unordered_map<Uint32, rtti::TypeInfo> sdl_mouse_map =
		{
			{ SDL_MOUSEBUTTONDOWN,  RTTI_OF(nap::PointerPressEvent) },
			{ SDL_MOUSEBUTTONUP,	RTTI_OF(nap::PointerReleaseEvent) },
			{ SDL_MOUSEMOTION,		RTTI_OF(nap::PointerMoveEvent) },
			{ SDL_MOUSEWHEEL,		RTTI_OF(nap::MouseWheelEvent) }
		};
		return sdl_mouse_map;
	}


	// Binds a specific sdl touch finger event to a touch event type
	using SDLTouchMap = std::unordered_map<Uint32, rtti::TypeInfo>;
	static const SDLTouchMap& getSDLTouchMap()
	{
		static const SDLTouchMap sdl_touch_map =
		{
			{ SDL_FINGERDOWN,		RTTI_OF(nap::TouchPressEvent) },
			{ SDL_FINGERUP,			RTTI_OF(nap::TouchReleaseEvent) },
			{ SDL_FINGERMOTION,		RTTI_OF(nap::TouchMoveEvent) }
		};
		return sdl_touch_map;
	}


	// Binds a specific sdl key event to a pointer event type
	using SDLKeyMap = std::unordered_map<Uint32, rtti::TypeInfo>;
	static const SDLKeyMap& getSDLKeyMap()
	{
		static const SDLKeyMap key_map =
		{
			{ SDL_KEYDOWN,		RTTI_OF(nap::KeyPressEvent)		},
			{ SDL_KEYUP,		RTTI_OF(nap::KeyReleaseEvent)	},
			{ SDL_TEXTINPUT,	RTTI_OF(nap::TextInputEvent)	}
		};
		return key_map;
	}


	// Binds specific sdl joystick and controller events to a nap controller event type
	using SDLControllerMap = std::unordered_map<Uint32, rtti::TypeInfo>;
	static const SDLControllerMap& getSDLControllerMap()
	{
		const static std::unordered_map<Uint32, rtti::TypeInfo> sdl_controller_map =
		{
			{ SDL_CONTROLLERBUTTONDOWN,		RTTI_OF(nap::ControllerButtonPressEvent) },
			{ SDL_CONTROLLERBUTTONUP,		RTTI_OF(nap::ControllerButtonReleaseEvent) },
			{ SDL_CONTROLLERAXISMOTION,		RTTI_OF(nap::ControllerAxisEvent) },
			{ SDL_JOYAXISMOTION,			RTTI_OF(nap::ControllerAxisEvent) },
			{ SDL_JOYBUTTONDOWN,			RTTI_OF(nap::ControllerButtonPressEvent) },
			{ SDL_JOYBUTTONUP,				RTTI_OF(nap::ControllerButtonReleaseEvent) },
			{ SDL_JOYDEVICEREMOVED,			RTTI_OF(nap::ControllerConnectionEvent) },
			{ SDL_JOYDEVICEADDED,			RTTI_OF(nap::ControllerConnectionEvent) }
		};
		return sdl_controller_map;
	}


	//////////////////////////////////////////////////////////////////////////
	// Helper Functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Helper function to convert an SDL KeyCode to nap KeyCode
	 */
	static nap::EKeyCode toNapKeyCode(SDL_Keycode key)
	{
		auto pos = getSDLKeyCodeMap().find(key);
		return pos != getSDLKeyCodeMap().end() ? pos->second :
			nap::EKeyCode::KEY_UNKNOWN;
	}


	/**
	 * Helper function to convert an SDL mouse button to nap MouseButton
	 */
	static nap::PointerClickEvent::EButton toNapPointerButton(uint8_t button)
	{
		switch (button)
		{
			case SDL_BUTTON_LEFT:
				return PointerClickEvent::EButton::LEFT;
			case SDL_BUTTON_MIDDLE:
				return PointerClickEvent::EButton::MIDDLE;
			case SDL_BUTTON_RIGHT:
				return PointerClickEvent::EButton::RIGHT;
			default:
				return PointerClickEvent::EButton::UNKNOWN;
		}
	}


	/**
	 * Helper function to convert an SDL mouse source to nap pointer source
	 */
	static nap::PointerEvent::ESource toNapPointerSource(uint32_t source)
	{
		return source == SDL_TOUCH_MOUSEID ? PointerEvent::ESource::Touch : PointerEvent::ESource::Mouse;
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
			PointerEvent::ESource source = sdlEvent.motion.which == SDL_TOUCH_MOUSEID ? PointerEvent::ESource::Touch : PointerEvent::ESource::Mouse;
			mouse_event = eventType.create<InputEvent>({ px, py, toNapPointerButton(sdlEvent.button.button), window_id, toNapPointerSource(sdlEvent.motion.which)});
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
			mouse_event = eventType.create<InputEvent>({ rx, ry, px, py, window_id, toNapPointerSource(sdlEvent.motion.which) });
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

		// Normalized coordinates 
		float nx = sdlEvent.tfinger.x;
		float ny = 1.0f - sdlEvent.tfinger.y;
		float pr = sdlEvent.tfinger.pressure;

		// Get window coordinates if window under touch event
		int px = input::invalid;
		int py = input::invalid;
		SDL_Window* window = SDL_GetWindowFromID(window_id);
		if (window != nullptr)
		{
			int sx, sy;
			SDL_GetWindowSize(window, &sx, &sy);
			px = static_cast<int>((sx - 1) * nx);
			py = static_cast<int>((sy - 1) * ny);
		}
		else
		{
			window_id = input::invalid;
		}

		// Touch identifiers
		int fid = static_cast<int>(sdlEvent.tfinger.fingerId);
		int tid = static_cast<int>(sdlEvent.tfinger.touchId);

		InputEvent* touch_event = nullptr;
		switch (sdlType)
		{
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
			{
				touch_event = eventType.create<InputEvent>(
					{
						fid, tid,
						nx, ny,
						pr,
						window_id,
						px, py
					});
				break;
			}
			case SDL_FINGERMOTION:
			{
				float dx =  sdlEvent.tfinger.dx;
				float dy = -sdlEvent.tfinger.dy;
				touch_event = eventType.create<InputEvent>(
					{
						fid, tid,
						nx, ny,
						pr,
						dx, dy,
						window_id,
						px, py
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
		const auto& mouse_map = getSDLMouseMap();
		auto mouse_it = mouse_map.find(sdlEvent.type);
		if (mouse_it == mouse_map.end())
			return nullptr;

		InputEvent* mouse_event = translateSDLMouseEvent(sdlEvent, mouse_it->first, mouse_it->second);
		return InputEventPtr(mouse_event);
	}



	nap::InputEventPtr SDLEventConverter::translateTouchEvent(SDL_Event& sdlEvent)
	{
		const auto& touch_map = getSDLTouchMap();
		auto touch_it = touch_map.find(sdlEvent.type);
		if (touch_it == touch_map.end())
			return nullptr;

		InputEvent* touch_event = translateSDLTouchEvent(sdlEvent, touch_it->first, touch_it->second);
		return InputEventPtr(touch_event);
	}


	nap::InputEventPtr SDLEventConverter::translateControllerEvent(SDL_Event& sdlEvent)
	{
		// If it's a controller event, create, map and return
		const auto& controller_map = getSDLControllerMap();
		auto control_it = controller_map.find(sdlEvent.type);
		if (control_it == controller_map.end())
			return nullptr;

		InputEvent* control_event = translateSDLControllerEvent(sdlEvent, control_it->first, control_it->second);
		return InputEventPtr(control_event);
	}


	// Translates the SDL event in to a NAP input event
	nap::InputEventPtr SDLEventConverter::translateInputEvent(SDL_Event& sdlEvent)
	{
		// If it's a key event, create, map and return
		const auto& key_map = getSDLKeyMap();
		auto key_it = key_map.find(sdlEvent.type);
		if (key_it != key_map.end())
		{
			InputEvent* key_event = translateSDLKeyEvent(sdlEvent, key_it->first, key_it->second);
			return InputEventPtr(key_event);
		}

		// If it's a pointer event it generally has a button except for a move operation
		const auto& mouse_map = getSDLMouseMap();
		auto mouse_it = mouse_map.find(sdlEvent.type);
		if (mouse_it != mouse_map.end())
		{
			InputEvent* mouse_event = translateSDLMouseEvent(sdlEvent, mouse_it->first, mouse_it->second);
			return InputEventPtr(mouse_event);
		}

		// If it's a touch event, create, map and return
		const auto& touch_map = getSDLTouchMap();
		auto touch_it = touch_map.find(sdlEvent.type);
		if (touch_it != touch_map.end())
		{
			InputEvent* touch_event = translateSDLTouchEvent(sdlEvent, touch_it->first, touch_it->second);
			return InputEventPtr(touch_event);
		}

		// If it's a controller event, create, map and return
		const auto& controller_map = getSDLControllerMap();
		auto control_it = controller_map.find(sdlEvent.type);
		if (control_it != controller_map.end())
		{
			InputEvent* control_event = translateSDLControllerEvent(sdlEvent, control_it->first, control_it->second);
			return InputEventPtr(control_event);
		}

		// SDL event could not be mapped to a valid nap input event
		return nullptr;
	}


	bool SDLEventConverter::isKeyEvent(SDL_Event& sdlEvent) const
	{
		const auto& key_map = getSDLKeyMap();
		return key_map.find(sdlEvent.type) != key_map.end();
	}


	nap::InputEventPtr SDLEventConverter::translateKeyEvent(SDL_Event& sdlEvent)
	{
		// If it's a key event, create, map and return
		const auto& key_map = getSDLKeyMap();
		auto key_it = key_map.find(sdlEvent.type);
		if (key_it == key_map.end())
			return nullptr;

		InputEvent* key_event = translateSDLKeyEvent(sdlEvent, key_it->first, key_it->second);
		return InputEventPtr(key_event);
	}


	bool SDLEventConverter::isMouseEvent(SDL_Event& sdlEvent) const
	{
		const auto& mouse_map = getSDLMouseMap();
		return mouse_map.find(sdlEvent.type) != mouse_map.end();
	}



	bool SDLEventConverter::isTouchEvent(SDL_Event& sdlEvent) const
	{
		const auto& touch_map = getSDLTouchMap();
		return touch_map.find(sdlEvent.type) != touch_map.end();
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
		const auto& controller_map = getSDLControllerMap();
		return controller_map.find(sdlEvent.type) != controller_map.end();
	}


	nap::WindowEventPtr SDLEventConverter::translateWindowEvent(SDL_Event& sdlEvent)
	{
		// Get the binding and create correct event
		// If the event can't be located there's no valid event mapping 
		auto window_it = getSDLWindowMap().find(sdlEvent.window.event);
		assert(window_it != getSDLWindowMap().end());

		// When destroying a window (for example, during real time editing), we still get events for the destroyed window, after it has already been destroyed
		// We deal with this by checking if the window ID is still known by SDL itself; if not, we ignore the event.
		int window_id = static_cast<int>(sdlEvent.window.windowID);
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
			getSDLWindowMap().find(sdlEvent.window.event) != getSDLWindowMap().end();
	}
}
