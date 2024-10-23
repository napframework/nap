/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
#include <perspcameracomponent.h>

namespace nap
{
	class AudioRoadComponentInstance;
	class ComputeComponentInstance;

	/**
	 * This is a visual effect component that renders a long mesh strip and moves it around in the space randomly. The FFT
	 * audio node is used to apply distortions to the mesh in reaction to peaks and changes in the sound.
	 *
	 * The component manages double-buffered copies of the position and vertex attributes of the reference mesh. This lets
	 * the compute shaders read and write from separate locations, preventing synchronization artifacts. Then, triangle
	 * index and adjacency buffers are generated to facilitate the parallel recalculation of the mesh normals in the
	 * `ComputeNormals` shader. This component fetches the latest FFT analysis and updates the compute instances
	 * appropriately. Additionally when a camera is specified, its transform will be updated to follow the trajectory of
	 * the audio road.
	 *
	 * Due to the constant nature of NAP resources you will find that the storage buffers defined in the data file have
	 * strict element counts that are difficult to infer by eye. In the default setup they are based on a grid size of
	 * 1024 x 256 vertices (and therefore 1023 x 255 cells). Please be aware that changing the resolution of this grid
	 * currently requires the element counts of several resources to be recomputed.
	 */
	class NAPAPI AudioRoadComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(AudioRoadComponent, AudioRoadComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<PlaneMeshVec4>							mReferenceMesh;
		uint												mFrameCount = 1024;

		ResourcePtr<ParameterFloat>							mBumpAmount;
		ResourcePtr<ParameterFloat>							mSwerveSpeed;
		ResourcePtr<ParameterFloat>							mSwerveIntensity;
		ResourcePtr<ParameterFloat>							mNoiseStrength;
		ResourcePtr<ParameterFloat>							mNoiseScale;
		ResourcePtr<ParameterFloat>							mNoiseSpeed;
		ResourcePtr<ParameterFloat>							mCameraFloatHeight;
		ResourcePtr<ParameterFloat>							mCameraFollowDistance;

		ComponentPtr<FFTAudioNodeComponent>					mFFT;
		ComponentPtr<FluxMeasurementComponent>				mFluxMeasurement;
		ComponentPtr<ComputeComponent>						mComputePopulate;
		ComponentPtr<ComputeComponent>						mComputeNormals;
		ComponentPtr<PerspCameraComponent>					mCamera;
	};


	/**
	 * This is a visual effect component that renders a long mesh strip and moves it around in the space randomly. The FFT
	 * audio node is used to apply distortions to the mesh in reaction to peaks and changes in the sound.
	 *
	 * The component manages double-buffered copies of the position and vertex attributes of the reference mesh. This lets
	 * the compute shaders read and write from separate locations, preventing synchronization artifacts. Then, triangle
	 * index and adjacency buffers are generated to facilitate the parallel recalculation of the meshn normals in the
	 * `ComputeNormals` shader. This component fetches the latest FFT analysis and updates the compute instances
	 * appropriately. Additionally when a camera is specified, its transform will be updated to follow the trajectory of
	 * the audio road.
	 */
	class NAPAPI AudioRoadComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		AudioRoadComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		/**
		 * Initialize AudioRoadComponentInstance based on the AudioRoadComponent resource
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update AudioRoadComponentInstance. This is called by NAP core automatically
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

		ComponentInstancePtr<FFTAudioNodeComponent> mFFT = { this, &AudioRoadComponent::mFFT };
		ComponentInstancePtr<FluxMeasurementComponent> mFluxMeasurement = { this, &AudioRoadComponent::mFluxMeasurement };
		ComponentInstancePtr<ComputeComponent> mComputePopulateInstance = { this, &AudioRoadComponent::mComputePopulate };
		ComponentInstancePtr<ComputeComponent> mComputeNormalsInstance = { this, &AudioRoadComponent::mComputeNormals };
		ComponentInstancePtr<PerspCameraComponent> mCamera = { this, &AudioRoadComponent::mCamera };

	private:
		AudioRoadComponent* mResource = nullptr;

		UniformFloatArrayInstance* mAmpsUniform = nullptr;
		UniformFloatArrayInstance* mPrevAmpsUniform = nullptr;
		UniformFloatInstance* mFluxUniform = nullptr;
		UniformFloatInstance* mBumpUniform = nullptr;
		UniformFloatInstance* mNoiseStrengthUniform = nullptr;
		UniformFloatInstance* mNoiseScaleUniform = nullptr;
		UniformFloatInstance* mNoiseSpeedUniform = nullptr;
		UniformFloatInstance* mElapsedTimeUniform = nullptr;

		UniformVec3Instance* mOriginUniform = nullptr;
		UniformVec3Instance* mDirectionUniform = nullptr;
		UniformVec3Instance* mTangentUniform = nullptr;
		UniformVec3Instance* mUpUniform = nullptr;

		std::unique_ptr<IMesh> mWireFrameCopyMesh;

		std::array<std::unique_ptr<VertexBufferVec4>, 2> mPositionBuffers;
		std::array<std::unique_ptr<VertexBufferVec4>, 2> mNormalBuffers;

		std::unique_ptr<GPUBufferUInt> mTriangleBuffer;
		std::unique_ptr<GPUBufferInt> mAdjacencyBuffer;

		VertexBufferVec4* mCurrentPositionBuffer = nullptr;
		VertexBufferVec4* mPrevPositionBuffer = nullptr;

		VertexBufferVec4* mCurrentNormalBuffer = nullptr;
		VertexBufferVec4* mPrevNormalBuffer = nullptr;

		std::array<FFTBuffer::AmplitudeSpectrum, 2> mSpectra;

		math::SmoothOperator<glm::vec3> mCameraTranslationSmoother = { { 0.0f, 0.0f, 0.0f }, 1.0f };
		math::SmoothOperator<float> mCameraPitchSmoother = { 0.0f, 1.0f };
		math::SmoothOperator<float> mCameraRollSmoother = { 0.0f, 1.0f };
		math::SmoothOperator<float> mCameraYawSmoother = { 0.0f, 1.0f };

		float mElapsedTime = 0.0f;
		float mFluxAccumulator = 0.0f;

		uint mFrameIndex = 0;

		// Plane space axes
		static constexpr glm::vec3 sPlaneForward	= { 0.0f, -1.0f, 0.0f };
		static constexpr glm::vec3 sPlaneRight		= { 1.0f, 0.0f, 0.0f };
		static constexpr glm::vec3 sPlaneUp			= { 0.0f, 0.0f, 1.0f };

		glm::vec3 mOrigin = { 0.0f, 0.0f, 0.0f };
		glm::vec3 mDirection = sPlaneForward;
		glm::vec3 mTangent = sPlaneRight;
		glm::vec3 mUp = sPlaneUp;

		glm::quat mCameraToMeshReferenceFrame = glm::identity<glm::quat>();
	};
}
