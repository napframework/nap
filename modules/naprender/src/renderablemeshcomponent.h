#pragma once

// Local Includes
#include "rendercomponent.h"
#include "vao.h"

// External Includes
#include <nap/objectptr.h>

namespace nap
{
	class Rect final
	{
	public:
		float mX = 0.0f;
		float mY = 0.0f;
		float mWidth = 0.0f;
		float mHeight = 0.0f;
	};

	class IMesh;
	class MaterialInstance;
	class TransformComponentInstance;
	class TransformComponent;
	class RenderableMeshComponentInstance;

	/**
	 * Resource class for RenderableMeshResource. Hold static data as read from file.
	 * The resource also holds a pointer to a mesh. this mesh is used by the instance of
	 * this class for drawing.
	 */
	class NAPAPI RenderableMeshComponent : public RenderableComponentResource
	{
		friend class RenderableMeshComponentInstance;
		RTTI_ENABLE(RenderableComponentResource)
	public:
		/**
		 * RenderableMesh uses transform to position itself in the world.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

		/**
		 * @return instance type to create for this resource.
		 */
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(RenderableMeshComponentInstance);
		}

		/**
		 * @return Mesh resource.
		 */
		IMesh& getMeshResource();

	public:
		ObjectPtr<IMesh>					mMesh = nullptr;					///< Resource to render
		MaterialInstanceResource			mMaterialInstance;					///< MaterialInstance, which is used to override uniforms for this instance
		Rect								mClipRect;							///< Clipping rectangle, in pixel coordinates
	};

	/**
	 * Represents a drawable mesh that can be used as a component in an object tree
	 * Every RenderableMeshComponent has a pointer to a RenderableMeshResource, which
	 * is an object that represents a mesh with a material assigned to it. To override
	 * uniform shader values from that material for this particular instance, the 
	 * component has a MaterialInstance.
	 */
	class NAPAPI RenderableMeshComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)

	public:
		RenderableMeshComponentInstance(EntityInstance& entity, Component& component);

		/**
		 * Acquires VAO, copies clipping rectangle, initializes material instance.
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * @return MaterialInstance for this component.
		 */
		MaterialInstance& getMaterialInstance();

		/**
		 * @return the MeshResource associated with this component for rendering.
		 */
		IMesh& getMesh();

		/**
		 * Set the mesh resource for this component to render
		 * @param mesh pointer to the mesh (resource) this component will render
		 * @param error contains the error message if the mesh could not be set successfully
		 * @param return if the mesh is set successfully
		 */
		bool setMesh(nap::ObjectPtr<IMesh> mesh, utility::ErrorState& error);

		/**
		 * @return the mesh instance that is rendered to screen
		 */
		MeshInstance& getMeshInstance();

		/**
		 * Toggles visibility.
		 */
		void setVisible(bool visible)													{ mVisible = visible; }

		/**
		 * Sets clipping rectangle on this instance.
		 * @param rect Rectangle in pixel coordinates.
		 */
		void setClipRect(const Rect& rect)												{ mClipRect = rect; }

	private:
		void pushUniforms();
		void setBlendMode();
		
		/**
		 * Tries to get a VAO handle for the current mesh / material combination
		 * @param mesh the mesh to acquire the handle for
		 * @param error contains the error state if the VAO could not be acquired
		 * @return unique ptr to the acquired VAO handle
		 */
		std::unique_ptr<nap::VAOHandle> acquireVertexArrayObject(nap::ObjectPtr<nap::IMesh> mesh, utility::ErrorState& error);

	private:
		TransformComponentInstance*					mTransformComponent;	// Cached pointer to transform
		std::unique_ptr<VAOHandle>					mVAOHandle;				// Handle to Vertex Array Object
		MaterialInstance							mMaterialInstance;		// MaterialInstance
		bool										mVisible = true;		// Whether this instance is visible or not
		Rect										mClipRect;				// Clipping rectangle for this instance, in pixel coordinates
		nap::RenderableMeshComponent*				mResource = nullptr;	// Resource of this instance
		nap::ObjectPtr<IMesh>						mMesh = nullptr;		// The mesh to render
	};
}
