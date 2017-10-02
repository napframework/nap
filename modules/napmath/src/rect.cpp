#include "Rect.h"
#include "mathutils.h"

RTTI_BEGIN_CLASS(nap::math::Rect)
	RTTI_PROPERTY("Min", &nap::math::Rect::mMinPosition, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Max", &nap::math::Rect::mMaxPosition, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace math
	{
		Rect::Rect(float x, float y, float width, float height)
		{
			mMinPosition = glm::vec2(x, y);
			mMaxPosition = mMinPosition + glm::vec2(width, height);
		}


		Rect::Rect(glm::vec2 min, glm::vec2 max) : mMinPosition(min), mMaxPosition(max)
		{}


		float Rect::getWidth() const
		{
			return abs(mMaxPosition.x - mMinPosition.x);
		}


		float Rect::getHeight() const
		{
			return abs(mMaxPosition.y - mMinPosition.y);
		}


		bool Rect::inside(const glm::vec2& point) const
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


		bool Rect::hasWidth() const
		{
			return getWidth() > 0.0f;
		}


		bool Rect::hasHeight() const
		{
			return getHeight() > 0.0f;
		}


		const glm::vec2& Rect::getMin() const
		{
			return mMinPosition;
		}


		const glm::vec2& Rect::getMax() const
		{
			return mMaxPosition;
		}

	}
}