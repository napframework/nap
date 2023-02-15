#pragma once

// External includes
#include <component.h>
#include <componentptr.h>
#include <cameracomponent.h>
#include <transformcomponent.h>

#include <parameter.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametermat.h>
#include <parametercolor.h>

// Local includes
#include "light.h"


namespace nap
{
	// Forward declares
	class LightComponentInstance;

	/**
	 * Light Type Flag
	 */
	enum class ELightType : uint
	{
		Custom			= 0,
		Directional		= 1,
		Point			= 2,
		Spot			= 3
	};

	/**
	 * Light Equation Flag
	 */
	enum class ELightEquation : uint
	{
		Custom			= 0,
		Directional		= 1,
		Point			= 2
	};

	/**
	 * Light Globals
	 */
	namespace uniform
	{
		inline constexpr const char* lightStruct = "light";							// Default light UBO struct name

		namespace light
		{
			inline constexpr const char* color = "color";
			inline constexpr const char* intensity = "intensity";
			inline constexpr const char* origin = "origin";
			inline constexpr const char* direction = "direction";
			inline constexpr const char* attenuation = "attenuation";
			inline constexpr const char* angle = "angle";
			inline constexpr const char* falloff = "falloff";
			inline constexpr const char* flags = "flags";

			inline constexpr const char* lightViewProjection = "lightViewProjection";
			inline constexpr const char* lights = "lights";
			inline constexpr const char* count = "count";
		}
	}

	namespace sampler
	{
		namespace light
		{
			inline constexpr const char* shadowMaps = "shadowMaps";
		}
	}

	using LightFlags = uint;
	using LightUniformDataMap = std::unordered_map<std::string, Parameter*>;


	/**
	 * LightComponent
	 */
	class NAPAPI LightComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LightComponent, LightComponentInstance)
	public:
		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterRGBColorFloat> mColor;				///< Property: 'Color'
		ResourcePtr<ParameterFloat> mIntensity;					///< Property: 'Intensity'
		bool mEnableShadows = false;							///< Property: 'Enable Shadows'
	};


	/**
	 * LightComponentInstance	
	 */
	class NAPAPI LightComponentInstance : public ComponentInstance
	{
		friend class RenderAdvancedService;
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		LightComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		// Destructor
		~LightComponentInstance();

		/**
		 * Initialize LightComponentInstance based on the LightComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return whether this light component produces shadows
		 */
		virtual bool isShadowEnabled()										{ return mIsShadowEnabled; }

		/**
		 * @return the light transform
		 */
		const TransformComponentInstance& getTransform() const				{ return *mTransform; }

		/**
		 * @return the shadow camera if available, else nullptr
		 */
		virtual CameraComponentInstance* getShadowCamera()					{ assert(false); return nullptr; }

		/**
		 * @return the light equation
		 */
		virtual ELightEquation getLightEquation() const						{ assert(false); return ELightEquation::Custom; }

		/**
		 * @return the light type
		 */
		virtual ELightType getLightType() const								{ assert(false); return ELightType::Custom; }

		/**
		 * @return the position of the light in world space
		 */
		const glm::vec3 getLightPosition() const							{ return math::extractPosition(getTransform().getGlobalTransform()); }

		/**
		 * @return the direction of the light in world space
		 */
		const glm::vec3 getLightDirection() const							{ return -glm::normalize(getTransform().getGlobalTransform()[2]); }

	protected:
		/**
		 * Registers a light uniform member for updating the shader interface.
		 * TODO: Remove them too to facilitate hot-reloads?
		 */
		void registerLightUniformMember(const std::string& memberName, Parameter* parameter);

		/**
		 * @return the light flags
		 */
		const uint getLightFlags() const									{ return mLightFlags; }

		LightComponent* mResource						= nullptr;
		TransformComponentInstance* mTransform			= nullptr;
		bool mIsShadowEnabled							= false;

		uint mLightFlags								= 0U;

	private:
		Parameter* getLightUniform(const std::string& memberName);

		LightUniformDataMap mUniformDataMap;
	};
}
