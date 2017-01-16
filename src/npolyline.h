#pragma once

// Local Includes
#include "nmesh.h"

// External Includes
#include <assert.h>

namespace opengl
{
	/**
	 * Defines a polygonal line that can be drawn
	 * Every polygonal line holds a set of vertices
	 * But can have additional uv's, colors and normals
	 * Multiple UV and color sets can be assigned to the line
	 * The line is in fact a mesh with a predefined drawing mode
	 */
	class PolyLine : public Mesh
	{
	public:
		/**
		 * Default constructor
		 */
		PolyLine() = default;

		/**
		 * Set if the line is closed or not.
		 * This changes the draw method to either be: GL_LINE_STRIP or GL_LINE_LOOP
		 * @param value setting this to true will close the line so it forms a loop
		 */
		bool			setClosed(bool value)				{ mClosed = value; }

		/**
		 * Override default draw behavior
		 * The GL draw mode is defined by the type of curve, ie: closed or open
		 */
		virtual void	draw() override;

		// Remove set / get draw mode
		void			setDrawMode(DrawMode mode)	= delete;
		DrawMode		getDrawMode() const			= delete;

	private:
		bool mClosed = false;					//< If the line should be closed when drawing
	};

}	// opengl
