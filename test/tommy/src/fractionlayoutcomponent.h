#pragma once

#include "nap/componentinstance.h"
#include "renderattributes.h"

namespace nap
{
	class FractionLayoutComponent;
	class TransformComponent;
	class RenderableMeshComponent;

	/**
	 * Struct to contain the properties of a FractionLayout, so that they can be shared between the Resource and Instance
	 */
	struct FractionLayoutProperties
	{
		/** 
		 * Enum used to specify which pivot to use for positioning this element
		 */
		enum class EPositionPivot
		{
			TopLeft,		// Pivot in top-left of the element
			Center			// Pivit in center of the element
		};

		/**
		 * Enum used to specify how this layout should determine its size
		 */
		enum class ESizeBehaviour
		{
			Default,						// Calculate size based on the user-specified fractions in both x and y
			WidthFromImageAspectRatio,		// Height specified by user in fraction of parent, width is calculated from aspect ratio of the image
			HeightFromImageAspectRatio		// Width specified by user in fraction of parent, height is calculated from aspect ratio of the image
		};

		EPositionPivot	mPositionPivot	= EPositionPivot::TopLeft;		// The pivot to use for positioning this element
		glm::vec3		mPosition;										// Position of this element, as a fraction of its parent
		ESizeBehaviour	mSizeBehaviour	= ESizeBehaviour::Default;		// The behaviour to use when determining the pixel-size of this element
		glm::vec2		mSize			= glm::vec2(1.0f, 1.0f);		// The size of this element, as a fraction of its parent
	};


	/**
	 * The Resource for the FractionLayoutComponent (i.e. what the user specifies in json)
	 */
	class FractionLayoutComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		/**
		 * Get the type of ComponentInstance to create
		 */
		virtual const rtti::TypeInfo getInstanceType() const 
		{ 
			return RTTI_OF(FractionLayoutComponent); 
		}
		
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components)
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		FractionLayoutProperties mProperties;
	};


	/**
	 * The FractionLayoutComponent (runtime instance). This component is used to position and size an entity, relative to its parent (i.e. in fractions)
	 * It is assumed that an orthographic camera is used, with pixel-space coordinates
	 */
	class FractionLayoutComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		FractionLayoutComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		/**
		 * Init this object after deserialization
		 */
		bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState);

		/** 
		 * Update the layout for this entity
		 *
		 * @param windowSize The size of the window (in pixels) we're being layed-out in.
		 * @param parentWorldTransform The WorldTransform of the parent layout
		 */
		void updateLayout(const glm::vec2& windowSize, const glm::mat4x4& parentWorldTransform);

	private:
		FractionLayoutProperties	mProperties;
		TransformComponent*			mTransformComponent = nullptr;
		RenderableMeshComponent*	mRenderableMeshComponent = nullptr;
	};
}