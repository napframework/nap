#include "smoothdamp.h"

RTTI_DEFINE_BASE(nap::math::BaseSmoothOperator)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::math::FloatSmoothOperator)
	RTTI_CONSTRUCTOR(const float&, float)
	RTTI_CONSTRUCTOR(const float&, float, float)
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
		}

		template<>
		void SmoothOperator<glm::vec2>::init()
		{
			mVelocity = { 0.0f, 0.0f };
		}

		template<>
		void SmoothOperator<glm::vec3>::init()
		{
			mVelocity = { 0.0f, 0.0f, 0.0f };
		}

		template<>
		void SmoothOperator<glm::vec4>::init()
		{
			mVelocity = { 0.0f, 0.0f, 0.0f, 0.0f };
		}
	}
}