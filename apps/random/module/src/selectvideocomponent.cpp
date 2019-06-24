#include "selectvideocomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <renderablemeshcomponent.h>
#include <rendertexture2d.h>

// nap::selectvideocomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectVideoComponent)
	RTTI_PROPERTY("Videos", &nap::SelectVideoComponent::mVideoFiles,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",	&nap::SelectVideoComponent::mIndex,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::selectvideocomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectVideoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectVideoComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RenderableMeshComponent));
	}


	bool SelectVideoComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over video
		nap::SelectVideoComponent* resource = getComponent<SelectVideoComponent>();
		for (auto& video : resource->mVideoFiles)
			mVideos.emplace_back(video.get());

		// Make sure we have some videos
		if (!errorState.check(mVideos.size() > 0, "No video files to select"))
			return false;

		// Get the render-able mesh that has the video material
		// We know it's there because we added it as a dependency above
		mVideoMesh = getEntityInstance()->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(mVideoMesh != nullptr, "%s: missing RenderableMeshComponent", mID.c_str()))
			return false;

		// Select one
		selectVideo(resource->mIndex);
		return true;
	}


	void SelectVideoComponentInstance::update(double deltaTime)
	{
		assert(mCurrentVideo != nullptr);
		assert(mVideoMesh != nullptr);

		// Do nothing if the current video is paused
		if (!mCurrentVideo->isPlaying()) return;

		// Set the texture on the material
		MaterialInstance& video_material = mVideoMesh->getMaterialInstance();
		video_material.getOrCreateUniform<UniformTexture2D>("yTexture").setTexture(mCurrentVideo->getYTexture());
		video_material.getOrCreateUniform<UniformTexture2D>("uTexture").setTexture(mCurrentVideo->getUTexture());
		video_material.getOrCreateUniform<UniformTexture2D>("vTexture").setTexture(mCurrentVideo->getVTexture());
	}


	void SelectVideoComponentInstance::playCurrentVideo()
	{
		assert(mCurrentVideo != nullptr);

		if (!mCurrentVideo->isPlaying())
			mCurrentVideo->play();
	}


	void SelectVideoComponentInstance::stopCurrentVideo()
	{
		assert(mCurrentVideo != nullptr);

		if (mCurrentVideo->isPlaying())
			mCurrentVideo->stop(true);
	}


	void SelectVideoComponentInstance::selectVideo(int index)
	{
		bool wasPlaying = false;

		if (mCurrentVideo != nullptr)
		{
			wasPlaying = mCurrentVideo->isPlaying();
			mCurrentVideo->stop(true);
		}
        
		mCurrentIndex = math::clamp<int>(index, 0, mVideos.size() - 1);
		mCurrentVideo = mVideos[mCurrentIndex];
		mCurrentVideo->mLoop = true;

		if (wasPlaying)
			mCurrentVideo->play();
	}


	SelectVideoComponentInstance::~SelectVideoComponentInstance()
	{
	}
}
