#pragma once

// External includes
#include <component.h>
#include <componentptr.h>
#include <cameracomponent.h>
#include <transformcomponent.h>
#include <parameterentrycolor.h>
#include <renderfrustumcomponent.h>
#include <rendergnomoncomponent.h>
#include <entity.h>

namespace nap
{
	// Forward declares
	class LightComponentInstance;
	class RenderAdvancedService;

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
	 * Light Component default light uniforms
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

	/**
	 * Light Component default shadow samplers
	 */
	namespace sampler
	{
		namespace light
		{
			inline constexpr const char* shadowMaps = "shadowMaps";
			inline constexpr const char* cubeShadowMaps = "cubeShadowMaps";
		}
	}


	/**
	 * Base class of light components for NAP RenderAdvanced's light system.
	 *
	 * When present in the scene, the render advanced service can update light uniform data for material instances that are
	 * compatible with the light's shader interface. On initialization, each light component sets up its own registry of
	 * light uniform data and registers itself at the render advanced service. This way, the service is aware of the lights
	 * in the scene and creates the necessary resources for light information and shadow maps. NAP supports a limited
	 * number of lights per scene (`RenderAdvancedService::getMaximumLightCount`). The way in which these blend/interact
	 * depends on the implementation of the shader program. Increasing the maximum number of lights is trivial, however,
	 * with the current implementation it would take up more shader resource slots.
	 *
	 * Each light component has three default uniforms that are set by the RenderAdvanced service:
	 * - `origin`: `vec3` world position of the light.
	 * - `direction`: `vec3` direction of the light. Some lights may choose to ignore this however (e.g. point lights).
	 * - `flags`: an unsigned integer encoding information such as whether shadows are enabled, see `lightflags.h`.
	 *
	 * Other uniforms may be defined by derived light types. They must be in accordance with the data and shader interface
	 * in the `light.glslinc` file in the RenderAdvanced shader folder. New light types can be added here in the future,
	 * or user implementations can use the 'Custom' enum.
	 * 
	 * NAP comes with a default nap::BlinnPhongShader that is compatible with the light system. Hooking this up to a
	 * nap::Material allows for quick scene lighting setups. Material surface uniforms as defined by the shader interface
	 * must be set in data or at runtime. A description of these can be found in the documentation of the shader or its
	 * source file.
	 *
	 * The depth format of shadow maps can be configured in the `nap::RenderAdvancedServiceConfiguration`.
	 *
	 * Rendering with lights requires an additional call to the render advanced service. You can either use `pushLights` on
	 * the render components you wish to render or `renderShadows` with the `updateMaterials` argument set to `true` if you
	 * wish to use shadows too.
	 * l
	 * Update light uniforms of lit components when shadows are disabled.
	 * ~~~~~{.cpp}
	 *	mRenderAdvancedService->pushLights(components_to_render, error_state);
	 *	// mRenderService->renderObjects ...
	 * ~~~~~
	 *
	 * Re-render shadow map and update light uniforms.
	 * ~~~~~{.cpp}
	 *	if (mRenderService->beginHeadlessRecording())
	 *	{
	 *		mRenderAdvancedService->renderShadows(render_comps, true);
	 *		mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
	 */
	class NAPAPI LightComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LightComponent, LightComponentInstance)
	public:
		/**
		 * Common serializable light locator properties
		 */
		struct Locator
		{
			float mLineWidth  = 1.0f;	///< Property: 'LineWidth' The line width of the drawable locator objects (origin gnomon etc.)
			float mGnomonSize = 1.0f;	///< Property: 'GnomonSize' The origin gnomon unit size.
		};

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		bool mEnabled = true;									///< Property: 'Enabled' Whether the light is enabled
		bool mCastShadows = false;								///< Property: 'CastShadows' Enables shadows and creates shadow map resources for this light.
		RGBColorFloat mColor = {1.0f, 1.0f, 1.0f};				///< Property: 'Color' The light color
		float mIntensity = 1.0f;								///< Property: 'Intensity' The light intensity
		float mShadowStrength = 1.0f;							///< Property: 'ShadowStrength' The amount of light the shadow consumes.
		Locator mLocator;										///< Property: 'Locator' Locator settings
	};


	/**
	 * Base class of light component instances for NAP RenderAdvanced's light system.
	 *
	 * When present in the scene, the render advanced service can update light uniform data for material instances that are
	 * compatible with the light's shader interface. On initialization, each light component sets up its own registry of
	 * light uniform data and registers itself at the render advanced service. This way, the service is aware of the lights
	 * in the scene and creates the necessary resources for light information and shadow maps. NAP supports a limited
	 * number of lights per scene (`RenderAdvancedService::getMaximumLightCount`). The way in which these blend/interact
	 * depends on the implementation of the shader program. Increasing the maximum number of lights is trivial, however,
	 * with the current implementation it would take up more shader resource slots.
	 *
	 * Each light component has three default uniforms that are set by the RenderAdvanced service:
	 * - `origin`: `vec3` world position of the light.
	 * - `direction`: `vec3` direction of the light. Some lights may choose to ignore this however (e.g. point lights).
	 * - `flags`: an unsigned integer encoding information such as whether shadows are enabled, see `lightflags.h`.
	 *
	 * Other uniforms may be defined by derived light types. They must be in accordance with the data and shader interface
	 * in the `light.glslinc` file in the RenderAdvanced shader folder. New light types can be added here in the future,
	 * or user implementations can use the 'Custom' enum.
	 *
	 * NAP comes with a default nap::BlinnPhongShader that is compatible with the light system. Hooking this up to a
	 * nap::Material allows for quick scene lighting setups. Material surface uniforms as defined by the shader interface
	 * must be set in data or at runtime. A description of these can be found in the documentation of the shader or its
	 * source file.
	 *
	 * The depth format of shadow maps can be configured in the `nap::RenderAdvancedServiceConfiguration`.
	 *
	 * Rendering with lights requires an additional call to the render advanced service. You can either use `pushLights` on
	 * the render components you wish to render or `renderShadows` with the `updateMaterials` argument set to `true` if you
	 * wish to use shadows too.
	 *
	 * Update light uniforms of lit components when shadows are disabled.
	 * ~~~~~{.cpp}
	 *	mRenderAdvancedService->pushLights(components_to_render, error_state);
	 *	// mRenderService->renderObjects ...
	 * ~~~~~
	 *
	 * Re-render shadow map and update light uniforms.
	 * ~~~~~{.cpp}
	 *	if (mRenderService->beginHeadlessRecording())
	 *	{
	 *		mRenderAdvancedService->renderShadows(render_comps, true);
	 *		mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
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
		 * Unregisters itself from the advanced render service
		 */
		virtual void onDestroy() override;
		
		/**
		 * Activates the light
		 * @param enable if the light is activate or not
		 */
		virtual void enable(bool enable)									{ mIsEnabled = enable; }

		/**
		 * @return whether this light is active
		 */
		 bool isEnabled() const												{ return mIsEnabled; };

		 /**
		 * Returns whether this light component can cast shadows.
		 * Override if your light doesn't support shadows.
		 * @return whether this light component can cast shadows
		 */
		bool canCastShadows() const											{ return mSpawnedCamera != nullptr; }

		/**
		 * @return whether this light component currently casts shadows
		 */
		bool getCastShadows() const											{ return canCastShadows() && mIsShadowEnabled; }

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
		virtual float getIntensity() const									{ return mIntensity; }

		/**
		 * Set the light intensity
		 */
		void setIntensity(float intensity)									{ mIntensity = intensity; }

		/**
		 * @return the shadow strength
		 */
		virtual float getShadowStrength() const								{ return mShadowStrength; }

		/**
		 * Sets the shadow strength
		 */
		virtual void setShadowStrength(float strength)						{ mShadowStrength = strength; }

		/**
		 * @return the light color
		 */
		virtual const RGBColorFloat& getColor() const						{ return mColor; }

		/**
		 * Set the light color
		 */
		void setColor(const RGBColorFloat& color)							{ mColor = color; }

		/**
		 * @return the position of the light in world space
		 */
		const glm::vec3 getLightPosition() const							{ return math::extractPosition(getTransform().getGlobalTransform()); }

		/**
		 * @return the direction of the light in world space
		 */
		const glm::vec3 getLightDirection() const							{ return -glm::normalize(getTransform().getGlobalTransform()[2]); }

		/**
		 * @return the light transform
		 */
		const TransformComponentInstance& getTransform() const				{ assert(mTransform != nullptr); return *mTransform; }

		/**
		 * @return the light transform
		 */
		TransformComponentInstance& getTransform()							{ assert(mTransform != nullptr); return *mTransform; }

		/**
		 * @return if this light has a shadow camera
		 */
		bool hasCamera() const												{ return mSpawnedCamera != nullptr; }

		/**
		 * @return the shadow camera.
		 */
		CameraComponentInstance& getCamera() const							{ assert(mSpawnedCamera != nullptr); return mSpawnedCamera->getComponent<CameraComponentInstance>(); }

		/**
		 * @return the shadow camera.
		 */
		CameraComponentInstance& getCamera()								{ assert(mSpawnedCamera != nullptr); return mSpawnedCamera->getComponent<CameraComponentInstance>(); }

		/** 
		 * @return the origin gnomon.
		 */
		const RenderGnomonComponentInstance& getGnomon() const				{ assert(mSpawnedCamera != nullptr); return mSpawnedCamera->getComponent<RenderGnomonComponentInstance>(); }

		/**
		 * @return the origin gnomon.
		 */
		RenderGnomonComponentInstance& getGnomon()							{ assert(mSpawnedCamera != nullptr); return mSpawnedCamera->getComponent<RenderGnomonComponentInstance>(); }

		/**
		 * @return the shadow camera frustrum.
		 */
		const RenderFrustumComponentInstance* getFrustrum() const			{ assert(mSpawnedCamera != nullptr); return mSpawnedCamera->findComponent<RenderFrustumComponentInstance>(); }

		/**
		 * @return the shadow camera frustrum.
		 */
		RenderFrustumComponentInstance* getFrustrum()						{ assert(mSpawnedCamera != nullptr); return mSpawnedCamera->findComponent<RenderFrustumComponentInstance>(); }

		float mIntensity = 1.0f;												
		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };

	protected:
		/**
		 * Registers a light property as a uniform light member, which is automatically pushed by the render advanced service.
		 * Note that the property must be must be defined for this or derived classes using the RTTI_PROPERTY macro.
		 * @param memberName light property member name, must be registered using the RTTI_PROPERTY macro.
		 */
		void registerUniformLightProperty(const std::string& memberName);

		/**
		 * Spawns a light camera entity. The lifetime of that entity is managed by this component.
		 * The light camera entity is used to calculate the shadow maps
		 * This entity is spawned into a dedicated light scene, independent from the regular user scene.
		 * Call this function on init of your derived light component, only once!
		 * @param entity the entity resource to spawn
		 * @param error contains the error if spawning fails
		 * @return the spawned light entity instance.
		 */
		SpawnedEntityInstance spawnShadowCamera(const nap::Entity& entity, nap::utility::ErrorState& error);

		LightComponent* mResource						= nullptr;
		TransformComponentInstance* mTransform			= nullptr;
		RenderAdvancedService* mService					= nullptr;
		SpawnedEntityInstance mSpawnedCamera;

		bool mIsEnabled									= true;
		bool mIsShadowEnabled							= false;
		float mShadowStrength							= 1.0f;
		uint mShadowMapSize								= 512;

	private:
		std::vector<nap::rtti::Property> mUniformList;
	};
}
