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

	const static std::vector<glm::vec3> unitLineBox			= getBoxFrameMeshVertices({ 1,1,1 });
	const static std::vector<glm::vec3> normalizedLineBox	= getBoxFrameMeshVertices({ 2,2,2 });

	constexpr static uint quadVertCount = 4;					//< Number of vertices per quad
	constexpr static uint quadCount = 2;						//< Total number of quad
	constexpr static uint lineVertCount = 2;					//< Total number of vertices per line
	constexpr static uint lineCount = 4;						//< Total number of lines
	constexpr static uint primitiveCount = 6;					//< Total number of primitives


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
		mMeshInstance->setNumVertices(unitLineBox.size());
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setPolygonMode(EPolygonMode::Line);
		mMeshInstance->setDrawMode(EDrawMode::LineStrip);
		mMeshInstance->setCullMode(ECullMode::None);

		if (!mIsSetupManually)
			setup();

		return mMeshInstance->init(errorState);
	}


	bool BoxFrameMesh::setup(const math::Box& box, utility::ErrorState& errorState)
	{
		// Generate the indices
		std::vector<uint32> indices(quadCount * quadVertCount + 2 + lineCount * lineVertCount + primitiveCount);
		uint32* index_ptr = indices.data();
		for (uint quad = 0; quad < quadCount; quad++)
		{
			const uint offset = quad * quadVertCount;
			*(index_ptr++) = 0 + offset;
			*(index_ptr++) = 1 + offset;
			*(index_ptr++) = 2 + offset;
			*(index_ptr++) = 3 + offset;
			*(index_ptr++) = 0 + offset;
			*(index_ptr++) = index::primitiveRestartIndex; // Base this on the index buffer data type (uint16 or uint32)
		}

		for (uint line = 0; line < lineCount; line++)
		{
			*(index_ptr++) = line;
			*(index_ptr++) = line + quadVertCount;
			*(index_ptr++) = index::primitiveRestartIndex;
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
		std::vector<uint32> indices(quadCount * quadVertCount + 2 + lineCount * lineVertCount + primitiveCount);
		uint32* index_ptr = indices.data();
		for (uint quad = 0; quad < quadCount; quad++)
		{
			const uint offset = quad * quadVertCount;
			*(index_ptr++) = 0 + offset;
			*(index_ptr++) = 1 + offset;
			*(index_ptr++) = 2 + offset;
			*(index_ptr++) = 3 + offset;
			*(index_ptr++) = 0 + offset;
			*(index_ptr++) = index::primitiveRestartIndex; // Base this on the index buffer data type (uint16 or uint32)
		}

		for (uint line = 0; line < lineCount; line++)
		{
			*(index_ptr++) = line;
			*(index_ptr++) = line + quadVertCount;
			*(index_ptr++) = index::primitiveRestartIndex;
		}

		// Create attributes
		auto& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		position_attribute.setData(unitLineBox);

		// Create the shape
		auto& shape = mMeshInstance->createShape();
		shape.setIndices(indices.data(), indices.size());
	}


	const std::vector<glm::vec3>& BoxFrameMesh::getUnitLineBox()
	{
		return unitLineBox;
	}


	const std::vector<glm::vec3>& BoxFrameMesh::getNormalizedLineBox()
	{
		return normalizedLineBox;
	}
}
