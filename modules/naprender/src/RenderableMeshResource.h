#pragma once

#include <nap/resource.h>
#include "rtti/factory.h"
#include "nvertexarrayobject.h"
#include "ndrawutils.h"

namespace nap
{
	class Material;
	class MeshResource;
	class RenderService;


	/**
	* This object binds a mesh to a material, making it renderable. It contains a vertex array object that 
	* represents how the mesh vertex attributes are bound to the shader vertex attributes.
	*/
	class RenderableMeshResource : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		RenderableMeshResource()
		{
			assert(false);
		}

		RenderableMeshResource(RenderService& renderService) : 
			mRenderService(&renderService)
		{
		}

		~RenderableMeshResource();

		/**
 		 * Loads model from file.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

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
			return *mVAO.get();  
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
		std::unique_ptr<opengl::VertexArrayObject>	mVAO;			///< Vertex Array Object, describing how to the mesh is bound to the applied shader/material
		std::unique_ptr<opengl::VertexArrayObject>	mPrevVAO;		///< Prev VAO, used for commit/rollback
		RenderService* mRenderService = nullptr;					///< RenderService, used for deferring destruction of VAO
	};


	/**
	* Factory for creating RenderableMeshResources. The factory is responsible for passing the RenderService
	* to the RenderableMeshResource on construction.
	*/
	class RenderableMeshResourceCreator : public rtti::IObjectCreator
	{
	public:
		RenderableMeshResourceCreator(RenderService& renderService) :
			mRenderService(renderService)
		{
		}

		virtual rtti::RTTIObject* create(rtti::TypeInfo typeInfo) override
		{
			return new RenderableMeshResource(mRenderService);
		}

	private:
		RenderService& mRenderService;
	};

} // nap

