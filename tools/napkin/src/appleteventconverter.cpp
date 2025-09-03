/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "appleteventconverter.h"

// External includes
#include <QKeyEvent>
#include <nap/logger.h>

namespace napkin
{
	//////////////////////////////////////////////////////////////////////////
	// Input mappings
	//////////////////////////////////////////////////////////////////////////

	// SDL KeyCode to nap KeyCodes
	using QtKeyCodeMap = std::unordered_map<Qt::Key, nap::EKeyCode>;
	static const QtKeyCodeMap& getQtKeyCodeMap()
	{
		static const QtKeyCodeMap key_code_map =
		{
			{ Qt::Key::Key_AsciiCircum,				nap::EKeyCode::KEY_CARET },
			{ Qt::Key::Key_AsciiTilde,				nap::EKeyCode::KEY_TILDE},
			{ Qt::Key::Key_QuoteLeft,				nap::EKeyCode::KEY_BACKQUOTE},
			{ Qt::Key::Key_Apostrophe,				nap::EKeyCode::KEY_QUOTE},
			{ Qt::Key::Key_Enter,					nap::EKeyCode::KEY_KP_ENTER},
			{ Qt::Key::Key_Return,					nap::EKeyCode::KEY_RETURN },
			{ Qt::Key::Key_Escape,					nap::EKeyCode::KEY_ESCAPE },
			{ Qt::Key::Key_Backspace,				nap::EKeyCode::KEY_BACKSPACE },
			{ Qt::Key::Key_Tab,						nap::EKeyCode::KEY_TAB },
			{ Qt::Key::Key_Space,					nap::EKeyCode::KEY_SPACE },
			{ Qt::Key::Key_Exclam,					nap::EKeyCode::KEY_EXCLAIM },
			{ Qt::Key::Key_QuoteDbl,				nap::EKeyCode::KEY_QUOTEDBL },
			{ Qt::Key::Key_Percent,					nap::EKeyCode::KEY_PERCENT },
			{ Qt::Key::Key_Dollar,					nap::EKeyCode::KEY_DOLLAR },
			{ Qt::Key::Key_NumberSign,				nap::EKeyCode::KEY_HASH},
			{ Qt::Key::Key_Ampersand,				nap::EKeyCode::KEY_AMPERSAND },
			{ Qt::Key::Key_ParenLeft,				nap::EKeyCode::KEY_LEFTPAREN },
			{ Qt::Key::Key_ParenRight,				nap::EKeyCode::KEY_RIGHTPAREN },
			{ Qt::Key::Key_Asterisk,				nap::EKeyCode::KEY_ASTERISK },
			{ Qt::Key::Key_Plus,					nap::EKeyCode::KEY_PLUS },
			{ Qt::Key::Key_Comma,					nap::EKeyCode::KEY_COMMA },
			{ Qt::Key::Key_Minus,					nap::EKeyCode::KEY_MINUS },
			{ Qt::Key::Key_Period,					nap::EKeyCode::KEY_PERIOD },
			{ Qt::Key::Key_Slash,					nap::EKeyCode::KEY_SLASH },
			{ Qt::Key::Key_0,						nap::EKeyCode::KEY_0 },
			{ Qt::Key::Key_1,						nap::EKeyCode::KEY_1 },
			{ Qt::Key::Key_2,						nap::EKeyCode::KEY_2 },
			{ Qt::Key::Key_3,						nap::EKeyCode::KEY_3 },
			{ Qt::Key::Key_4,						nap::EKeyCode::KEY_4 },
			{ Qt::Key::Key_5,						nap::EKeyCode::KEY_5 },
			{ Qt::Key::Key_6,						nap::EKeyCode::KEY_6 },
			{ Qt::Key::Key_7,						nap::EKeyCode::KEY_7 },
			{ Qt::Key::Key_8,						nap::EKeyCode::KEY_8 },
			{ Qt::Key::Key_9,						nap::EKeyCode::KEY_9 },
			{ Qt::Key::Key_Colon,					nap::EKeyCode::KEY_COLON },
			{ Qt::Key::Key_Semicolon,				nap::EKeyCode::KEY_SEMICOLON },
			{ Qt::Key::Key_Less,					nap::EKeyCode::KEY_LESS },
			{ Qt::Key::Key_Equal,					nap::EKeyCode::KEY_EQUALS },
			{ Qt::Key::Key_Greater,					nap::EKeyCode::KEY_GREATER },
			{ Qt::Key::Key_Question,				nap::EKeyCode::KEY_QUESTION },
			{ Qt::Key::Key_At,						nap::EKeyCode::KEY_AT },
			{ Qt::Key::Key_BracketLeft,				nap::EKeyCode::KEY_LEFTBRACKET },
			{ Qt::Key::Key_Backslash,				nap::EKeyCode::KEY_BACKSLASH },
			{ Qt::Key::Key_BracketRight,			nap::EKeyCode::KEY_RIGHTBRACKET },
			{ Qt::Key::Key_Underscore,				nap::EKeyCode::KEY_UNDERSCORE },
			{ Qt::Key::Key_A,						nap::EKeyCode::KEY_a },
			{ Qt::Key::Key_B,						nap::EKeyCode::KEY_b },
			{ Qt::Key::Key_C,						nap::EKeyCode::KEY_c },
			{ Qt::Key::Key_D,						nap::EKeyCode::KEY_d },
			{ Qt::Key::Key_E,						nap::EKeyCode::KEY_e },
			{ Qt::Key::Key_F,						nap::EKeyCode::KEY_f },
			{ Qt::Key::Key_G,						nap::EKeyCode::KEY_g },
			{ Qt::Key::Key_H,						nap::EKeyCode::KEY_h },
			{ Qt::Key::Key_I,						nap::EKeyCode::KEY_i },
			{ Qt::Key::Key_J,						nap::EKeyCode::KEY_j },
			{ Qt::Key::Key_K,						nap::EKeyCode::KEY_k },
			{ Qt::Key::Key_L,						nap::EKeyCode::KEY_l },
			{ Qt::Key::Key_M,						nap::EKeyCode::KEY_m },
			{ Qt::Key::Key_N,						nap::EKeyCode::KEY_n },
			{ Qt::Key::Key_O,						nap::EKeyCode::KEY_o },
			{ Qt::Key::Key_P,						nap::EKeyCode::KEY_p },
			{ Qt::Key::Key_Q,						nap::EKeyCode::KEY_q },
			{ Qt::Key::Key_R,						nap::EKeyCode::KEY_r },
			{ Qt::Key::Key_S,						nap::EKeyCode::KEY_s },
			{ Qt::Key::Key_T,						nap::EKeyCode::KEY_t },
			{ Qt::Key::Key_U,						nap::EKeyCode::KEY_u },
			{ Qt::Key::Key_V,						nap::EKeyCode::KEY_v },
			{ Qt::Key::Key_W,						nap::EKeyCode::KEY_w },
			{ Qt::Key::Key_X,						nap::EKeyCode::KEY_x },
			{ Qt::Key::Key_Y,						nap::EKeyCode::KEY_y },
			{ Qt::Key::Key_Z,						nap::EKeyCode::KEY_z },
			{ Qt::Key::Key_CapsLock,				nap::EKeyCode::KEY_CAPSLOCK },
			{ Qt::Key::Key_F1,						nap::EKeyCode::KEY_F1 },
			{ Qt::Key::Key_F2,						nap::EKeyCode::KEY_F2 },
			{ Qt::Key::Key_F3,						nap::EKeyCode::KEY_F3 },
			{ Qt::Key::Key_F4,						nap::EKeyCode::KEY_F4 },
			{ Qt::Key::Key_F5,						nap::EKeyCode::KEY_F5 },
			{ Qt::Key::Key_F6,						nap::EKeyCode::KEY_F6 },
			{ Qt::Key::Key_F7,						nap::EKeyCode::KEY_F7 },
			{ Qt::Key::Key_F8,						nap::EKeyCode::KEY_F8 },
			{ Qt::Key::Key_F9,						nap::EKeyCode::KEY_F9 },
			{ Qt::Key::Key_F10,						nap::EKeyCode::KEY_F10 },
			{ Qt::Key::Key_F11,						nap::EKeyCode::KEY_F11 },
			{ Qt::Key::Key_F12,						nap::EKeyCode::KEY_F12 },
			{ Qt::Key::Key_Print,					nap::EKeyCode::KEY_PRINTSCREEN },
			{ Qt::Key::Key_ScrollLock,				nap::EKeyCode::KEY_SCROLLLOCK },
			{ Qt::Key::Key_Pause,					nap::EKeyCode::KEY_PAUSE },
			{ Qt::Key::Key_Insert,					nap::EKeyCode::KEY_INSERT },
			{ Qt::Key::Key_Home,					nap::EKeyCode::KEY_HOME },
			{ Qt::Key::Key_PageUp,					nap::EKeyCode::KEY_PAGEUP },
			{ Qt::Key::Key_Delete,					nap::EKeyCode::KEY_DELETE },
			{ Qt::Key::Key_End,						nap::EKeyCode::KEY_END },
			{ Qt::Key::Key_PageDown,				nap::EKeyCode::KEY_PAGEDOWN },
			{ Qt::Key::Key_Right,					nap::EKeyCode::KEY_RIGHT },
			{ Qt::Key::Key_Left,					nap::EKeyCode::KEY_LEFT },
			{ Qt::Key::Key_Down,					nap::EKeyCode::KEY_DOWN },
			{ Qt::Key::Key_Up,						nap::EKeyCode::KEY_UP },
			{ Qt::Key::Key_Clear,					nap::EKeyCode::KEY_NUMLOCKCLEAR },
			{ Qt::Key::Key_ApplicationLeft,			nap::EKeyCode::KEY_APPLICATION },
			{ Qt::Key::Key_ApplicationRight,		nap::EKeyCode::KEY_APPLICATION },
			{ Qt::Key::Key_PowerDown,				nap::EKeyCode::KEY_POWER },
			{ Qt::Key::Key_PowerOff,				nap::EKeyCode::KEY_POWER },
			{ Qt::Key::Key_F13,						nap::EKeyCode::KEY_F13 },
			{ Qt::Key::Key_F14,						nap::EKeyCode::KEY_F14 },
			{ Qt::Key::Key_F15,						nap::EKeyCode::KEY_F15 },
			{ Qt::Key::Key_F16,						nap::EKeyCode::KEY_F16 },
			{ Qt::Key::Key_F17,						nap::EKeyCode::KEY_F17 },
			{ Qt::Key::Key_F18,						nap::EKeyCode::KEY_F18 },
			{ Qt::Key::Key_F19,						nap::EKeyCode::KEY_F19 },
			{ Qt::Key::Key_F20,						nap::EKeyCode::KEY_F20 },
			{ Qt::Key::Key_F21,						nap::EKeyCode::KEY_F21 },
			{ Qt::Key::Key_F22,						nap::EKeyCode::KEY_F22 },
			{ Qt::Key::Key_F23,						nap::EKeyCode::KEY_F23 },
			{ Qt::Key::Key_F24,						nap::EKeyCode::KEY_F24 },
			{ Qt::Key::Key_Execute,					nap::EKeyCode::KEY_EXECUTE },
			{ Qt::Key::Key_Help,					nap::EKeyCode::KEY_HELP },
			{ Qt::Key::Key_Menu,					nap::EKeyCode::KEY_MENU },
			{ Qt::Key::Key_Select,					nap::EKeyCode::KEY_SELECT },
			{ Qt::Key::Key_Undo,					nap::EKeyCode::KEY_UNDO },
			{ Qt::Key::Key_Cut,						nap::EKeyCode::KEY_CUT },
			{ Qt::Key::Key_Copy,					nap::EKeyCode::KEY_COPY },
			{ Qt::Key::Key_Paste,					nap::EKeyCode::KEY_PASTE },
			{ Qt::Key::Key_Find,					nap::EKeyCode::KEY_FIND },
			{ Qt::Key::Key_VolumeMute,				nap::EKeyCode::KEY_MUTE },
			{ Qt::Key::Key_VolumeUp,				nap::EKeyCode::KEY_VOLUMEUP },
			{ Qt::Key::Key_VolumeDown,				nap::EKeyCode::KEY_VOLUMEDOWN },
			{ Qt::Key::Key_SysReq,					nap::EKeyCode::KEY_SYSREQ },
			{ Qt::Key::Key_Cancel,					nap::EKeyCode::KEY_CANCEL },
			{ Qt::Key::Key_Clear,					nap::EKeyCode::KEY_CLEAR },
			{ Qt::Key::Key_currency,				nap::EKeyCode::KEY_CURRENCYUNIT },
			{ Qt::Key::Key_BraceLeft,				nap::EKeyCode::KEY_KP_LEFTBRACE },
			{ Qt::Key::Key_BraceRight,				nap::EKeyCode::KEY_KP_RIGHTBRACE },
			{ Qt::Key::Key_Control,					nap::EKeyCode::KEY_LCTRL },
			{ Qt::Key::Key_Shift,					nap::EKeyCode::KEY_LSHIFT },
			{ Qt::Key::Key_Alt,						nap::EKeyCode::KEY_LALT },
			{ Qt::Key::Key_Mode_switch,				nap::EKeyCode::KEY_MODE },
			{ Qt::Key::Key_MediaNext,				nap::EKeyCode::KEY_AUDIONEXT },
			{ Qt::Key::Key_MediaPrevious,			nap::EKeyCode::KEY_AUDIOPREV },
			{ Qt::Key::Key_MediaStop,				nap::EKeyCode::KEY_AUDIOSTOP },
			{ Qt::Key::Key_MediaPlay,				nap::EKeyCode::KEY_AUDIOPLAY },
			{ Qt::Key::Key_OpenUrl,					nap::EKeyCode::KEY_WWW },
			{ Qt::Key::Key_LaunchMail,				nap::EKeyCode::KEY_MAIL },
			{ Qt::Key::Key_Calculator,				nap::EKeyCode::KEY_CALCULATOR },
			{ Qt::Key::Key_Search,					nap::EKeyCode::KEY_AC_SEARCH },
			{ Qt::Key::Key_MonBrightnessDown,		nap::EKeyCode::KEY_BRIGHTNESSDOWN },
			{ Qt::Key::Key_MonBrightnessUp,			nap::EKeyCode::KEY_BRIGHTNESSUP },
			{ Qt::Key::Key_Display,					nap::EKeyCode::KEY_DISPLAYSWITCH },
			{ Qt::Key::Key_Eject,					nap::EKeyCode::KEY_EJECT },
			{ Qt::Key::Key_Sleep,					nap::EKeyCode::KEY_SLEEP }
		};
		return key_code_map;
	}


	// Binds a specific Qt key event to a pointer event type
	using QtKeyMap = std::unordered_map<nap::uint32, nap::rtti::TypeInfo>;
	static const QtKeyMap& getQtKeyMap()
	{
		static const QtKeyMap qt_key_map =
		{
			{ QEvent::KeyPress,			RTTI_OF(nap::KeyPressEvent)			},
			{ QEvent::KeyRelease,		RTTI_OF(nap::KeyReleaseEvent)	}
		};
		return qt_key_map;
	}


	// Binds a specific Qt mouse event to a pointer event type
	using QtMouseMap = std::unordered_map<nap::uint32, nap::rtti::TypeInfo>;
	static const QtMouseMap& getQtMouseMap()
	{
		static const QtMouseMap qt_mouse_map =
		{
			{ QEvent::MouseButtonPress,		RTTI_OF(nap::PointerPressEvent) },
			{ QEvent::MouseButtonRelease,	RTTI_OF(nap::PointerReleaseEvent) },
			{ QEvent::MouseMove,			RTTI_OF(nap::PointerMoveEvent) },
			{ QEvent::Wheel,				RTTI_OF(nap::MouseWheelEvent)}
		};
		return qt_mouse_map;
	}


	// Binds a specific Qt mouse event to a pointer event type
	using QtMouseMap = std::unordered_map<nap::uint32, nap::rtti::TypeInfo>;
	static const QtMouseMap& getQtWindowMap()
	{
		static const QtMouseMap qt_window_map =
		{
			{ QEvent::Resize,				RTTI_OF(nap::WindowResizedEvent) },
			{ QEvent::Show,					RTTI_OF(nap::WindowShownEvent) },
			{ QEvent::Hide,					RTTI_OF(nap::WindowHiddenEvent) },
			{ QEvent::Move,					RTTI_OF(nap::WindowMovedEvent) }
		};
		return qt_window_map;
	}


	//////////////////////////////////////////////////////////////////////////
	// Helper Functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Helper function to convert an SDL KeyCode to nap KeyCode
	 */
	static nap::EKeyCode toNapKeyCode(Qt::Key key)
	{
		auto pos = getQtKeyCodeMap().find(key);
		return pos != getQtKeyCodeMap().end() ? pos->second :
			nap::EKeyCode::KEY_UNKNOWN;
	}


	/**
	 * Helper function to convert an SDL KeyCode to nap KeyCode
	 */
	nap::uint8 toNapKeyModifier(const Qt::KeyboardModifiers& mods)
	{
		nap::uint8 nap_key_mod = 0;
		nap_key_mod |= mods.testFlag(Qt::ShiftModifier)	? static_cast<nap::uint8>(nap::EKeyModifier::Shift) : 0;
		nap_key_mod |= mods.testFlag(Qt::AltModifier) ?	static_cast<nap::uint8>(nap::EKeyModifier::Alt) : 0;
		nap_key_mod |= mods.testFlag(Qt::ControlModifier) ?	static_cast<nap::uint8>(nap::EKeyModifier::Control) : 0;
		return nap_key_mod;
	}


	nap::InputEvent* AppletEventConverter::translateQtKeyEvent(const QEvent& qtEvent, const nap::rtti::TypeInfo& eventType) const
	{
		nap::InputEvent* key_event = nullptr;
		switch (qtEvent.type())
		{
		case QEvent::KeyRelease:
		case QEvent::KeyPress:
		{
			int wid = static_cast<int>(nap::SDL::getWindowId(mWindow));
			const auto& qt_key_event = static_cast<const QKeyEvent&>(qtEvent);
			key_event = eventType.create<nap::InputEvent>(
				{
					toNapKeyCode(static_cast<Qt::Key>(qt_key_event.key())),
					toNapKeyModifier(qt_key_event.modifiers()),
					wid
				});
			break;
		}
		default:
			break;
		}
		return key_event;
	}


	/**
	 * Helper function to convert an SDL mouse button to nap MouseButton
	 */
	static nap::PointerClickEvent::EButton toNapPointerButton(Qt::MouseButton button)
	{
		switch (button)
		{
			case Qt::LeftButton:
				return nap::PointerClickEvent::EButton::LEFT;
			case Qt::MiddleButton:
				return nap::PointerClickEvent::EButton::MIDDLE;
			case Qt::RightButton:
				return nap::PointerClickEvent::EButton::RIGHT;
			default:
				break;
		}
		return nap::PointerClickEvent::EButton::UNKNOWN;
	}


	nap::InputEvent* AppletEventConverter::translateQtMouseEvent(const QEvent& qtEvent, QPoint& ioPrevious, const nap::rtti::TypeInfo& eventType) const
	{
		// Get window id
		int wid = static_cast<int>(nap::SDL::getWindowId(mWindow));

		// Create event based on qt mouse event type
		nap::InputEvent* mouse_event = nullptr;
		switch (qtEvent.type())
		{
		case QEvent::Wheel:
		{
			const auto& qt_wheel_event = static_cast<const QWheelEvent&>(qtEvent);
			int dix = qt_wheel_event.angleDelta().rx();
			int diy = qt_wheel_event.angleDelta().ry();
			mouse_event = eventType.create<nap::InputEvent>({ dix, diy, wid });
			break;
		}
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonPress:
		{
			const auto& qt_mouse_event = static_cast<const QMouseEvent&>(qtEvent);
			auto ws = nap::SDL::getWindowSize(mWindow);
			float ratio = getPixelRatio();
			int px = static_cast<int>(static_cast<float>(qt_mouse_event.position().x()) * ratio);
			int py = ws.y - 1 - static_cast<int>(static_cast<float>(qt_mouse_event.position().y()) * ratio);
			mouse_event = eventType.create<nap::InputEvent>(
				{
					px, py, toNapPointerButton(qt_mouse_event.button()), wid, nap::PointerEvent::ESource::Mouse
				});
			ioPrevious = { -1, -1 };
			break;
		}
		case QEvent::MouseMove:
		{
			// Get position
			const auto& qt_mouse_event = static_cast<const QMouseEvent&>(qtEvent);
			auto ws = nap::SDL::getWindowSize(mWindow);
			float ratio = getPixelRatio();
			int px = static_cast<int>(static_cast<float>(qt_mouse_event.position().x()) * ratio);
			int py = ws.y - 1 - static_cast<int>(static_cast<float>(qt_mouse_event.position().y()) * ratio);
			int rx = ioPrevious.x() >= 0 ? px - ioPrevious.x() : 0;
			int ry = ioPrevious.y() >= 0 ? py - ioPrevious.y() : 0;

			mouse_event = eventType.create<nap::InputEvent>(
				{
					rx, ry, px, py, wid, nap::PointerEvent::ESource::Mouse
				});

			ioPrevious = { px, py };
			break;
		}
		default:
			assert(false);
			break;
		}
		return mouse_event;
	}


	nap::WindowEvent* AppletEventConverter::translateQtWindowEvent(const QEvent& qtEvent, const nap::rtti::TypeInfo& eventType)
	{
		// Get window id
		int wid = static_cast<int>(nap::SDL::getWindowId(mWindow));

		// Create event based on qt mouse event type
		nap::WindowEvent* window_event = nullptr;

		switch (qtEvent.type())
		{
			case QEvent::Resize:
			{
				const auto& resize_event = static_cast<const QResizeEvent&>(qtEvent);
				float ratio = getPixelRatio();
				int d1 = static_cast<int>(static_cast<float>(resize_event.size().width())  * ratio);
				int d2 = static_cast<int>(static_cast<float>(resize_event.size().height()) * ratio);
				window_event = eventType.create<nap::WindowEvent>({ d1, d2, wid });
				break;
			}
			case QEvent::Move:
			{
				const auto& move_event = static_cast<const QMoveEvent&>(qtEvent);
				window_event = eventType.create<nap::WindowMovedEvent>({
						move_event.pos().x(),move_event.pos().y(), wid
					});
				break;
			}
			default:
			{
				window_event = eventType.create<nap::WindowEvent>({ wid });
				break;
			}
		}
		return window_event;
	}


	//////////////////////////////////////////////////////////////////////////


	bool AppletEventConverter::isInputEvent(const QEvent& qtEvent) const
	{
		return isKeyEvent(qtEvent) || isMouseEvent(qtEvent);
	}


	nap::InputEventPtr AppletEventConverter::translateInputEvent(const QEvent& qtEvent)
	{
		// If it's a key event, create, map and return
		const auto& key_map = getQtKeyMap();
		auto key_it = key_map.find(qtEvent.type());
		if (key_it != key_map.end())
		{
			nap::InputEvent* key_event = translateQtKeyEvent(qtEvent, key_it->second);
			return nap::InputEventPtr(key_event);
		}

		// If it's a pointer event it generally has a button except for a move operation
		const auto& mouse_map = getQtMouseMap();
		auto mouse_it = mouse_map.find(qtEvent.type());
		if (mouse_it != mouse_map.end())
		{
			nap::InputEvent* mouse_event = translateQtMouseEvent(qtEvent, mLocation, mouse_it->second);
			return nap::InputEventPtr(mouse_event);
		}

		return nullptr;
	}


	bool AppletEventConverter::isKeyEvent(const QEvent& qtEvent) const
	{
		const auto& key_map = getQtKeyMap();
		return key_map.find(qtEvent.type()) != key_map.end();
	}


	nap::InputEventPtr AppletEventConverter::translateKeyEvent(const QEvent& qtEvent)
	{
		// Fetch the type of key event
		const auto& key_map = getQtKeyMap();
		auto key_it = key_map.find(qtEvent.type());
		if (key_it == key_map.end())
			return nullptr;

		// Translate and return unique instance
		auto* input_event = translateQtKeyEvent(qtEvent, key_it->second);
		return nap::InputEventPtr(input_event);
	}


	bool AppletEventConverter::isMouseEvent(const QEvent& qtEvent) const
	{
		const auto& mouse_map = getQtMouseMap();
		return mouse_map.find(qtEvent.type()) != mouse_map.end();
	}


	nap::InputEventPtr AppletEventConverter::translateMouseEvent(const QEvent& qtEvent)
	{
		// If it's a pointer event it generally has a button except for a move operation
		const auto& mouse_map = getQtMouseMap();
		auto mouse_it = mouse_map.find(qtEvent.type());
		if (mouse_it == mouse_map.end())
			return nullptr;

		nap::InputEvent* mouse_event = translateQtMouseEvent(qtEvent, mLocation, mouse_it->second);
		return nap::InputEventPtr(mouse_event);
	}


	bool AppletEventConverter::isWindowEvent(const QEvent& qtEvent) const
	{
		const auto& map = getQtWindowMap();
		return map.find(qtEvent.type()) != map.end();
	}


	nap::WindowEventPtr AppletEventConverter::translateWindowEvent(const QEvent& qtEvent)
	{
		const auto& map = getQtWindowMap();
		auto window_it = map.find(qtEvent.type());
		if (window_it == map.end())
			return nullptr;

		nap::WindowEvent* window_event = translateQtWindowEvent(qtEvent, window_it->second);
		return nap::WindowEventPtr(window_event);
	}


	float AppletEventConverter::getPixelRatio() const
	{
		return static_cast<float>(mContainer->devicePixelRatio());
	}
}
