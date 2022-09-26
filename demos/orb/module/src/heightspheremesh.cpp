/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "heightspheremesh.h"

// External Includes
#include <meshutils.h>
#include <nap/logger.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>

// nap::heightmesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HeightSphereMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Heightmap",		&nap::HeightSphereMesh::mHeightmap,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",			&nap::HeightSphereMesh::mHeight,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Usage",			&nap::HeightSphereMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::HeightSphereMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::HeightSphereMesh::mPolygonMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Radius",			&nap::HeightSphereMesh::mRadius,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rings",			&nap::HeightSphereMesh::mRings,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sectors",		&nap::HeightSphereMesh::mSectors,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",			&nap::HeightSphereMesh::mColor,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	HeightSphereMesh::HeightSphereMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	/**
	 * Initialize this height map, the end result is a plane where every vertex is displaced along it's normal
	 * After displacement the normals are re-calculated
	 */
	bool HeightSphereMesh::init(utility::ErrorState& errorState)
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
				float const y = sin(-(math::PI_2)+math::PI * r * dr);
				float const x = cos(math::PIX2 * s * ds) * sin(math::PI * r * dr) * -1.0f;
				float const z = sin(math::PIX2 * s * ds) * sin(math::PI * r * dr);

				// Set texture coordinates
				*t++ = { s * ds, r * dr, 0.5f };

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

		nap::Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		nap::Vec3VertexAttribute& normal_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::normal);
		nap::Vec3VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		nap::Vec4VertexAttribute& color_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

		position_attribute.setData(vertices.data(), vertex_count);
		normal_attribute.setData(normals.data(), vertex_count);
		uv_attribute.setData(texcoords.data(), vertex_count);
		color_attribute.setData({ vertex_count, mColor.toVec4() });

		MeshShape& shape = mMeshInstance->createShape();
		shape.setIndices(indices.data(), index_count);

		// Get the just created mesh instance
		nap::MeshInstance& mesh_instance = getMeshInstance();

		// Get attributes
		Vec3VertexAttribute& pos_attr = mesh_instance.getAttribute<glm::vec3>(vertexid::position);
		Vec3VertexAttribute& uvs_attr = mesh_instance.getAttribute<glm::vec3>(vertexid::getUVName(0));
		Vec3VertexAttribute& nor_attr = mesh_instance.getAttribute<glm::vec3>(vertexid::normal);

		// Store the original point positions in a new attribute
		mOriginalPosAttr = &mesh_instance.getOrCreateAttribute<glm::vec3>("OriginalPosition");
		mOriginalNorAttr = &mesh_instance.getOrCreateAttribute<glm::vec3>("OriginalNormal");

		mOriginalPosAttr->setData(pos_attr.getData());
		mOriginalNorAttr->setData(nor_attr.getData());

		std::vector<glm::vec3>& uvs_data = uvs_attr.getData();
		std::vector<glm::vec3>& pos_data = pos_attr.getData();
		std::vector<glm::vec3>& nor_data = nor_attr.getData();

		// Get the bitmap we want to get the color value from @uv
		const nap::Bitmap& bitmap = mHeightmap->getBitmap();
		float width = static_cast<float>(bitmap.getWidth() - 1);
		float heigh = static_cast<float>(bitmap.getHeight() - 1);

		int vert_count = mesh_instance.getNumVertices();

		// Create the pixel that we need for sampling the bitmap data
		std::unique_ptr<BaseColor> pixel = bitmap.makePixel();

		// This will hold the pixel data as float values
		RColorFloat target_pixel;

		// We also pre-fetch our converter for fast lookups later
		BaseColor::Converter converter = pixel->getConverter(target_pixel);

		for (int i = 0; i < vert_count; i++)
		{
			// Get current vertex uv data
			glm::vec3& uvs = uvs_data[i];

			// Get current vector normal 
			glm::vec3& nor = nor_data[i];

			// Calculate pixel coordinate
			int pixel_x = static_cast<int>(uvs.x * width);
			int pixel_y = static_cast<int>(uvs.y * heigh);

			// Get our color at the x, y coordinates
			bitmap.getPixel(pixel_x, pixel_y, *pixel);

			// Convert
			converter(*pixel, target_pixel, 0);

			// Set the new vertex position
			pos_data[i] = pos_data[i] + (nor * target_pixel.getRed() * mHeight);
		}

		// Update our mesh normals to ensure light calculations work in the shader
		utility::computeNormals(mesh_instance, pos_attr, nor_attr);

		// Store position attribute
		mDisplacedPosAttr = &pos_attr;

		// Initialize the mesh -> attributes are verified and data is uploaded to the GPU
		return mesh_instance.init(errorState);
	}
}
