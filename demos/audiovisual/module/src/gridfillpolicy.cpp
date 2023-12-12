/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gridfillpolicy.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::GridFillPolicy)
	RTTI_PROPERTY("Size", &nap::GridFillPolicy::mSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rows", &nap::GridFillPolicy::mRows, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Columns", &nap::GridFillPolicy::mColumns, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool GridFillPolicy::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mRows * mColumns > 0, "Rows x Columns must be at least 1"))
			return false;

		return true;
	}


	void GridFillPolicy::fill(uint numElements, glm::vec4* data) const
	{
		// Get incremental stepping values
		float inc_row_v = mSize.y / static_cast<float>(mRows);
		float inc_col_v = mSize.x / static_cast<float>(mColumns);

		float inc_row_uv = 1.0f / static_cast<float>(mRows);
		float inc_col_uv = 1.0f / static_cast<float>(mColumns);

		// Create buffers for all attributes
		uint row_vert_count = mRows + 1;
		uint col_vert_count = mColumns + 1;
		uint vert_count = row_vert_count * col_vert_count;

		if (vert_count > numElements)
		{
			nap::Logger::info("%s: The number of elements must be at least %d, it is currently %d", mID.c_str(), vert_count, numElements);
			return;
		}

		std::vector<glm::vec4> vertices(vert_count, { 0.0f,0.0f,0.0f,1.0f });
		std::vector<glm::vec3> uvs(vert_count, { 0.0f,0.0f,0.0f });

		uint idx = 0;
		float min_x = 0.0f;
		float min_y = 0.0f;

		// Fill vertex uv and position vertex buffers
		glm::vec4* ptr = data;
		for (uint row = 0; row < row_vert_count; row++)
		{
			// Calculate y values
			float ve_y = min_y + (row * inc_row_v);
			//float uv_y = row * inc_row_uv;

			for (int col = 0; col < col_vert_count; col++)
			{
				(*ptr).x = min_x + (col * inc_col_v);
				(*ptr).y = ve_y;

				//auto& uv = uvs[idx];
				//uv.x = col * inc_col_uv;
				//uv.y = uv_y;

				++ptr;
			}
		}
	}
}
