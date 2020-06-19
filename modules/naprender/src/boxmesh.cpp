#include "boxmesh.h"
#include "renderservice.h"
#include "nap/core.h"

// nap::boxmesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BoxMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Size",		&nap::BoxMesh::mSize,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Position",	&nap::BoxMesh::mPosition,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static constexpr int planeVertCount = 4;					//< Number of vertices per plane
	static constexpr int boxVertCount = planeVertCount * 6;		//< Total number of box vertices
	static constexpr int triCount = 6 * 2;						//< Total number of box triangles

	BoxMesh::BoxMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }

	bool BoxMesh::init(utility::ErrorState& errorState)
	{
		// Setup box
		setup();

		// Initialize mesh
		if (!mMeshInstance->init(errorState))
			return false;

		return true;
	}


	void BoxMesh::setup()
	{
		// Create mesh instance
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		// Compute box and construct mesh
		mBox = math::Box(mSize.x, mSize.y, mSize.z, mPosition);
		constructBox(mBox, *mMeshInstance);
	}


	void BoxMesh::constructBox(const math::Box& box, nap::MeshInstance& mesh)
	{
		std::vector<glm::vec3> vertices(boxVertCount,	{ 0.0f, 0.0f, 0.0f });			//< All box vertices
		std::vector<glm::vec3> normals(boxVertCount,	{ 0.0f, 0.0f, 0.0f });			//< Normals per plane
		std::vector<glm::vec3> uvs(boxVertCount,		{ 0.0f, 0.0f, 0.0f });			//< Every plane has the same uvs
		std::vector<glm::vec4> colors(boxVertCount,		{ 1.0f, 1.0f, 1.0f, 1.0f });	//< Default color is white

		const glm::vec3& min = box.getMin();
		const glm::vec3& max = box.getMax();

		// Fill side A (front)
		vertices[0]  = { min.x, min.y, max.z };	//< Front Lower left
		vertices[1]  = { max.x, min.y, max.z };	//< Front Lower right
		vertices[2]  = { min.x, max.y, max.z };	//< Front Top left
		vertices[3]  = { max.x, max.y, max.z };	//< Front Top right

		// Fill normals side A
		for (int v = 0; v < planeVertCount; v++)
			normals[v] = { 0.0f, 0.0f, 1.0f };

		// Fill side B (right)
		vertices[4]  = { max.x, min.y, max.z };	//< Right Lower Left
		vertices[5]  = { max.x, min.y, min.z };	//< Right Lower Right
		vertices[6]  = { max.x, max.y, max.z };	//< Right Top left
		vertices[7]  = { max.x, max.y, min.z };	//< Right Top right

		// Fill normals side B
		for (int v = 1 * planeVertCount; v < planeVertCount * 2; v++)
			normals[v] = { 1.0f, 0.0f, 0.0f };

		// Fill side C (back)
		vertices[8]  = { max.x, min.y, min.z };	//< Back Lower left
		vertices[9]  = { min.x, min.y, min.z };	//< Back lower right
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
		for (int i = 0; i < boxVertCount ; i += planeVertCount)
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
		nap::Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		nap::Vec3VertexAttribute& normal_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		nap::Vec3VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		nap::Vec4VertexAttribute& color_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));

		// Set numer of vertices this mesh contains
		mesh.setNumVertices(boxVertCount);
		mesh.setDrawMode(EDrawMode::Triangles);

		// Set data
		position_attribute.setData(vertices.data(), boxVertCount);
		normal_attribute.setData(normals.data(), boxVertCount);
		uv_attribute.setData(uvs.data(), boxVertCount);
		color_attribute.setData(colors.data(), boxVertCount);

		// Create the shape
		MeshShape& shape = mesh.createShape();
		shape.setIndices(indices.data(), indices.size());
	}

}