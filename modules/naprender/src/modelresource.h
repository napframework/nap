#pragma once

#include <nap/resource.h>
#include "nvertexarrayobject.h"
#include "ndrawutils.h"

namespace nap
{
	class Material;
	class MeshResource;

	class ModelResource : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		ModelResource() = default;

		/**
 		 * Loads model from file.
 		 */
		virtual bool init(ErrorState& errorState) override;

		/**
 		 * Performs commit or rollback of changes made in init.
 		 */
		virtual void finish(Resource::EFinishMode mode) override;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string getDisplayName() const override;
		
		/**
		* @return The Vertex Array Object that contains the description of how the
		mesh is bound to the shader.
		*/
		opengl::VertexArrayObject& getVAO() 
		{ 
			return mVAO;  
		}

		/**
		* @return The Mesh Resource
		*/
		MeshResource* getMeshResource() 
		{ 
			return mMeshResource; 
		}

		/**
		* @return The Material that is applied to the mesh.
		*/
		Material* getMaterial() 
		{ 
			return mMaterialResource;  
		}

	public:
		Material*		mMaterialResource = nullptr;
		MeshResource*	mMeshResource = nullptr;

	private:
		opengl::VertexArrayObject	mVAO;			///< Vertex Array Object, describing how to the mesh is bound to the applied shader/material

	};


} // nap

