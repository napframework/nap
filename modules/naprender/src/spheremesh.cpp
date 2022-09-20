/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "spheremesh.h"
#include "mesh.h"
#include "material.h"
#include "renderservice.h"
#include "renderglobals.h"
#include "meshutils.h"

// External Includes
#include <glm/glm.hpp>
#include <cmath>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SphereMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::SphereMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::SphereMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::SphereMesh::mPolygonMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Radius",			&nap::SphereMesh::mRadius,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rings",			&nap::SphereMesh::mRings,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sectors",		&nap::SphereMesh::mSectors,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",			&nap::SphereMesh::mColor,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	SphereMesh::SphereMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	bool SphereMesh::init(utility::ErrorState& errorState)
	{
		// Validate mesh
		if (!errorState.check(mRings > 2 && mSectors > 2, "The number of rings and sectors must be higher than 2"))
			return false;

		// Get total amount of vertices
		uint vertex_count = mRings * mSectors;

		// Vertex data
		std::vector<glm::vec3> vertices(vertex_count);
		std::vector<glm::vec3>::iterator v = vertices.begin();
		std::vector<glm::vec3> normals(vertex_count);
		std::vector<glm::vec3> ::iterator n = normals.begin();
		std::vector<glm::vec3> texcoords(vertex_count);
		std::vector<glm::vec3>::iterator t = texcoords.begin();

		float const dr = 1.0f / static_cast<float>(mRings - 1);
		float const ds = 1.0f / static_cast<float>(mSectors - 1);

		for (uint r = 0; r < mRings; r++)
		{
			for (uint s = 0; s < mSectors; s++)
			{
				float const y = sin(-(math::PI_2) + math::PI * r * dr);
				float const x = cos(math::PIX2 * s * ds) * sin(math::PI * r * dr) * -1.0f;
				float const z = sin(math::PIX2 * s * ds) * sin(math::PI * r * dr);

				// Set texture coordinates
				*t++ = {s*ds, r*dr, 0.5f };

				// Set vertex coordinates
				*v++ = { x * mRadius, y * mRadius, z * mRadius };

				// Set normal coordinates
				*n++ = { x, y, z };
			}
		}

		// Calculate sphere indices
		uint irings = mRings - 1;
		uint isectors = mSectors - 1;
		uint index_count = irings * isectors * 6;

		std::vector<uint32> indices(index_count);
		std::vector<uint32>::iterator i = indices.begin();

		for (uint r = 0; r < irings; r++)
		{
			for (uint s = 0; s < isectors; s++)
			{
				// Triangle A
				*i++ = (r * mSectors) + s;
				*i++ = (r * mSectors) + (s + 1);
				*i++ = ((r + 1) * mSectors) + (s + 1);

				// Triangle B
				*i++ = (r * mSectors) + s;
				*i++ = ((r + 1) * mSectors) + (s + 1);
				*i++ = ((r + 1) * mSectors) + s;
			}
		}

		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
		mMeshInstance->setNumVertices(vertex_count);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(mCullMode);
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setPolygonMode(mPolygonMode);

		nap::Vec3VertexAttribute& position_attribute	= mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		nap::Vec3VertexAttribute& normal_attribute		= mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::normal);
		nap::Vec3VertexAttribute& uv_attribute			= mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		nap::Vec4VertexAttribute& color_attribute		= mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

		position_attribute.setData(vertices.data(), vertex_count);
		normal_attribute.setData(normals.data(), vertex_count);
		uv_attribute.setData(texcoords.data(), vertex_count);
		color_attribute.setData({vertex_count, mColor.toVec4()});
		
		MeshShape& shape = mMeshInstance->createShape();
		shape.setIndices(indices.data(), index_count);

		return mMeshInstance->init(errorState);
	}
}
