#pragma once

// Internal includes
#include "sequencetracksegmentcolor.h"

// External Includes
#include <glm/glm.hpp>
#include <color.h>

namespace nap
{
namespace utility
{
    namespace colorspace
    {
        /**
         * Struct representing a color in the OKLab color space
         */
        struct NAPAPI OKLabColor
        {
            float L;
            float a;
            float b;
            float alpha;
        };

        /**
         * Converts a RGBA color to the OKLab color space
         * @param c the RGBA color to convert
         * @return the converted color in the OKLab color space
         */
        RGBAColorFloat NAPAPI rgbToOKLab(const RGBColorFloat& c);

        /**
         * Converts a color in the OKLab color space to RGB
         * @param c the color in the OKLab color space to convert
         * @return the converted color in the RGB color space
         */
        RGBAColorFloat NAPAPI OKLabToRGB(const OKLabColor& c);

        /**
         * Blends two colors in the specified color space
         * @param colorOne first color
         * @param colorTwo second color
         * @param blend the amount to blend the two colors, 0.0 is colorOne, 1.0 is colorTwo
         * @param type the color space to blend in
         * @return returns the blended color in RGBA color space
         */
        RGBAColorFloat NAPAPI blendColors(const RGBAColorFloat& colorOne, const RGBAColorFloat& colorTwo, float blend, SequenceTrackSegmentColor::EColorSpace type);
    }
}
}