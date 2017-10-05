#include "renderstate.h"
#include "assert.h"

namespace nap
{
	// Forces the setting of all render states as currently set.
	void RenderState::force()
	{
		opengl::enableMultiSampling(mEnableMultiSampling);
		glAssert();
		opengl::setLineWidth(mLineWidth);
        glAssert(); 
		opengl::setPointSize(mPointSize);
		glAssert(); 
		opengl::setPolygonMode(mPolygonMode);
		glAssert(); 
	}


	// Switches all render states as set in @targetRenderState. Only the renderStates that are different
	// will actually cause openGL calls.
	void RenderState::update(const RenderState& targetRenderState)
	{
		if (targetRenderState.mEnableMultiSampling != mEnableMultiSampling)
		{
			opengl::enableMultiSampling(targetRenderState.mEnableMultiSampling);
		}
		if (targetRenderState.mLineWidth != mLineWidth)
		{
			opengl::setLineWidth(targetRenderState.mLineWidth);
		}
		if (targetRenderState.mPointSize != mPointSize)
		{
			opengl::setPointSize(targetRenderState.mPointSize);
		}
		if (targetRenderState.mPolygonMode != mPolygonMode)
		{
			opengl::setPolygonMode(targetRenderState.mPolygonMode);
		}
		*this = targetRenderState;
	}
}