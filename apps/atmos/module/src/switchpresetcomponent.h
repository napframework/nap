#pragma once

#include <component.h>
#include <nap/signalslot.h>
#include <parameternumeric.h>
#include <parameterservice.h>
#include <parametercolor.h>
#include "updatematerialcomponent.h"

namespace nap
{
	class SwitchPresetComponentInstance;

	/**
	 *	SelectPresetComponent
	 */
	class NAPAPI SwitchPresetComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectPresetComponent, SwitchPresetComponentInstance)
	public:
		ResourcePtr<ParameterGroup> mPresetParameterGroup;
		ResourcePtr<ParameterGroup> mFogParameterGroup;
	
		int								mPresetIndex = 1;
		RGBColorFloat					mFadeColor;

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * SelectPresetComponentInstance	
	 */
	class NAPAPI SwitchPresetComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SwitchPresetComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize SelectPresetComponentInstance based on the SelectPresetComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the SelectPresetComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update SelectPresetComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;
		
		/**
		* selectPreset SelectPresetComponentInstance. Switch between presets with a nice fog fade
		* @param presetIndex index of the preset
		* @param animationDuration duration of the transition in seconds
		*/
		void selectPresetByIndex(unsigned int presetIndex, float animationDuration);

		/**
		* fadeOutTransitionStarted is fired when fade out transition starts. the float parameter is the duration of the fade.
		*/
		nap::Signal<float> fadeOutTransitionStarted;
		
		/**
		* revealTransitionStarted is fired when reveal transition starts. the float parameter is the duration of the fade.
		*/
		nap::Signal<float> revealTransitionStarted;

	private:
		
		enum PresetSwitchTransitionState {
			NONE,
			FADE_OUT_CURRENT,
			LOAD_NEXT,
			WAIT_FOR_LOAD,
			REVEAL_NEXT
		};

		//Transition state:
		PresetSwitchTransitionState mPresetSwitchAnimationState = PresetSwitchTransitionState::NONE;
		float mAnimationDuration;
		float mAnimationTime;
		RGBColorFloat mFadeColor;
		RGBColorFloat mCurrentColor;
		glm::vec4 mFogSettingsStart;
		glm::vec4 mFogSettingsEnd;

		//fog parameters
		ResourcePtr<ParameterGroup> mFogGroup;

		//preset parameters
		ParameterService* mParameterService;
		ResourcePtr<ParameterGroup> mPresetGroup;

		int mPresetIndex;
		std::string mNextPreset;
		std::vector<std::string> mPresets;

		bool mAnimationStart = true;

		/**
		* transitionToPreset SelectPresetComponentInstance. starts the preset transition
		* @param presetName the name of the preset
		*/
		void transitionToPreset(const std::string& presetName, float animationDuration);

		/**
		* loadPreset SelectPresetComponentInstance. loads the new preset into the parameter service effectively switching the scene
		* @param presetName the name of the preset
		*/
		void loadPreset(const std::string& presetName);

		/**
		* updatePresetTransition SelectPresetComponentInstance. updates the transition according to the state of mPresetSwitchAnimationState
		* @param deltaTime the delta time
		*/
		void updatePresetTransition(double deltaTime);
			
		/**
		* updateFogFade SelectPresetComponentInstance. lerps the fog values between mFogSettingsStart and mFogSettingsEnd
		* @param fadeProgress the progress of the fade between 0 to 1.
		*/
		void updateFogFade(double fadeProgress);
	
		/**
		* getFogColor SelectPresetComponentInstance.
		* @return fog color parameter value
		*/
		RGBColorFloat& getFogColor();
		
		/**
		* updateFogFade SelectPresetComponentInstance. 
		* @param color color for the fog color parameter
		*/
		void setFogColor(RGBColorFloat& color);

		/**
		* getFogSettings SelectPresetComponentInstance. retrieve 
		* @return subset of the fogSettings in glm::vec4. x = Fog Min, y = Fog Max, z = Fog Power, a = Fog Influence
		*/
		glm::vec4 getFogSettings();

		/**
		* setFogSettings SelectPresetComponentInstance. sets a subset of the fog parameter settings for the fade
		* @param fogSettings x = Fog Min, y = Fog Max, z = Fog Power, a = Fog Influence
		*/
		void setFogSettings(glm::vec4& fogSettings);

		/**
		* startTransition SelectPresetComponentInstance. initializes the Fade and Reveal transition
		* @param presetSwitchTransitionState valid values are FADE_OUT_CURRENT and REVEAL_NEXT
		*/
		void startTransition(const PresetSwitchTransitionState& presetSwitchTransitionState);
		
		/**
		* onPresetLoaded SelectPresetComponentInstance. called when a preset is loaded, will start the reveal animation.
		* @param presetFile the name of the preset file that has been loaded.
		*/
		void onPresetLoaded(const std::string& presetFile);

		/**
		* listens to preset loaded event in ParameterService
		*/
		nap::Slot<std::string> mPresetLoaded = { this, &SwitchPresetComponentInstance::onPresetLoaded };
	};
}
