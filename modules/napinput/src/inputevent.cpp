#include <inputevent.h>

// RTTI Definitions
RTTI_BEGIN_BASE_CLASS(nap::InputEvent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::KeyEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::KeyPressEvent, nap::EKeyCode)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::KeyReleaseEvent, nap::EKeyCode)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::PointerEvent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::PointerClickEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR4(nap::PointerPressEvent, int, int, nap::EMouseButton, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR4(nap::PointerReleaseEvent, int, int, nap::EMouseButton, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR4(nap::PointerDragEvent, int, int, nap::EMouseButton, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR3(nap::PointerMoveEvent, int, int, int)
RTTI_END_CLASS