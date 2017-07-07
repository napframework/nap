#include "renderwindowresource.h"
#include "nap\windowevent.h"

RTTI_BEGIN_CLASS(nap::RenderWindowResource)
	RTTI_PROPERTY("Width",			&nap::RenderWindowResource::mWidth,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",			&nap::RenderWindowResource::mHeight,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Borderless",		&nap::RenderWindowResource::mBorderless,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resizable",		&nap::RenderWindowResource::mResizable,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Title",			&nap::RenderWindowResource::mTitle,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	RenderWindowResource::RenderWindowResource(RenderService& renderService) :
		mRenderService(&renderService)
	{
	}


	RenderWindowResource::~RenderWindowResource()
	{
		if (mWindow != nullptr)
			mRenderService->removeWindow(*this);
	}


	bool RenderWindowResource::init(utility::ErrorState& errorState)
	{
		// Let the renderservice create a window
		mWindow = mRenderService->addWindow(*this, errorState);
		if (!errorState.check(mWindow != nullptr, "Failed to create window"))
			return false;

		// We want to respond to resize events for this window
		onEvent.connect(std::bind(&RenderWindowResource::handleEvent, this, std::placeholders::_1));
		return true;
	}


	void RenderWindowResource::handleEvent(const Event& event)
	{
		// Update window size when resizing
		const WindowResizedEvent* resized_event = rtti_cast<const WindowResizedEvent>(&event);
		if (resized_event != nullptr)
		{
			mWindow->setSize(glm::ivec2(resized_event->mX, resized_event->mY));
		}
	}
}


