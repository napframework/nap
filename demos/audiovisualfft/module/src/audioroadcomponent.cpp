/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audioroadcomponent.h"

// External Includes
#include <entity.h>
#include <computecomponent.h>
#include <renderglobals.h>
#include <planemeshvec4.h>
#include <mesh.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/noise.hpp>

// nap::AudioRoadComponent run time class definition 
RTTI_BEGIN_CLASS(nap::AudioRoadComponent)
	RTTI_PROPERTY("ReferenceMesh",			&nap::AudioRoadComponent::mReferenceMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameCount",				&nap::AudioRoadComponent::mFrameCount,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BumpAmount",				&nap::AudioRoadComponent::mBumpAmount,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SwerveSpeed",			&nap::AudioRoadComponent::mSwerveSpeed,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SwerveIntensity",		&nap::AudioRoadComponent::mSwerveIntensity,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NoiseStrength",			&nap::AudioRoadComponent::mNoiseStrength,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NoiseScale",				&nap::AudioRoadComponent::mNoiseScale,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NoiseSpeed",				&nap::AudioRoadComponent::mNoiseSpeed,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FFT",					&nap::AudioRoadComponent::mFFT,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FluxMeasurement",		&nap::AudioRoadComponent::mFluxMeasurement,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComputePopulate",		&nap::AudioRoadComponent::mComputePopulate,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComputeNormals",			&nap::AudioRoadComponent::mComputeNormals,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Camera",					&nap::AudioRoadComponent::mCamera,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraFloatHeight",		&nap::AudioRoadComponent::mCameraFloatHeight,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraFollowDistance",	&nap::AudioRoadComponent::mCameraFollowDistance,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::AudioRoadComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioRoadComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void AudioRoadComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(ComputeComponent));
	}


	bool AudioRoadComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<AudioRoadComponent>();

		auto& mesh_instance = mResource->mReferenceMesh->getMeshInstance();
		auto& gpu_mesh = mesh_instance.getGPUMesh();

		// Create double buffered copy of the position attribute
		for (auto& position_buffer : mPositionBuffers)
		{
			auto buffer = std::make_unique<VertexBufferVec4>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);
			buffer->mCount = gpu_mesh.findVertexBuffer(vertexid::position)->getCount();
			if (!buffer->init(errorState))
				return false;

			auto reference_attribute = mesh_instance.findAttribute<glm::vec4>(vertexid::position);
			assert(reference_attribute != nullptr);
			if (!buffer->setData(reference_attribute->getData(), errorState))
				return false;

			position_buffer = std::move(buffer);
		}

		// Create double buffered copy of the normal attribute
		for (auto& normal_buffer : mNormalBuffers)
		{
			auto buffer = std::make_unique<VertexBufferVec4>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);
			buffer->mCount = gpu_mesh.findVertexBuffer(vertexid::normal)->getCount();
			if (!buffer->init(errorState))
				return false;

			auto reference_attribute = mesh_instance.findAttribute<glm::vec4>(vertexid::normal);
			assert(reference_attribute != nullptr);
			if (!buffer->setData(reference_attribute->getData(), errorState))
				return false;

			normal_buffer = std::move(buffer);
		}

		// Compute Populate
		{
			// Validate material
			auto* positions_ubo = mComputePopulateInstance->getMaterialInstance().getOrCreateUniform("UBO");
			if (!errorState.check(positions_ubo != nullptr, "Missing uniform struct with name `UBO`"))
				return false;

			mAmpsUniform = positions_ubo->getOrCreateUniform<UniformFloatArrayInstance>("amps");
			if (!errorState.check(mAmpsUniform != nullptr, "Missing uniform member with name `amps`"))
				return false;

			mPrevAmpsUniform = positions_ubo->getOrCreateUniform<UniformFloatArrayInstance>("prevAmps");
			if (!errorState.check(mPrevAmpsUniform != nullptr, "Missing uniform member with name `prevAmps`"))
				return false;

			mFluxUniform = positions_ubo->getOrCreateUniform<UniformFloatInstance>("flux");
			if (!errorState.check(mFluxUniform != nullptr, "Missing uniform member with name `flux`"))
				return false;

			mBumpUniform = positions_ubo->getOrCreateUniform<UniformFloatInstance>("bump");
			if (!errorState.check(mBumpUniform != nullptr, "Missing uniform member with name `bump`"))
				return false;

			mNoiseStrengthUniform = positions_ubo->getOrCreateUniform<UniformFloatInstance>("noiseStrength");
			if (!errorState.check(mNoiseStrengthUniform != nullptr, "Missing uniform member with name `noiseStrength`"))
				return false;

			mNoiseScaleUniform = positions_ubo->getOrCreateUniform<UniformFloatInstance>("noiseScale");
			if (!errorState.check(mNoiseScaleUniform != nullptr, "Missing uniform member with name `noiseScale`"))
				return false;

			mElapsedTimeUniform = positions_ubo->getOrCreateUniform<UniformFloatInstance>("elapsedTime");
			if (!errorState.check(mElapsedTimeUniform != nullptr, "Missing uniform member with name `elapsedTime`"))
				return false;

			mOriginUniform = positions_ubo->getOrCreateUniform<UniformVec3Instance>("origin");
			mDirectionUniform = positions_ubo->getOrCreateUniform<UniformVec3Instance>("direction");
			mTangentUniform = positions_ubo->getOrCreateUniform<UniformVec3Instance>("tangent");
			mUpUniform = positions_ubo->getOrCreateUniform<UniformVec3Instance>("up");

			for (auto& buf : mPositionBuffers)
				if (!errorState.check(buf != nullptr, "Missing position buffer"))
					return false;

			uint invocation_count = (*mPositionBuffers.begin())->getCount();
			mComputePopulateInstance->setInvocations(invocation_count);
			mComputeNormalsInstance->setInvocations(invocation_count);
		}

		// Check amplitude buffer
		if (!errorState.check(mAmpsUniform->getNumElements() <= mFFT->getFFTBuffer().getBinCount(), "The FFT bin count (%d) must be equal to or higher than the element count of the `amps` array uniform (%d)", mFFT->getFFTBuffer().getBinCount(), mAmpsUniform->getNumElements()))
			return false;

		// Triangles
		// Each three elements lists three indices of a triangle. Note that triangle index =/= vertex index.
		{
			mTriangleBuffer = std::make_unique<GPUBufferUInt>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);

			assert(mesh_instance.getNumShapes() > 0);
			static const uint sVertsPerTriangle = 3;
			mTriangleBuffer->mCount = mesh_instance.getShape(0).getNumIndices();
			if (!mTriangleBuffer->init(errorState))
				return false;

			std::vector<uint> triangle_buffer(mTriangleBuffer->getCount(), 0);
			auto* tri_ptr = triangle_buffer.data();

			TriangleIterator it(mesh_instance);
			while (!it.isDone())
			{
				auto tri = it.next();
				const auto indices = tri.indices();
				std::memcpy(tri_ptr, indices.data(), sizeof(int) * sVertsPerTriangle);
				tri_ptr += sVertsPerTriangle;
			}
			if (!mTriangleBuffer->setData(triangle_buffer, errorState))
				return false;
		}

		// Adjacency
		// Each six elements lists up to six neighboring triangle (indices)
		{
			static const uint sMaxAdjacentTrisPerVert = 6;
			const auto& connectivity = mResource->mReferenceMesh->getConnectivityMap();
			std::vector<int> adjacency_buffer(connectivity.size() * sMaxAdjacentTrisPerVert, -1);
			auto* adj_ptr = adjacency_buffer.data();

			for (const auto& tri_set : connectivity)
			{
				assert(tri_set.size() <= sMaxAdjacentTrisPerVert);
				auto* local_adj_ptr = adj_ptr;
				for (const auto& tri : tri_set)
					*local_adj_ptr++ = tri.getTriangleIndex();
				adj_ptr += 6;
			}

			mAdjacencyBuffer = std::make_unique<VertexBufferInt>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);
			mAdjacencyBuffer->mCount = adjacency_buffer.size();
			if (!mAdjacencyBuffer->init(errorState))
				return false;

			if (!mAdjacencyBuffer->setData(adjacency_buffer, errorState))
				return false;
		}

		// Camera
		if (mCamera != nullptr)
		{
			const auto& transform = mCamera->getEntityInstance()->getComponent<TransformComponentInstance>();
			mCameraTranslationSmoother = { math::extractPosition(transform.getGlobalTransform()), 1.0f };
			mCameraPitchSmoother = { 0.0f, 1.0f };
			mCameraRollSmoother = { 0.0f, 1.0f };
			mCameraYawSmoother = { 0.0f, 1.0f };

			// Create a quaternion such that the camera is aligned with the initial travel direction of the plane mesh
			// Plane object space = { x:right, y:forward, z:up }
			// World space = { x:right, y:up, z:forward }

			// Rotate 90 degrees over x
			mCameraToMeshReferenceFrame = glm::rotate(glm::identity<glm::quat>(), glm::half_pi<float>(), math::X_AXIS);

			// Rotate 180 degrees over y
			mCameraToMeshReferenceFrame *= glm::rotate(glm::identity<glm::quat>(), glm::pi<float>(), math::Y_AXIS);
		}

		return true;
	}


	void AudioRoadComponentInstance::update(double deltaTime)
	{
		auto delta_time = static_cast<float>(deltaTime);
		mElapsedTime += delta_time * mResource->mNoiseSpeed->mValue;
		mElapsedTimeUniform->setValue(mElapsedTime);

		auto cur_idx = mFrameIndex % static_cast<uint>(mPositionBuffers.size());
		auto prev_idx = (mFrameIndex + 1) % static_cast<uint>(mPositionBuffers.size());

		auto& cur_spectrum = mSpectra[cur_idx];
		cur_spectrum = mFFT->getFFTBuffer().getAmplitudeSpectrum();
		assert(mAmpsUniform->getNumElements() <= cur_spectrum.size());

		auto amps_cut = std::vector(cur_spectrum.begin(), cur_spectrum.begin() + mAmpsUniform->getNumElements());
		mPrevAmpsUniform->setValues(mAmpsUniform->getValues());
		mAmpsUniform->setValues(amps_cut);
		mBumpUniform->setValue(mResource->mBumpAmount->mValue);
		mNoiseStrengthUniform->setValue(mResource->mNoiseStrength->mValue);
		mNoiseScaleUniform->setValue(mResource->mNoiseScale->mValue);

		mFluxUniform->setValue(mFluxMeasurement->getFlux());
		mFluxAccumulator += delta_time * mResource->mSwerveSpeed->mValue;

		auto max_angle = glm::half_pi<float>() * mResource->mSwerveIntensity->mValue;
		auto roll_noise = glm::simplex<float>(glm::vec3(mFluxAccumulator, 0.0f, 0.0f));
		auto roll_theta = max_angle * roll_noise;
		auto roll = glm::rotate(glm::identity<glm::quat>(), roll_theta, sPlaneForward);

		auto pitch_noise = glm::simplex<float>(glm::vec3(0.0f, mFluxAccumulator, 0.0f));
		auto pitch_theta = max_angle * pitch_noise;
		auto pitch = glm::rotate(glm::identity<glm::quat>(), pitch_theta, sPlaneRight);

		auto yaw_noise = glm::simplex<float>(glm::vec3(0.0f, 0.0f, mFluxAccumulator));
		auto yaw_theta = max_angle * yaw_noise;
		auto yaw = glm::rotate(glm::identity<glm::quat>(), yaw_theta, -sPlaneUp);

		auto composite = roll * pitch * yaw;

		mUp = sPlaneUp * composite;
		mUpUniform->setValue(mUp);

		mDirection = sPlaneForward * composite;
		mDirectionUniform->setValue(mDirection);

		mTangent = sPlaneRight * composite;
		mTangentUniform->setValue(mTangent);

		mOrigin += mDirection * delta_time;
		mOriginUniform->setValue(mOrigin);

		if (mResource->mCamera != nullptr)
		{
			auto height = mResource->mCameraFloatHeight != nullptr ? mResource->mCameraFloatHeight->mValue : 1.0f;
			auto distance = mResource->mCameraFollowDistance != nullptr ? mResource->mCameraFollowDistance->mValue : 1.0f;
			
			auto follow_origin = mOrigin + mUp * height - mDirection * distance;
			const auto& new_translate = mCameraTranslationSmoother.update(follow_origin, delta_time);

			auto& camera_transform = mCamera->getEntityInstance()->getComponent<TransformComponentInstance>();
			const auto up_offset = mUp * 0.1f; // Move the camera up a bit to reduce clipping into the plane
			camera_transform.setTranslate(new_translate + up_offset);

			auto focus_theta = glm::atan(height/distance);
			auto cam_pitch_theta = pitch_theta - focus_theta;
			cam_pitch_theta = mCameraPitchSmoother.update(cam_pitch_theta, delta_time);
			auto cam_pitch = glm::rotate(glm::identity<glm::quat>(), cam_pitch_theta, math::X_AXIS);

			auto cam_roll_theta = mCameraRollSmoother.update(roll_theta, delta_time);
			auto cam_roll = glm::rotate(glm::identity<glm::quat>(), cam_roll_theta, math::Z_AXIS);

			auto cam_yaw_theta = mCameraYawSmoother.update(yaw_theta, delta_time);
			auto cam_yaw = glm::rotate(glm::identity<glm::quat>(), cam_yaw_theta, math::Y_AXIS);

			camera_transform.setRotate(mCameraToMeshReferenceFrame * cam_roll * cam_pitch * cam_yaw);
		}

		// Positions
		auto& populate_mtl = mComputePopulateInstance->getMaterialInstance();

		mCurrentPositionBuffer = mPositionBuffers[cur_idx].get();
		mPrevPositionBuffer = mPositionBuffers[prev_idx].get();
		static_cast<BufferBindingVec4Instance*>(populate_mtl.getOrCreateBuffer("InPositions"))->setBuffer(*mPrevPositionBuffer);
		static_cast<BufferBindingVec4Instance*>(populate_mtl.getOrCreateBuffer("OutPositions"))->setBuffer(*mCurrentPositionBuffer);

		// Normals
		auto& normals_mtl = mComputeNormalsInstance->getMaterialInstance();

		mCurrentNormalBuffer = mNormalBuffers[cur_idx].get();
		mPrevNormalBuffer = mNormalBuffers[prev_idx].get();
		static_cast<BufferBindingVec4Instance*>(normals_mtl.getOrCreateBuffer("InNormals"))->setBuffer(*mPrevNormalBuffer);
		static_cast<BufferBindingVec4Instance*>(normals_mtl .getOrCreateBuffer("OutNormals"))->setBuffer(*mCurrentNormalBuffer);

		static_cast<BufferBindingVec4Instance*>(normals_mtl.getOrCreateBuffer("Positions"))->setBuffer(*mCurrentPositionBuffer);
		static_cast<BufferBindingUIntInstance*>(normals_mtl.getOrCreateBuffer("Triangles"))->setBuffer(*mTriangleBuffer);
		static_cast<BufferBindingIntInstance*>(normals_mtl.getOrCreateBuffer("Adjacency"))->setBuffer(*mAdjacencyBuffer);

		// Update for subsequent frame
		++mFrameIndex;
	}
}
