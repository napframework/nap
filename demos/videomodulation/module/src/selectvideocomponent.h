#pragma once

// Local Includes
#include "video.h"

#include <component.h>
#include <vector>
#include <renderablemeshcomponent.h>

namespace nap
{
	class SelectVideoComponentInstance;

	/**
	 * Selects a video based on an index
	 * This is the resource that can be declared in JSON.
	 * This resource holds a list of video files that the instance can switch between
	 */
	class NAPAPI SelectVideoComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectVideoComponent, SelectVideoComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		std::vector<ObjectPtr<Video>> mVideoFiles;		///< Property: "Videos" link to videos
		int mIndex = 0;									///< Property: "Index" current video index
	};


	/**
	 * Instance (runtime version) of the video selector
	 * Makes sure there is at least 1 available video and
	 * needs a render-able mesh component with a video shader to automatically
	 * update it's texture inputs. This happens every frame
	 */
	class NAPAPI SelectVideoComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectVideoComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~SelectVideoComponentInstance() override;

		/**
		 * Initialize selectvideocomponentInstance based on the selectvideocomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectvideocomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update selectvideocomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the current playing video, valid after initialization
		 */
		Video* getCurrentVideo()								{ return mCurrentVideo; }

		/**
		 * Selects a new video
		 * @param index new video index, clamped to video range
		 */
		void selectVideo(int index);

		/**
		 * @return the number of selectable videos
		 */
		int getCount() const									{ return mVideos.size(); }

		/**
		* @return current mesh index
		*/
		int getIndex() const									{ return mCurrentIndex; }

	private:
		std::vector<Video*> mVideos;							//< All video files from loaded resource
		int mCurrentIndex = 0;									//< Current video index
		Video* mCurrentVideo = nullptr;							//< Current playing video
		RenderableMeshComponentInstance* mVideoMesh = nullptr;	//< Videoplane
	};
}
