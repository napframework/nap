#include "windowevent.h"

RTTI_BEGIN_BASE_CLASS(nap::WindowEvent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::ParameterizedWindowEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowShownEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowHiddenEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR2(nap::WindowMovedEvent, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR2(nap::WindowResizedEvent, int, int)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowMinimizedEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowMaximizedEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowRestoredEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowEnterEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowLeaveEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowFocusGainedEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowFocusLostEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WindowCloseEvent)
RTTI_END_CLASS
