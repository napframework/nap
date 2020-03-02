#pragma once

// Local Includes
#include "parameterblendgroup.h"
#include "parameterblender.h"

// External Includes
#include <component.h>
#include <parameternumeric.h>
#include <parameterservice.h>
#include <rtti/deserializeresult.h>
#include <nap/signalslot.h>

namespace nap
{
	class ParameterBlendComponentInstance;

	/**
	 * Blends a set of parameters over time based on a given index.
	 * The parameters that are blended are defined by the 'BlendParameterGroup'.
	 * The preset index controls the preset to sample the parameters from.
	 */
	class NAPAPI ParameterBlendComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ParameterBlendComponent, ParameterBlendComponentInstance)
	public:

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		nap::ResourcePtr<ParameterBlendGroup> mBlendParameters = nullptr;		///< Property: 'Parameters' all the parameters to blend over time
		nap::ResourcePtr<ParameterInt>	mPresetIndex = nullptr;					///< Property: 'PresetIndex' index of the preset to blend to
		nap::ResourcePtr<ParameterFloat> mPresetBlendTime = nullptr;			///< Property: 'PresetBlendTime' time it takes to blend parameters (seconds)
		bool mEnableBlending = false;											///< Property: 'If blending is enabled or not
	};


	/**
	 * Blends a set of parameters over time based on a given index.
	 * The parameters that are blended are defined by the 'BlendParameterGroup'.
	 * The preset index controls the preset to sample the parameters from.
	 */
	class NAPAPI ParameterBlendComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ParameterBlendComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		// Destructor
		~ParameterBlendComponentInstance();

		/**
		 * Initialize blendparameterscomponentInstance based on the blendparameterscomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the blendparameterscomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update blendparameterscomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Returns if blending is enabled.
		 * Preset changes without blending enabled have no effect.
		 * @return if blending is enabled.
		 */
		bool enabled() const													{ return mEnableBlending; }

		/**
		 * Turns blending on / off.
		 * @param value if blending is turned on
		 */
		void enable(bool value);

		/**
		 * @return if the component is currently blending.
		 */
		bool isBlending() const;

		/**
		 * @return current blend value from 0 - 1
		 */
		float getBlendValue();

		/**
		 * @return all the presets that can be blended.
		 */
		const std::vector<std::string>& getPresets() const						{ return mPresets; }

		/**
		 * Reloads all the presets
		 */
		bool reload(nap::utility::ErrorState& error);

	private:
		ParameterBlendGroup* mBlendParameters = nullptr;
		ParameterInt* mPresetIndex = nullptr;
		ParameterFloat* mPresetBlendTime = nullptr;
		bool mEnableBlending = false;
		ParameterService* mParameterService = nullptr;
		std::vector<std::string> mPresets;										///< All available preset names
		std::vector<std::unique_ptr<rtti::DeserializeResult>> mPresetData;		///< All available preset data 
		std::vector<ParameterGroup*> mPresetGroups;								///< Cached preset groups
		std::vector<std::unique_ptr<BaseParameterBlender>> mBlenders;			///< Individual parameter blenders
		double mElapsedTime = 0.0;
		bool mBlending = false;

		/**
		 * Sources all the presets from disk.
		 */
		bool sourcePresets(nap::utility::ErrorState& error);

		/**
		 * Creates a blender for every parameter in the blend group, if a blender is available.
		 * Parameters that do not have a blender available for their type are ignored. 
		 * A warning is issued.
		 */
		bool createBlenders(nap::utility::ErrorState& error);

		/**
		 * Updates all the blenders with a new target value.
		 * @param index new preset index
		 */
		void changePreset(int index);

		/**
		 * Slot which is called when the preset index changes
		 */
		nap::Slot<int> mIndexChangedSlot = { this, &ParameterBlendComponentInstance::changePreset };
	};
}
