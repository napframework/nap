#pragma once

#include "nap/entity.h"
#include "renderattributes.h"

namespace nap
{
	struct FractionLayoutProperties
	{
		enum class EPositionPivot
		{
			TopLeft,
			Center
		};

		enum class ESizeBehaviour
		{
			Default,
			WidthFromImageAspectRatio,
			HeightFromImageAspectRatio
		};

		EPositionPivot	mPositionPivot = EPositionPivot::TopLeft;
		glm::vec2		mPosition;
		ESizeBehaviour	mSizeBehaviour = ESizeBehaviour::Default;
		glm::vec2		mSize = glm::vec2(1.0f, 1.0f);
	};

	class FractionLayoutComponent;
	class TransformComponent;
	class RenderableMeshComponent;
	class FractionLayoutComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(FractionLayoutComponent); }
		
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components)
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		FractionLayoutProperties mProperties;
	};

	class FractionLayoutComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		FractionLayoutComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState);
		void updateLayout(const glm::vec2& windowSize, const glm::mat4x4& parentWorldTransform);

		const glm::vec2& getPosition() const { return mProperties.mPosition; }
		const glm::vec2& getSize() const { return mProperties.mSize; }

	private:
		FractionLayoutProperties	mProperties;
		TransformComponent*			mTransformComponent = nullptr;
		RenderableMeshComponent*	mRenderableMeshComponent = nullptr;
	};
}