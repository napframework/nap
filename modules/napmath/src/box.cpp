#include "box.h"

RTTI_BEGIN_CLASS(nap::math::Box)
	RTTI_CONSTRUCTOR(float, float, float)
	RTTI_CONSTRUCTOR(const glm::vec2&, const glm::vec2&, const glm::vec2&)
	RTTI_CONSTRUCTOR(const glm::vec3&, const glm::vec3&)
	RTTI_PROPERTY("Min", &nap::math::Box::mMinCoordinates, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Max", &nap::math::Box::mMaxCoordinates, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace math
	{
		Box::Box(float width, float height, float depth)
		{
			mMinCoordinates = { 0.0f - (width / 2.0f), 0.0f - (height / 2.0f) , 0.0f - (depth / 2.0f) };
			mMaxCoordinates = { 0.0f + (width / 2.0f), 0.0f + (height / 2.0f) , 0.0f + (depth / 2.0f) };
		}


		Box::Box(float width, float height, float depth, const glm::vec3& position)
		{
			mMinCoordinates = { position.x - (width / 2.0f), position.y - (height / 2.0f) , position.z - (depth / 2.0f) };
			mMaxCoordinates = { position.x + (width / 2.0f), position.y + (height / 2.0f) , position.z + (depth / 2.0f) };
		}


		Box::Box(const glm::vec2& xCoordinates, const glm::vec2& yCoordinates, const glm::vec2& zCoordinates)
		{
			mMinCoordinates = { xCoordinates.x, yCoordinates.x, zCoordinates.x };
			mMaxCoordinates = { xCoordinates.y, yCoordinates.y, zCoordinates.y };
		}


		Box::Box(const glm::vec3& min, const glm::vec3& max) : mMinCoordinates(min), mMaxCoordinates(max)
		{
			
		}


		float Box::getWidth() const
		{
			return fabs(mMaxCoordinates.x - mMinCoordinates.x);
		}


		float Box::getHeight() const
		{
			return fabs(mMaxCoordinates.y - mMinCoordinates.y);
		}


		float Box::getDepth() const
		{
			return fabs(mMaxCoordinates.z - mMinCoordinates.z);
		}


		bool Box::inside(const glm::vec3& point) const
		{
			// Ensure bounds are accurate, ie: min max are not flipped
			float min_x = mMinCoordinates.x < mMaxCoordinates.x ? mMinCoordinates.x : mMaxCoordinates.x;
			float max_x = mMaxCoordinates.x > mMinCoordinates.x ? mMaxCoordinates.x : mMinCoordinates.x;
			float min_y = mMinCoordinates.y < mMaxCoordinates.y ? mMinCoordinates.y : mMaxCoordinates.y;
			float max_y = mMaxCoordinates.y > mMinCoordinates.y ? mMaxCoordinates.y : mMinCoordinates.y;
			float min_z = mMinCoordinates.z < mMaxCoordinates.z ? mMinCoordinates.z : mMaxCoordinates.z;
			float max_z = mMaxCoordinates.z > mMinCoordinates.z ? mMaxCoordinates.z : mMinCoordinates.z;

			bool in_x = point.x > min_x && point.x < max_x;
			bool in_y = point.y > min_y && point.y < max_y;
			bool in_z = point.z > min_z && point.z < max_z;
			return in_x && in_y && in_z;
		}

	}
}
