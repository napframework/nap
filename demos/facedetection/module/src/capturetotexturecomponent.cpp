#include "capturetotexturecomponent.h"

// External Includes
#include <entity.h>
#include <cvframe.h>
#include <nap/logger.h>

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
		mConversionFrame.addNew();
		mCaptureComponent->frameReceived.connect(mCaptureSlot);

		// Get material instance
		nap::MaterialInstance& mat_instance = mRenderComponent->getMaterialInstance();
		return true;
	}


	void CaptureToTextureComponentInstance::update(double deltaTime)
	{
		// Get detected blobs
		std::vector<math::Rect> blobs = mClassifyComponent->getObjects();

		// Set blob count
		nap::MaterialInstance& material = mRenderComponent->getMaterialInstance();
		UniformInt& blob_count_uniform = material.getOrCreateUniform<UniformInt>("blobCount");
		blob_count_uniform.setValue(blobs.size());

		// Set blob data
		int count = 0;
		for (const auto& blob : blobs)
		{
			// Set blob center
			std::string center_uniform_name = utility::stringFormat("blobs[%d].mCenter", count);
			UniformVec2& center_uniform = material.getOrCreateUniform<UniformVec2>(center_uniform_name);
			center_uniform.setValue(glm::vec2
			(
				blob.getMin().x + (blob.getWidth()  / 2.0),
				blob.getMin().y + (blob.getHeight() / 2.0)
			));

			// Set blob size
			std::string size_uniform_name = utility::stringFormat("blobs[%d].mSize", count);
			UniformFloat& size_uniform = material.getOrCreateUniform<UniformFloat>(size_uniform_name);
			size_uniform.setValue(blob.getHeight() / 2.0f);
			
			count++;
		}
	}


	void CaptureToTextureComponentInstance::onFrameCaptured(const CVFrameEvent& frameEvent)
	{
		const CVFrame* frame = frameEvent.findFrame(*mAdapter);
		if (frame == nullptr)
			return;

		// Ensure channel count is the same
		const CVFrame& cv_frame = *frame;
		int cv_channels = cv_frame[mMatrixIndex].channels();
		int te_channels = mRenderTexture->getChannelCount();
		if (!(cv_channels == te_channels))
		{
			nap::Logger::warn("%s: invalid number of channels, got %d, expect: %d", mID.c_str(),
				cv_channels, te_channels);
			return;
		}

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
		
		// Update texture
		mRenderTexture->update(cv_frame[mMatrixIndex].getMat(cv::ACCESS_READ).data);
	}
}