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
	 * Shadow Map Type
	 */
	enum class EShadowMapType : uint
	{
		Quad = 0,
		Cube = 1
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

			inline constexpr const char* viewProjectionMatrix = "viewProjectionMatrix";
			inline constexpr const char* nearFar = "nearFar";
			inline constexpr const char* lights = "lights";
			inline constexpr const char* count = "count";
		}
	}

	namespace sampler
	{
		namespace light
		{
			inline constexpr const char* shadowMaps = "shadowMaps";
			inline constexpr const char* cubeShadowMaps = "cubeShadowMaps";
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
		uint mShadowSampleCount = 4U;							///< Property: 'ShadowSampleCount'
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

		/**
		 * Derived lights destructors must remove themselves by calling
		 * LightComponentInstance::removeLightComponent();
		 */
		virtual ~LightComponentInstance() { };

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
		virtual bool isShadowEnabled() const								{ return mIsShadowEnabled; }

		/**
		 * @return whether this light was registered with the render advanced service
		 */
		bool isRegistered() const											{ return mIsRegistered; }

		/**
		 * @return the light transform
		 */
		const TransformComponentInstance& getTransform() const				{ return *mTransform; }

		/**
		 * @return the shadow camera if available, else nullptr
		 */
		virtual CameraComponentInstance* getShadowCamera() = 0;

		/**
		 * @return the light type
		 */
		virtual ELightType getLightType() const = 0;

		/**
		 * @return the shadow map type
		 */
		virtual EShadowMapType getShadowMapType() const = 0;

		/**
		 * @return the shadow map resolution
		 */
		virtual uint getShadowMapSize() const								{ return mShadowMapSize; }

		/**
		 * @return the shadow sample count
		 */
		virtual uint getShadowSampleCount() const							{ return mShadowSampleCount; }

		/**
		 * @return the light intensity
		 */
		virtual float getIntensity() const									{ return mResource->mIntensity->mValue; }

		/**
		 * @return the light color
		 */
		virtual const RGBColorFloat& getColor() const						{ return mResource->mColor->getValue(); }

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
		 * Removes the current light from the render service.
		 */
		void removeLightComponent();

		/**
		 * Registers a light uniform member for updating the shader interface.
		 */
		void registerLightUniformMember(const std::string& memberName, Parameter* parameter);

		LightComponent* mResource						= nullptr;
		TransformComponentInstance* mTransform			= nullptr;
		bool mIsShadowEnabled							= false;
		bool mIsRegistered								= false;
		uint mShadowSampleCount							= 4U;
		uint mShadowMapSize								= 512U;

	private:
		Parameter* getLightUniform(const std::string& memberName);

		LightUniformDataMap mUniformDataMap;
	};
}
