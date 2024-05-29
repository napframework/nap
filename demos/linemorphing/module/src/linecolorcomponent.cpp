/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "linecolorcomponent.h"

// External Includes
#include <nap/logger.h>
#include <entity.h>
#include <nap/numeric.h>

RTTI_BEGIN_CLASS(nap::LineColorComponent, "Applies color to the blended poly line")
	RTTI_PROPERTY("BlendComponent",			&nap::LineColorComponent::mBlendComponent,		nap::rtti::EPropertyMetaData::Required, "Link to the line blender")
	RTTI_PROPERTY("FirstColor",				&nap::LineColorComponent::mColorOne,			nap::rtti::EPropertyMetaData::Required, "Start color")
	RTTI_PROPERTY("SecondColor",			&nap::LineColorComponent::mColorTwo,			nap::rtti::EPropertyMetaData::Required, "End color")
	RTTI_PROPERTY("Intensity",				&nap::LineColorComponent::mIntensity,			nap::rtti::EPropertyMetaData::Default,	"Spline intensity")
	RTTI_PROPERTY("Wrap",					&nap::LineColorComponent::mWrap,				nap::rtti::EPropertyMetaData::Default,	"If the color values should be wrapped")
	RTTI_PROPERTY("WrapPower",				&nap::LineColorComponent::mWrapPower,			nap::rtti::EPropertyMetaData::Default,	"Wrap intensity")
	RTTI_PROPERTY("Link",					&nap::LineColorComponent::mLink,				nap::rtti::EPropertyMetaData::Default,	"Only use the first color")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineColorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Set intensity and color values
		mColorOne = getComponent<LineColorComponent>()->mColorOne.get();
		mColorTwo = getComponent<LineColorComponent>()->mColorTwo.get();

		setIntensity(getComponent<LineColorComponent>()->mIntensity);

		mWrap = getComponent<LineColorComponent>()->mWrap;
		mPower = getComponent<LineColorComponent>()->mWrapPower;
		mLink = getComponent<LineColorComponent>()->mLink;

		// Force first smoother to be the first color
		mColorOneSmoother.setValue(mColorOne->mValue.toVec3());

		// Force second smoother to be the second color
		mColorTwoSmoother.setValue(mColorTwo->mValue.toVec3());

		// Copy over intensity smooth time
		mIntensitySmoother.setValue(getComponent<LineColorComponent>()->mIntensity);

		return true;
	}


	void LineColorComponentInstance::update(double deltaTime)
	{
		// Update start smoothing operator
		mColorOneSmoother.update(mColorOne->mValue.toVec3(), deltaTime);

		// Update end smoothing operator
		mColorTwoSmoother.update(mColorTwo->mValue.toVec3(), deltaTime);

		// Update intensity smooth operator
		mIntensitySmoother.update(mIntensity, deltaTime);

		// Get the line and color attribute we want to update
		nap::PolyLine& line = mBlendComponent->getLine();
		nap::Vec4VertexAttribute& color_attr = line.getColorAttr();

		// Get data to set (interpolate)
		int vert_count = color_attr.getCount();
		assert(vert_count > 1);
		std::vector<glm::vec4>& color_data = color_attr.getData();

		// Get inc blend value
		float inc = 1.0f / static_cast<float>(vert_count - 1);

		// Will hold the lerped color
		glm::vec3 lerped_color;

		// Update color values along line
		for (int i = 0; i < vert_count; i++)
		{
			float lerp_v = static_cast<float>(i) * inc;
			lerp_v = mWrap ? math::bell<float>(lerp_v, mPower) : lerp_v;

			// Get interpolated uv coordinates
			lerped_color = math::lerp<glm::vec3>(mColorOneSmoother.getValue(), mColorTwoSmoother.getValue(), lerp_v);

			// Set color
			color_data[i] = glm::vec4(lerped_color, mIntensitySmoother.getValue());
		}

		// Update line
		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn("unable to change color: %s", error.toString().c_str());
		}
	}


	void LineColorComponentInstance::setColorSmoothSpeed(float speed)
	{
		mColorOneSmoother.mSmoothTime = speed;
		mColorTwoSmoother.mSmoothTime = speed;
	}


	void LineColorComponentInstance::setIntensity(float intensity)
	{
		mIntensity = math::clamp<float>(intensity, 0.0f, 1.0f);
	}
}
