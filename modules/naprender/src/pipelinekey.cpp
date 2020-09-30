#include "pipelinekey.h"

namespace nap
{
	PipelineKey::PipelineKey(const Shader& shader, EDrawMode drawMode, EDepthMode depthMode, EBlendMode blendMode, ECullWindingOrder cullWindingOrder, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, bool sampleShading, ECullMode cullMode) :
		mShader(&shader),
		mDrawMode(drawMode),
		mDepthMode(depthMode),
		mBlendMode(blendMode),
		mCullWindingOrder(cullWindingOrder),
		mColorFormat(colorFormat),
		mDepthFormat(depthFormat),
		mSampleCount(sampleCount),
		mSampleShading(sampleShading),
		mCullMode(cullMode)
	{ }


	bool PipelineKey::operator==(const PipelineKey& rhs) const
	{
		return mShader == rhs.mShader && 
			mDrawMode == rhs.mDrawMode &&
			mDepthMode == rhs.mDepthMode &&
			mBlendMode == rhs.mBlendMode &&
			mCullWindingOrder == rhs.mCullWindingOrder &&
			mColorFormat == rhs.mColorFormat &&
			mDepthFormat == rhs.mDepthFormat &&
			mSampleCount == rhs.mSampleCount &&
			mSampleShading == rhs.mSampleShading &&
			mCullMode == rhs.mCullMode;
	}
}