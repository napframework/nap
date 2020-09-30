// Local Includes
#include "waveform.h"
#include "mathutils.h"

// External Includes
#include <glm/glm.hpp>
#include <cmath>

RTTI_BEGIN_ENUM(nap::math::EWaveform)
	RTTI_ENUM_VALUE(nap::math::EWaveform::SINE,		"Sine"),
	RTTI_ENUM_VALUE(nap::math::EWaveform::SQUARE,	"Square"),
	RTTI_ENUM_VALUE(nap::math::EWaveform::SAW,		"Saw"),
	RTTI_ENUM_VALUE(nap::math::EWaveform::TRIANGLE,	"Triangle")
RTTI_END_ENUM

namespace nap
{
	namespace math
	{
		static float sineWave(float time, float frequency)
		{
			return (sin(time * frequency * M_PI * 2.0f) / 2.0f) + 0.5f;
		}

		
		static float squareWave(float time, float frequency)
		{
			return ((sign<float>(sin(time * frequency * M_PI * 2.0f)) / 2.0f) + 0.5f);
		}


		static float sawWave(float time, float frequency)
		{
			return fmod(fabs(time), (1.0f / frequency)) * (1.0f / (1.0f / frequency));
		}


		float triangleWave(float time, float frequency)
		{
			return fabs(fmod((fabs(time) *  frequency * 2.0f), 2.0f) - 1.0f);
		}


		float waveform(nap::math::EWaveform  type, float time, float frequency)
		{
			switch(type)
			{
			case EWaveform::SINE:
				return sineWave(time, frequency);
			case EWaveform::SAW:
				return sawWave(time, frequency);
			case EWaveform::SQUARE:
				return squareWave(time, frequency);
			case EWaveform::TRIANGLE:
				return triangleWave(time, frequency);
			default:
				assert(false);
			}
			return 0.0f;
		}
	}
}
