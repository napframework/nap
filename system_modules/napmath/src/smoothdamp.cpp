/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "smoothdamp.h"

RTTI_DEFINE_BASE(nap::math::BaseSmoothOperator)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::math::FloatSmoothOperator)
	RTTI_CONSTRUCTOR(const float&, float)
	RTTI_CONSTRUCTOR(const float&, float, float)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::math::DoubleSmoothOperator)
	RTTI_CONSTRUCTOR(const double&, float)
	RTTI_CONSTRUCTOR(const double&, float, float)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::math::Vec2SmoothOperator)
	RTTI_CONSTRUCTOR(const glm::vec2&, float)
	RTTI_CONSTRUCTOR(const glm::vec2&, float, float)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::math::Vec3SmoothOperator)
	RTTI_CONSTRUCTOR(const glm::vec3&, float)
	RTTI_CONSTRUCTOR(const glm::vec3&, float, float)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::math::Vec4SmoothOperator)
	RTTI_CONSTRUCTOR(const glm::vec4&, float)
	RTTI_CONSTRUCTOR(const glm::vec4&, float, float)
RTTI_END_CLASS

namespace nap
{
	namespace math
	{
		template<>
		void SmoothOperator<float>::init()
		{
			mVelocity = 0.0f;
			mTarget = 0.0f;
		}

		template<>
		void SmoothOperator<double>::init()
		{
			mVelocity = 0.0;
			mTarget = 0.0;
		}

		template<>
		void SmoothOperator<glm::vec2>::init()
		{
			mVelocity = { 0.0f, 0.0f };
			mTarget = { 0.0f, 0.0f };

		}

		template<>
		void SmoothOperator<glm::vec3>::init()
		{
			mVelocity = { 0.0f, 0.0f, 0.0f };
			mTarget	= { 0.0f, 0.0f, 0.0f };
		}

		template<>
		void SmoothOperator<glm::vec4>::init()
		{
			mVelocity = { 0.0f, 0.0f, 0.0f, 0.0f };
			mTarget	= { 0.0f, 0.0f, 0.0f, 0.0f };
		}

		template<>
		void SmoothOperator<float>::setValue(const float& value)
		{
			mValue = value;
			mVelocity = 0.0f;
		}

		template<>
		void SmoothOperator<double>::setValue(const double& value)
		{
			mValue = value;
			mVelocity = 0.0;
		}

		template<>
		void SmoothOperator<glm::vec2>::setValue(const glm::vec2& value)
		{
			mValue = value;
			mVelocity = { 0.0f, 0.0f };
		}

		template<>
		void SmoothOperator<glm::vec3>::setValue(const glm::vec3& value)
		{
			mValue = value;
			mVelocity = { 0.0f, 0.0f, 0.0f };
		}

		template<>
		void SmoothOperator<glm::vec4>::setValue(const glm::vec4& value)
		{
			mValue = value;
			mVelocity = { 0.0f, 0.0f, 0.0f, 0.0f };
		}
	}
}