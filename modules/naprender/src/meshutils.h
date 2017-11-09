#pragma once

#include <utility/dllexport.h>
#include <mesh.h>

namespace nap
{
	/**
	 * Check if the mesh has any indices associated with it. Indices are used
	 * as a lookup for generating triangles on the GPU and save memory because
	 * the user doesn't have to specify duplicate vertices
	 * @param mesh the mesh to check for indices
	 * @return if the mesh has any indices associated with it
	 */
	bool NAPAPI hasIndices(const nap::MeshInstance& mesh);

	/**
	 * @param mesh the mesh to check
	 * @return if the mesh contains triangles, is of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
	 */
	bool NAPAPI isTriangleMesh(const nap::MeshInstance& mesh);

	/**
	* This call only works for triangle based meshes: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
	* @return the number of triangles associated with a mesh, -1 if the mesh isn't a triangle based mesh
	*/
	int NAPAPI getTriangleCount(const nap::MeshInstance& mesh);

	/**
	 * Returns the vertex indices associated with a triangle.
	 * The indices can be used to perform a vertex attribute lookup on the mesh
	 * Note that this function only works for meshes that are of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
	 * This call asserts when the triangle number is out of bounds, the mesh has no indices or the draw mode is not of type triangle
	 * Note that although it's unlikely but in the event of indices associated with a strip or fan this call will work.
	 * The most likely use case is with a mesh that is of a type: TRIANGLES
	 * @param mesh the mesh to get the indices from
	 * @param number the triangle number to get the for
	 * @param indices the 3 indices associated with triangle @number, when invalid @indices will be -1,-1,-1
	 * @return if the triangle indices are valid
	 */
	void NAPAPI getTriangleIndices(const nap::MeshInstance& mesh, int number, glm::ivec3& indices);

	template<typename T>
	using TriangleData = std::array<T, 3>;

	/**
	 * Returns the attribute data associated with a triangle in a mesh. The vertex data associated with the mesh 
	 * is given as a separate vector because of performance considerations. Vertex attribute lookups are rather expensive
	 * This call asserts when the triangle number is out of bounds.
	 * @param mesh the mesh that holds @vertexData. It's important that the @vertexData source is the @mesh
	 * @param number the triangle number to query
	 * @param vertexData the mesh attribute data to sample from
	 * @param outTriangle the array (containing 3 elements) that is filled with the vertex data
	 */
	template<typename T>
	void getTriangleValue(const nap::MeshInstance& mesh, int number, const std::vector<T>& vertexData, TriangleData<T>& outTriangle);


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void getTriangleValue(const MeshInstance& mesh, int number, const std::vector<T>& vertexData, TriangleData<T>& outVertexData)
	{
		// Assert when the triangle is out of bounds or when it's not a triangle mesh
		assert(number < getTriangleCount(mesh));

		// Take in to consideration if the mesh is drawn using indices, if so query data based on that
		if (mesh.hasIndices())
		{
			glm::ivec3 triangle_indices = { 0.0,0.0,0.0 };
			getTriangleIndices(mesh, number, triangle_indices);
			outVertexData[0] = vertexData[triangle_indices.r];
			outVertexData[1] = vertexData[triangle_indices.g];
			outVertexData[2] = vertexData[triangle_indices.b];
			return;
		}

		// Get triangle values for non indexed meshes
		switch (mesh.getDrawMode())
		{
		case opengl::EDrawMode::TRIANGLE_FAN:
		{
			const T* id = vertexData.data();
			outVertexData[0] = *id;
			outVertexData[1] = *(id + number + 1);
			outVertexData[2] = *(id + number + 2);
			break;
		}
		case opengl::EDrawMode::TRIANGLE_STRIP:
		{
			const T* id = vertexData.data() + number;
			outVertexData[0] = *(id + 0);
			outVertexData[1] = *(id + 1);
			outVertexData[2] = *(id + 2);
			break;
		}
		case opengl::EDrawMode::TRIANGLES:
		{
			const T* id = vertexData.data() + (number * 3);
			outVertexData[0] = *(id + 0);
			outVertexData[1] = *(id + 1);
			outVertexData[2] = *(id + 2);
			break;
		}
		default:
			assert(false);
			break;
		}
	}

}
