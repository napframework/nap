#pragma once

#include "surfacedescriptor.h"
#include "utility/dllexport.h"

// External Includes

namespace nap
{
	NAPAPI void copyImageData(const uint8_t* source, unsigned int sourcePitch, ESurfaceChannels sourceChannels, uint8_t* target, unsigned int targetPitch, ESurfaceChannels targetChannels, int width, int height);
}