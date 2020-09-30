#include "rect.h"
#include "mathutils.h"

RTTI_BEGIN_CLASS(nap::math::Rect)
	RTTI_CONSTRUCTOR(float, float, float, float)
	RTTI_CONSTRUCTOR(glm::vec2, glm::vec2)
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


		Rect::Rect(const glm::vec2& min, const glm::vec2& max) : mMinPosition(min), mMaxPosition(max)
		{}


		float Rect::getWidth() const
		{
			return fabs(mMaxPosition.x - mMinPosition.x);
		}


		float Rect::getHeight() const
		{
			return fabs(mMaxPosition.y - mMinPosition.y);
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
			return getWidth() > math::epsilon<float>();
		}


		bool Rect::hasHeight() const
		{
			return getHeight() > math::epsilon<float>();
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
