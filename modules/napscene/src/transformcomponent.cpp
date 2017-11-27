// local includes
#include "transformcomponent.h"
#include "entity.h"

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

RTTI_BEGIN_CLASS(nap::TransformComponent)
	RTTI_PROPERTY("Properties", &nap::TransformComponent::mProperties, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TransformComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION("getTranslate",		&nap::TransformComponentInstance::getTranslate)
	RTTI_FUNCTION("setTranslate",		&nap::TransformComponentInstance::setTranslate)
	RTTI_FUNCTION("setRotate",			&nap::TransformComponentInstance::setRotate)
	RTTI_FUNCTION("getRotate",			&nap::TransformComponentInstance::getRotate)
	RTTI_FUNCTION("setScale",			&nap::TransformComponentInstance::setScale)
	RTTI_FUNCTION("getScale",			&nap::TransformComponentInstance::getScale)
	RTTI_FUNCTION("setUniformScale",	&nap::TransformComponentInstance::setUniformScale)
	RTTI_FUNCTION("getUniformScale",	&nap::TransformComponentInstance::getUniformScale)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// TransformComponent
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool TransformComponentInstance::init(utility::ErrorState& errorState)
	{
		TransformComponent* xform_resource = getComponent<TransformComponent>();
		mTranslate = xform_resource->mProperties.mTranslate;
		mRotate = glm::quat(glm::vec3(glm::radians(xform_resource->mProperties.mRotate.x), 
			glm::radians(xform_resource->mProperties.mRotate.y), 
			glm::radians(xform_resource->mProperties.mRotate.z)));
		mScale = xform_resource->mProperties.mScale;
		mUniformScale = xform_resource->mProperties.mUniformScale;
		return true;
	}

	// Constructs and returns this components local transform
	const glm::mat4x4& TransformComponentInstance::getLocalTransform() const
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
	const glm::mat4x4& TransformComponentInstance::getGlobalTransform() const
	{
		return mGlobalMatrix;
	}


	// Sets local flag dirty
	void TransformComponentInstance::setDirty()
	{
		mLocalDirty = true;
		mWorldDirty = true;
	}


	// Updates it's global and local matrix
	void TransformComponentInstance::update(const glm::mat4& parentTransform)
	{
		mGlobalMatrix = parentTransform * getLocalTransform();
		mWorldDirty = false;
	}


	void TransformComponentInstance::setTranslate(const glm::vec3& translate)
	{
		mTranslate = translate;
		setDirty();
	}

	void TransformComponentInstance::setRotate(const glm::quat& rotate)
	{
		mRotate = rotate;
		setDirty();
	}

	void TransformComponentInstance::setScale(const glm::vec3& scale)
	{
		mScale = scale;
		setDirty();
	}

	void TransformComponentInstance::setUniformScale(float scale)
	{
		mUniformScale = scale;
		setDirty();
	}
}
