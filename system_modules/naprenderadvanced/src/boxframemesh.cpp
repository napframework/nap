/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // External Includes
#include <nap/core.h>
#include <nap/numeric.h>
#include <cameracomponent.h>
#include <renderglobals.h>
#include <renderservice.h>

// Local Includes
#include "boxframemesh.h"

// nap::BoxFrameMesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BoxFrameMesh, "Predefined box line mesh of 1 unit")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",				&nap::BoxFrameMesh::mUsage,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static std::vector<glm::vec3> getBoxFrameMeshVertices(const math::Box& box)
	{
		return
		{
			{ box.getMax().x, box.getMin().y, box.getMin().z },
			{ box.getMin().x, box.getMin().y, box.getMin().z },
			{ box.getMin().x, box.getMax().y, box.getMin().z },
			{ box.getMax().x, box.getMax().y, box.getMin().z },

			{ box.getMax().x, box.getMin().y, box.getMax().z },
			{ box.getMin().x, box.getMin().y, box.getMax().z },
			{ box.getMin().x, box.getMax().y, box.getMax().z },
			{ box.getMax().x, box.getMax().y, box.getMax().z },
		};
	}

	const static std::vector<glm::vec3> sUnitLineBox		= getBoxFrameMeshVertices({1, 1, 1 });
	const static std::vector<glm::vec3> sNormalizedLineBox	= getBoxFrameMeshVertices({2, 2, 2 });

	constexpr static uint sQuadVertCount = 4;	//< Number of vertices per quad
	constexpr static uint sQuadCount = 2;		//< Total number of quad
	constexpr static uint sLineVertCount = 2;	//< Total number of vertices per line
	constexpr static uint sLineCount = 4;		//< Total number of lines


	//////////////////////////////////////////////////////////////////////////
	// BoxFrameMesh
	//////////////////////////////////////////////////////////////////////////

	BoxFrameMesh::BoxFrameMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool BoxFrameMesh::init(utility::ErrorState& errorState)
	{
		// Create mesh instance
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		// Persistent configuration
		mMeshInstance->setNumVertices(sNormalizedLineBox.size());
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setPolygonMode(EPolygonMode::Line);
		mMeshInstance->setDrawMode(EDrawMode::Lines);
		mMeshInstance->setCullMode(ECullMode::None);

		if (!mIsSetupManually)
			setup();

		return mMeshInstance->init(errorState);
	}


	bool BoxFrameMesh::setup(const math::Box& box, utility::ErrorState& errorState)
	{
		// Generate the indices
		std::vector<uint32> indices;
		indices.reserve(sQuadCount * sQuadVertCount * 2 + sLineCount * sLineVertCount);

		// Index offsets to shape a single quad face
		static const std::vector quad_index_offsets = { 0, 1, 1, 2, 2, 3, 3, 0 };

		//  Repeat twice generating two quads
		for (uint i = 0; i < sQuadCount; i++)
		{
			const uint quad_offset = i * sQuadVertCount;
			for (uint off : quad_index_offsets)
				indices.emplace_back(quad_offset + off);
		}

		// Connect the mirroring edges of the quads
		for (uint i = 0; i < sLineCount; i++)
		{
			indices.emplace_back(i);
			indices.emplace_back(i + sQuadVertCount);
		}

		// Create attributes
		auto& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		position_attribute.setData(getBoxFrameMeshVertices(box));

		// Create the shape
		auto& shape = mMeshInstance->createShape();
		shape.setIndices(indices.data(), indices.size());

		mIsSetupManually = true;
		return true;
	}


	void BoxFrameMesh::setup()
	{
		// Generate the indices
		std::vector<uint32> indices;
		indices.reserve(sQuadCount * sQuadVertCount * 2 + sLineCount * sLineVertCount);

		// Index offsets to shape a single quad face
		static const std::vector quad_index_offsets = { 0, 1, 1, 2, 2, 3, 3, 0 };

		//  Repeat twice generating two quads
		for (uint i = 0; i < sQuadCount; i++)
		{
			const uint quad_offset = i * sQuadVertCount;
			for (uint off : quad_index_offsets)
				indices.emplace_back(quad_offset + off);
		}

		// Connect the mirroring edges of the quads
		for (uint i = 0; i < sLineCount; i++)
		{
			indices.emplace_back(i);
			indices.emplace_back(i + sQuadVertCount);
		}

		// Create attributes
		auto& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		position_attribute.setData(sUnitLineBox);

		// Create the shape
		auto& shape = mMeshInstance->createShape();
		shape.setIndices(indices.data(), indices.size());
	}


	const std::vector<glm::vec3>& BoxFrameMesh::getUnitLineBox()
	{
		return sUnitLineBox;
	}


	const std::vector<glm::vec3>& BoxFrameMesh::getNormalizedLineBox()
	{
		return sNormalizedLineBox;
	}
}
