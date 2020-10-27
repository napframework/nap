/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "copyimagedata.h"

namespace nap
{
	void copyImageData(const uint8_t* source, unsigned int sourcePitch, ESurfaceChannels sourceChannels, uint8_t* target, unsigned int targetPitch, ESurfaceChannels targetChannels, int width, int height)
	{
		assert(targetPitch <= sourcePitch);

		// If the dest & source pitches are the same, we can do a straight memcpy (most common/efficient case)
		if (targetPitch == sourcePitch)
		{
			memcpy(target, source, sourcePitch * height);
		}
		else if (sourceChannels == targetChannels)
		{
			// If the pitch of the source & destination buffers are different, we need to copy the image data line by line (happens for weirdly-sized images)
			const uint8_t* source_line = source;
			uint8_t* target_line = target;

			for (int y = 0; y < height; ++y)
			{
				memcpy(target_line, source_line, targetPitch);
				source_line += sourcePitch;
				target_line += targetPitch;
			}
		}
		else
		{
			// If the pitch of the source & destination buffers are different, we need to copy the image data line by line (happens for weirdly-sized images)
			const uint8_t* source_line = source;
			uint8_t* target_line = target;

			// Get the amount of bytes every pixel occupies
			int source_stride = sourcePitch / width;
			int target_stride = targetPitch / width;

			for (int y = 0; y < height; ++y)
			{
				const uint8_t* source_loc = source_line;
				uint8_t* target_loc = target_line;
				for (int x = 0; x < width; ++x)
				{
					memcpy(target_loc, source_loc, target_stride);
					target_loc += target_stride;
					source_loc += source_stride;
				}

				source_line += sourcePitch;
				target_line += targetPitch;
			}
		}		
	}
}
