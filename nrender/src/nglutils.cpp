// Local Includes
#include "nglutils.h"

// External Includes
#include <unordered_set>
#include <assert.h>

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
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(r, g, b, a);
		opengl::clear(GL_COLOR_BUFFER_BIT);
	}


	// Clears the depth buffer of the currently active context
	void clearDepth()
	{
		glDepthMask(GL_TRUE);
		opengl::clear(GL_DEPTH_BUFFER_BIT);
	}


	// Clears the stencil buffer of the currently active context
	void clearStencil()
	{
		glStencilMask(1);
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


	// Set the render viewport
	void setViewport(int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	// If line smoothing should be turned on or off
	void enableLineSmoothing(bool value)
	{
		if (value)
		{
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_LINE_SMOOTH);
			return;
		}
		glDisable(GL_LINE_SMOOTH);
	}


	// Set line width of rasterized lines
	void setLineWidth(float value)
	{
		glLineWidth(value);
	}


	// Get the currently active OpenGL Context
	GLContext getCurrentContext()
	{
		return SDL_GL_GetCurrentContext();
	}


	// Mode to use when drawing polygons
	void setPolygonMode(PolygonMode mode)
	{
		switch (mode)
		{
		case PolygonMode::FILL:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		case PolygonMode::LINE:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case PolygonMode::POINT:
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			break;
		default:
			assert(false);
		}
	}


	// Set render point size
	void setPointSize(float size)
	{
		glPointSize(size);
	}


	// Turn smoothing of points on or off
	void enablePointSmoothing(bool value)
	{
		if (value)
		{
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_POINT_SMOOTH);
			return;
		}
		glDisable(GL_POINT_SMOOTH);
	}


	bool printErrorMessage(char *file, int line)
	{
		GLenum error_code;

		// Get error
		error_code = glGetError();
		if (error_code != GL_NO_ERROR)
		{
			printMessage(MessageType::ERROR, "file: %s, line: %d, %s", file, line, glewGetErrorString(error_code));
#ifdef __APPLE__
            return false;
#else
            return true;
#endif
		}
		return false;
	}
}
