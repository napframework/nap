#include "renderwindow.h"
#include <windowevent.h>
#include "nap/core.h"

RTTI_BEGIN_ENUM(nap::RenderWindow::EPresentationMode)
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::Immediate,	"Immediate"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::Mailbox,		"Mailbox"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::FIFO,			"FIFO")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RenderWindow)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Borderless",		&nap::RenderWindow::mBorderless,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resizable",		&nap::RenderWindow::mResizable,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleShading",	&nap::RenderWindow::mSampleShading,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Title",			&nap::RenderWindow::mTitle,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Width",			&nap::RenderWindow::mWidth,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",			&nap::RenderWindow::mHeight,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Mode",			&nap::RenderWindow::mMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor",		&nap::RenderWindow::mClearColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",		&nap::RenderWindow::mRequestedSamples,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	RenderWindow::RenderWindow(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}


	RenderWindow::~RenderWindow()
	{
		if (mWindow != nullptr)
			mRenderService->removeWindow(*this);
	}


	bool RenderWindow::init(utility::ErrorState& errorState)
	{
		if (!mRenderService->addWindow(*this, errorState))
			return false;

		// Set color to clear
		mWindow->getBackbuffer().setClearColor(mClearColor);

		// We want to respond to resize events for this window
		mWindowEvent.connect(std::bind(&RenderWindow::handleEvent, this, std::placeholders::_1));
		return true;
	}
    
    
    int RenderWindow::getHeightPixels() const
    {
        return mWindow->getBackbuffer().getSize().y;
    }
    

	int RenderWindow::getWidthPixels() const
    {
        return mWindow->getBackbuffer().getSize().x;
    }


	void RenderWindow::show()
	{
		mWindow->showWindow();
	}


	void RenderWindow::hide()
	{
		mWindow->hideWindow();
	}


	void RenderWindow::setFullscreen(bool value)
	{
		mWindow->setFullScreen(value);
		mFullscreen = value;
	}


	void RenderWindow::toggleFullscreen()
	{
		setFullscreen(!mFullscreen);
	}


	void RenderWindow::setWidth(int width)
	{
		mWindow->setSize({ width, mWindow->getSize().y });
	}


	void RenderWindow::setHeight(int height)
	{
		mWindow->setSize({ mWindow->getSize().x, height });
	}


	void RenderWindow::setClearColor(const glm::vec4& color)
	{
		mClearColor = color;
	}


	void RenderWindow::setPosition(const glm::ivec2& position)
	{
		mWindow->setPosition(position);
	}


	const glm::ivec2 RenderWindow::getPosition() const
	{
		return mWindow->getPosition();
	}


	uint RenderWindow::getNumber() const
	{
		return static_cast<uint>(getWindow()->getNumber());
	}


	math::Rect RenderWindow::getRect() const
	{
		return { 0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()) };
	}


	math::Rect RenderWindow::getRectPixels() const
	{
		return{ 0.0f, 0.0f, static_cast<float>(getWidthPixels()), static_cast<float>(getHeightPixels()) };
	}


	const BackbufferRenderTarget& RenderWindow::getBackbuffer() const
	{
		return mWindow->getBackbuffer();
	}


	BackbufferRenderTarget& RenderWindow::getBackbuffer()
	{
		return mWindow->getBackbuffer();
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


	void RenderWindow::beginRendering()
	{
		mWindow->getBackbuffer().beginRendering();
	}


	void RenderWindow::endRendering()
	{
		mWindow->getBackbuffer().endRendering();
	}
}


