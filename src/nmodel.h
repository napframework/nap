#pragma once

// Local Includes
#include "nmesh.h"

// External Includes
#include <memory>
#include <vector>

namespace opengl
{
	/**
	 * Defines a polygonal model
	 *
	 * Every model consists out of a set of meshes
	 * Every mesh carries it's own set of vertex data
	 * This objects owns it's mesh data, on destruction all meshes are destroyed
	 */
	class Model
	{
	public:
		// Default constructor
		Model() = default;

		// Default destructor
		virtual ~Model() = default;

		// Copy is not allowed for now
		// TODO: Create copy method
		Model(const Model& other) = delete;
		Model& operator=(const Model& other) = delete;

		/**
		 * Returns a mesh
		 * @param index The index of the mesh to return
		 * @return the associated Mesh, nullptr mesh at index does not exist
		 */
		Mesh* getMesh(unsigned int index) const;

		/**
		 * @return the number of associated meshes
		 */
		unsigned int getMeshCount() const;

		/**
		 * @return total number of vertices
		 */
		unsigned int getVertCount() const;

		/**
		 * draws the model to currently active context
		 */
		void draw(GLenum mode = GL_TRIANGLES);

		/**
		* Clears all associated mesh data
		* Note that this call will invalidate any previously fetched mesh pointers!
		*/
		void clear();

		/**
		* Adds a mesh
		* When adding a mesh the model takes ownership of the mesh
		* If the mesh is not allocated it will be initialized
		* On destruction all meshes are cleared, including GPU resources
		* Note that this call won't perform a nullptr check
		* @param mesh the mesh to add and take ownership of
		*/
		void addMesh(Mesh* mesh);

		/**
		 * @return if the model has any mesh data
		 */
		bool isEmpty() const;

	private:
		using MeshContainer = std::vector<std::unique_ptr<Mesh>>;
		MeshContainer mMeshes;		// All model associated meshes
	};
} //  opengl
