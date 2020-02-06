#pragma once

#include <component.h>
#include <nap/signalslot.h>
#include <parameternumeric.h>
#include <parameterservice.h>
#include <parametercolor.h>

namespace nap
{
	enum PresetSwitchAnimationState {
		NONE,
		FADE_OUT_CURRENT,
		LOAD_NEXT, 
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
		ResourcePtr<ParameterRGBColorFloat> mFogColor;
		ResourcePtr<ParameterRGBColorFloat> mBackgroundColor;


		int								mPresetIndex = 0;
		std::vector<std::string>		mPresets;
		RGBColorFloat					mFadeColor;
		float							mFadeDurationInSeconds;

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
		void selectPreset(int presetIndex);
		
		/**
		* selectPreset SelectPresetComponentInstance. Switch between presets with a nice fog fade
		* @param presetName the name of the preset
		*/
		void selectPreset(const std::string& presetName);

		void loadPreset(const std::string& presetName);

		std::string getCurrentPreset();

	private:
		PresetSwitchAnimationState mPresetSwitchAnimationState = PresetSwitchAnimationState::NONE;
		std::string mNextPreset;

		int mPresetIndex;
		ParameterService* mParameterService;
		std::vector<std::string> mPresets;

		float mAnimationDuration;
		float mAnimationTime;
		RGBColorFloat mFadeColor;
		RGBColorFloat mCurrentColor;

		/**
		* updatePresetSwitchAnimation SelectPresetComponentInstance. This is called in the update loop
		* @param deltaTime the delta time
		*/
		void updatePresetSwitchAnimation(double deltaTime);
		void FadeToFadeColor(float fadeProgress);

		RGBColorFloat LerpColors(RGBColorFloat color1, RGBColorFloat color2, float lerpValue);
	};
}
