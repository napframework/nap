#pragma once

#include <utility/dllexport.h>
#include <mesh.h>
#include <box.h>

namespace nap
{
	// Holds a copy of vertex data associated with a triangle in a mesh
	template<typename T>
	using TriangleData = std::array<T, 3>;

	// Holds 3 pointers to the vertices associated with a triangle in the mesh
	template<typename T>
	using TriangleDataPointer = std::array <T*, 3>;

	// Binds all the points to a set of triangular faces
	using MeshConnectivityMap = std::vector<std::vector<int>>;

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

	/**
	 * Returns the attribute data associated with a triangle in a mesh. This call performs a copy of the vertex data to @outTriangle
	 * is given as a separate vector because of performance considerations.
	 * This call asserts when the triangle number is out of bounds.
	 * @param mesh the mesh that holds @vertexData. It's important that the @vertexData source is the @mesh
	 * @param number the triangle number to query
	 * @param vertexData the mesh attribute data to sample from
	 * @param outTriangle the array (containing 3 elements) that is filled with the vertex data
	 */
	template<typename T>
	void getTriangleValues(const nap::MeshInstance& mesh, int number, const std::vector<T>& vertexData, TriangleData<T>& outTriangle);

	/**
	* Returns the attribute data associated with a triangle as a set of pointers in to the vertex data of a mesh. 
	* Use this function to change attributes associated with a triangle
	* This call asserts when the triangle number is out of bounds.
	* @param mesh the mesh that holds @vertexData. It's important that the @vertexData source is the @mesh
	* @param number the triangle number to query
	* @param vertexData the mesh attribute data to sample from
	* @param outTriangle the array (containing 3 elements) that is filled with the vertex data
	*/
	template<typename T>
	void getTriangleValues(const nap::MeshInstance& mesh, int number, std::vector<T>& vertexData, TriangleDataPointer<T>& outTriangle);

	/**
	* Returns the attribute data associated with a triangle in a mesh. This call performs a copy of the vertex data to @outTriangle
	* is given as a separate vector because of performance considerations.
	* This call asserts when the triangle number is out of bounds.
	* @param mesh the mesh that holds @vertexData. It's important that the @vertexData source is the @mesh
	* @param number the triangle number to query
	* @param vertexData the mesh attribute data to sample from
	* @param outTriangle the array (containing 3 elements) that is filled with the vertex data
	*/
	template<typename T>
	void getTriangleValues(const nap::MeshInstance& mesh, int number, const VertexAttribute<T>& vertexAttribute, TriangleData<T>& outTriangle);

	/**
	* Returns the attribute data associated with a triangle as a set of pointers in to the vertex data of a mesh.
	* Use this function to change attributes associated with a triangle
	* This call asserts when the triangle number is out of bounds.
	* @param mesh the mesh that holds @vertexData. It's important that the @vertexData source is the @mesh
	* @param number the triangle number to query
	* @param vertexAttribute the mesh attribute to sample from
	* @param outTriangle the array (containing 3 elements) that is filled with the vertex data
	*/
	template<typename T>
	void getTriangleValues(const nap::MeshInstance& mesh, int number, VertexAttribute<T>& vertexAttribute, TriangleDataPointer<T>& outTriangle);

	/**
	 * Sets the vertex attribute values for a specific triangle in a mesh
	 * @param mesh the mesh that holds @vertexData. It's important that @vertexData is from @mesh
	 * @param number the triangle to apply the new values
	 * @param vertexData the mesh attribute data that receives @triangleData
	 * @param triangleValues the new vertex values for the triangle.
	 */
	template<typename T>
	void setTriangleValues(const nap::MeshInstance& mesh, int number, std::vector<T>& vertexData, const TriangleData<T>& triangleValues);

	/**
	* Sets the vertex attribute values for a specific triangle in a mesh
	* @param mesh the mesh that holds @vertexData. It's important that @vertexData is from @mesh
	* @param number the triangle to apply the new values
	* @param vertexData the mesh attribute that receives @triangleData
	* @param triangleValues the new vertex values for the triangle.
	*/
	template<typename T>
	void setTriangleValues(const nap::MeshInstance& mesh, int number, VertexAttribute<T>& vertexData, const TriangleData<T>& triangleValues);

	/**
	 * Computes the bounding box of a mesh using it's associated position data
	 * Note that indices are not considered. This call loops over all available
	 * points regardless of whether if they're drawn or not
	 * @param mesh the mesh to get the bounding box for
	 * @param outBox the computed bounding box
	 */
	void NAPAPI computeBoundingBox(const nap::MeshInstance& mesh, nap::math::Box& outBox);

	/**
	 * Computes the bounding box of a mesh using it's associated position data
	 * Note that indices are not considered. This call loops over all available
	 * points regardless of whether they're drawn or not
	 * @param mesh the mesh to get the bounding box for
	 * @return the computed bounding box
	 */
	math::Box NAPAPI computeBoundingBox(const nap::MeshInstance& mesh);

	/**
	 * Builds a 'map' that binds points (mesh index values) to faces
	 * The index in the array corresponds to a mesh vertex index (point). 
	 * This call only works for meshes that have indices. When the mesh does not have indices this call asserts
	 * Try to avoid building the map regularly, it's a heavy operation
	 * This call asserts when the mesh is not a triangular mesh or has no indices associated with it
	 * @param mesh the mesh to get build the array from
	 * @param outConnectivityMap the array that is populated with the triangles associated with a single index
	 */
	void NAPAPI computeConnectivity(const nap::MeshInstance& mesh, MeshConnectivityMap& outConnectivityMap);

	/**
	* Computes the normal that is associated with a triangular face. The normal is weighted (not normalized)
	* This call asserts when the index is out of bounds or the mesh is not a triangular mesh
	* @param number the number of the face
	* @param vertices the vertex position attribute
	* @param mesh the triangular mesh
	* @return the weighted (not normalized) normal associated with a face
	*/
	glm::vec3 NAPAPI computeTriangleNormal(const nap::MeshInstance& mesh, int number, const nap::VertexAttribute<glm::vec3>& vertices);

	/**
	 * Computes the point normal based on triangle connectivity, the result is the normalized sum of all the triangle normals
	 * Points are used to describe the individual values of the indices associated with a mesh.
	 * Requires an already build connectivity map to speed up computation, call computeConnectivity to build one prior to this call
	 * @param mesh the triangular mesh
	 * @param index the point number to compute the normal for
	 * @param vertices the vertices associated with the mesh
	 * @param connectivityMap the map that binds points to faces
	 */
	glm::vec3 NAPAPI computePointNormal(const nap::MeshInstance& mesh, int index, const nap::VertexAttribute<glm::vec3>& vertices, const MeshConnectivityMap& connectivityMap);

	/**
	 * Automatically re-computes all the normals of a mesh
	 * When the mesh has indices the normal is computed based on connectivity
	 * Meshes without indices receive the triangular face normal
	 * @param mesh the triangular mesh
	 * @param vertices the vertex position attribute
	 * @param outNormals the recomputed normals, the normals have to be initialized and of the same length as the vertices
	 */
	void NAPAPI computeNormals(const nap::MeshInstance& mesh, const nap::VertexAttribute<glm::vec3>& vertices, nap::VertexAttribute<glm::vec3>& outNormals);

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void getTriangleValues(const MeshInstance& mesh, int number, const std::vector<T>& vertexData, TriangleData<T>& outVertexData)
	{
		// Assert when the triangle is out of bounds or when it's not a triangle mesh
		assert(number < getTriangleCount(mesh));

		// Take in to consideration if the mesh is drawn using indices, if so query data based on that
		if (mesh.hasIndices())
		{
			glm::ivec3 triangle_indices = { 0.0,0.0,0.0 };
			getTriangleIndices(mesh, number, triangle_indices);
			outVertexData[0] = vertexData[triangle_indices.x];
			outVertexData[1] = vertexData[triangle_indices.y];
			outVertexData[2] = vertexData[triangle_indices.z];
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


	template<typename T>
	void getTriangleValues(const MeshInstance& mesh, int number, const VertexAttribute<T>& vertexAttribute, TriangleData<T>& outTriangle)
	{
		return getTriangleValues<T>(mesh, number, vertexAttribute.getData(), outTriangle);
	}
	

	template<typename T>
	void getTriangleValues(const MeshInstance& mesh, int number, std::vector<T>& vertexData, TriangleDataPointer<T>& outTriangle)
	{
		// Assert when the triangle is out of bounds or when it's not a triangle mesh
		assert(number < getTriangleCount(mesh));

		// Take in to consideration if the mesh is drawn using indices, if so query data based on that
		if (mesh.hasIndices())
		{
			glm::ivec3 triangle_indices = { 0.0,0.0,0.0 };
			getTriangleIndices(mesh, number, triangle_indices);
			T* id = vertexData.data();
			outTriangle[0] = id + triangle_indices.x;
			outTriangle[1] = id + triangle_indices.y;
			outTriangle[2] = id + triangle_indices.z;
			return;
		}

		// Get triangle values for non indexed meshes
		switch (mesh.getDrawMode())
		{
		case opengl::EDrawMode::TRIANGLE_FAN:
		{
			T* id = vertexData.data();
			outTriangle[0] = id;
			outTriangle[1] = id + number + 1;
			outTriangle[2] = id + number + 2;
			break;
		}
		case opengl::EDrawMode::TRIANGLE_STRIP:
		{
			T* id = vertexData.data() + number;
			outTriangle[0] = id + 0;
			outTriangle[1] = id + 1;
			outTriangle[2] = id + 2;
			break;
		}
		case opengl::EDrawMode::TRIANGLES:
		{
			T* id = vertexData.data() + (number * 3);
			outTriangle[0] = id + 0;
			outTriangle[1] = id + 1;
			outTriangle[2] = id + 2;
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	template<typename T>
	void getTriangleValues(const MeshInstance& mesh, int number, VertexAttribute<T>& vertexAttribute, TriangleDataPointer<T>& outTriangle)
	{
		return getTriangleValues<T>(mesh, number, vertexAttribute.getData(), outTriangle);
	}


	template<typename T>
	void setTriangleValues(const MeshInstance& mesh, int number, std::vector<T>& vertexData, const TriangleData<T>& triangleData)
	{
		TriangleDataPointer<T> pointer;
		getTriangleValues(mesh, number, vertexData, pointer);
		*(pointer[0]) = triangleData[0];
		*(pointer[1]) = triangleData[1];
		*(pointer[2]) = triangleData[2];
	}


	template<typename T>
	void setTriangleValues(const MeshInstance& mesh, int number, VertexAttribute<T>& vertexData, const TriangleData<T>& triangleData)
	{
		setTriangleValues<T>(mesh, number, vertexData.getData(), triangleData);
	}

}
