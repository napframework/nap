/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "QuiltCameraBoxMesh.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <nap/core.h>
#include <nap/numeric.h>

// nap::QuiltCameraBoxMesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::QuiltCameraBoxMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::QuiltCameraBoxMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::QuiltCameraBoxMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::QuiltCameraBoxMesh::mPolygonMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::QuiltCameraBoxMesh::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Position",		&nap::QuiltCameraBoxMesh::mPosition,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",			&nap::QuiltCameraBoxMesh::mColor,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	const static std::vector<glm::vec3> unitBox =
	{
		{ 0.5f,-0.5f,0.5f },
		{ -0.5f,-0.5f,0.5f },
		{ 0.5f,0.5f,0.5f },
		{ -0.5f,0.5f,0.5f },
		{ -0.5f,-0.5f,-0.5f },
		{ 0.5f,-0.5f,-0.5f },
		{ -0.5f,0.5f,-0.5f },
		{ 0.5f,0.5f,-0.5f },
		{ -0.5f,0.5f,-0.5f },
		{ 0.5f,0.5f,-0.5f },
		{ -0.5f,0.5f,0.5f },
		{ 0.5f,0.5f,0.5f },
		{ 0.5f,-0.5f,-0.5f },
		{ -0.5f,-0.5f,-0.5f },
		{ 0.5f,-0.5f,0.5f },
		{ -0.5f,-0.5f,0.5f },
		{ 0.5f,-0.5f,-0.5f },
		{ 0.5f,-0.5f,0.5f },
		{ 0.5f,0.5f,-0.5f },
		{ 0.5f,0.5f,0.5f },
		{ -0.5f,-0.5f,0.5f },
		{ -0.5f,-0.5f,-0.5f },
		{ -0.5f,0.5f,0.5f },
		{ -0.5f,0.5f,-0.5f }
	};


	const static std::vector<glm::vec3> boxNormals =
	{
		{ 0.0f,0.0f,-1.0f },
		{ 0.0f,0.0f,-1.0f },
		{ 0.0f,0.0f,-1.0f },
		{ 0.0f,0.0f,-1.0f },
		{ 0.0f,0.0f,1.0f },
		{ 0.0f,0.0f,1.0f },
		{ 0.0f,0.0f,1.0f },
		{ 0.0f,0.0f,1.0f },
		{ 0.0f,-1.0f,0.0f },
		{ 0.0f,-1.0f,0.0f },
		{ 0.0f,-1.0f,0.0f },
		{ 0.0f,-1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ -1.0f,0.0f,0.0f },
		{ -1.0f,0.0f,0.0f },
		{ -1.0f,0.0f,0.0f },
		{ -1.0f,0.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 1.0f,0.0f,0.0f }
	};


	const std::vector<glm::vec3> boxUVs =
	{
		{ 1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 1.0f,1.0f,0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f },
		{ 1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,1.0f,0.0f },
		{ 0.0f,1.0f,0.0f }
	};

	constexpr nap::uint planeVertCount = 4;					//< Number of vertices per plane
	constexpr nap::uint boxVertCount = planeVertCount * 6;	//< Total number of box vertices
	constexpr nap::uint triCount = 6 * 2;					//< Total number of box triangles

	QuiltCameraBoxMesh::QuiltCameraBoxMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool QuiltCameraBoxMesh::init(utility::ErrorState& errorState)
	{
		setup();
		return mMeshInstance->init(errorState);
	}


	void QuiltCameraBoxMesh::setup()
	{
		// Create mesh instance
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		// Compute box and construct mesh
		mBox = math::Box(mSize.x, mSize.y, mSize.z, mPosition);
		constructBox(mBox, *mMeshInstance);
	}


	void QuiltCameraBoxMesh::constructBox(const math::Box& box, nap::MeshInstance& mesh)
	{
		// Generate the indices
		std::vector<uint32> indices(triCount * 3, 0);
		uint32* index_ptr = indices.data();
		for (int side = 0; side < 6; side++)
		{
			// Current lowest vertex index
			int vi = side * planeVertCount;
			
			// Compute triangle a
			*(index_ptr++) = vi + 3;
			*(index_ptr++) = vi + 1;
			*(index_ptr++) = vi + 0;

			// Compute triangle b
			*(index_ptr++) = vi + 2;
			*(index_ptr++) = vi + 3;
			*(index_ptr++) = vi + 0;
		}

		// Create attributes
		nap::Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		nap::Vec3VertexAttribute& normal_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::normal);
		nap::Vec3VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		nap::Vec4VertexAttribute& color_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

		// Set numer of vertices this mesh contains
		mesh.setNumVertices((int)boxVertCount);
		mesh.setDrawMode(EDrawMode::Triangles);
		mesh.setCullMode(mCullMode);
		mesh.setUsage(mUsage);
		mesh.setDrawMode(mPolygonMode);

		// Create vertex data based on scale factor
		std::vector<glm::vec3> vertex_data(boxVertCount);
		for (nap::uint i = 0; i < boxVertCount; i++)
			vertex_data[i] = (unitBox[i] * mSize) + mPosition;

		// Set data
		position_attribute.setData(vertex_data);
		normal_attribute.setData(boxNormals);
		uv_attribute.setData(boxUVs);
		color_attribute.setData({boxVertCount, mColor.toVec4()});

		// Create the shape
		MeshShape& shape = mesh.createShape();
		shape.setIndices(indices.data(), indices.size());
	}
}
