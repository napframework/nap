#pragma once

// Local Includes
#include "rendercomponent.h"
#include "renderablemesh.h"

// External Includes
#include <rtti/objectptr.h>
#include <transformcomponent.h>
#include <rect.h>

namespace nap
{
	class RenderableMeshComponentInstance;
	
	/**
	* Resource class for RenderableMeshResource. Hold static data as read from file.
	*/
	class NAPAPI RenderableMeshComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableMeshComponent, RenderableMeshComponentInstance)
	public:
		/**
		* RenderableMesh uses transform to position itself in the world.
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

		/**
		* @return Mesh resource.
		*/
		IMesh& getMeshResource()			{ return *mMesh; }

	public:
		rtti::ObjectPtr<IMesh>				mMesh;								///< Property: 'Mesh' Resource to render
		MaterialInstanceResource			mMaterialInstanceResource;			///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance
		math::Rect							mClipRect;							///< Property: 'ClipRect' Optional clipping rectangle, in pixel coordinates
	};


	/**
	 * Represents a renderable mesh that can be used as a component in an entity hierarchy.
	 * A mesh becomes 'renderable' when it is used in combination with a material. Such a mesh-material combination
	 * forms a 'RenderableMesh'. It is validated that a mesh-material combination form a legal combination: vertex attributes
	 * in shader and mesh should match. Otherwise, the RenderableMesh is invalid. The RenderableMesh is an object that is created
	 * internally based on the mesh and the material that are set in the RenderableMeshComponent.
	 * It is, however, also possible to switch the mesh and/or material from the RenderableMeshComponent to some other mesh and/or 
	 * material. To do so, other components should create their own RenderableMesh by calling createRenderableMesh, and pass the returned
	 * object to setMesh. The object calling createRenderableMesh should own any custom mesh and/or material and it should validate in it's
	 * init() that the creation of the renderable mesh succeeded.
	 */
	class NAPAPI RenderableMeshComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)

	public:
		RenderableMeshComponentInstance(EntityInstance& entity, Component& component);

		/**
		 * Acquires VAO, copies clipping rectangle, initializes material instance.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates a renderable mesh that can be used to switch to another mesh and/or material at runtime. This function should be called from
		 * init() functions on other components, and the result should be validated.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param materialInstance The material instance that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState);

		/**
		 * Creates a renderable mesh that can be used to switch to another mesh at runtime. The material remains the same material as the one that
		 * was already set on the RenderableMeshComponent.. This function should be called from init() functions on other components, and the result 
		 * should be validated.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState);

		/**
		 * Switches the mesh and/or the material that is rendered. The renderable mesh should be created through createRenderableMesh, and must
		 * be created from an init() function.
		 * @param mesh The mesh that was retrieved through createRenderableMesh.
		 */
		void setMesh(const RenderableMesh& mesh);

		/**
		* Renders the model from the ModelResource, using the material on the ModelResource.
		*/
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		* @return MaterialInstance for this component.
		*/
		MaterialInstance& getMaterialInstance();

		/**
		* @return Currently active mesh.
		*/
		IMesh& getMesh()										{ return *mRenderableMesh.mMesh; }

		/**
		* @return Currently active mesh instance.
		*/
		MeshInstance& getMeshInstance()							{ return getMesh().getMeshInstance(); }

		/**
		* Toggles visibility.
		*/
		void setVisible(bool visible)							{ mVisible = visible; }

		/**
		* Sets clipping rectangle on this instance.
		* @param rect Rectangle in pixel coordinates.
		*/
		void setClipRect(const math::Rect& rect)				{ mClipRect = rect; }

	private:
		void pushUniforms();
		void setBlendMode();

	private:
		TransformComponentInstance*				mTransformComponent;	// Cached pointer to transform
		MaterialInstance						mMaterialInstance;		// The MaterialInstance as created from the resource. 
		bool									mVisible = true;		// Whether this instance is visible or not
		math::Rect								mClipRect;				// Clipping rectangle for this instance, in pixel coordinates
		RenderableMesh							mRenderableMesh;		// The currently active renderable mesh, either set during init() or set by setMesh.
	};
}