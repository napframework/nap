#pragma once

// External Includes
#include "nglutils.h"
#include <nap/dllexport.h>

namespace nap
{
	/**
	 * Global render state object. Users can freely change values in this object.
	 *
	 * Purpose of this object is to:
	 * 1) Provide an OpenGL independent way of setting render state
	 * 2) Provide a state object that can be duplicated for multiple GL contexts
	 * 3) Minimize openGL state changes
	 *
	 * RenderService will maintain a global render state, as well as render states per GL context.
	 * Whenever objects are rendered, the RenderService will diff the global state against the
	 * context's state and update only the GL states that are necessary.
	 */
	struct NAPAPI RenderState
	{
		bool				mEnableMultiSampling = true;
		float				mLineWidth = 1.0f;
		float				mPointSize = 1.0;
		opengl::PolygonMode mPolygonMode = opengl::PolygonMode::FILL;

	private:
		friend class RenderService;

		/**
		* Forces the setting of all render states as currently set.
		*/
		void force();

		/**
		* Switches all render states as set in @targetRenderState. Only the renderStates that are different
		will actually cause openGL calls.
		*/
		void update(const RenderState& targetRenderState);
	};
}