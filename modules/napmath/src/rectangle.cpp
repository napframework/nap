#include "rectangle.h"

RTTI_BEGIN_CLASS(nap::math::Rectangle)
	RTTI_PROPERTY("Min", &nap::math::Rectangle::mMinPosition, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Max", &nap::math::Rectangle::mMaxPosition, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace math
	{
		Rectangle::Rectangle(float x, float y, float width, float height)
		{
			mMinPosition = glm::vec2(x, y);
			mMaxPosition = mMinPosition + glm::vec2(width, height);
		}


		Rectangle::Rectangle(glm::vec2 min, glm::vec2 max) : mMinPosition(min), mMaxPosition(max)
		{}


		float Rectangle::getWidth() const
		{
			return abs(mMaxPosition.x - mMinPosition.x);
		}


		float Rectangle::getHeight() const
		{
			return abs(mMaxPosition.y - mMinPosition.y);
		}


		bool Rectangle::inside(const glm::vec2& point) const
		{
			// Ensure bounds are accurate, ie: min max are not flipped
			float min_x = mMinPosition.x < mMaxPosition.x ? mMinPosition.x : mMaxPosition.x;
			float max_x = mMaxPosition.x > mMinPosition.x ? mMaxPosition.x : mMinPosition.x;
			float min_y = mMinPosition.y < mMaxPosition.y ? mMinPosition.y : mMaxPosition.y;
			float max_y = mMaxPosition.y > mMinPosition.y ? mMaxPosition.y : mMinPosition.y;

			bool in_x = point.x > min_x && point.x < max_x;
			bool in_y = point.y > min_y && point.y < max_y;
			return in_x && in_y;
		}
	}
}