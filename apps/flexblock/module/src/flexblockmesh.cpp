#include "flexblockmesh.h"
#include "meshutils.h"

RTTI_BEGIN_CLASS(nap::FlexBlockMesh)
RTTI_END_CLASS

namespace nap
{
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

		// 
		int rows = 2;
		int colums = 2;
		int vertCount = ( rows + 1 ) * ( colums + 1 ) ;

		std::vector<glm::vec3> vertices(vertCount, { 0.0f, 0.0f, 0.0f });			
		std::vector<glm::vec3> normals(vertCount, { 0.0f, 0.0f, 0.0f });			
		std::vector<glm::vec3> uvs(vertCount, { 0.0f, 0.0f, 0.0f });			
		std::vector<glm::vec4> colors(vertCount, { 1.0f, 1.0f, 1.0f, 1.0f });	

		glm::vec3 upperLeft = { -1.0f, -0.6f , 0.0f };
		glm::vec3 upperRight = { 1.5f, -1.0f , 0.0f };
		glm::vec3 bottomRight = { 1.0f, .5f , 0.0f };
		glm::vec3 bottomLeft = { -1.0f, 2.0f , 0.0f };

		float x_part = 1.0f / colums;
		float y_part = 1.0f / rows;

		int idx = 0;
		for (int i = 0; i < rows + 1; i++)
		{
			glm::vec3 c = { lerp(upperLeft.x, bottomLeft.x, y_part * i), lerp(upperLeft.y, bottomLeft.y, y_part * i), 0.0f };
			glm::vec3 d = { lerp(upperRight.x, bottomRight.x, x_part * i), lerp(upperRight.y, bottomRight.y, x_part * i), 0.0f };

			for (int j = 0; j < colums + 1; j++)
			{
				//
				glm::vec3 a = { lerp(upperLeft.x, upperRight.x, x_part * j), lerp(upperLeft.y, upperRight.y, x_part * j), 0.0f };
				glm::vec3 b = { lerp(bottomLeft.x, bottomRight.x, x_part * j), lerp(bottomLeft.y, bottomRight.y, x_part * j), 0.0f };

				vertices[idx] = lineLineIntersection(a, b, c, d);

				idx++;
			}
		}

		for (int v = 0; v < vertCount; v++)
			normals[v] = { 0.0f, 0.0f, 1.0f };

		// Create indices, every cell in the grid contains 2 triangles
		int triangle_count = rows * colums *2;
		std::vector<unsigned int> indices(triangle_count * 3, 0);
		unsigned int* index_ptr = indices.data();

		for (int row = 0; row < rows; row++)
		{
			for (int col = 0; col < colums; col++)
			{
				
				// Compute triangle a
				*(index_ptr++) = (row * (colums + 1)) + col;							//< Bottom Left
				*(index_ptr++) = (row * (colums + 1)) + (col + 1);					//< Bottom Right
				*(index_ptr++) = ((row + 1) * (colums + 1)) + (col + 1);				//< Top right

																						// Compute triangle b
				*(index_ptr++) = (row * (colums + 1)) + col;							//< Bottom Left
				*(index_ptr++) = ((row + 1) * (colums + 1)) + (col + 1);				//< Top right
				*(index_ptr++) = ((row + 1) * (colums + 1)) + (col + 0);				//< Top left
			}
		}

		// Create attributes
		nap::Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		nap::Vec3VertexAttribute& normal_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		nap::Vec3VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		nap::Vec4VertexAttribute& color_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));

		// Set numer of vertices this mesh contains
		mMeshInstance->setNumVertices(vertCount);

		// Set data
		position_attribute.setData(vertices.data(), vertCount);
		normal_attribute.setData(normals.data(), vertCount);
		uv_attribute.setData(uvs.data(), vertCount);
		color_attribute.setData(colors.data(), vertCount);

		// Create the shape
		MeshShape& shape = mMeshInstance->createShape();
		shape.setDrawMode(opengl::EDrawMode::TRIANGLES);
		shape.setIndices(indices.data(), indices.size());
	}

	float FlexBlockMesh::lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	glm::vec3 FlexBlockMesh::lineLineIntersection(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D)
	{
		// Line AB represented as a1x + b1y = c1 
		float a1 = B.y - A.y;
		float b1 = A.x - B.x;
		float c1 = a1*(A.x) + b1*(A.y);

		// Line CD represented as a2x + b2y = c2 
		float a2 = D.y - C.y;
		float b2 = C.x - D.x;
		float c2 = a2*(C.x) + b2*(C.y);

		float determinant = a1*b2 - a2*b1;

		if (determinant == 0)
		{
			// The lines are parallel. This is simplified 
			// by returning a pair of FLT_MAX 
			return{ FLT_MAX, FLT_MAX, 0.0f };
		}
		else
		{
			float x = (b2*c1 - b1*c2) / determinant;
			float y = (a1*c2 - a2*c1) / determinant;
			return{ x, y, 0.0f };
		}
	}
}
