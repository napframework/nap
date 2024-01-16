#pragma once

#include <renderablemeshcomponent.h>
#include <uniforminstance.h>
#include <AudioRoadcomponent.h>

namespace nap
{
	class RenderAudioRoadComponentInstance;

	/**
	 *	RenderAudioRoadComponent
	 */
	class NAPAPI RenderAudioRoadComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderAudioRoadComponent, RenderAudioRoadComponentInstance)
	public:
		ComponentPtr<AudioRoadComponent> mAudioRoadComponent;
	};


	/**
	 * RenderAudioRoadComponentInstance	
	 */
	class NAPAPI RenderAudioRoadComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		RenderAudioRoadComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableMeshComponentInstance(entity, resource) { }

		/**
		 * Initialize RenderAudioRoadComponentInstance based on the RenderAudioRoadComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderAudioRoadComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * 
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		ComponentInstancePtr<AudioRoadComponent> mAudioRoadComponent = { this, &RenderAudioRoadComponent::mAudioRoadComponent };

	private:
		RenderAudioRoadComponent* mResource = nullptr;
	};
}
