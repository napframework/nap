#pragma once

// Spatial includes
#include <Spatial/Utility/ParameterTypes.h>

// Nap includes
#include <component.h>
#include <parametercolor.h>
#include <parametervec.h>


// Glm includes
#include <glm/glm.hpp>




namespace nap
{
	// Forward declarations
    class ParameterComponentInstance;

	namespace spatial
	{
		// Forward declarations
		class IdentificationComponentInstance;

		/**
		 * The identification component manages some meta data used to identify entities.
		 * It's primarily used within the monitor, to show the display name and to color
		 * things using the display color of an entity.
		 */
		class NAPAPI IdentificationComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(IdentificationComponent, IdentificationComponentInstance)
			
		public:
			IdentificationComponent() : Component() { }

		private:
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override final;
		};
		
		/**
		 * Instance of a @IdentificationComponent
		 */
		class NAPAPI IdentificationComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			IdentificationComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
			
			// Initialize the component
			virtual bool init(utility::ErrorState& errorState) override final;
			
			/**
			 * Returns the display color associated with the entity as rgba value.
			 **/
			glm::vec4 getDisplayColorAsRGBA(const float opacity = 1.f) const;
		
			/**
			 * Sets the hue of the display color associated with the entity.
			 * @hue: The hue of the display color. The value is in degrees of the color wheel. The value wraps around from 0 to 360.
			 **/
			void setDisplayColorFromHue(const float hue);
		
			/**
			 * Sets the rgb color associated with the entity.
			 * @rgb: The red, green and blue components (0 to 1).
			 **/
			void setDisplayColorFromRgb(const glm::vec3 & rgb);
			
			/**
			 * Returns the display name associated with the entity. This could be the display
			 * name (if set), or the name of the entity as stored inside the dictionary
			 * component otherwise.
			 **/
			const char * getDisplayName() const;
			
		private:
			enum ColorMode
			{
				kColorMode_Normal,
				kColorMode_Blink,
				kColorMode_FastBlink,
				kColorMode_ColorCycle
			};
			
			ColorMode mColorMode = kColorMode_Normal;
			
			ParameterComponentInstance * mParameterComponent = nullptr;
			
            nap::ParameterString* mDisplayName = nullptr;
            nap::ParameterFloat* mDisplayColorHue = nullptr;
            nap::ParameterVec3* mDisplayColorRgb = nullptr;
		};
	}
}
