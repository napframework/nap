/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <inputevent.h>

RTTI_DEFINE_BASE(nap::InputEvent)
RTTI_DEFINE_BASE(nap::WindowInputEvent)
RTTI_DEFINE_BASE(nap::ControllerEvent)
RTTI_DEFINE_BASE(nap::ControllerButtonEvent)
RTTI_DEFINE_BASE(nap::KeyEvent)
RTTI_DEFINE_BASE(nap::PointerEvent)
RTTI_DEFINE_BASE(nap::PointerClickEvent)
RTTI_DEFINE_BASE(nap::TouchEvent)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KeyPressEvent)
	RTTI_CONSTRUCTOR(nap::EKeyCode, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KeyReleaseEvent)
	RTTI_CONSTRUCTOR(nap::EKeyCode, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerPressEvent)
	RTTI_CONSTRUCTOR(int, int, nap::EMouseButton, int, nap::PointerEvent::ESource)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerReleaseEvent)
	RTTI_CONSTRUCTOR(int, int, nap::EMouseButton, int, nap::PointerEvent::ESource)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerMoveEvent)
	RTTI_CONSTRUCTOR(int, int, int, int, int, nap::PointerEvent::ESource)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MouseWheelEvent)
	RTTI_CONSTRUCTOR(int, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TouchPressEvent)
	RTTI_CONSTRUCTOR(nap::int64, nap::int64, int, int, float, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TouchReleaseEvent)
	RTTI_CONSTRUCTOR(nap::int64, nap::int64, int, int, float, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TouchMoveEvent)
	RTTI_CONSTRUCTOR(nap::int64, nap::int64, int, int, float, int, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControllerButtonPressEvent)
	RTTI_CONSTRUCTOR(int, nap::EControllerButton, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControllerButtonReleaseEvent)
	RTTI_CONSTRUCTOR(int, nap::EControllerButton, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControllerAxisEvent)
	RTTI_CONSTRUCTOR(int, nap::EControllerAxis, int, double)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControllerConnectionEvent)
	RTTI_CONSTRUCTOR(int, bool)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TextInputEvent)
	RTTI_CONSTRUCTOR(const std::string&, int)
RTTI_END_CLASS
