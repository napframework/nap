#include "renderer.h"

// External Includes
#include <nopengl.h>
#include <utility/errorstate.h>

RTTI_DEFINE_CLASS(nap::Renderer)

RTTI_BEGIN_CLASS(nap::RendererSettings)
	RTTI_PROPERTY("DoubleBuffer",			&nap::RendererSettings::mDoubleBuffer,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableMultiSampling",	&nap::RendererSettings::mEnableMultiSampling,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MultiSampleSamples",		&nap::RendererSettings::mMultiSampleSamples,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	const static int minGLVersionMajor = 3;
	const static int minGLVersionMinor = 2;

	/**
	* Attributes
	*
	* Set of default opengl attributes that should be initialized before creating a context
	*/
	struct OpenGLAttributes
	{
		OpenGLAttributes() = default;
		~OpenGLAttributes() = default;

		int  versionMajor = 3;		// Major GL Version
		int  versionMinor = 2;		// Minor GL Version
		bool debug = false;			// Whether to use the debug version of the OpenGL driver. Provides more debugging output.
	};


	// Sets OpenGL attributes
	void setOpenGLAttributes(const OpenGLAttributes& attributes, const RendererSettings& rendererSettings)
	{
		// Set our OpenGL version.
		// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
		opengl::setAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
		// 4.1 is the highest available number on most OSX devices
		int cur_minor = attributes.versionMinor;
		int cur_major = attributes.versionMajor;

		// Calculate min required gl version 
		int min_version = minGLVersionMajor * 10 + minGLVersionMinor;
		int cur_version = cur_major * 10 + cur_minor;

		// CLamp based on settings
		if (cur_version < min_version)
		{
			cur_minor = minGLVersionMinor;
			cur_major = minGLVersionMajor;
		}

		// Set settings
		opengl::setAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, cur_major);
		opengl::setAttribute(SDL_GL_CONTEXT_MINOR_VERSION, cur_minor);

		// Set double buffering
		int double_buffer = static_cast<int>(rendererSettings.mDoubleBuffer);
		opengl::setAttribute(SDL_GL_DOUBLEBUFFER, double_buffer);

		// Set multi sample parameters
		if (rendererSettings.mEnableMultiSampling)
		{
			opengl::setAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			opengl::setAttribute(SDL_GL_MULTISAMPLESAMPLES, rendererSettings.mMultiSampleSamples);
		}

		// Enable debug
		if (attributes.debug)
		{
			opengl::setAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		}
	}


	//////////////////////////////////////////////////////////////////////////

	bool Renderer::init(const RendererSettings& rendererSettings, utility::ErrorState& errorState)
	{
		if (!errorState.check(opengl::initVideo(), "Failed to init SDL"))
			return false;

		// Set GL Attributes for creation of native render window
		OpenGLAttributes attrs;
		attrs.versionMinor = 3;
		attrs.versionMajor = 3;
#ifdef _DEBUG
		attrs.debug = true;
#endif
		setOpenGLAttributes(attrs, rendererSettings);

		// Create primary window
		if (!createPrimaryWindow(errorState))
			return false;

		// Make sure initialization of that window succeeded
		if (!errorState.check(opengl::init(), "Failed to init OpenGL"))
			return false;

		// Primary window is activated so that any resource construction happens on the primary window/context
		mPrimaryWindow->makeCurrent();

		return true;
	}


	// Create an opengl window
	std::shared_ptr<GLWindow> Renderer::createRenderWindow(const RenderWindowSettings& settings, const std::string& inID, utility::ErrorState& errorState)
	{
		// The primary window always exists. This is necessary to initialize openGL, and we need a window and an associated GL context for creating
		// resources before a window resource becomes available. The first RenderWindow that is created will share the primary window with the Renderer.
		// The settings that are passed here are applied to the primary window.
		// Because of the nature of the real-time editing system, when Windows are edited, new Windows will be created before old Windows are destroyed.
		// We need to make sure that an edit to a RenderWindow that was previously created as primary window will not create a new window, but share the 
		// existing primary window. We do this by testing if the IDs are the same.
		if (mPrimaryWindow.unique() || mPrimaryWindowID == inID)
		{
			mPrimaryWindowID = inID;
			mPrimaryWindow->applySettings(settings);
			return mPrimaryWindow;
		}

		// Construct and return new window
		std::shared_ptr<GLWindow> new_window = std::make_shared<GLWindow>();
		if (!new_window->init(settings, mPrimaryWindow.get(), errorState))
			return nullptr;

		return new_window;
	}


	// Closes all opengl systems
	void Renderer::shutdown()
	{
		opengl::shutdown();
	}


	bool Renderer::createPrimaryWindow(utility::ErrorState& error)
	{
		// Create the primary window, this window is invisible and only
		// used to synchronize resources. Therefore it does not need to be v-synced
		RenderWindowSettings settings;
		settings.visible = false;
		settings.sync = false;
		settings.borderless = false;
		settings.height = 512;
		settings.width = 512;
		settings.title = "";
		settings.resizable = true;
		settings.x = SDL_WINDOWPOS_CENTERED;
		settings.y = SDL_WINDOWPOS_CENTERED;

		// Create primary window
		mPrimaryWindow = std::make_shared<GLWindow>();
		return mPrimaryWindow->init(settings, nullptr, error);
	}
}
