#pragma once

#include <renderablemeshcomponent.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parametercolor.h>
#include <uniforminstance.h>
#include <fftaudionodecomponent.h>

namespace nap
{
	class RenderFFTComponentInstance;
	class RenderToTextureComponentInstance;
	class ComputeComponentInstance;

	/**
	 *	RenderFFTComponent
	 */
	class NAPAPI RenderFFTComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderFFTComponent, RenderFFTComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterRGBAColorFloat>	mAmbient;
		ResourcePtr<ParameterRGBColorFloat>		mDiffuse;
		ResourcePtr<ParameterRGBColorFloat>		mSpecular;
		ResourcePtr<ParameterFloat>				mShininess;

		ComponentPtr<FFTAudioNodeComponent>		mFFT;
	};


	/**
	 * RenderFFTComponentInstance	
	 */
	class NAPAPI RenderFFTComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		RenderFFTComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableMeshComponentInstance(entity, resource) { }

		/**
		 * Initialize RenderFFTComponentInstance based on the RenderFFTComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderFFTComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update RenderFFTComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * 
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		ComponentInstancePtr<FFTAudioNodeComponent> mFFT = { this, &RenderFFTComponent::mFFT };

	private:
		RenderFFTComponent* mResource = nullptr;
		RenderToTextureComponentInstance* mRenderComponent = nullptr;
		ComputeComponentInstance* mComputeInstance = nullptr;

		UniformFloatArrayInstance* mAmpsUniform = nullptr;
	};
}
