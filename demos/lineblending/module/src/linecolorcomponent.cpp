// Local Includes
#include "linecolorcomponent.h"

// External Includes
#include <nap/logger.h>
#include <entity.h>
#include <nap/numeric.h>

RTTI_BEGIN_CLASS(nap::LineColorComponent)
	RTTI_PROPERTY("BlendComponent",			&nap::LineColorComponent::mBlendComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FirstColor",				&nap::LineColorComponent::mColorOne,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SecondColor",			&nap::LineColorComponent::mColorTwo,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity",				&nap::LineColorComponent::mIntensity,			nap::rtti::EPropertyMetaData::Default)	
	RTTI_PROPERTY("Wrap",					&nap::LineColorComponent::mWrap,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapPower",				&nap::LineColorComponent::mWrapPower,			nap::rtti::EPropertyMetaData::Default)		
	RTTI_PROPERTY("Link",					&nap::LineColorComponent::mLink,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineColorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Set intensity and color values
		setFirstColor(getComponent<LineColorComponent>()->mColorOne);
		setSecondColor(getComponent<LineColorComponent>()->mColorTwo);
		setIntensity(getComponent<LineColorComponent>()->mIntensity);

		mWrap = getComponent<LineColorComponent>()->mWrap;
		mPower = getComponent<LineColorComponent>()->mWrapPower;
		mLink = getComponent<LineColorComponent>()->mLink;

		// Force first smoother to be the first color
		mColorOneSmoother.setValue(mFirstColor.toVec3());

		// Force second smoother to be the second color
		mColorTwoSmoother.setValue(mSecondColor.toVec3());

		// Copy over intensity smooth time
		mIntensitySmoother.setValue(getComponent<LineColorComponent>()->mIntensity);

		return true;
	}


	void LineColorComponentInstance::update(double deltaTime)
	{
		// Update start smoothing operator
		mColorOneSmoother.update(mFirstColor.toVec3(), deltaTime);

		// Update end smoothing operator
		mColorTwoSmoother.update(mSecondColor.toVec3(), deltaTime);

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


	void LineColorComponentInstance::setFirstColor(const RGBColorFloat& color)
	{
		mFirstColor = color;
	}


	void LineColorComponentInstance::setSecondColor(const RGBColorFloat& color)
	{
		mSecondColor = color;
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