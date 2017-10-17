#include "renderer.h"

// External Includes
#include <nopengl.h>
#include <utility/errorstate.h>

RTTI_DEFINE(nap::Renderer)

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

		int  versionMajor = 3;			// Major GL Version
		int  versionMinor = 2;			// Minor GL Version
		bool doubleBuffer = true;		// Enables / Disabled double buffering
		bool debug = false;				// Whether to use the debug version of the OpenGL driver. Provides more debugging output.

										// TODO: FIGURE OUT WHY THERE DON'T SEEM TO HAVE AN EFFECT ON WINDOWS
		bool enableMultiSampling = 1;	// Enables / Disables multi sampling.
		int  multiSampleSamples = 4;	// Number of samples per pixel when multi sampling is enabled
	};


	// Sets OpenGL attributes
	void setOpenGLAttributes(const OpenGLAttributes& attributes)
	{
		// Set our OpenGL version.
		// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, cur_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, cur_minor);

		// Set double buffering
		int double_buffer = static_cast<int>(attributes.doubleBuffer);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, double_buffer);

		// Set multi sample parameters
		if (attributes.enableMultiSampling)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, attributes.multiSampleSamples);
		}

		// Enable debug
		if (attributes.debug)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		}
	}


	//////////////////////////////////////////////////////////////////////////

	bool Renderer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(opengl::initVideo(), "Failed to init SDL"))
			return false;

		// Set GL Attributes for creation of native render window
		OpenGLAttributes attrs;
		attrs.doubleBuffer = true;
		attrs.versionMinor = 3;
		attrs.versionMajor = 3;
		attrs.enableMultiSampling = true;
		attrs.multiSampleSamples = 8;
#ifdef _DEBUG
		attrs.debug = true;
#endif
		setOpenGLAttributes(attrs);

		// Create the primary window, this window is invisible and only
		// used to synchronize resources. Therefore it does not need to be v-synced
		RenderWindowSettings settings;
		settings.visible = false;
		settings.sync = false;

		// Create primary window
		mPrimaryWindow = std::make_shared<GLWindow>();
		if (!mPrimaryWindow->init(settings, nullptr, errorState))
			return nullptr;

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

}
