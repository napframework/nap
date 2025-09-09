/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "keyboard.h"
#include <unordered_map>

namespace nap
{
	nap::uint toUtf8(EKeyCode key, KeyModifier mod)
	{
		static const std::unordered_map<EKeyCode, KeyModifier> utf8Map =
		{
			{ EKeyCode::KEY_SPACE,			0x20 },
			{ EKeyCode::KEY_EXCLAIM,		0x21 },
			{ EKeyCode::KEY_QUOTEDBL,		0x22 },
			{ EKeyCode::KEY_HASH,			0x23 },
			{ EKeyCode::KEY_DOLLAR,			0x24 },
			{ EKeyCode::KEY_PERCENT,		0x25 },
			{ EKeyCode::KEY_AMPERSAND,		0x26 },
			{ EKeyCode::KEY_QUOTE,			0x27 },
			{ EKeyCode::KEY_LEFTPAREN,		0x28 },
			{ EKeyCode::KEY_KP_LEFTPAREN,	0x28 },
			{ EKeyCode::KEY_RIGHTPAREN,		0x29 },
			{ EKeyCode::KEY_KP_RIGHTPAREN,	0x29 },
			{ EKeyCode::KEY_ASTERISK,		0x2A },
			{ EKeyCode::KEY_PLUS,			0x2B },
			{ EKeyCode::KEY_KP_PLUS,		0x2B },
			{ EKeyCode::KEY_COMMA,			0x2C },
			{ EKeyCode::KEY_KP_COMMA,		0x2C },
			{ EKeyCode::KEY_MINUS,			0x2D },
			{ EKeyCode::KEY_KP_MINUS,		0x2D },
			{ EKeyCode::KEY_PERIOD,			0x2E },
			{ EKeyCode::KEY_SLASH,			0x2F },
			{ EKeyCode::KEY_SLASH,			0x2F },
			{ EKeyCode::KEY_0,				0x30 },
			{ EKeyCode::KEY_1,				0x31 },
			{ EKeyCode::KEY_2,				0x32 },
			{ EKeyCode::KEY_3,				0x33 },
			{ EKeyCode::KEY_4,				0x34 },
			{ EKeyCode::KEY_5,				0x35 },
			{ EKeyCode::KEY_6,				0x36 },
			{ EKeyCode::KEY_7,				0x37 },
			{ EKeyCode::KEY_8,				0x38 },
			{ EKeyCode::KEY_9,				0x39 },
			{ EKeyCode::KEY_COLON,			0x3A },
			{ EKeyCode::KEY_KP_COLON,		0x3A },
			{ EKeyCode::KEY_SEMICOLON,		0x3B },
			{ EKeyCode::KEY_LESS,			0x3C },
			{ EKeyCode::KEY_KP_LESS,		0x3C },
			{ EKeyCode::KEY_EQUALS,			0x3D },
			{ EKeyCode::KEY_KP_EQUALS,		0x3D },
			{ EKeyCode::KEY_KP_GREATER,		0x3E },
			{ EKeyCode::KEY_QUESTION,		0x3F },
			{ EKeyCode::KEY_AT,				0x40 },
			{ EKeyCode::KEY_AT,				0x40 },
			{ EKeyCode::KEY_LEFTBRACKET,	0x5B },
			{ EKeyCode::KEY_BACKSLASH,		0x5C },
			{ EKeyCode::KEY_RIGHTBRACKET,	0x5D },
			{ EKeyCode::KEY_CARET,			0x5E },
			{ EKeyCode::KEY_UNDERSCORE,		0x5F },
			{ EKeyCode::KEY_BACKQUOTE,		0x60 },
			{ EKeyCode::KEY_a,				0x61 },
			{ EKeyCode::KEY_b,				0x62 },
			{ EKeyCode::KEY_c,				0x63 },
			{ EKeyCode::KEY_d,				0x64 },
			{ EKeyCode::KEY_e,				0x65 },
			{ EKeyCode::KEY_f,				0x66 },
			{ EKeyCode::KEY_g,				0x67 },
			{ EKeyCode::KEY_h,				0x68 },
			{ EKeyCode::KEY_i,				0x69 },
			{ EKeyCode::KEY_j,				0x6A },
			{ EKeyCode::KEY_k,				0x6B },
			{ EKeyCode::KEY_l,				0x6C },
			{ EKeyCode::KEY_m,				0x6D },
			{ EKeyCode::KEY_n,				0x6E },
			{ EKeyCode::KEY_o,				0x6F },
			{ EKeyCode::KEY_p,				0x70 },
			{ EKeyCode::KEY_q,				0x71 },
			{ EKeyCode::KEY_r,				0x72 },
			{ EKeyCode::KEY_s,				0x73 },
			{ EKeyCode::KEY_t,				0x74 },
			{ EKeyCode::KEY_u,				0x75 },
			{ EKeyCode::KEY_v,				0x76 },
			{ EKeyCode::KEY_w,				0x77 },
			{ EKeyCode::KEY_x,				0x78 },
			{ EKeyCode::KEY_y,				0x79 },
			{ EKeyCode::KEY_z,				0x7A },
			{ EKeyCode::KEY_KP_LEFTBRACE,	0x7B },
			{ EKeyCode::KEY_LEFTBRACE,		0x7B },
			{ EKeyCode::KEY_KP_VERTICALBAR,	0x7C },
			{ EKeyCode::KEY_PIPE,			0x7C },
			{ EKeyCode::KEY_KP_RIGHTBRACE,	0x7D },
			{ EKeyCode::KEY_RIGHTBRACE,		0x7D },
			{ EKeyCode::KEY_TILDE,			0x7E }
		};

		// Bail if code isn't available
		auto uni_it = utf8Map.find(key);
		if (uni_it == utf8Map.end())
			return 0x00; // NULL

		// Offset when shift is active
		nap::uint code = uni_it->second;
		if ((mod & static_cast<nap::uint8>(nap::EKeyModifier::Shift)) > 0 &&
			uni_it->second >= 0x61 && uni_it->second <= 0x7A)
			code -= 0x20;
		return code;
	}
}

