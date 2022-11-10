/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	// Forward Declares
	class ParameterBlendComponentInstance;

	/**
	 * Smoothly blends a set of parameters over time towards a given preset.
	 * The parameters that are blended are defined by the 'BlendGroup'.
	 * The preset index controls the preset to sample the parameters from.
	 *
	 * Note that the 'PresetBlendTime' and 'PresetIndex' parameter links
	 * should not be part of the 'BlendGroup'.
	 *
	 * A blender needs to be available for every parameter that is blended.
	 * The system issues a warning on initialization when there is no blender available for a specific parameter.
	 * By default float, double, vec2, vec3 and color parameters are supported.
	 * If a preset does not contain a specific parameter a warning is issued.
	 */
	class NAPAPI ParameterBlendComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ParameterBlendComponent, ParameterBlendComponentInstance)
	public:

		nap::ResourcePtr<ParameterBlendGroup> mBlendGroup = nullptr;			///< Property: 'BlendGroup' all the parameters to blend over time
		nap::ResourcePtr<ParameterInt>	mPresetIndex = nullptr;					///< Property: 'PresetIndex' index of the preset to blend to
		nap::ResourcePtr<ParameterFloat> mPresetBlendTime = nullptr;			///< Property: 'PresetBlendTime' time it takes to blend parameters (seconds)
		bool mEnableBlending = false;											///< Property: 'If blending is enabled or not
		bool mIgnoreNonBlendableParameters = true;                              ///< Property: 'IgnoreNonBlendable' if false, throws error when parameters cannot be blended
	};


	/**
	 * Runtime part of the parameter blend component.
	 * Smoothly blends a set of parameters over time towards a given preset.
	 * The parameters that are blended are defined by the 'BlendGroup' of the resource.
	 * The preset index controls the preset to sample the parameters from.
	 *
	 * A blender needs to be available for every parameter that is blended.
	 * The system issues a warning on initialization when there is no blender available for a specific parameter.
	 * By default float, double, vec2, vec3 and color parameters are supported.
	 * If a preset does not contain a specific parameter a warning is issued.
	 *
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
		 * Initializes the component.
		 * All presets associated with the 'RootGroup' of the 'BlendGroup' are sourced from disk.
		 * A blender is created for every parameter and stored for future use on update.
		 * @param errorState contains the error message if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Blends the parameter values.
		 * @param deltaTime time in between frames in seconds.
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
		 * @return the names of all the presets that can be blended.
		 */
		const std::vector<std::string>& getPresets() const						{ return mPresets; }

		/**
		 * Reloads all the presets from disk.
		 * @param error contains the error if reloading fails.
		 * @return if reloading the presets succeeded.
		 */
		bool reload(nap::utility::ErrorState& error);

		/**
		 * @return if there are any presets to blend
		 */
		bool hasPresets() const													{ return !(mPresets.empty()); }

	private:
		ParameterBlendGroup* mBlendParameters = nullptr;						///< Parameters that are blended over time
		ParameterInt* mPresetIndex = nullptr;									///< Current preset blend index
		ParameterFloat* mPresetBlendTime = nullptr;								///< Current blend time in seconds
		bool mEnableBlending = false;											///< Blending toggle
		ParameterService* mParameterService = nullptr;							///< The parameter service
		std::vector<std::string> mPresets;										///< All available preset names
		std::vector<std::unique_ptr<rtti::DeserializeResult>> mPresetData;		///< All available preset data
		std::vector<ParameterGroup*> mPresetGroups;								///< Cached preset groups
		std::vector<std::unique_ptr<BaseParameterBlender>> mBlenders;			///< Individual parameter blenders
		double mElapsedTime = 0.0;												///< Current elapsed time in seconds
		bool mBlending = false;													///< If the component is currently blending values
        bool mIgnoreNonBlendableParameters = true;                              ///< If we should ignore non-blendable parameters

		/**
		 * Sources all the presets from disk.
		 * @param error contains the error if sourcing fails.
		 * @return if sourcing succeeded.
		 */
		bool sourcePresets(nap::utility::ErrorState& error);

		/**
		 * Creates a blender for every parameter in the blend group, if a blender is available.
		 * Parameters that do not have a blender available for their type are ignored.
		 * A warning is issued.
		 * @param error contains the error if the operation fails.
		 * @return if the operation succeeded.
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
