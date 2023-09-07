/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <utility/dllexport.h>
#include <rtti/rtti.h>
#include <glm/glm.hpp>

namespace nap
{
	namespace math
	{
		/**
		 *	Simple 2D rectangle.
		 */
		class NAPAPI Rect final
		{
			RTTI_ENABLE()
		public:
			/**
			 *	Default constructor
			 */
			constexpr Rect() = default;

			/**
			 * Utility constructor
			 * @param x the start x position of the rectangle
			 * @param y the start y position of the rectangle
			 * @param width the width of the rectangle
			 * @param height the height of the triangle
			 */
			constexpr Rect(float x, float y, float width, float height) :
				mMinPosition({ x, y }), mMaxPosition(mMinPosition + glm::vec2(width, height)) {};

			/**
			 * Utility min max constructor
			 * @param min: the min x, y position of the rectangle
			 * @param max: the max x, y position of the rectangle
			 */
			constexpr Rect(const glm::vec2& min, const glm::vec2& max) :
				mMinPosition(min), mMaxPosition(max) {}

			/**
			 * Utility min max move constructor
			 * @param min: the min x, y position of the rectangle
			 * @param max: the max x, y position of the rectangle
			 */
			constexpr Rect(glm::vec2&& min, glm::vec2&& max) :
				mMinPosition(std::move(min)), mMaxPosition(std::move(max)) {}

			// Equal-to operator overload
			bool operator==(const Rect& other) const;

			// Not-equal-to operator overload
			bool operator!=(const Rect& other) const;

			/**
			 *	@return the absolute width of the rectangle
			 */
			float getWidth() const;

			/**
			 *	@return the absolute height of the rectangle
			 */
			float getHeight() const;

			/**
			 *	@return if the rect has a width associated with it
			 */
			bool hasWidth() const;

			/**
			 *	@return if the rect has a height associated with it
			 */
			bool hasHeight() const;

			/**
			 *	@return if a point is inside the rectangle, this excludes the exact edge
			 */
			bool inside(const glm::vec2& point) const;

			/**
			 *	@return the min rectangle coordinates
			 */
			const glm::vec2& getMin() const					{ return mMinPosition; }

			/**
			 *	@return the max rectangle coordinates
			 */
			const glm::vec2& getMax() const					{ return mMaxPosition; }

			glm::vec2 mMinPosition = { 0.0f, 0.0f };		// min x,y position
			glm::vec2 mMaxPosition = { 0.0f, 0.0f };		// max x,y position
		};

		// Static rectangles
		static constexpr Rect topRightRect		{ {  0.0f,	 0.0f }, { 1.0f,  1.0f} };
		static constexpr Rect topLeftRect		{ { -1.0f,	 0.0f }, { 0.0f,  1.0f} };
		static constexpr Rect bottomLeftRect	{ { -1.0f,  -1.0f }, { 0.0f,  0.0f} };
		static constexpr Rect bottomRightRect	{ {  0.0f,  -1.0f }, { 1.0f,  0.0f} };
		static constexpr Rect centeredRect		{ { -0.5f,  -0.5f }, { 0.5f,  0.5f} };
	}
}
