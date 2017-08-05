#include "windowevent.h"

RTTI_BEGIN_BASE_CLASS(nap::WindowEvent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::ParameterizedWindowEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowShownEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowHiddenEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR3(nap::WindowMovedEvent, int, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR3(nap::WindowResizedEvent, int, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowMinimizedEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowMaximizedEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowRestoredEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowEnterEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowLeaveEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowFocusGainedEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowFocusLostEvent, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::WindowCloseEvent, int)
RTTI_END_CLASS