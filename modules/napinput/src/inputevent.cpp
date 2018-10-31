#include <inputevent.h>

// RTTI Definitions
RTTI_DEFINE_BASE(nap::InputEvent)
RTTI_DEFINE_BASE(nap::WindowInputEvent)
RTTI_DEFINE_BASE(nap::ControllerEvent)
RTTI_DEFINE_BASE(nap::ControllerButtonEvent)
RTTI_DEFINE_BASE(nap::KeyEvent)
RTTI_DEFINE_BASE(nap::PointerEvent)
RTTI_DEFINE_BASE(nap::PointerClickEvent)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KeyPressEvent)
	RTTI_CONSTRUCTOR(nap::EKeyCode, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KeyReleaseEvent)
	RTTI_CONSTRUCTOR(nap::EKeyCode, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerPressEvent)
	RTTI_CONSTRUCTOR(int, int, nap::EMouseButton, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerReleaseEvent)
	RTTI_CONSTRUCTOR(int, int, nap::EMouseButton, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerMoveEvent)
	RTTI_CONSTRUCTOR(int, int, int, int, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MouseWheelEvent)
	RTTI_CONSTRUCTOR(int, int, int)
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