#pragma once

#include <component.h>
#include <nap/signalslot.h>
#include <parameternumeric.h>
#include <parameterservice.h>
#include <parametercolor.h>
#include "updatematerialcomponent.h"

namespace nap
{
	enum PresetSwitchTransitionState {
		NONE,
		FADE_OUT_CURRENT,
		LOAD_NEXT,
		WAIT_FOR_LOAD,
		REVEAL_NEXT
	};

	class SelectPresetComponentInstance;

	/**
	 *	SelectPresetComponent
	 */
	class NAPAPI SelectPresetComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectPresetComponent, SelectPresetComponentInstance)
	public:
		ResourcePtr<ParameterGroup> mPresetParameterGroup;
		ResourcePtr<ParameterGroup> mFogParameterGroup;
		
		//NOT USED NOW, BEcause using update material directly
		ResourcePtr<ParameterRGBColorFloat> mFogColor;
		ResourcePtr<ParameterRGBColorFloat> mBackgroundColor;

		int								mPresetIndex = 0;
		std::vector<std::string>		mPresets;
		RGBColorFloat					mFadeColor;
		float							mAnimationDuration;

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * SelectPresetComponentInstance	
	 */
	class NAPAPI SelectPresetComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectPresetComponentInstance(EntityInstance& entity, Component& resource) :
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
		*/
		void selectPresetByIndex(int presetIndex);

	private:
		PresetSwitchTransitionState mPresetSwitchAnimationState = PresetSwitchTransitionState::NONE;
		std::string mCurrentPreset;
		std::string mNextPreset;
		
		ParameterService* mParameterService;
		ResourcePtr<ParameterGroup> mPresetGroup;
		ResourcePtr<ParameterRGBColorFloat> mFogColor;
		ResourcePtr<ParameterGroup> mFogGroup;
		
		int mPresetIndex;
		std::vector<std::string> mPresets;

		float mAnimationDuration;
		float mAnimationTime;
		RGBColorFloat mFadeColor;
		RGBColorFloat mCurrentColor;
		glm::vec4 mFogSettingsStart;
		glm::vec4 mFogSettingsEnd;

		/**
		* selectPreset SelectPresetComponentInstance. Switch between presets with a nice fog fade
		* @param presetName the name of the preset
		*/
		void transitionToPreset(const std::string& presetName);


		void loadPreset(const std::string& presetName);

		/**
		* updatePresetSwitchAnimation SelectPresetComponentInstance. This is called in the update loop
		* @param deltaTime the delta time
		*/
		void updatePresetSwitchAnimation(double deltaTime);
			
		//TODO comments:
		void updateFogFade(double fadeProgress);
	
		RGBColorFloat& getFogColor();
		void setFogColor(RGBColorFloat& color);

		glm::vec4 getFogSettings();
		void setFogSettings(glm::vec4& fogSettings);

		void startTransition(const PresetSwitchTransitionState& presetSwitchTransitionState);
		void onPresetLoaded(std::string& presetFile);

		nap::Slot<std::string> mPresetLoaded = { this, &SelectPresetComponentInstance::onPresetLoaded };
	};
}
