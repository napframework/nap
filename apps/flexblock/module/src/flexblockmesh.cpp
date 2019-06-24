// Local Includes
#include "flexblockmesh.h"

// External Includes
#include <meshutils.h>


RTTI_BEGIN_CLASS(nap::FlexBlockMesh)
RTTI_PROPERTY("Size", &nap::FlexBlockMesh::mSize, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static constexpr int planeVertCount = 4;					//< Number of vertices per plane
	static constexpr int boxVertCount = planeVertCount * 6;		//< Total number of box vertices
	static constexpr int triCount = 6 * 2;						//< Total number of box triangles

	FlexBlockMesh::~FlexBlockMesh() { }


	bool FlexBlockMesh::init(utility::ErrorState& errorState)
	{
		// Setup box
		setup();

		// Initialize mesh
		if (!mMeshInstance->init(errorState))
			return false;

		return true;
	}


	void FlexBlockMesh::setup()
	{
		// Create mesh instance
		mMeshInstance = std::make_unique<MeshInstance>();

		// Compute box and construct mesh
		mBox = math::Box(mSize.x, mSize.y, mSize.z, glm::vec3(0.0f, 0.0f, 0.0f));
		constructBox(mBox, *mMeshInstance);
	}


	void FlexBlockMesh::setControlPoints(const std::vector<glm::vec3>& controlPoints)
	{
		auto verts = mNormalsAttr->getData();

		verts[0] = controlPoints[0]; //< Front Lower left
		verts[13] = controlPoints[0]; //< Left Lower right
		verts[16] = controlPoints[0];	//< Bottom Lower left

											// m2
		verts[1] = controlPoints[1];	//< Front Lower right
		verts[4] = controlPoints[1];	//< Right Lower Left
		verts[17] = controlPoints[1]; //< Bottom Lower right

										  // m3
		verts[2] = controlPoints[2];	//< Front Top left
		verts[15] = controlPoints[2]; //< Left Top right
		verts[20] = controlPoints[2]; //< Top Lower left

										  // m4
		verts[3] = controlPoints[3];	//< Front Top right
		verts[6] = controlPoints[3];	//< Right Top left
		verts[21] = controlPoints[3];	//< Top Lower right

											// m5
		verts[8] = controlPoints[4];	//< Back Lower left
		verts[19] = controlPoints[4];	//< Bottom Top right
		verts[5] = controlPoints[4]; //< Right Lower Right

										 // m6
		verts[9] = controlPoints[5];	//< Back lower right
		verts[12] = controlPoints[5]; //< Left Lower left
		verts[18] = controlPoints[5]; //< Bottom Top left

										  // m7
		verts[10] = controlPoints[6]; //< Back Top left
		verts[23] = controlPoints[6]; //< Top Top right
		verts[7] = controlPoints[6]; //< Right Top right

										 // m8
		verts[11] = controlPoints[7]; //< Back Top right
		verts[14] = controlPoints[7]; //< Left Top left
		verts[22] = controlPoints[7]; //< Top Top left

		mPositionAttr->setData(verts);

		utility::computeNormals(*mMeshInstance, *mPositionAttr, *mNormalsAttr);

		utility::ErrorState error;
		mMeshInstance->update(error);
	}


	void FlexBlockMesh::constructBox(const math::Box& box, nap::MeshInstance& mesh)
	{
		std::vector<glm::vec3> vertices(boxVertCount, { 0.0f, 0.0f, 0.0f });			//< All box vertices
		std::vector<glm::vec3> normals(boxVertCount, { 0.0f, 0.0f, 0.0f });			//< Normals per plane
		std::vector<glm::vec3> uvs(boxVertCount, { 0.0f, 0.0f, 0.0f });			//< Every plane has the same uvs
		std::vector<glm::vec4> colors(boxVertCount, { 1.0f, 1.0f, 1.0f, 1.0f });	//< Default color is white

		const glm::vec3& min = box.getMin();
		const glm::vec3& max = box.getMax();

		// Fill side A (front)
		vertices[0] = { min.x, min.y, max.z };	//< Front Lower left
		vertices[1] = { max.x, min.y, max.z };	//< Front Lower right
		vertices[2] = { min.x, max.y, max.z };	//< Front Top left
		vertices[3] = { max.x, max.y, max.z };	//< Front Top right

		// Fill normals side A
		for (int v = 0; v < planeVertCount; v++)
			normals[v] = { 0.0f, 0.0f, 1.0f };

		// Fill side B (right)
		vertices[4] = { max.x, min.y, max.z };	//< Right Lower Left
		vertices[5] = { max.x, min.y, min.z };	//< Right Lower Right
		vertices[6] = { max.x, max.y, max.z };	//< Right Top left
		vertices[7] = { max.x, max.y, min.z };	//< Right Top right

												// Fill normals side B
		for (int v = 1 * planeVertCount; v < planeVertCount * 2; v++)
			normals[v] = { 1.0f, 0.0f, 0.0f };

		// Fill side C (back)
		vertices[8] = { max.x, min.y, min.z };	//< Back Lower left
		vertices[9] = { min.x, min.y, min.z };	//< Back lower right
		vertices[10] = { max.x, max.y, min.z }; //< Back Top left
		vertices[11] = { min.x, max.y, min.z };	//< Back Top right

		// Fill normals side C
		for (int v = 2 * planeVertCount; v < planeVertCount * 3; v++)
			normals[v] = { 0.0f, 0.0f, -1.0f };

		// Fill side D (left)
		vertices[12] = { min.x, min.y, min.z }; //< Left Lower left
		vertices[13] = { min.x, min.y, max.z };	//< Left Lower right
		vertices[14] = { min.x, max.y, min.z }; //< Left Top left
		vertices[15] = { min.x, max.y, max.z }; //< Left Top right

												// Fill normals side D
		for (int v = 3 * planeVertCount; v < planeVertCount * 4; v++)
			normals[v] = { -1.0f, 0.0f, 0.0f };

		// Fill side E (bottom)
		vertices[16] = { min.x, min.y, max.z };	//< Bottom Lower left
		vertices[17] = { max.x, min.y, max.z };	//< Bottom Lower right
		vertices[18] = { min.x, min.y, min.z };	//< Bottom Top left
		vertices[19] = { max.x, min.y, min.z };	//< Bottom Top right

												// Fill normals side E
		for (int v = 4 * planeVertCount; v < planeVertCount * 5; v++)
			normals[v] = { 0.0f, -1.0f, 0.0f };

		// Fill side F (top)
		vertices[20] = { min.x, max.y, max.z }; //< Top Lower left
		vertices[21] = { max.x, max.y, max.z };	//< Top Lower right
		vertices[22] = { min.x, max.y, min.z };	//< Top Top left
		vertices[23] = { max.x, max.y, min.z };	//< Top Top right

												// Fill normals side F
		for (int v = 5 * planeVertCount; v < planeVertCount * 6; v++)
			normals[v] = { 0.0f, 1.0f, 0.0f };

		// Calculate UVs
		for (int i = 0; i < boxVertCount; i += planeVertCount)
		{
			uvs[i + 0] = { 0.0f, 0.0f, 0.0f };
			uvs[i + 1] = { 1.0f, 0.0f, 0.0f };
			uvs[i + 2] = { 0.0f, 1.0f, 0.0f };
			uvs[i + 3] = { 1.0f, 1.0f, 0.0f };
		}

		// Generate the indices

		std::vector<unsigned int> indices(triCount * 3, 0);
		unsigned int* index_ptr = indices.data();

		for (int side = 0; side < 6; side++)
		{
			// Current lowest vertex index
			int vi = side * planeVertCount;

			// Compute triangle a
			*(index_ptr++) = vi + 0;
			*(index_ptr++) = vi + 1;
			*(index_ptr++) = vi + 3;

			// Compute triangle b
			*(index_ptr++) = vi + 0;
			*(index_ptr++) = vi + 3;
			*(index_ptr++) = vi + 2;
		}

		// Create attributes
		mPositionAttr = &mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		mNormalsAttr = &mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		nap::Vec3VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		nap::Vec4VertexAttribute& color_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));

		// Set number of vertices this mesh contains
		mesh.setNumVertices(boxVertCount);

		// Set data
		mPositionAttr->setData(vertices.data(), boxVertCount);
		mNormalsAttr->setData(normals.data(), boxVertCount);
		uv_attribute.setData(uvs.data(), boxVertCount);
		color_attribute.setData(colors.data(), boxVertCount);

		// Create the shape
		MeshShape& shape = mesh.createShape();
		shape.setDrawMode(opengl::EDrawMode::TRIANGLES);
		shape.setIndices(indices.data(), indices.size());
	}

}