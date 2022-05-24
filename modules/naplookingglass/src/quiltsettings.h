/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <glm/vec2.hpp>

namespace nap
{
	/**
	 * A struct describing the layout of a quilt.
	 * For an explanation of quilts, see: https://docs.lookingglassfactory.com/keyconcepts/quilts
	 */
	struct NAPAPI QuiltSettings
	{
		uint mWidth = 4096U;
		uint mHeight = 4096U;
		uint mColumns = 5U;
		uint mRows = 9U;

		/**
		 * @return the number of views in the quilt.
		 */
		uint getViewCount() const { return mRows * mColumns; }

		/**
		 * @return the aspect ratio of the quilt texture.
		 */
		float getAspect() const { return mWidth / static_cast<float>(mHeight); }

		/**
		 * @return the size of a single view.
		 */
		glm::uvec2 getViewSize() const { return { mWidth / mColumns, mHeight / mRows }; };

		/**
		 * @return the aspect ratio of a single view.
		 */
		float getViewAspect() const { return (mWidth / mColumns) / static_cast<float>(mHeight / mRows); };
	};
}
