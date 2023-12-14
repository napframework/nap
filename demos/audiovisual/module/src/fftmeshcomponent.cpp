#include "fftmeshcomponent.h"

// External Includes
#include <entity.h>
#include <computecomponent.h>
#include <renderglobals.h>
#include <planemeshvec4.h>
#include <mesh.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/vec_swizzle.hpp>

// nap::FFTMeshComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FFTMeshComponent)
	RTTI_PROPERTY("ReferenceMesh",			&nap::FFTMeshComponent::mReferenceMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameCount",				&nap::FFTMeshComponent::mFrameCount,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FFT",					&nap::FFTMeshComponent::mFFT,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FluxMeasurement",		&nap::FFTMeshComponent::mFluxMeasurement,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComputePopulate",		&nap::FFTMeshComponent::mComputePopulate,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComputeNormals",			&nap::FFTMeshComponent::mComputeNormals,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Camera",					&nap::FFTMeshComponent::mCamera,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraFloatHeight",		&nap::FFTMeshComponent::mCameraFloatHeight,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraFollowDistance",	&nap::FFTMeshComponent::mCameraFollowDistance,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::FFTMeshComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FFTMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void FFTMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(ComputeComponent));
	}


	bool FFTMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<FFTMeshComponent>();

		auto& mesh_instance = mResource->mReferenceMesh->getMeshInstance();
		GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();

		// Create double buffered copy of the position attribute
		for (uint i = 0; i < mPositionBuffers.size(); i++)
		{
			auto buffer = std::make_unique<VertexBufferVec4>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);
			buffer->mCount = gpu_mesh.findVertexBuffer(vertexid::position)->getCount();
			if (!buffer->init(errorState))
				return false;

			auto reference_attribute = mesh_instance.findAttribute<glm::vec4>(vertexid::position);
			assert(reference_attribute != nullptr);
			if (!buffer->setData(reference_attribute->getData(), errorState))
				return false;

			mPositionBuffers[i] = std::move(buffer);
		}

		// Create double buffered copy of the normal attribute
		for (uint i = 0; i < mNormalBuffers.size(); i++)
		{
			auto buffer = std::make_unique<VertexBufferVec4>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);
			buffer->mCount = gpu_mesh.findVertexBuffer(vertexid::normal)->getCount();
			if (!buffer->init(errorState))
				return false;

			auto reference_attribute = mesh_instance.findAttribute<glm::vec4>(vertexid::normal);
			assert(reference_attribute != nullptr);
			if (!buffer->setData(reference_attribute->getData(), errorState))
				return false;

			mNormalBuffers[i] = std::move(buffer);
		}

		// Compute Populate
		{
			// Validate material
			auto* populate_ubo = mComputePopulateInstance->getMaterialInstance().getOrCreateUniform("UBO");
			if (!errorState.check(populate_ubo != nullptr, "Missing uniform struct with name `UBO`"))
				return false;

			mAmpsUniform = populate_ubo->getOrCreateUniform<UniformFloatArrayInstance>("amps");
			if (!errorState.check(mAmpsUniform != nullptr, "Missing uniform member with name `amps`"))
				return false;

			mPrevAmpsUniform = populate_ubo->getOrCreateUniform<UniformFloatArrayInstance>("prevAmps");
			if (!errorState.check(mPrevAmpsUniform != nullptr, "Missing uniform member with name `prevAmps`"))
				return false;

			mFluxUniform = populate_ubo->getOrCreateUniform<UniformFloatInstance>("flux");
			if (!errorState.check(mFluxUniform != nullptr, "Missing uniform member with name `flux`"))
				return false;	

			mOriginUniform = populate_ubo->getOrCreateUniform<UniformVec3Instance>("origin");
			mDirectionUniform = populate_ubo->getOrCreateUniform<UniformVec3Instance>("direction");
			mTangentUniform = populate_ubo->getOrCreateUniform<UniformVec3Instance>("tangent");

			for (auto& buf : mPositionBuffers)
				if (!errorState.check(buf != nullptr, "Missing position buffer"))
					return false;

			uint invocation_count = (*mPositionBuffers.begin())->getCount();
			mComputePopulateInstance->setInvocations(invocation_count);
			mComputeNormalsInstance->setInvocations(invocation_count);
		}

		// Triangles
		{
			mTriangleBuffer = std::make_unique<GPUBufferUInt>(*getEntityInstance()->getCore(), EMemoryUsage::Static, true);

			assert(getMeshInstance().getNumShapes() > 0);
			static const uint sVertsPerTriangle = 3;
			mTriangleBuffer->mCount = (mesh_instance.getShape(0).getNumIndices() / sVertsPerTriangle) * sVertsPerTriangle;
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

			// Create a quaternion such that the camera is aligned with the initial travel direction of the plane mesh
			// Plane object space = { x:right, y:forward, z:up }
			// World space = { x:right, y:up, z:forward }

			// Rotate 90 degrees over x
			mCameraToMeshReferenceFrame = glm::rotate(glm::identity<glm::quat>(), glm::half_pi<float>(), sWorldRight);

			// Rotate 180 degrees over y
			mCameraToMeshReferenceFrame *= glm::rotate(glm::identity<glm::quat>(), glm::pi<float>(), sWorldUp);
		}

		return true;
	}


	void FFTMeshComponentInstance::update(double deltaTime)
	{
		float delta_time = static_cast<float>(deltaTime);
		mElapsedTime += delta_time;

		uint cur_idx = mFrameIndex % static_cast<uint>(mPositionBuffers.size());
		uint prev_idx = (mFrameIndex + 1) % static_cast<uint>(mPositionBuffers.size());

		auto& cur_spectrum = mSpectra[cur_idx];
		cur_spectrum = mFFT->getFFTBuffer().getAmplitudeSpectrum();
		assert(mAmpsUniform->getNumElements() <= amps.size());

		auto amps_cut = std::vector(cur_spectrum.begin(), cur_spectrum.begin() + mAmpsUniform->getNumElements());
		mPrevAmpsUniform->setValues(mAmpsUniform->getValues());
		mAmpsUniform->setValues(amps_cut);

		const auto& flux_params = mFluxMeasurement->getParameterItems();
		if (!flux_params.empty())
		{
			float offset = (flux_params.front()->mOffset != nullptr) ? flux_params.front()->mOffset->mValue : 0.0f;
			float value = flux_params.front()->mParameter->mValue - offset;
			mFluxUniform->setValue(value);
			mFluxAccumulator += value * delta_time;
		}
		float flux_time = mElapsedTime * 0.1f + mFluxAccumulator * 2.0f;

		// Test
		flux_time = mElapsedTime * 0.5f;

		float roll_noise = glm::simplex<float>(glm::vec3(flux_time, 0.0f, 0.0f));
		float roll_theta = 0.5f * glm::quarter_pi<float>() * roll_noise;
		auto roll = glm::rotate(glm::identity<glm::quat>(), roll_theta, sPlaneForward);

		float pitch_noise = glm::simplex<float>(glm::vec3(0.0f, flux_time, 0.0f));
		float pitch_theta = glm::quarter_pi<float>() * pitch_noise;
		auto pitch = glm::rotate(glm::identity<glm::quat>(), pitch_theta, sPlaneRight);

		auto composite = pitch * roll;
		auto new_direction = mDirection * composite;
		mOrigin += new_direction * delta_time;

		mOriginUniform->setValue(mOrigin);
		mDirectionUniform->setValue(new_direction);

		auto new_tangent = mTangent * composite;
		mTangentUniform->setValue(new_tangent);

		if (mResource->mCamera != nullptr)
		{
			glm::vec3 camera_up = glm::normalize(glm::cross(new_direction, new_tangent));
			glm::vec3 follow_origin = mOrigin + camera_up * mResource->mCameraFloatHeight - new_direction * mResource->mCameraFollowDistance;
			const glm::mat4& mesh_world = getEntityInstance()->getComponent<TransformComponentInstance>().getGlobalTransform();
			glm::vec3 follow_origin_world = glm::xyz(mesh_world * glm::vec4(follow_origin, 1.0f));
			const glm::vec3& new_translate = mCameraTranslationSmoother.update(follow_origin_world, delta_time);

			auto& camera_transform = mCamera->getEntityInstance()->getComponent<TransformComponentInstance>();
			camera_transform.setTranslate(new_translate);

			float focus_theta = glm::tan(mResource->mCameraFloatHeight/mResource->mCameraFollowDistance);
			float cam_pitch_theta = pitch_theta - focus_theta;
			cam_pitch_theta = mCameraPitchSmoother.update(cam_pitch_theta, delta_time);
			glm::quat cam_pitch = glm::rotate(glm::identity<glm::quat>(), cam_pitch_theta, sWorldRight);

			float cam_roll_theta = mCameraRollSmoother.update(roll_theta, delta_time);
			glm::quat cam_roll = glm::rotate(glm::identity<glm::quat>(), cam_roll_theta, sWorldForward);
			camera_transform.setRotate(mCameraToMeshReferenceFrame * cam_pitch * cam_roll);
		}

		// Positions
		auto& populate_mtl = mComputePopulateInstance->getMaterialInstance();

		mCurrentPositionBuffer = mPositionBuffers[cur_idx].get();
		mPrevPositionBuffer = mPositionBuffers[prev_idx].get();
		static_cast<BufferBindingVec4Instance*>(&populate_mtl.getBinding("InPositions"))->setBuffer(*mPrevPositionBuffer);
		static_cast<BufferBindingVec4Instance*>(&populate_mtl.getBinding("OutPositions"))->setBuffer(*mCurrentPositionBuffer);

		// Normals
		auto& normals_mtl = mComputeNormalsInstance->getMaterialInstance();

		mCurrentNormalBuffer = mNormalBuffers[cur_idx].get();
		mPrevNormalBuffer = mNormalBuffers[prev_idx].get();
		static_cast<BufferBindingVec4Instance*>(&normals_mtl.getBinding("InNormals"))->setBuffer(*mPrevNormalBuffer);
		static_cast<BufferBindingVec4Instance*>(&normals_mtl .getBinding("OutNormals"))->setBuffer(*mCurrentNormalBuffer);

		static_cast<BufferBindingVec4Instance*>(&normals_mtl.getBinding("Positions"))->setBuffer(*mCurrentPositionBuffer);
		static_cast<BufferBindingUIntInstance*>(&normals_mtl.getBinding("Triangles"))->setBuffer(*mTriangleBuffer);
		static_cast<BufferBindingIntInstance*>(&normals_mtl.getBinding("Adjacency"))->setBuffer(*mAdjacencyBuffer);

		// Update for subsequent frame
		++mFrameIndex;
	}
}
