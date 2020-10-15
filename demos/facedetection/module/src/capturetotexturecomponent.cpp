/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "capturetotexturecomponent.h"

// External Includes
#include <entity.h>
#include <cvframe.h>
#include <nap/logger.h>
#include <mathutils.h>

// nap::cvdisplaycapturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CaptureToTextureComponent)
	RTTI_PROPERTY("CaptureComponent",	&nap::CaptureToTextureComponent::mCaptureComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClassifyComponent",	&nap::CaptureToTextureComponent::mClassifyComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderComponent",	&nap::CaptureToTextureComponent::mRenderComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderTexture",		&nap::CaptureToTextureComponent::mRenderTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Adapter",			&nap::CaptureToTextureComponent::mAdapter,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MatrixIndex",		&nap::CaptureToTextureComponent::mMatrixIndex,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::cvdisplaycapturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CaptureToTextureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CaptureToTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource and copy render texture
		CaptureToTextureComponent* resource = getComponent<CaptureToTextureComponent>();
		mRenderTexture  = resource->mRenderTexture.get();
		mAdapter		= resource->mAdapter.get();
		mMatrixIndex	= resource->mMatrixIndex;

		// Ensure adapter is part of capture device
		if (!errorState.check(mCaptureComponent->getDevice().manages(*mAdapter), "%s: adapter: %s not part of %s", 
			resource->mID.c_str(),
			mAdapter->mID.c_str(), mCaptureComponent->getDevice().mID.c_str()))
			return false;

		// Now ensure matrix capture is in range
		if (!errorState.check(mMatrixIndex < mAdapter->getMatrixCount(),
			"%s: matrix index out of range, adapter: %s has only %d matrices available", resource->mID.c_str(), 
			mAdapter->mID.c_str(), mAdapter->getMatrixCount()))
			return false;

		// Assign slot when new frame is captured
		mCaptureComponent->frameReceived.connect(mCaptureSlot);

		// Try to find general uniform buffer object in fragment shader
		nap::MaterialInstance& mat_instance = mRenderComponent->getMaterialInstance();
		UniformStructInstance* ubo = mat_instance.getOrCreateUniform("UBO");
		if (!errorState.check(ubo != nullptr, "%s: missing uniform buffer object with name: 'UBO'", mID.c_str()))
			return false;

		// Locate blob count uniform, if present we can create our unique handle and store it for future use
		mBlobCountUniform = ubo->getOrCreateUniform<UniformIntInstance>("blobCount");
		if (!errorState.check(mBlobCountUniform != nullptr, "%s: missing 'blobCount' uniform", mID.c_str()))
			return false;

		// Locate blobs uniform struct array, we update individual blob elements on update
		mBlobsUniform = ubo->getOrCreateUniform<UniformStructArrayInstance>("blobs");
		if (!errorState.check(mBlobsUniform != nullptr, "%s: missing 'blobs' uniform", mID.c_str()))
			return false;

		// Add 1 matrix
		mConversionFrame.addNew();
		return true;
	}


	void CaptureToTextureComponentInstance::update(double deltaTime)
	{
		// Get detected blobs
		std::vector<math::Rect> blobs = mClassifyComponent->getObjects();

		// Set blob count
		nap::MaterialInstance& material = mRenderComponent->getMaterialInstance();
		mBlobCountUniform->setValue(blobs.size());

		// Limit amount of blobs to 20 (as defined in shader, could be a property)
		int blob_count = math::min<int>(blobs.size(), 20);
		for (int i=0; i<blob_count; i++)
		{
			// Set blob center
			UniformVec2Instance* center_uniform = (*mBlobsUniform)[i].getOrCreateUniform<UniformVec2Instance>("mCenter");
			assert(center_uniform != nullptr);
			center_uniform->setValue(glm::vec2
			(
				blobs[i].getMin().x + (blobs[i].getWidth()  / 2.0),
				blobs[i].getMin().y + (blobs[i].getHeight() / 2.0)
			));

			// Set blob size
			UniformFloatInstance* size_uniform = (*mBlobsUniform)[i].getOrCreateUniform<UniformFloatInstance>("mSize");
			assert(size_uniform != nullptr);
			size_uniform->setValue(blobs[i].getHeight() / 2.0f);
		}
	}


	void CaptureToTextureComponentInstance::onFrameCaptured(const CVFrameEvent& frameEvent)
	{
		// Ensure the event contains the data of the adapter we are interested in.
		const CVFrame* frame = frameEvent.findFrame(*mAdapter);
		if (frame == nullptr)
			return;

		// Ensure channel count is the same
		const CVFrame& cv_frame = *frame;

		// Ensure dimensions are the same
		glm::vec2 tex_size = mRenderTexture->getSize();
		if (cv_frame[mMatrixIndex].cols != tex_size.x || cv_frame[mMatrixIndex].rows != tex_size.y)
		{
			nap::Logger::warn("%s: invalid size, got %d:%d, expect: %d%d", mID.c_str(),
				tex_size.x,
				tex_size.y,
				cv_frame[mMatrixIndex].cols,
				cv_frame[mMatrixIndex].rows);
			return;
		}
		
		// Convert to RGBA, required by the Texture2D
		cv::cvtColor(cv_frame[mMatrixIndex], mConversionFrame[0], cv::COLOR_RGB2RGBA);

		// Update texture on GPU
		mRenderTexture->update(mConversionFrame[0].getMat(cv::ACCESS_READ).data, mRenderTexture->getDescriptor());
	}
}