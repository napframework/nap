/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"

// External Includes
#include "utility/dllexport.h"

namespace nap
{
	/**
	 * Copies pixel data from source to target, using the given parameters.
	 * Note that the target pitch needs to be less or equal to source pitch.
	 * @param source pointer to source data
	 * @param sourcePitch source number of bytes in a row
	 * @param sourceChannels number of channels per pixel
	 * @param target pointer to the target memory
	 * @param targetPitch target number of bytes per row
	 * @param targetChannels number of channels per pixel
	 * @param width width of the image in pixels
	 * @param height height of the image in pixels
	 */
	NAPAPI void copyImageData(const uint8_t* source, unsigned int sourcePitch, ESurfaceChannels sourceChannels, uint8_t* target, unsigned int targetPitch, ESurfaceChannels targetChannels, int width, int height);
}