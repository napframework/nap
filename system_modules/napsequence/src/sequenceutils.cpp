#include "sequenceutils.h"

namespace nap
{
namespace utility
{
    namespace colorspace
    {
        OKLabColor rgbToOKLab(const RGBAColorFloat& c)
        {
            float l = 0.4122214708f * c.getRed() + 0.5363325363f * c.getGreen() + 0.0514459929f * c.getBlue();
            float m = 0.2119034982f * c.getRed() + 0.6806995451f * c.getGreen() + 0.1073969566f * c.getBlue();
            float s = 0.0883024619f * c.getRed() + 0.2817188376f * c.getGreen() + 0.6299787005f * c.getBlue();

            float l_ = cbrtf(l);
            float m_ = cbrtf(m);
            float s_ = cbrtf(s);

            return
            {
                0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
                1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
                0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
                c.getAlpha()
            };
        }


        RGBAColorFloat OKLabToRGB(const OKLabColor& c)
        {
            float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
            float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
            float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

            float l = l_*l_*l_;
            float m = m_*m_*m_;
            float s = s_*s_*s_;

            glm::vec3 result
            {
                +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
                -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
                -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
            };
            result = glm::clamp(result, 0.0f, 1.0f);

            return { result.r, result.g, result.b, c.alpha };
        }


        RGBAColorFloat blendColors(const RGBAColorFloat& colorOne, const RGBAColorFloat& colorTwo, float blend, EType type)
        {
            switch(type)
            {
                case EType::RGB:
                {
                    return RGBAColorFloat
                            {
                                    glm::mix(colorOne.getRed(), colorTwo.getRed(), blend),
                                    glm::mix(colorOne.getGreen(), colorTwo.getGreen(), blend),
                                    glm::mix(colorOne.getBlue(), colorTwo.getBlue(), blend),
                                    glm::mix(colorOne.getAlpha(), colorTwo.getAlpha(), blend)
                            };
                }
                case EType::OKLab:
                {
                    OKLabColor oklabColorOne = rgbToOKLab(colorOne);
                    OKLabColor oklabColorTwo = rgbToOKLab(colorTwo);
                    OKLabColor blendedColor
                    {
                        glm::mix(oklabColorOne.L, oklabColorTwo.L, blend),
                        glm::mix(oklabColorOne.a, oklabColorTwo.a, blend),
                        glm::mix(oklabColorOne.b, oklabColorTwo.b, blend),
                        glm::mix(oklabColorOne.alpha, oklabColorTwo.alpha, blend)
                    };
                    return OKLabToRGB(blendedColor);
                }

                default:
                    assert(false); // unknown color space type
                    break;
            }

            return {0,0,0,0};
        }
    } // namespace colorspace
} // namespace utility
} // namespace nap
