/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "light.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Light)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PointLight)
	RTTI_PROPERTY("Origin", &nap::PointLight::mOrigin, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DirectionalLight)
	RTTI_PROPERTY("Direction", &nap::DirectionalLight::mDirection, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SpotLight)
	RTTI_PROPERTY("Origin", &nap::SpotLight::mOrigin, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InnerAngle", &nap::SpotLight::mInnerAngle, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OuterAngle", &nap::SpotLight::mOuterAngle, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Radius", &nap::SpotLight::mRadius, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Light
	//////////////////////////////////////////////////////////////////////////

	bool Light::init(utility::ErrorState& errorState)
	{
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Point
	//////////////////////////////////////////////////////////////////////////

	bool PointLight::init(utility::ErrorState& errorState)
	{
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Directional
	//////////////////////////////////////////////////////////////////////////

	bool DirectionalLight::init(utility::ErrorState& errorState)
	{
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Spot
	//////////////////////////////////////////////////////////////////////////

	bool SpotLight::init(utility::ErrorState& errorState)
	{
		return true;
	}
}
