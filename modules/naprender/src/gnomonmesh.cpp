/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gnomonmesh.h"

// External Includes
#include <renderservice.h>
#include <nap/core.h>
#include <renderglobals.h>
#include <meshutils.h>

// nap::gnomon run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GnomonMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Size",		&nap::GnomonMesh::mSize,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Position",	&nap::GnomonMesh::mPosition,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	GnomonMesh::GnomonMesh(Core& core) : mRenderService(core.getService<nap::RenderService>())
	{ }

	bool GnomonMesh::init(utility::ErrorState& errorState)
	{
		static const std::vector<glm::vec3> gnomonPositions
		{
			{ 0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f }
		};

		static const std::vector<glm::vec4> gnomonColors
		{
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f }
		};

		// Create mesh
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<nap::MeshInstance>(*mRenderService);

		// Add vertex position and color attribute
		nap::VertexAttribute<glm::vec3>& pattr = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		nap::VertexAttribute<glm::vec4>& cattr = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::color);

		// Calculate vertices
		std::vector<glm::vec3> v_pos;
		v_pos.reserve(gnomonPositions.size());
		for (const auto& it : gnomonPositions)
			v_pos.emplace_back((it * mSize) + mPosition);

		// Set data
		pattr.setData(v_pos);
		cattr.setData(gnomonColors);

		// Create shape and generate indicies
		nap::MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, 6, false);

		// Set other mesh properties
		mMeshInstance->setCullMode(ECullMode::None);
		mMeshInstance->setNumVertices(6);
		mMeshInstance->setDrawMode(EDrawMode::Lines);
		mMeshInstance->setUsage(EMemoryUsage::Static);

		// Initialize
		return mMeshInstance->init(errorState);
	}
}