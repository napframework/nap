// Local Includes
#include "openglrenderer.h"

// External Includes
#include <nopengl.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	bool OpenGLRenderer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(opengl::initVideo(), "Failed to init SDL"))
			return false;

		// Set GL Attributes
		opengl::Attributes attrs;
		attrs.doubleBuffer = true;
		attrs.versionMinor = 3;
		attrs.versionMajor = 3;
		attrs.enableMultiSampling = true;
		attrs.multiSampleSamples = 8;
#if _DEBUG
		attrs.debug = true;
#endif
		opengl::setAttributes(attrs);

		RenderWindowSettings settings;
		settings.visible = false;
		mPrimaryWindow = createRenderWindow(settings, errorState);

		if (!errorState.check(opengl::init(), "Failed to init OpenGL"))
			return false;

		mPrimaryWindow->makeCurrent();

		return true;
	}
	

	// Create an opengl window
	std::unique_ptr<RenderWindow> OpenGLRenderer::createRenderWindow(const RenderWindowSettings& settings, utility::ErrorState& errorState)
	{
		// Convert settings to gl settings
		opengl::WindowSettings gl_window_settings;
		gl_window_settings.borderless = settings.borderless;
		gl_window_settings.resizable = settings.resizable;
		gl_window_settings.visible = settings.visible;
		gl_window_settings.width = settings.width;
		gl_window_settings.height = settings.height;
		gl_window_settings.title = settings.title;

		if (mPrimaryWindow != nullptr)
			gl_window_settings.share = mPrimaryWindow->getContainer();

		// Construct new window using these settings
		std::unique_ptr<opengl::Window> new_window = opengl::createWindow(gl_window_settings, errorState);
		if (new_window == nullptr)
			return nullptr;

		// Construct and return new window
		return std::make_unique<RenderWindow>(settings, std::move(new_window));
	}


	// Closes all opengl systems
	void OpenGLRenderer::shutdown()
	{
		opengl::shutdown();
	}

}

RTTI_DEFINE(nap::OpenGLRenderer)
