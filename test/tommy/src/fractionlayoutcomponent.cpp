#include "fractionlayoutcomponent.h"
#include "transformcomponent.h"
#include "renderablemeshcomponent.h"
#include "uniforms.h"

RTTI_BEGIN_ENUM(nap::FractionLayoutProperties::ESizeBehaviour)
	RTTI_ENUM_VALUE(nap::FractionLayoutProperties::ESizeBehaviour::Default,						"Default"),
	RTTI_ENUM_VALUE(nap::FractionLayoutProperties::ESizeBehaviour::WidthFromImageAspectRatio,	"WidthFromImageAspectRatio"),
	RTTI_ENUM_VALUE(nap::FractionLayoutProperties::ESizeBehaviour::HeightFromImageAspectRatio,	"HeightFromImageAspectRatio")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::FractionLayoutProperties::EPositionPivot)
	RTTI_ENUM_VALUE(nap::FractionLayoutProperties::EPositionPivot::TopLeft,						"TopLeft"),
	RTTI_ENUM_VALUE(nap::FractionLayoutProperties::EPositionPivot::Center,						"Center")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::FractionLayoutProperties)
	RTTI_PROPERTY("Position",		&nap::FractionLayoutProperties::mPosition,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PositionPivot",	&nap::FractionLayoutProperties::mPositionPivot,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::FractionLayoutProperties::mSize,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SizeBehaviour",	&nap::FractionLayoutProperties::mSizeBehaviour,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FractionLayoutComponentResource)
	RTTI_PROPERTY("Properties", &nap::FractionLayoutComponentResource::mProperties, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::FractionLayoutComponent, nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{
	bool FractionLayoutComponent::init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState)
	{
		mProperties = rtti_cast<FractionLayoutComponentResource>(resource.get())->mProperties;

		mTransformComponent = getEntity()->findComponent<TransformComponent>();
		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
			return false;

		if (mProperties.mSizeBehaviour != FractionLayoutProperties::ESizeBehaviour::Default)
		{
			mRenderableMeshComponent = getEntity()->findComponent<RenderableMeshComponent>();
			if (!errorState.check(mRenderableMeshComponent != nullptr, "FractionLayoutComponent requires a RenderableMeshComponent if the size behaviour is not Default"))
				return false;
		}

		return true;
	}

	void FractionLayoutComponent::updateLayout()
	{
		glm::vec2 parent_pos = mTransformComponent->getTranslate();
		glm::vec2 parent_size = mTransformComponent->getScale();

		for (EntityInstance* child_entity : getEntity()->getChildren())
		{
			FractionLayoutComponent* child_layout = child_entity->findComponent<FractionLayoutComponent>();
			if (child_layout == nullptr)
				continue;

			TransformComponent& child_transform = child_entity->getComponent<TransformComponent>();

			glm::vec2 relative_child_size_frac = child_layout->getSize();
			glm::vec2 child_size = parent_size * relative_child_size_frac;

			if (child_layout->mProperties.mSizeBehaviour != FractionLayoutProperties::ESizeBehaviour::Default)
			{
				UniformTexture2D* texture_uniform = child_layout->mRenderableMeshComponent->getMaterialInstance()->findUniform<UniformTexture2D>("mTexture");
				float aspect_ratio = 1.0f;
				if (texture_uniform)
				{
					const glm::vec2 image_size = texture_uniform->mTexture->getSize();
					aspect_ratio = image_size.x / image_size.y;
				}

				if (child_layout->mProperties.mSizeBehaviour == FractionLayoutProperties::ESizeBehaviour::WidthFromImageAspectRatio)
					child_size.x = child_size.y * aspect_ratio;
				if (child_layout->mProperties.mSizeBehaviour == FractionLayoutProperties::ESizeBehaviour::HeightFromImageAspectRatio)
					child_size.y = child_size.x / aspect_ratio;
			}
			
			relative_child_size_frac = child_size / parent_size;

			glm::vec2 relative_child_pos_frac = child_layout->getPosition();
			if (child_layout->mProperties.mPositionPivot == FractionLayoutProperties::EPositionPivot::TopLeft)
				relative_child_pos_frac += relative_child_size_frac * 0.5f;

			child_transform.setTranslate(glm::vec3(relative_child_pos_frac, -50.0f));
			child_transform.setScale(glm::vec3(relative_child_size_frac, 1.0f));

			child_layout->updateLayout();
		}
	}
}