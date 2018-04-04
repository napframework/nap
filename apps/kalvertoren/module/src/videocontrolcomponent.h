#pragma once

#include <component.h>
#include <video.h>
#include <nap/resourceptr.h>

namespace nap
{
	class VideoControlComponentInstance;

	/**
	 *	controlvideocomponent
	 */
	class NAPAPI VideoControlComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(VideoControlComponent, VideoControlComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<nap::Video> mVideoPlayer = nullptr;
	};


	/**
	 * controlvideocomponentInstance	
	 */
	class NAPAPI VideoControlComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		VideoControlComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize controlvideocomponentInstance based on the controlvideocomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the controlvideocomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update controlvideocomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Renders the video to a texture target
		 */
		void render();

	private:
		nap::Video* mVideoPlayer = nullptr;
	};
}
