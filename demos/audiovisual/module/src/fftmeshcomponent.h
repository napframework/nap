#pragma once

#include <renderablemeshcomponent.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parametercolor.h>
#include <uniforminstance.h>
#include <fftaudionodecomponent.h>
#include <fluxmeasurementcomponent.h>
#include <computecomponent.h>
#include <planemeshvec4.h>

namespace nap
{
	class FFTMeshComponentInstance;
	class ComputeComponentInstance;

	/**
	 *	FFTMeshComponent
	 */
	class NAPAPI FFTMeshComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FFTMeshComponent, FFTMeshComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<PlaneMeshVec4>							mReferenceMesh;
		uint												mFrameCount = 1024;
		ComponentPtr<FFTAudioNodeComponent>					mFFT;
		ComponentPtr<FluxMeasurementComponent>				mFluxMeasurement;
		ComponentPtr<ComputeComponent>						mComputePopulate;
		ComponentPtr<ComputeComponent>						mComputeNormals;
	};


	/**
	 * FFTMeshComponentInstance	
	 */
	class NAPAPI FFTMeshComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FFTMeshComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		/**
		 * Initialize FFTMeshComponentInstance based on the FFTMeshComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FFTMeshComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update FFTMeshComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return last updated position buffer
		 */
		const VertexBufferVec4& getPositionBuffer() const				{ return *mCurrentPositionBuffer; }

		/**
		 * @return last normal buffer
		 */
		const VertexBufferVec4& getNormalBuffer() const					{ return *mCurrentNormalBuffer; }

		ComponentInstancePtr<FFTAudioNodeComponent> mFFT = { this, &FFTMeshComponent::mFFT };
		ComponentInstancePtr<FluxMeasurementComponent> mFluxMeasurement = { this, &FFTMeshComponent::mFluxMeasurement };
		ComponentInstancePtr<ComputeComponent> mComputePopulateInstance = { this, &FFTMeshComponent::mComputePopulate };
		ComponentInstancePtr<ComputeComponent> mComputeNormalsInstance = { this, &FFTMeshComponent::mComputeNormals };

	private:
		FFTMeshComponent* mResource = nullptr;

		UniformFloatArrayInstance* mAmpsUniform = nullptr;
		UniformFloatInstance* mFluxUniform = nullptr;

		std::unique_ptr<IMesh> mWireFrameCopyMesh;

		std::array<std::unique_ptr<VertexBufferVec4>, 2> mPositionBuffers;
		std::array<std::unique_ptr<VertexBufferVec4>, 2> mNormalBuffers;

		std::unique_ptr<GPUBufferUInt> mTriangleBuffer;
		std::unique_ptr<GPUBufferInt> mAdjacencyBuffer;

		VertexBufferVec4* mCurrentPositionBuffer = nullptr;
		VertexBufferVec4* mPrevPositionBuffer = nullptr;

		VertexBufferVec4* mCurrentNormalBuffer = nullptr;
		VertexBufferVec4* mPrevNormalBuffer = nullptr;

		uint mFrameIndex = 0;
	};
}
