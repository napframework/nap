// Local Includes
#include "linecolorcomponent.h"

// External Includes
#include <nap/logger.h>
#include <entity.h>
#include <nap/configure.h>

RTTI_BEGIN_CLASS(nap::LineColorComponent)
	RTTI_PROPERTY("BlendComponent",			&nap::LineColorComponent::mBlendComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LookupImage",			&nap::LineColorComponent::mLookupImage,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StartPosition",			&nap::LineColorComponent::mStartPos,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StartSmoothTime",		&nap::LineColorComponent::mStartSmoothTime,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EndPosition",			&nap::LineColorComponent::mEndPos,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EndSmoothTime",			&nap::LineColorComponent::mEndSmoothTime,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity",				&nap::LineColorComponent::mIntensity,		nap::rtti::EPropertyMetaData::Default)	
	RTTI_PROPERTY("IntensitySmoothTime",	&nap::LineColorComponent::mIntensitySmoothTime,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Wrap",					&nap::LineColorComponent::mWrap,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapPower",				&nap::LineColorComponent::mWrapPower,		nap::rtti::EPropertyMetaData::Default)		
	RTTI_PROPERTY("Link",					&nap::LineColorComponent::mLink,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineColorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get necessary objects
		mLookupImage	= getComponent<LineColorComponent>()->mLookupImage.get();

		// Set start and end uv sample position
		setStartPosition(getComponent<LineColorComponent>()->mStartPos);
		setEndPosition(getComponent<LineColorComponent>()->mEndPos);
		setIntensity(getComponent<LineColorComponent>()->mIntensity);

		mWrap = getComponent<LineColorComponent>()->mWrap;
		mPower = getComponent<LineColorComponent>()->mWrapPower;
		mLink = getComponent<LineColorComponent>()->mLink;

		// Copy over start smooth times
		mStartSmootherX.mSmoothTime = getComponent<LineColorComponent>()->mStartSmoothTime.x;
		mStartSmootherX.setValue(getComponent<LineColorComponent>()->mStartPos.x);

		mStartSmootherY.mSmoothTime = getComponent<LineColorComponent>()->mStartSmoothTime.y;
		mStartSmootherY.setValue(getComponent<LineColorComponent>()->mStartPos.y);

		// Copy over end smooth times
		mEndSmootherX.mSmoothTime = getComponent<LineColorComponent>()->mEndSmoothTime.x;
		mEndSmootherX.setValue(getComponent<LineColorComponent>()->mEndPos.x);

		mEndSmootherY.mSmoothTime = getComponent<LineColorComponent>()->mEndSmoothTime.y;
		mEndSmootherY.setValue(getComponent<LineColorComponent>()->mEndPos.y);

		// Copy over intensity smooth time
		mIntensitySmoother.mSmoothTime = getComponent<LineColorComponent>()->mIntensitySmoothTime;

		// Ensure the image is at least RGB
		if (!(mLookupImage->getPixmap().getNumberOfChannels() >= 3))
			return errorState.check(false, "lookup image does not have 3 or more color channels");

		return true;
	}


	void LineColorComponentInstance::update(double deltaTime)
	{
		// Update start smoothing operator
		mStartSmootherX.update(mStartPosition.x, deltaTime);
		mStartSmootherY.update(mStartPosition.y, deltaTime);

		// Update end smoothing operator
		mEndSmootherX.update(mEndPosition.x, deltaTime);
		mEndSmootherY.update(mEndPosition.y, deltaTime);

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

		// Will hold the lerped color value in uv-space
		glm::vec3 lerp_color;

		// Will hold the interpolated uv coordinates
		glm::vec2 lerped_uv_coordinates;

		// End sample position, is the same as the beginning if linking is turned on
		glm::vec2 end_pos = mLink ? glm::vec2(mStartSmootherX.getValue(), mStartSmootherY.getValue()) : glm::vec2(mEndSmootherX.getValue(), mEndSmootherY.getValue());
		end_pos.x = math::clamp<float>(end_pos.x, 0.0f, 1.0f);
		end_pos.y = math::clamp<float>(end_pos.y, 0.0f, 1.0f);

		// Get start position
		glm::vec2 sta_pos = {mStartSmootherX.getValue(), mStartSmootherY.getValue()};
		sta_pos.x = math::clamp<float>(sta_pos.x, 0.0f, 1.0f);
		sta_pos.y = math::clamp<float>(sta_pos.y, 0.0f, 1.0f);

		// Update color values along line
		for (int i = 0; i < vert_count; i++)
		{
			float lerp_v = static_cast<float>(i) * inc;
			lerp_v = mWrap ? math::bell<float>(lerp_v, mPower) : lerp_v;

			// Get interpolated uv coordinates
			lerped_uv_coordinates = math::lerp<glm::vec2>(sta_pos, end_pos, lerp_v);

			// Get and set color
			getColor(lerped_uv_coordinates, lerp_color);
			color_data[i] = glm::vec4(lerp_color, mIntensitySmoother.getValue());
		}

		// Update line
		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn("unable to change color: %s", error.toString().c_str());
		}
	}


	void LineColorComponentInstance::setStartPosition(const glm::vec2& startPosition)
	{
		mStartPosition.x = math::clamp<float>(startPosition.x, 0.0f, 1.0f);
		mStartPosition.y = math::clamp<float>(startPosition.y, 0.0f, 1.0f);
	}


	void LineColorComponentInstance::setEndPosition(const glm::vec2& endPosition)
	{
		mEndPosition.x = math::clamp<float>(endPosition.x, 0.0f, 1.0f);
		mEndPosition.y = math::clamp<float>(endPosition.y, 0.0f, 1.0f);
	}


	void LineColorComponentInstance::setIntensity(float intensity)
	{
		mIntensity = math::clamp<float>(intensity, 0.0f, 1.0f);
	}


	void LineColorComponentInstance::getColor(const glm::vec2& uvPos, glm::vec3& outColor)
	{
		// Get max width and height in bitmap space
		const Pixmap& pixmap = mLookupImage->getPixmap();
		int x = static_cast<int>(static_cast<float>(pixmap.getWidth()  - 1) * uvPos.x);
		int y = static_cast<int>(static_cast<float>(pixmap.getHeight() - 1) * uvPos.y);

		// Get bitmap values, ie: where
		RGBColorFloat rcolor = pixmap.getPixel<RGBColorFloat>(x, y);

		// Set color
		outColor = glm::vec3(rcolor.getRed(), rcolor.getGreen(), rcolor.getBlue());
	}
}