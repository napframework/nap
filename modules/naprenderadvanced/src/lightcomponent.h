#pragma once

// External includes
#include <component.h>
#include <componentptr.h>
#include <cameracomponent.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <transformcomponent.h>

// Local includes
#include "light.h"

namespace nap
{
	// Forward declares
	class LightComponentInstance;
	class DirectionalLightComponentInstance;

	/**
	 * Light Globals
	 */
	namespace uniform
	{
		inline constexpr const char* lightStruct = "light";							// Default light UBO struct name

		namespace light
		{
			namespace directional
			{
				inline constexpr const char* color = "color";
				inline constexpr const char* intensity = "intensity";
				inline constexpr const char* direction = "direction";
			}

			namespace point
			{
				inline constexpr const char* origin = "origin";
				inline constexpr const char* color = "color";
				inline constexpr const char* intensity = "intensity";
			}

			namespace spot
			{

			}

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

		bool mEnableShadows = false;
	};


	/**
	 * LightComponentInstance	
	 */
	class NAPAPI LightComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		LightComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

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
		 * update LightComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return whether this light component produces shadows
		 */
		virtual bool isShadowEnabled() { return mIsShadowEnabled; }

		/**
		 * @return the shadow camera if available, else nullptr
		 */
		virtual CameraComponentInstance* getShadowCamera() { return nullptr; }

		/**
		 * @return the light transform
		 */
		const TransformComponentInstance& getTransform() const { return *mTransform; }

	protected:
		LightComponent* mResource						= nullptr;
		TransformComponentInstance* mTransform			= nullptr;

		bool mIsShadowEnabled							= false;
	};


	/**
	 *	DirectionalLightComponent
	 */
	class NAPAPI DirectionalLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(DirectionalLightComponent, DirectionalLightComponentInstance)
	public:
		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };			///< Property: 'Color'
		float mIntensity = 1.0f;								///< Property: 'Intensity'

		ComponentPtr<OrthoCameraComponent> mShadowCamera;		///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
	};


	/**
	 * DirectionalLightComponentInstance
	 */
	class NAPAPI DirectionalLightComponentInstance : public LightComponentInstance
	{
		RTTI_ENABLE(LightComponentInstance)
	public:
		DirectionalLightComponentInstance(EntityInstance& entity, Component& resource) :
			LightComponentInstance(entity, resource) { }

		/**
		 * Initialize LightComponentInstance based on the LightComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update LightComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the shadow camera if available, else nullptr
		 */
		virtual CameraComponentInstance* getShadowCamera()		{ return (mShadowCamera != nullptr) ? &(*mShadowCamera) : nullptr; }

		/**
		 * @return the position of the light in world space
		 */
		const glm::vec3 getLightPosition() const				{ return math::extractPosition(getTransform().getGlobalTransform()); }

		/**
		 * @return the direction of the light in world space
		 */
		const glm::vec3 getLightDirection() const				{ return -glm::normalize(getTransform().getGlobalTransform()[2]); }

		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };
		float mIntensity = 1.0f;

	private:
		// Shadow camera
		ComponentInstancePtr<OrthoCameraComponent> mShadowCamera = { this, &DirectionalLightComponent::mShadowCamera };

		// Shadow map rendering
		glm::mat4 mLightView;
		glm::mat4 mLightViewProjection;
	};
}
