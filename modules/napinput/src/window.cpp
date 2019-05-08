// Local Includes
#include "window.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Window)
RTTI_END_CLASS

namespace nap
{
	void Window::addEvent(WindowEventPtr inEvent)
	{
		mWindowEvents.emplace_back(std::move(inEvent));
	}

	void Window::processEvents()
	{
		for (auto& event : mWindowEvents)
			mWindowEvent(*event);

		mWindowEvents.clear();
	}
}