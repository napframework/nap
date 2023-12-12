#pragma once

#include <renderablemeshcomponent.h>
#include <uniforminstance.h>
#include <fftmeshcomponent.h>

namespace nap
{
	class RenderFFTMeshComponentInstance;

	/**
	 *	RenderFFTMeshComponent
	 */
	class NAPAPI RenderFFTMeshComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderFFTMeshComponent, RenderFFTMeshComponentInstance)
	public:
		ComponentPtr<FFTMeshComponent> mFFTMeshComponent;
	};


	/**
	 * RenderFFTMeshComponentInstance	
	 */
	class NAPAPI RenderFFTMeshComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		RenderFFTMeshComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableMeshComponentInstance(entity, resource) { }

		/**
		 * Initialize RenderFFTMeshComponentInstance based on the RenderFFTMeshComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderFFTMeshComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * 
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		ComponentInstancePtr<FFTMeshComponent> mFFTMeshComponent = { this, &RenderFFTMeshComponent::mFFTMeshComponent };

	private:
		RenderFFTMeshComponent* mResource = nullptr;
	};
}
