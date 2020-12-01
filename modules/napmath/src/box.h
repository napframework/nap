/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <rtti/rtti.h>
#include <glm/glm.hpp>

namespace nap
{
	namespace math
	{
		/**
		 * Simple 3D box.
		 * when using the default constructor the box dimensions are 1 unit in every axis 
		 * normalized around center (0,0,0).
		 */
		class NAPAPI Box final
		{
			RTTI_ENABLE()
		public:
			Box() = default;

			/**
			 * Creates a box around the origin using provided dimensions
			 * @param width the width of the box
			 * @param height the height of the box
			 * @param depth the depth of the box
			 */
			Box(float width, float height, float depth);

			/**
			 * Creates a box at the given position using the provided dimensions
			 * @param width the width of the box
			 * @param height the height of the box
			 * @param depth the depth of the box
			 * @param position of the box
			 */
			Box(float width, float height, float depth, const glm::vec3& position);

			/**
			 * Creates a box using the specified dimensions in every axis
			 * @param xCoordinates the min / max horizontal coordinates of the box
			 * @param yCoordinates the min / max vertical coordinates of the box
			 * @param zCoordinates the min / max depth coordinates of the box
			 */
			Box(const glm::vec2& xCoordinates, const glm::vec2& yCoordinates, const glm::vec2& zCoordinates);

			/**
			 * Creates a box using the 2 specified min / max coordinates
			 * @param min the min point coordinates
			 * @param max the max point coordinates
			 */
			Box(const glm::vec3& min, const glm::vec3& max);

			/**
			 * @return the absolute width of the box
			 */
			float getWidth() const;

			/**
			 * @return the absolute height of the box
			 */
			float getHeight() const;

			/**
			 * @return the absolute depth (length) of the box
			 */
			float getDepth() const;

			/**
			 * @return if the point is inside the box, this excludes the edge
			 */
			bool inside(const glm::vec3& point) const;

			/**
			 *	@return the min box coordinates
			 */
			const glm::vec3& getMin() const									{ return mMinCoordinates; }

			/**
			 *	@return the max box coordinates
			 */
			const glm::vec3& getMax() const									{ return mMaxCoordinates; }

			/**
			 * @return the absolute bounding box dimension (width, height, depth)
			 */
			glm::vec3 getDimensions() const;

			/**
			 * @return center coordinates of the box
			 */
			glm::vec3 getCenter() const;

			glm::vec3 mMinCoordinates = { -0.5f, -0.5f, -0.5f };			///< Box min coordinates
			glm::vec3 mMaxCoordinates = { 0.5f, 0.5f, 0.5f };				///< Box max coordinates
		};
	}
}