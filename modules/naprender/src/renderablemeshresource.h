#pragma once

#include <nap/resource.h>
#include <nap/objectptr.h>
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
		RenderableMeshResource() = default;

		RenderableMeshResource(RenderService& renderService);

		~RenderableMeshResource();

		/**
 		 * Loads model from file.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string getDisplayName() const override;
		
		/**
		* @return The Vertex Array Object that contains the description of how the
		mesh is bound to the shader.
		*/
		opengl::VertexArrayObject& getVAO()				{ return *mVAO.get(); }

		/**
		* @return The Mesh Resource
		*/
		MeshResource* getMeshResource();

		/**
		* @return The Material that is applied to the mesh.
		*/
		Material* getMaterial();

	public:
		ObjectPtr<Material>		mMaterialResource = nullptr;
		ObjectPtr<MeshResource>	mMeshResource = nullptr;

	private:
		std::unique_ptr<opengl::VertexArrayObject>	mVAO;			///< Vertex Array Object, describing how to the mesh is bound to the applied shader/material
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
			mRenderService(renderService) { }

		/**
		* @return Type of RenderableMeshResource
		*/
		rtti::TypeInfo getTypeToCreate() const override;

		/**
		* @return creates a renderableMeshResource
		*/
		virtual rtti::RTTIObject* create() override;

	private:
		RenderService& mRenderService;
	};

} // nap

