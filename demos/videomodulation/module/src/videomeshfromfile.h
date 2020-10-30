/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/object.h>
#include <mesh.h>

namespace nap
{
	class RenderService;
	class Core;

	/**
	 * Loads a .mesh file from disk and adds / applies vertex displacement attributes
	 * The .mesh file is a converted .fbx file. 
	 * Behind the scenes assimp is used to convert .fbx files in to .mesh files.
	 * This happens after compiling your application as a post build step and occurs when the fbx is new or has changed.
	 * 
	 * The result after load is a MeshInstance that holds all the vertex attributes exposed by the fbx. 
	 * We add two new ones: UVCenter and DisplacementDirection. Both are vec3 vertex attributes
	 * The UVCenter contains the uv sample location for displacement and is the average of all three triangle vertices
	 * The DisplacementDirection is the primitive (triangle) normal. The default normals contain light information
	 * and we don't want to change that, so we store that information in a new vertex buffer.
	 * 
	 * Both attributes are used by the shader to calculate the displacement direction and value based on a video texture
	 */
	class VideoMeshFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		VideoMeshFromFile(Core& core);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return MeshInstance as created during init().
		*/
		virtual MeshInstance& getMeshInstance() override						{ return *mMeshInstance; }

		/**
		* @return MeshInstance as created during init().
		*/
		virtual const MeshInstance& getMeshInstance() const override			{ return *mMeshInstance; }


		std::string mPath;														///< Property: "Path" to mesh on disk

	private:
		RenderService*						mRenderService;
		std::unique_ptr<MeshInstance>		mMeshInstance;						///< Holds the loaded mesh
		nap::VertexAttribute<glm::vec3>*	mDirectionAttribute = nullptr;		///< Displacement direction attribute
		nap::VertexAttribute<glm::vec3>*	mNormalAttribute = nullptr;			///< Normal attribute used for light calculations
		nap::VertexAttribute<glm::vec3>*	mUVAttribute = nullptr;				///< UV attribute
		nap::VertexAttribute<glm::vec3>*	mPositionAttribute = nullptr;		///< Vertex position attribute
		nap::VertexAttribute<glm::vec3>*	mUVCenterAttribute = nullptr;		///< UV center Attribute
	};
}
