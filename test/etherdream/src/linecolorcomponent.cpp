// Local Includes
#include "linecolorcomponent.h"

// External Includes
#include <nap/logger.h>
#include <nap/entity.h>
#include <nap/configure.h>

RTTI_BEGIN_CLASS(nap::LineColorComponent)
	RTTI_PROPERTY("BlendComponent",		&nap::LineColorComponent::mBlendComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LookupImage",		&nap::LineColorComponent::mLookupImage,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StartPosition",		&nap::LineColorComponent::mStartPos,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EndPosition",		&nap::LineColorComponent::mEndPos,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity",			&nap::LineColorComponent::mIntensity,		nap::rtti::EPropertyMetaData::Default)	
	RTTI_PROPERTY("Wrap",				&nap::LineColorComponent::mWrap,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapPower",			&nap::LineColorComponent::mWrapPower,		nap::rtti::EPropertyMetaData::Default)		
	RTTI_PROPERTY("Link",				&nap::LineColorComponent::mLink,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	/**
	 *	Utility function that returns a normalized float color value based on type T
	 */
	template<typename T>
	void getPixelColor(const opengl::Bitmap& bitmap, const glm::ivec2& pixelCoordinates, glm::vec3& color, float divider)
	{
		T* pixel_color = bitmap.getPixel<T>(pixelCoordinates.x, pixelCoordinates.y);
		switch (bitmap.getColorType())
		{
		case opengl::BitmapColorType::RGB:
		case opengl::BitmapColorType::RGBA:
			color.r = static_cast<float>(pixel_color[0]) / divider;
			color.g = static_cast<float>(pixel_color[1]) / divider;
			color.b = static_cast<float>(pixel_color[2]) / divider;
			break;
		case opengl::BitmapColorType::BGR:
		case opengl::BitmapColorType::BGRA:
			color.r = static_cast<float>(pixel_color[2]) / divider;
			color.g = static_cast<float>(pixel_color[1]) / divider;
			color.b = static_cast<float>(pixel_color[0]) / divider;
			break;
		default:
			assert(false);
			break;
		}
	}


	bool LineColorComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get necessary objects
		mBlendComponent = getComponent<LineColorComponent>()->mBlendComponent.get();
		mLookupImage	= getComponent<LineColorComponent>()->mLookupImage.get();

		// Set start and end uv sample position
		setStartPosition(getComponent<LineColorComponent>()->mStartPos);
		setEndPosition(getComponent<LineColorComponent>()->mEndPos);
		setIntensity(getComponent<LineColorComponent>()->mIntensity);

		mWrap = getComponent<LineColorComponent>()->mWrap;
		mPower = getComponent<LineColorComponent>()->mWrapPower;
		mLink = getComponent<LineColorComponent>()->mLink;

		// Ensure the image is at least RGB
		if (!(mLookupImage->getBitmap().getColorType() >=  opengl::BitmapColorType::RGB))
			return errorState.check(false, "lookup image does not have 3 or more color channels");

		return true;
	}


	void LineColorComponentInstance::update(double deltaTime)
	{
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
		glm::vec2 end_pos = mLink ? mStartPosition : mEndPosition;

		// Update color values along line
		for (int i = 0; i < vert_count; i++)
		{
			float lerp_v = static_cast<float>(i) * inc;
			lerp_v = mWrap ? math::bell<float>(lerp_v, mPower) : lerp_v;

			// Get interpolated uv coordinates
			lerped_uv_coordinates = math::lerp<glm::vec2>(mStartPosition, end_pos, lerp_v);

			// Get pixel color
			getColor(lerped_uv_coordinates, lerp_color);
			
			// Set color
			color_data[i] = glm::vec4(lerp_color, mIntensity);
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
		const opengl::Bitmap& bitmap = mLookupImage->getBitmap();
		glm::ivec2 bitmap_coordinates;
		bitmap_coordinates.x = static_cast<int>(static_cast<float>(bitmap.getWidth()  - 1) * uvPos.x);
		bitmap_coordinates.y = static_cast<int>(static_cast<float>(bitmap.getHeight() - 1) * uvPos.y);

		// Get pixel color data for current lerped bitmap coordinate value
		switch (bitmap.getDataType())
		{
		case opengl::BitmapDataType::BYTE:
			getPixelColor<nap::uint8>(bitmap, bitmap_coordinates, outColor, math::max<nap::uint8>());
			break;
		case opengl::BitmapDataType::FLOAT:
			getPixelColor<float>(bitmap, bitmap_coordinates, outColor, 1.0f);
			break;
		case opengl::BitmapDataType::USHORT:
			getPixelColor<nap::uint16>(bitmap, bitmap_coordinates, outColor, math::max<nap::uint16>());
			break;
		default:
			assert(false);
			break;
		}
	}

}