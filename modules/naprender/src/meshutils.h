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

	// Holds a copy of vertex data associated with a triangle in a mesh
	template<typename T>
	using TriangleData = std::array<T, 3>;

	// Holds 3 pointers to the vertices associated with a triangle in the mesh
	template<typename T>
	using TriangleDataPointer = std::array <T*, 3>;

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
