#include "IdentificationComponent.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Utility/ParameterTypes.h>

// Nap includes
#include <parametercolor.h>
#include <entity.h>
#include <glm/gtx/color_space.hpp>

// Std includes
#include <chrono>


// RTTI
RTTI_BEGIN_CLASS(nap::spatial::IdentificationComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::IdentificationComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION("setDisplayColorFromHue", &nap::spatial::IdentificationComponentInstance::setDisplayColorFromHue)
    RTTI_FUNCTION("setDisplayColorFromRgb", &nap::spatial::IdentificationComponentInstance::setDisplayColorFromRgb)
RTTI_END_CLASS

namespace nap
{
	namespace spatial
	{
		static const float kInitialColorValue = 1e-3f; // we use a magic number to detect the initial parameter-change signal that occurs at startup and ignore it if it's the initial value
		
		static const auto startupTime = std::chrono::system_clock::now();
		
		void IdentificationComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{
			components.push_back(RTTI_OF(ParameterComponent));
		}

		//

		bool IdentificationComponentInstance::init(utility::ErrorState& errorState)
		{
			// fetch components
			
			mParameterComponent = &getEntityInstance()->getComponent<ParameterComponentInstance>();
			
			// check if the required components were found
			
			if (!errorState.check(mParameterComponent != nullptr, "Missing required component: ParameterComponentInstance"))
				return false;

            // create parameters
			
            mDisplayName = &mParameterComponent->addParameterSimple<std::string>("displayName", "");
			
            mDisplayColorHue = &mParameterComponent->addParameterNumeric<float>("hue", kInitialColorValue, 0.f, 360.f);
			mDisplayColorHue->valueChanged.connect([&](float x)
				{
					setDisplayColorFromHue(x);
				});
			
            mDisplayColorRgb = &mParameterComponent->addParameterVec<glm::vec3>("color", glm::vec3(kInitialColorValue), 0.f, 1.f);
            mDisplayColorRgb->valueChanged.connect([&](glm::vec3 x)
            	{
            		setDisplayColorFromRgb(x);
				});
			
			return true;
		}
		
		glm::vec4 IdentificationComponentInstance::getDisplayColorAsRGBA(const float opacity) const
		{
			if (mColorMode == kColorMode_Normal)
			{
				return glm::vec4(mDisplayColorRgb->mValue, opacity);
			}
			else if (mColorMode == kColorMode_ColorCycle)
			{
				const auto now = std::chrono::system_clock::now();
				const auto elapsed = now - startupTime;
				const auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
				
				const float hue = fmodf(elapsedMilliseconds/1000.f*360.f/10.f, 360.f);
				
				const glm::vec3 rgb = glm::rgbColor(glm::vec3(hue, 1.f, 1.f));
				const glm::vec4 color = glm::vec4(rgb, opacity);
				
				return color;
			}
			else if (mColorMode == kColorMode_Blink)
			{
				const auto now = std::chrono::system_clock::now();
				const auto elapsed = now - startupTime;
				const auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
				
				if ((elapsedMilliseconds % 500) < 250)
					return glm::vec4(0.f);
				else
				{
					const glm::vec3 rgb = glm::rgbColor(glm::vec3(mDisplayColorHue->mValue, 1.f, 1.f));
					return glm::vec4(rgb, opacity);
				}
			}
			else if (mColorMode == kColorMode_FastBlink)
			{
				const auto now = std::chrono::system_clock::now();
				const auto elapsed = now - startupTime;
				const auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
				
				if ((elapsedMilliseconds % 250) < 125)
					return glm::vec4(0.f);
				else
				{
					const glm::vec3 rgb = glm::rgbColor(glm::vec3(mDisplayColorHue->mValue, 1.f, 1.f));
					return glm::vec4(rgb, opacity);
				}
			}
			else
			{
				assert(false);
				return glm::vec4(1.f);
			}
		}
		
		void IdentificationComponentInstance::setDisplayColorFromHue(const float hue)
		{
			if (hue == kInitialColorValue)
			{
				// this is just the startup color. ignore it to avoid an incorrect hue from being set
				return;
			}

			mColorMode = kColorMode_Normal;
			
			const glm::vec3 rgb = glm::rgbColor(glm::vec3(hue, 1.f, 1.f));
			
			mDisplayColorRgb->mValue = rgb;
		}

		void IdentificationComponentInstance::setDisplayColorFromRgb(const glm::vec3 & rgb)
		{
			if (rgb == glm::vec3(kInitialColorValue))
			{
				// this is just the startup color. ignore it to avoid an incorrect hue from being set
				return;
			}
			
			mDisplayColorRgb->setValue(rgb);
			
			// check for easter egg mode
			
			if (rgb.r == .303f)
			{
				if (rgb.b == .1f)
					mColorMode = kColorMode_Blink;
				else if (rgb.b == .2f)
					mColorMode = kColorMode_FastBlink;
				else
					mColorMode = kColorMode_ColorCycle;
			}
			else
			{
				mColorMode = kColorMode_Normal;
			}
		}
		
		const char * IdentificationComponentInstance::getDisplayName() const
		{
			const char * result = mDisplayName->mValue.c_str();
			
			if (result[0] == 0)
			{
				result = mParameterComponent->getGroupName().c_str();
			}
			
			return result;
		}
    }
}
