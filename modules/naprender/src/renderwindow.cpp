#include "renderwindow.h"
#include "nap\windowevent.h"

RTTI_BEGIN_CLASS(nap::RenderWindow)
	RTTI_PROPERTY("Width",			&nap::RenderWindow::mWidth,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",			&nap::RenderWindow::mHeight,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Borderless",		&nap::RenderWindow::mBorderless,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resizable",		&nap::RenderWindow::mResizable,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Title",			&nap::RenderWindow::mTitle,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	RenderWindow::RenderWindow(RenderService& renderService) :
		mRenderService(&renderService)
	{
	}


	RenderWindow::~RenderWindow()
	{
		if (mWindow != nullptr)
			mRenderService->removeWindow(*this);
	}


	bool RenderWindow::init(utility::ErrorState& errorState)
	{
		mWindow = mRenderService->addWindow(*this, errorState);
		if (!errorState.check(mWindow != nullptr, "Failed to create window"))
			return false;

		// We want to respond to resize events for this window
		onWindowEvent.connect(std::bind(&RenderWindow::handleEvent, this, std::placeholders::_1));
		return true;
	}


	uint RenderWindow::getNumber() const
	{
		return static_cast<uint>(getWindow()->getNumber());
	}


	void RenderWindow::handleEvent(const Event& event)
	{
		// Update window size when resizing
		const WindowResizedEvent* resized_event = rtti_cast<const WindowResizedEvent>(&event);
		if (resized_event != nullptr)
		{
			mWindow->setSize(glm::ivec2(resized_event->mX, resized_event->mY));
		}
	}
}


