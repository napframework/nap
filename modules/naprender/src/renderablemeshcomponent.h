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

	class Mesh;
	class MaterialInstance;
	class TransformComponentInstance;
	class TransformComponent;
	class RenderableMeshComponentInstance;

	/**
	 * Resource class for RenderableMeshResource. Hold static data as read from file.
	 */
	class NAPAPI RenderableMeshComponent : public RenderableComponentResource
	{
		RTTI_ENABLE(RenderableComponentResource)

		/**
		 * RenderableMesh uses transform to position itself in the world.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components)
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

		/**
		 * @return instance type to create for this resource.
		 */
		virtual const rtti::TypeInfo getInstanceType() const
		{
			return RTTI_OF(RenderableMeshComponentInstance);
		}

	public:
		ObjectPtr<Mesh>						mMeshResource;						///< Resource to render
		MaterialInstanceResource			mMaterialInstanceResource;			///< MaterialInstance, which is used to override uniforms for this instance
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
		RenderableMeshComponentInstance(EntityInstance& entity);

		/**
		 * Acquires VAO, copies clipping rectangle, initializes material instance.
		 */
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * @return MaterialInstance for this component.
		 */
		MaterialInstance& getMaterialInstance();

		/**
		 * Toggles visibility.
		 */
		void setVisible(bool visible) { mVisible = visible; }

		/**
		 * Sets clipping rectangle on this instance.
		 * @param rect Rectangle in pixel coordinates.
		 */
		void setClipRect(const Rect& rect) { mClipRect = rect; }

	private:
		void pushUniforms();
		void setBlendMode();

	private:
		ObjectPtr<RenderableMeshComponent>	mResource;				// Pointer to resource of this instance
		TransformComponentInstance*							mTransformComponent;	// Cached pointer to transform
		std::unique_ptr<VAOHandle>					mVAOHandle;				// Handle to Vertex Array Object
		MaterialInstance							mMaterialInstance;		// MaterialInstance
		bool										mVisible = true;		// Whether this instance is visible or not
		Rect										mClipRect;				// Clipping rectangle for this instance, in pixel coordinates
	};
}
