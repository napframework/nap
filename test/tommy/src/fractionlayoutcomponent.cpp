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

	void FractionLayoutComponent::updateLayout(const glm::vec2& windowSize, const glm::mat4x4& parentWorldTransform)
	{
		// Get world transform of this element
		const glm::mat4x4 world_transform = parentWorldTransform * mTransformComponent->getLocalTransform();
		glm::vec3 world_parent_pos(world_transform[3][0], world_transform[3][1], world_transform[3][2]);
		glm::vec2 world_parent_size(world_transform[0][0], world_transform[1][1]);

		// Loop over child entities to find both RenderableMeshComponents en LayoutComponents
		for (EntityInstance* child_entity : getEntity()->getChildren())
		{
			// Set the clip rectangle for RenderableMeshComponents to this layout's size
			RenderableMeshComponent* child_renderable_mesh = child_entity->findComponent<RenderableMeshComponent>();
			if (child_renderable_mesh)
			{
				Rect clip_rect;
				clip_rect.mX		= world_parent_pos.x - world_parent_size.x * 0.5f;
				clip_rect.mY		= (windowSize.y - world_parent_pos.y) - world_parent_size.y * 0.5f;
				clip_rect.mWidth	= world_parent_size.x;
				clip_rect.mHeight	= world_parent_size.y;
				child_renderable_mesh->setClipRect(clip_rect);
			}

			FractionLayoutComponent* child_layout = child_entity->findComponent<FractionLayoutComponent>();
			if (child_layout == nullptr)
				continue;

			// Get size of child in world space
			TransformComponent& child_transform = child_entity->getComponent<TransformComponent>();
			glm::vec2 relative_child_size_frac = child_layout->getSize();
			glm::vec2 world_child_size = world_parent_size * relative_child_size_frac;

			// Handle aspect ratio's that are derived from textures
			if (child_layout->mProperties.mSizeBehaviour != FractionLayoutProperties::ESizeBehaviour::Default)
			{
				UniformTexture2D* texture_uniform = child_layout->mRenderableMeshComponent->getMaterialInstance().findUniform<UniformTexture2D>("mTexture");
				float aspect_ratio = 1.0f;
				if (texture_uniform)
				{
					const glm::vec2 image_size = texture_uniform->mTexture->getSize();
					aspect_ratio = image_size.x / image_size.y;
				}

				if (child_layout->mProperties.mSizeBehaviour == FractionLayoutProperties::ESizeBehaviour::WidthFromImageAspectRatio)
					world_child_size.x = world_child_size.y * aspect_ratio;
				if (child_layout->mProperties.mSizeBehaviour == FractionLayoutProperties::ESizeBehaviour::HeightFromImageAspectRatio)
					world_child_size.y = world_child_size.x / aspect_ratio;
			}
			
			// Calculate world space size back to a fraction
			relative_child_size_frac = world_child_size / world_parent_size;

			// Get the child position. The components are expressed as a fraction relative to the parent.
			glm::vec3 relative_child_pos_frac = child_layout->getPosition();
			
			// The plane that we use has it's pivot at the center. We need to correct half the size to calculate from the top-left
			if (child_layout->mProperties.mPositionPivot == FractionLayoutProperties::EPositionPivot::TopLeft)
				relative_child_pos_frac += glm::vec3(relative_child_size_frac.x * 0.5f, relative_child_size_frac.y * 0.5f, 0.0f);

			// The parent's final position is also expressed relative to it's center, so we correct for this as well:
			relative_child_pos_frac -= glm::vec3(0.5f, 0.5f, 0.0f);

			child_transform.setTranslate(relative_child_pos_frac);
			child_transform.setScale(glm::vec3(relative_child_size_frac, 1.0f));

			child_layout->updateLayout(windowSize, world_transform);
		}
	}
}