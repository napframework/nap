/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "transformcomponent.h"
#include "entity.h"

// External includes
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <mathutils.h>

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::TransformProperties)
	RTTI_PROPERTY("Translate",		&nap::TransformProperties::mTranslate,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rotate",			&nap::TransformProperties::mRotate,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale",			&nap::TransformProperties::mScale,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UniformScale",	&nap::TransformProperties::mUniformScale,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::TransformInstanceProperties)
	RTTI_PROPERTY("Translate",		&nap::TransformInstanceProperties::mTranslate,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rotate",			&nap::TransformInstanceProperties::mRotate,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale",			&nap::TransformInstanceProperties::mScale,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UniformScale",	&nap::TransformInstanceProperties::mUniformScale,	nap::rtti::EPropertyMetaData::Default)
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
		mRotate = math::eulerToQuat(radians(xform_resource->mProperties.mRotate));
		mScale = xform_resource->mProperties.mScale;
		mUniformScale = xform_resource->mProperties.mUniformScale;
		return true;
	}

	// Constructs and returns this components local transform
	const glm::mat4x4& TransformComponentInstance::getLocalTransform() const
	{
		if (mLocalDirty)
		{
			mLocalMatrix = math::composeMatrix(mTranslate, mRotate, mScale * mUniformScale);
			mLocalDirty = false;
		}
		return mLocalMatrix;
	}


	void TransformComponentInstance::setLocalTransform(const glm::mat4x4& matrix)
	{
		// Decompose matrix so individual components are represented correctly
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, mScale, mRotate, mTranslate, skew, perspective);

		// Store matrix
		mLocalMatrix = matrix;
		mLocalDirty = false;
		mUniformScale = 1.0f;

		// Recompute global matrix when asked
		mWorldDirty = true;
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


	TransformInstanceProperties TransformComponentInstance::getInstanceProperties() const
	{
		return { mTranslate, mRotate, mScale, mUniformScale };
	}


	void TransformComponentInstance::setInstanceProperties(const TransformInstanceProperties& props)
	{
		mTranslate = props.mTranslate;
		mRotate = props.mRotate;
		mScale = props.mScale;
		mUniformScale = props.mUniformScale;
		setDirty();
	}
}
