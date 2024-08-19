/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rect.h"
#include "mathutils.h"

// External includes
#include <glm/gtc/epsilon.hpp>

RTTI_BEGIN_STRUCT(nap::math::Rect)
	RTTI_VALUE_CONSTRUCTOR(float, float, float, float)
	RTTI_CONSTRUCTOR(glm::vec2, glm::vec2)
	RTTI_PROPERTY("Min", &nap::math::Rect::mMinPosition, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Max", &nap::math::Rect::mMaxPosition, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

namespace nap
{
	namespace math
	{
		// Equal-to operator overload
		bool Rect::operator==(const Rect& other) const
		{
			const auto eps2 = glm::vec2(math::epsilon<float>());
			return glm::all(glm::epsilonEqual(getMin(), other.getMin(), eps2)) && glm::all(glm::epsilonEqual(getMax(), other.getMax(), eps2));
		}


		// Not-equal-to operator overload
		bool Rect::operator!=(const Rect& other) const
		{
			const auto eps2 = glm::vec2(math::epsilon<float>());
			return glm::any(glm::epsilonNotEqual(getMin(), other.getMin(), eps2)) || glm::any(glm::epsilonNotEqual(getMax(), other.getMax(), eps2));
		}


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
	}
}
