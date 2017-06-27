// local includes
#include "transformcomponent.h"
#include "nap/entityinstance.h"

// External includes
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

RTTI_BEGIN_CLASS(nap::TransformProperties)
	RTTI_PROPERTY("Translate",		&nap::TransformProperties::mTranslate,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rotate",			&nap::TransformProperties::mRotate,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale",			&nap::TransformProperties::mScale,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UniformScale",	&nap::TransformProperties::mUniformScale,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::TransformComponentResource)
	RTTI_PROPERTY("Properties", &nap::TransformComponentResource::mProperties, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::TransformComponent, nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{
	bool TransformComponent::init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mProperties = rtti_cast<TransformComponentResource>(resource.get())->mProperties;
		return true;
	}

	// Constructs and returns this components local transform
	const glm::mat4x4& TransformComponent::getLocalTransform() const
	{
		if (mLocalDirty)
		{
			glm::mat4x4 xform_matrix = glm::translate(glm::mat4x4(), mProperties.mTranslate);
			glm::mat4x4 rotat_matrix = glm::toMat4(mProperties.mRotate);
			glm::mat4x4 scale_matrix = glm::scale(glm::mat4x4(), mProperties.mScale * mProperties.mUniformScale);
			mLocalMatrix = (xform_matrix * rotat_matrix * scale_matrix);
			mLocalDirty = false;
		}
		return mLocalMatrix;
	}


	// Return the global transform
	const glm::mat4x4& TransformComponent::getGlobalTransform() const
	{
		return mGlobalMatrix;
	}


	// Sets local flag dirty
	void TransformComponent::setDirty()
	{
		mLocalDirty = true;
		mWorldDirty = true;
	}


	// Updates it's global and local matrix
	void TransformComponent::update(const glm::mat4& parentTransform)
	{
		mGlobalMatrix = parentTransform * getLocalTransform();
		mWorldDirty = false;
	}

	void TransformComponent::setTranslate(const glm::vec3& translate)
	{
		mProperties.mTranslate = translate;
		setDirty();
	}

	void TransformComponent::setRotate(const glm::quat& rotate)
	{
		mProperties.mRotate = rotate;
		setDirty();
	}

	void TransformComponent::setScale(const glm::vec3& scale)
	{
		mProperties.mScale = scale;
		setDirty();
	}

	void TransformComponent::setUniformScale(float scale)
	{
		mProperties.mUniformScale = scale;
		setDirty();
	}

	//////////////////////////////////////////////////////////////////////////

	void updateTransformsRecursive(EntityInstance& entity, bool parentDirty, const glm::mat4& parentTransform)
	{
		glm::mat4 new_transform = parentTransform;

		bool is_dirty = parentDirty;
		TransformComponent* transform = entity.findComponent<TransformComponent>();
		if (transform && (transform->isDirty() || parentDirty))
		{
			is_dirty = true;
			transform->update(parentTransform);
			new_transform = transform->getGlobalTransform();
		}

		for (EntityInstance* child : entity.getChildren())
			updateTransformsRecursive(*child, is_dirty, new_transform);
	}


	void updateTransforms(EntityInstance& entityInstance)
	{
		updateTransformsRecursive(entityInstance, false, glm::mat4(1.0f));
	}
}
