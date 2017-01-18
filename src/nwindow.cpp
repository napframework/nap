#include "nwindow.h"

namespace opengl
{
	// Destructor
	Window::~Window()
	{
		// Make sure object is valid
		assert(getContext() != nullptr);
		assert(getWindow() != nullptr);

		// Destroy context and window
		SDL_GL_DeleteContext(mContext);
		SDL_DestroyWindow(mWindow);
	}
}