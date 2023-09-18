#pragma once

// External includes
#include <component.h>
#include <componentptr.h>
#include <cameracomponent.h>
#include <transformcomponent.h>

#include <parameterentrynumeric.h>
#include <parameterentrycolor.h>

namespace nap
{
	// Forward declares
	class LightComponentInstance;

	/**
	 * Light Type Flag
	 */
	enum class ELightType : uint8
	{
		Custom			= 0,
		Directional		= 1,
		Point			= 2,
		Spot			= 3
	};

	/**
	 * Shadow Map Type
	 */
	enum class EShadowMapType : uint8
	{
		Quad = 0,
		Cube = 1
	};

	/**
	 * Light Globals
	 */
	namespace uniform
	{
		inline constexpr const char* lightStruct = "light";						// Default light UBO struct name
		inline constexpr const char* shadowStruct = "shadow";					// Default shadow UBO struct name

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
			inline constexpr const char* lights = "lights";
			inline constexpr const char* count = "count";
			inline constexpr const uint defaultMemberCount = 8;
		}

		namespace shadow
		{
			inline constexpr const char* lightViewProjectionMatrix = "lightViewProjectionMatrix";
            inline constexpr const char* nearFar = "nearFar";
            inline constexpr const char* strength = "strength";
            inline constexpr const char* flags = "flags";
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

	using LightUniformDataMap = std::unordered_map<std::string, Parameter*>;
	using LightParameterList = std::vector<std::unique_ptr<Parameter>>;


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

		bool mEnabled = true;									///< Property: 'Enabled'
		ResourcePtr<ParameterEntryRGBColorFloat> mColor;		///< Property: 'Color'
		ResourcePtr<ParameterEntryFloat> mIntensity;			///< Property: 'Intensity'

		float mShadowStrength = 1.0f;							///< Property: 'ShadowStrength'
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
		 * Enables the light
		 */
		virtual void enable(bool enable)									{ mIsEnabled = enable; }

		/**
		 * @return whether this light is active
		 */
		virtual bool isEnabled() const										{ return mIsEnabled; };

		/**
		 * Returns whether this light component supports shadows. Override this call on a derived
		 * light component to enable shadow support.
		 * @return whether this light component supports shadows
		 */
		virtual bool isShadowSupported() const								{ return false; }

		/**
		 * @return whether this light component currently produces shadows
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
		 * @return the light transform
		 */
		TransformComponentInstance& getTransform()							{ return *mTransform; }

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
		 * @return the light intensity
		 */
		virtual float getIntensity() const { return 1.0f; }// mResource->mIntensity.getValue(); }

		/**
		 * @return the shadow strength
		 */
		virtual float getShadowStrength() const								{ return mShadowStrength; }

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
		 * @param memberName the uniform member name of the light variable
		 * @param parameter pointer to the parameter to register. If nullptr, creates and registers a default parameter at runtime
		 */
		template <typename ParameterType, typename DataType>
		void registerLightUniformMember(const std::string& memberName, Parameter* parameter, const DataType& value);

		LightComponent* mResource						= nullptr;
		TransformComponentInstance* mTransform			= nullptr;

		bool mIsEnabled									= true;
		bool mIsShadowEnabled							= false;
		float mShadowStrength							= 1.0f;
		uint mShadowMapSize								= 512U;

		LightParameterList mParameterList;				// List of parameters that are owned by light component instead of the the resource manager

	private:
		Parameter* getLightUniform(const std::string& memberName);

		LightUniformDataMap mUniformDataMap;			// Maps uniform names to parameters
		bool mIsRegistered = false;
	};
}


template <typename ParameterType, typename DataType>
void nap::LightComponentInstance::registerLightUniformMember(const std::string& memberName, Parameter* parameter, const DataType& value)
{
	auto* param = parameter;
	if (param == nullptr)
	{
		param = mParameterList.emplace_back(std::make_unique<ParameterType>()).get();
		auto* typed_param = static_cast<ParameterType*>(param);
		typed_param->mName = memberName;
		typed_param->mValue = value;

		utility::ErrorState error_state;
		if (!param->init(error_state))
			assert(false);
	}
	assert(param != nullptr);

	const auto it = mUniformDataMap.insert({ memberName, param });
	assert(it.second);
}
