#include "selectvideocomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <renderablemeshcomponent.h>
#include <rendertexture2d.h>

// nap::selectvideocomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectVideoComponent)
	RTTI_PROPERTY("Videos", &nap::SelectVideoComponent::mVideoFiles,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",		&nap::SelectVideoComponent::mIndex,			nap::rtti::EPropertyMetaData::Default)
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
		components.emplace_back(RTTI_OF(nap::audio::VideoAudioComponent));
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
		mVideoMesh = getEntityInstance()->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(mVideoMesh != nullptr, "%s: missing RenderableMeshComponent", mID.c_str()))
			return false;

		// Get the audio component
		/*
		mAudioComponent = getEntityInstance()->findComponent<audio::VideoAudioComponentInstance>();
		if (!errorState.check(mAudioComponent != nullptr, "%s: missing VideoAudioComponent", mID.c_str()))
			return false;
		*/

		// Select one
		selectVideo(resource->mIndex);
		return true;
	}


	void SelectVideoComponentInstance::update(double deltaTime)
	{
		assert(mCurrentVideo != nullptr);
		assert(mVideoMesh != nullptr);

		// Set the texture on the material
		MaterialInstance& video_material = mVideoMesh->getMaterialInstance();
		video_material.getOrCreateSampler<Sampler2DInstance>("yTexture")->setTexture(mCurrentVideo->getYTexture());
		video_material.getOrCreateSampler<Sampler2DInstance>("uTexture")->setTexture(mCurrentVideo->getUTexture());
		video_material.getOrCreateSampler<Sampler2DInstance>("vTexture")->setTexture(mCurrentVideo->getVTexture());
	}


	void SelectVideoComponentInstance::selectVideo(int index)
	{
		if (mCurrentVideo != nullptr)
			mCurrentVideo->stop(true);
        
		mCurrentIndex = math::clamp<int>(index, 0, mVideos.size() - 1);
		mCurrentVideo = mVideos[mCurrentIndex];
		mCurrentVideo->mLoop = true;
		//mAudioComponent->setVideo(*mCurrentVideo);
		mCurrentVideo->play();
	}


	SelectVideoComponentInstance::~SelectVideoComponentInstance()
	{

	}
}
