// Local Includes
#include "nglutils.h"

// External Includes
#include <unordered_set>

namespace opengl
{
	// Clears the back buffer bit of the currently active context
	void clear(GLuint bit)
	{
		glClear(bit);
	}


	// Clears color buffer of the currently active context
	void clearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		opengl::clear(GL_COLOR_BUFFER_BIT);
	}


	// Clears the depth buffer of the currently active context
	void clearDepth()
	{
		opengl::clear(GL_DEPTH_BUFFER_BIT);
	}


	// Clears the stencil buffer of the currently active context
	void clearStencil()
	{
		opengl::clear(GL_STENCIL_BUFFER_BIT);
	}


	// Clears the accumulated buffer of the currently active context
	void clearAccumulated()
	{
		opengl::clear(GL_ACCUM_BUFFER_BIT);
	}


	// Disable / Enable depth test
	void enableDepthTest(bool value)
	{
		if (value)
		{
			glEnable(GL_DEPTH_TEST);
			return;
		}
		glDisable(GL_DEPTH_TEST);
	}


	// Checks if the specified filter supports mip mapping
	bool isMipMap(GLint filterType)
	{
		static std::unordered_set<GLint> mipMapMinFilterTypes;
		if (mipMapMinFilterTypes.empty())
		{
			mipMapMinFilterTypes.emplace(GL_NEAREST_MIPMAP_NEAREST);
			mipMapMinFilterTypes.emplace(GL_LINEAR_MIPMAP_NEAREST);
			mipMapMinFilterTypes.emplace(GL_NEAREST_MIPMAP_LINEAR);
			mipMapMinFilterTypes.emplace(GL_LINEAR_MIPMAP_LINEAR);
		}
		return mipMapMinFilterTypes.find(filterType) != mipMapMinFilterTypes.end();
	}

}