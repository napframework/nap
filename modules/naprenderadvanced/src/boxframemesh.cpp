/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // External Includes
#include <nap/core.h>
#include <nap/numeric.h>
#include <cameracomponent.h>
#include <orthocameracomponent.h>
#include <perspcameracomponent.h>

// Local Includes
#include "boxframemesh.h"
#include "renderservice.h"
#include "renderglobals.h"

// nap::BoxFrameMesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BoxFrameMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("PolygonMode",		&nap::BoxFrameMesh::mPolygonMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",				&nap::BoxFrameMesh::mUsage,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	const static std::vector<glm::vec3> unitLineBox =
	{
		{ 1, -1, 1 },
		{ -1, -1, 1 },
		{ -1, 1, 1 },
		{ 1, 1, 1 },

		{ 1, -1, -1 },
		{ -1, -1, -1 },
		{ -1, 1, -1 },
		{ 1, 1, -1 }
	};

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
		setup();
		return mMeshInstance->init(errorState);
	}


	void BoxFrameMesh::setup()
	{
		// Create mesh instance
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

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
			*(index_ptr++) = std::numeric_limits<uint32>::max(); // Base this on the index buffer data type (uint16 or uint32)
		}

		for (uint line = 0; line < lineCount; line++)
		{
			*(index_ptr++) = line;
			*(index_ptr++) = line + quadVertCount;
			*(index_ptr++) = std::numeric_limits<uint32>::max();
		}

		// Create attributes
		nap::Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);

		// Set numer of vertices this mesh contains
		mMeshInstance->setNumVertices(unitLineBox.size());
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setPolygonMode(mPolygonMode);
		mMeshInstance->setDrawMode(EDrawMode::LineStrip);
		mMeshInstance->setCullMode(ECullMode::None);

		// Set data
		position_attribute.setData(unitLineBox);

		// Create the shape
		MeshShape& shape = mMeshInstance->createShape();
		shape.setIndices(indices.data(), indices.size());
	}


	const std::vector<glm::vec3>& BoxFrameMesh::getUnitLineBox()
	{
		return unitLineBox;
	}
}
