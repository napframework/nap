// Local Includes
#include "nglutils.h"

// External Includes
#include <unordered_set>

namespace opengl
{
	// Simple opengl gl disable / enable function
	static void enableGLParam(GLenum param, bool enable)
	{
		if (enable)
		{
			glEnable(param);
			return;
		}
		glDisable(param);
	}


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
		enableGLParam(GL_DEPTH_TEST, value);
	}


	//force execution of GL commands in finite time
	void flush()
	{
		glFlush();
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


	// Turns alpha blending on / off
	void enableBlending(bool value)
	{
		enableGLParam(GL_BLEND, value);
	}


	// Turns multi sampling on / off
	void enableMultiSampling(bool value)
	{
		enableGLParam(GL_MULTISAMPLE_ARB, value);
	}
}