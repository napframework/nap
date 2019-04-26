#pragma once

// Local Includes
#include "updatematerialcomponent.h"
#include "selectvideocomponent.h"

// External Includes
#include <component.h>
#include <componentptr.h>
#include <texture2d.h>
#include <nap/resourceptr.h>

namespace nap
{
	enum class LightingModes { Off, Sun, Video, Static, Party, Sound };

	class LightingModeComponentInstance;

	/**
	 * LightingModeComponent
	 */
	class NAPAPI LightingModeComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LightingModeComponent, LightingModeComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<UpdateMaterialComponent>	mUpdateMaterialComponent;		///< Property: Pointer to the update material component
		ComponentPtr<SelectVideoComponent>		mSelectVideoComponent;			///< Property: Pointer to the select video component
		ResourcePtr<Texture2D>					mOffColorTexture = nullptr;		///< Property: Pointer to the texture for Lighting Mode: Off
		ResourcePtr<Texture2D>					mSunColorTexture = nullptr;		///< Property: Pointer to the texture for Lighting Mode: Sun
		ResourcePtr<Texture2D>					mVideoColorTexture = nullptr;	///< Property: Pointer to the texture for Lighting Mode: Video
		ResourcePtr<Texture2D>					mStaticColorTexture = nullptr;	///< Property: Pointer to the texture for Lighting Mode: Static
		ResourcePtr<Texture2D>					mPartyColorTexture = nullptr;	///< Property: Pointer to the texture for Lighting Mode: Party
		ResourcePtr<Texture2D>					mSoundColorTexture = nullptr;	///< Property: Pointer to the texture for Lighting Mode: Sound
	};


	/**
	 * LightingModeComponentInstance	
	 */
	class NAPAPI LightingModeComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LightingModeComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		 * Initialize LightingModeComponentInstance based on the LightingModeComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightingModeComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update LightingModeComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;
		
		// Pointers to the run time Component Instances, set during de-serialization
		ComponentInstancePtr<UpdateMaterialComponent> mUpdateMaterialComponent = { this, &LightingModeComponent::mUpdateMaterialComponent };
		ComponentInstancePtr<SelectVideoComponent> mSelectVideoComponent = { this, &LightingModeComponent::mSelectVideoComponent };

		// Properties for storing the lighting mode state
		const char*		mLightingModes[6] = { "Off", "Sun", "Video", "Static", "Party", "Sound" };
		int				mLightingModeInt = 0;
		LightingModes	mLightingModeEnum = LightingModes::Off;
		LightingModes	mOldLightingModeEnum;

		// Properties for controlling the lighting mode transition
		bool			mLightingModeTransitionActive = false;
		float			mLightingModeTransitionVelocity = 0.0f;

		bool isLightingModeSelected(LightingModes lightingMode);
		bool isLightingModeRendered(LightingModes lightingMode);

		// Store references to the lighting mode textures
		nap::Texture2D* mOffColorTexture = nullptr;
		nap::Texture2D* mSunColorTexture = nullptr;
		nap::Texture2D* mVideoColorTexture = nullptr;
		nap::Texture2D* mStaticColorTexture = nullptr;
		nap::Texture2D* mPartyColorTexture = nullptr;
		nap::Texture2D* mSoundColorTexture = nullptr;

		// Get the appropriate texture for a lighing mode
		nap::Texture2D* getTextureForLightingMode(LightingModes& lightingMode);

		void startLightingModeTransition();
		void updateLightingModeTransition(double deltaTime);
	};
}
