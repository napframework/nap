/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pipelinekey.h"
#include "materialinstance.h"

namespace nap
{
	PipelineKey::PipelineKey(const Shader& shader, EDrawMode drawMode, EDepthMode depthMode, EBlendMode blendMode, ShaderConstantHash constantHash, ECullWindingOrder cullWindingOrder, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, bool sampleShading, ECullMode cullMode, EPolygonMode polygonMode) :
		mShader(&shader),
		mDrawMode(drawMode),
		mDepthMode(depthMode),
		mBlendMode(blendMode),
		mConstantHash(constantHash),
		mCullWindingOrder(cullWindingOrder),
		mColorFormat(colorFormat),
		mDepthFormat(depthFormat),
		mSampleCount(sampleCount),
		mSampleShading(sampleShading),
		mCullMode(cullMode),
		mPolygonMode(polygonMode)
	{}


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
			mCullMode == rhs.mCullMode &&
			mPolygonMode == rhs.mPolygonMode &&
			mConstantHash == rhs.mConstantHash;
	}


	ComputePipelineKey::ComputePipelineKey(const ComputeShader& computeShader, ShaderConstantHash constantHash) :
		mShader(&computeShader),
		mConstantHash(constantHash)
	{}


	bool ComputePipelineKey::operator==(const ComputePipelineKey& rhs) const
	{
		return mShader == rhs.mShader &&
			mConstantHash == rhs.mConstantHash;
	}
}
