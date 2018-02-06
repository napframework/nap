#pragma once

// External Includes
#include <GL/glew.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <iostream>
#include <array>

namespace opengl
{
	using GLContext = void*;

	/**
	* Clears the back-buffer bit of the currently active context
	*/
	void clear(GLuint bit);


	/**
	 * Clears color buffer of the currently active context
	 */
	void clearColor(float r, float g, float b, float a);


	/**
	 * Clears the depth buffer of the currently active context
	 */
	void clearDepth();


	/**
	 * Clears the stencil buffer of the currently active context
	 */
	void clearStencil();


	/**
	 * Clears the accumulated buffer of the currently active context
	 */
	void clearAccumulated();


	/**
	 * Enable / Disable depth test
	 */
	void enableDepthTest(bool value);


	/**
	 * Checks if the specified filter supports mip mapping
	 * @param filterType the gl texture resize filter type
	 */
	bool isMipMap(GLint filterType);


	/**
	 * Enables / Disables opengl alpha blending
	 * @param value if blending is enabled or disabled
	 */
	void enableBlending(bool value);


	/**
	 * Enables / Disables opengl multi sampling
	 * @param value if multi sampling is disabled / enabled
	 */
	void enableMultiSampling(bool value);


	/**
	* force execution of GL commands in finite time
	*/
	void flush();


	/**
	* updates the render viewport
	*/
	void setViewport(int width, int height);


	/**
	 * Enables / disables line smoothing
	 * @param value: if line smoothing should be turned on or off
	 */
	void enableLineSmoothing(bool value);


	/**
	 * Sets the line width
	 * @param value: the line width of rasterized lines
	 */
	void setLineWidth(float value);


	/**
	 * Enum used for specifying polygon mode
	 */
	enum class PolygonMode : std::uint8_t
	{
		POINT	= 0,			///< Render as points
		LINE	= 1,			///< Render as lines
		FILL	= 2				///< Render polygons
	};

	/**
	 * Select the polygon rasterization mode
	 * @param mode: the rasterization mode to use
	 */
	void setPolygonMode(PolygonMode mode);

	/**
	 * Set rasterization point size
	 * @param size: new point size used when rendering
	 */
	void setPointSize(float size);

	/**
	 * Turns point smoothing on / off
	 * @param value: if point smoothing should be turned on or off
	 */
	void enablePointSmoothing(bool value);

	/**
	 * Reads the depth value from a depth buffer at the x and y pixel coordinate
	 * The currently bound depth buffer is used to retrieve the value.
	 * Uses the opengl pixel mapping where 0,0 is the lower left corner
	 * Don't use this call to iterate over a set of pixels, only for single lookups
	 * @param x the horizontal pixel coordinate
	 * @param y the vertical pixel coordinate
	 * @return depth value from active depth buffer
	 */
	float getDepth(int x, int y);

	/**
	 * Reads the color value from a color buffer at the x and y pixel coordinate
	 * The currently active color buffer is used to retrieve the value
	 * Don't use this call to iterate over a set of pixels, only for single lookups
	 * The call doesn't convert the pixel data and only works with RGB color buffers (a window)
	 * @param x the horizontal pixel coordinate
	 * @param y the vertical pixel coordinate
	 * @return color value from active color buffer
	 */
	void getColor(int x, int y, std::array<std::uint8_t, 3>& color);

	/**
	 * Simple string formatter
	 */
	template <typename... Args>	
	std::string formatString(const std::string& format, Args&&... args)
	{
		size_t size = (size_t)(snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1); // Extra space for '\0'
		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, format.c_str(), std::forward<Args>(args)...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}


	/**
	 * Prints a message
	 */
	enum class MessageType : uint8_t
	{
		INFO = 0,
		WARNING = 1,
		ERROR = 2
	};

	/**
	 * Print opengl related message with x amount of arguments
	 */
	template <typename... Args>
	void printMessage(MessageType type, const std::string& msg, Args&&... args)
	{
		std::string output_msg = "OpenGL ";
		switch (type)
		{
		case MessageType::INFO:
			output_msg += "INFO: ";
			break;
		case MessageType::WARNING:
			output_msg += "WARNING: ";
			break;
		case MessageType::ERROR:
			output_msg += "ERROR: ";
			break;
		}
		
		output_msg += opengl::formatString(msg, std::forward<Args>(args)...);
		std::cout << output_msg.c_str() << std::endl;
	}


	/**
	 * Assert when something went wrong with an opengl call
	 */
	#define glAssert() assert(!opengl::printErrorMessage((char*)__FILE__, __LINE__))


	 /**
	 * Checks for opengl errors and print error as message
	 * @return: true if something went wrong
	 */
	bool printErrorMessage(char *file, int line);
}	// opengl
