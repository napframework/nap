#include "fftmeshcomponent.h"

// External Includes
#include <entity.h>
#include <computecomponent.h>
#include <renderglobals.h>
#include <planemeshvec4.h>
#include <mesh.h>

// nap::FFTMeshComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FFTMeshComponent)
	RTTI_PROPERTY("ReferenceMesh",		&nap::FFTMeshComponent::mReferenceMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameCount",			&nap::FFTMeshComponent::mFrameCount,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FFT",				&nap::FFTMeshComponent::mFFT,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FluxMeasurement",	&nap::FFTMeshComponent::mFluxMeasurement,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComputePopulate",	&nap::FFTMeshComponent::mComputePopulate,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComputeNormals",		&nap::FFTMeshComponent::mComputeNormals,		nap::rtti::EPropertyMetaData::Required)
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

			mFluxUniform = populate_ubo->getOrCreateUniform<UniformFloatInstance>("flux");
			if (!errorState.check(mFluxUniform != nullptr, "Missing uniform member with name `flux`"))
				return false;

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

		return true;
	}


	void FFTMeshComponentInstance::update(double deltaTime)
	{
		const auto& amps = mFFT->getFFTBuffer().getAmplitudeSpectrum();
		assert(mAmpsUniform->getNumElements() <= amps.size());

		auto amps_cut = std::vector(amps.begin(), amps.begin() + mAmpsUniform->getNumElements());
		mAmpsUniform->setValues(amps_cut);

		const auto& flux_params = mFluxMeasurement->getParameterItems();
		if (!flux_params.empty())
		{
			float offset = (flux_params.front()->mOffset != nullptr) ? flux_params.front()->mOffset->mValue : 0.0f;
			mFluxUniform->setValue(flux_params.front()->mParameter->mValue - offset);
		}

		// Positions
		auto& populate_mtl = mComputePopulateInstance->getMaterialInstance();

		uint cur = mFrameIndex % static_cast<uint>(mPositionBuffers.size());
		uint prev = (mFrameIndex + 1) % static_cast<uint>(mPositionBuffers.size());

		mCurrentPositionBuffer = mPositionBuffers[cur].get();
		mPrevPositionBuffer = mPositionBuffers[prev].get();
		static_cast<BufferBindingVec4Instance*>(&populate_mtl.getBinding("InPositions"))->setBuffer(*mPrevPositionBuffer);
		static_cast<BufferBindingVec4Instance*>(&populate_mtl.getBinding("OutPositions"))->setBuffer(*mCurrentPositionBuffer);

		// Normals
		auto& normals_mtl = mComputeNormalsInstance->getMaterialInstance();

		mCurrentNormalBuffer = mNormalBuffers[cur].get();
		mPrevNormalBuffer = mNormalBuffers[prev].get();
		static_cast<BufferBindingVec4Instance*>(&normals_mtl.getBinding("InNormals"))->setBuffer(*mPrevNormalBuffer);
		static_cast<BufferBindingVec4Instance*>(&normals_mtl .getBinding("OutNormals"))->setBuffer(*mCurrentNormalBuffer);

		static_cast<BufferBindingVec4Instance*>(&normals_mtl.getBinding("Positions"))->setBuffer(*mCurrentPositionBuffer);
		static_cast<BufferBindingUIntInstance*>(&normals_mtl.getBinding("Triangles"))->setBuffer(*mTriangleBuffer);
		static_cast<BufferBindingIntInstance*>(&normals_mtl.getBinding("Adjacency"))->setBuffer(*mAdjacencyBuffer);

		// Update for subsequent frame
		++mFrameIndex;
	}
}
