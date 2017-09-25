#pragma once

#include <utility/dllexport.h>
#include <rtti/rtti.h>
#include <glm/glm.hpp>

namespace nap
{
	namespace math
	{
		/**
		 *	Simple 2D rectangle
		 */
		class NAPAPI Rectangle final
		{
		public:
			/**
			 *	Default constructor
			 */
			Rectangle() = default;

			/**
			 * Utility constructor
			 * @param x the start x position of the rectangle
			 * @param y the start y position of the rectangle
			 * @param width the width of the rectangle
			 * @param height the height of the triangle
			 */
			Rectangle(float x, float y, float width, float height);

			/**
			 * Utility constructor
			 * @param min: the min x, y position of the rectangle
			 * @param max: the max x, y position of the rectangle
			 */
			Rectangle(glm::vec2 min, glm::vec2 max);

			/**
			 *	@return the absolute width of the rectangle
			 */
			float getWidth() const;

			/**
			 *	@return the absolute height of the rectangle
			 */
			float getHeight() const;

			/**
			 *	@return if a point is inside the rectangle, this excludes the exact edge
			 */
			bool inside(const glm::vec2& point) const;

			glm::vec2 mMinPosition = { 0.0f, 0.0f };		// min x,y position
			glm::vec2 mMaxPosition = { 0.0f, 0.0f };		// max x,y position
		};
	}
}
