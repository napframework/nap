#pragma once

// External includes
#include <component.h>
#include <vector>
#include <renderablemeshcomponent.h>
#include <videoaudiocomponent.h>
#include <videoplayer.h>
#include <nap/resourceptr.h>


namespace nap
{
	class SyncVideoComponentInstance;

	/**
	 * Resource part of the SyncVideoComponent.
	 * Binds the YUV (video) textures to the video material.
	 */
	class NAPAPI SyncVideoComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SyncVideoComponent, SyncVideoComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		nap::ResourcePtr<VideoPlayer> mVideoPlayer;		///< Property: "VideoPlayer" link to the video player
	};


	/**
	 * Instance (runtime version) of the SyncVideoComponent.
	 * Binds the YUV (video) textures to the video material.
	 * This happens on initialization and every time a new video is selected.
	 */
	class NAPAPI SyncVideoComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SyncVideoComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize SyncVideoComponentInstance based on the SyncVideoComponentInstance resource
		 * @param errorState contains the error if initialization fails
		 * @return if the SyncVideoComponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		nap::VideoPlayer* mVideoPlayer = nullptr;						//< Video player
		RenderableMeshComponentInstance* mVideoMesh = nullptr;			//< Video plane mesh

		/**
		 * Called every time a new video is selected
		 * @param player the video player that switched the video.
		 */
		void videoChanged(VideoPlayer& player);

		// Called when video selection changes
		nap::Slot<VideoPlayer&> mVideoChangedSlot = { this, &SyncVideoComponentInstance::videoChanged };
	};
}
