// local includes
#include "transformcomponent.h"
#include "nap/entityinstance.h"

// External includes
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
// TransformComponent
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool TransformComponent::init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		TransformComponentResource* xform_resource = rtti_cast<TransformComponentResource>(resource.get());
		mTranslate = xform_resource->mProperties.mTranslate;
		mRotate = glm::quat(glm::vec3(glm::radians(xform_resource->mProperties.mRotate.x), 
			glm::radians(xform_resource->mProperties.mRotate.y), 
			glm::radians(xform_resource->mProperties.mRotate.z)));
		mScale = xform_resource->mProperties.mScale;
		mUniformScale = xform_resource->mProperties.mUniformScale;
		return true;
	}

	// Constructs and returns this components local transform
	const glm::mat4x4& TransformComponent::getLocalTransform() const
	{
		if (mLocalDirty)
		{
			glm::mat4x4 xform_matrix = glm::translate(glm::mat4x4(), mTranslate);
			glm::mat4x4 rotat_matrix = glm::toMat4(mRotate);
			glm::mat4x4 scale_matrix = glm::scale(glm::mat4x4(), mScale * mUniformScale);
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
		mTranslate = translate;
		setDirty();
	}

	void TransformComponent::setRotate(const glm::quat& rotate)
	{
		mRotate = rotate;
		setDirty();
	}

	void TransformComponent::setScale(const glm::vec3& scale)
	{
		mScale = scale;
		setDirty();
	}

	void TransformComponent::setUniformScale(float scale)
	{
		mUniformScale = scale;
		setDirty();
	}
}
