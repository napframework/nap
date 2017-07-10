#include <inputevent.h>

// RTTI Definitions
RTTI_BEGIN_BASE_CLASS(nap::InputEvent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::KeyEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR2(nap::KeyPressEvent, nap::EKeyCode, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR2(nap::KeyReleaseEvent, nap::EKeyCode, int)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::PointerEvent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::PointerClickEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR5(nap::PointerPressEvent, int, int, nap::EMouseButton, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR5(nap::PointerReleaseEvent, int, int, nap::EMouseButton, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR5(nap::PointerDragEvent, int, int, nap::EMouseButton, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR4(nap::PointerMoveEvent, int, int, int, int)
RTTI_END_CLASS