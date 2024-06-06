/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "heightmesh.h"

// External Includes
#include <meshutils.h>
#include <nap/logger.h>
#include <nap/core.h>
#include <renderglobals.h>

// nap::heightmesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HeightMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Heightmap",	&nap::HeightMesh::mHeightmap,	nap::rtti::EPropertyMetaData::Required, "Height map image")
	RTTI_PROPERTY("Height",		&nap::HeightMesh::mHeight,		nap::rtti::EPropertyMetaData::Required, "Max mesh elevation level with a pixel value of 1")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	HeightMesh::HeightMesh(Core& core) : PlaneMesh(core)
	{ }


	/**
	 * Initialize this height map, the end result is a plane where every vertex is displaced along it's normal
	 * After displacement the normals are re-calculated
	 */
	bool HeightMesh::init(utility::ErrorState& errorState)
	{
		// Setup our plane with the right amount of rows / columns + default vertex attributes.
		// Initialization is postponed to the end of this call, when the data is uploaded to the GPU.
		// If we would have called init() here, the data would be uploaded twice, here and at the end, something we want to avoid.
		if (!PlaneMesh::setup(errorState))
			return false;

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
