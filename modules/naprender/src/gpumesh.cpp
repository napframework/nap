/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpumesh.h"
#include "renderservice.h"

// External includes
#include <nap/assert.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// GPUMesh
	//////////////////////////////////////////////////////////////////////////

	GPUMesh::GPUMesh(RenderService& renderService, EMemoryUsage inUsage) :
		mRenderService(&renderService),
		mUsage(inUsage)
	{ }


	const GPUBufferNumeric* GPUMesh::findVertexBuffer(const std::string& id) const
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		if (attribute != mAttributes.end())
			return attribute->second.get();
		return nullptr;
	}


	GPUBufferNumeric& GPUMesh::getVertexBuffer(const std::string& id)
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		assert(attribute != mAttributes.end());
		return *attribute->second;
	}


	IndexBuffer& GPUMesh::getOrCreateIndexBuffer(int index)
	{
		if (index < mIndexBuffers.size())
			return *mIndexBuffers[index];

		auto index_buffer = std::make_unique<IndexBuffer>(mRenderService->getCore(), mUsage, false);
		mIndexBuffers.emplace_back(std::move(index_buffer));
		return *mIndexBuffers.back();
	}


	const IndexBuffer& GPUMesh::getIndexBuffer(int index) const
	{
		assert(index < mIndexBuffers.size());
		return *mIndexBuffers[index];
	}
}
