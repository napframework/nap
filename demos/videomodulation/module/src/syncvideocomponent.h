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
	 * Selects a video based on an index
	 * This is the resource that can be declared in JSON.
	 * This resource holds a list of video files that the instance can switch between
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
	 * Instance (runtime version) of the video selector
	 * Makes sure there is at least 1 available video and
	 * needs a render-able mesh component with a video shader to automatically
	 * update it's texture inputs. This happens every frame
	 */
	class NAPAPI SyncVideoComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SyncVideoComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectvideocomponentInstance based on the selectvideocomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectvideocomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		nap::VideoPlayer* mVideoPlayer = nullptr;						//< Video player
		RenderableMeshComponentInstance* mVideoMesh = nullptr;			//< Video plane mesh

		/**
		 * @param player the player that switched the video
		 */
		void videoChanged(VideoPlayer& player);

		// Called when video selection changes
		nap::Slot<VideoPlayer&> mVideoChangedSlot = { this, &SyncVideoComponentInstance::videoChanged };
	};
}
