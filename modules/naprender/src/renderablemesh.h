/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "mesh.h"
#include "materialinstance.h"
#include "nap/signalslot.h"

namespace nap
{
	// Forward Declares
	class RenderService;

	/**
	 * Represents the coupling between a mesh and a material that can be rendered to screen.
	 * Call RenderService.createRenderableMesh() from your own renderable component to create a renderable mesh on initialization.
	 * Renderable meshes are hashable, equality is based on the contained mesh and material.
	 */
	class NAPAPI RenderableMesh final
	{
		friend class RenderService;
	public:
		/**
		 * Default constructor, by default this represents an invalid mesh / material combination.
		 * Can be used to declare a member of this type that is invalid on initialization.
		 */
		RenderableMesh() = default;

		// Copy constructor
		RenderableMesh(const RenderableMesh& rhs);

		// Copy assignment operator
		RenderableMesh& operator=(const RenderableMesh& rhs);

		// Object is similar when sharing mesh / material combination
		bool operator==(const RenderableMesh& rhs) const;

		/**
		* @return whether the material and mesh form a valid combination. The combination is valid when the vertex attributes
		* of a mesh match the vertex attributes of a shader.
		*/
		bool isValid() const													{ return mMesh != nullptr; }

		/**
		 * @return The mesh object used to create this object.
		 */
		IMesh& getMesh()														{ return *mMesh; }

		/**
		 * @return The mesh object used to create this object.
		 */
		const IMesh& getMesh() const											{ return *mMesh; }

		/**
		 * @return The material instance object used to create this object.
		 */
		MaterialInstance& getMaterialInstance()									{ return *mMaterialInstance; }

		/**
		 * @return The material instance object used to create this object.
		 */
		const MaterialInstance& getMaterialInstance() const						{ return *mMaterialInstance; }

		const std::vector<VkBuffer>& getVertexBuffers();
		const std::vector<VkDeviceSize>& getVertexBufferOffsets() const			{ return mVertexBufferOffsets; }

		int RenderableMesh::getVertexBufferBindingIndex(std::string meshVertexAttributeID) const;

	protected:
		/**
		 * Constructor
		 * @param mesh the mesh that is rendered
		 * @param materialInstance the material the mesh is rendered with
		 */
		RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance);

	private:
		void onVertexBufferDataChanged();

	private:
		MaterialInstance*			mMaterialInstance = nullptr;	///< Material instance
		IMesh*						mMesh = nullptr;				///< Mesh
		std::vector<VkBuffer>		mVertexBuffers;
		std::vector<VkDeviceSize>	mVertexBufferOffsets;
		bool						mVertexBuffersDirty = true;
		nap::Slot<>					mVertexBufferDataChangedSlot = { [&]() { onVertexBufferDataChanged(); } };
	};
}


//////////////////////////////////////////////////////////////////////////
// hash
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template<>
	struct hash<nap::RenderableMesh>
	{
		size_t operator()(const nap::RenderableMesh& key) const
		{
			assert(key.isValid());
			size_t mesh_hash = hash<size_t>{}((size_t)&key.getMesh());
			size_t mate_hash = hash<size_t>{}((size_t)&key.getMaterialInstance());
			return mesh_hash ^ mate_hash;
		}
	};
}
