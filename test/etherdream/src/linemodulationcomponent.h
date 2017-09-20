#pragma once

// Local Includes
#include "lineblendcomponent.h"

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <glm/glm.hpp>
#include <waveform.h>

namespace nap
{
	class LineModulationComponentInstance;

	/**
	 *	Properties associated with the line modulation component
	 */
	struct ModulationProperties
	{
		float mFrequency = 1.0f;							// Frequency of the waveform
		float mSpeed = 0.0f;								// Speed in seconds to move the waveform
		float mOffset = 0.0f;								// Offset along the line
		float mAmplitude = 1.0f;							// Amplitude of the modulation
		bool  mNormalize = false;							// Normalizes modulation frequency along the spline (0-1)
		math::EWaveform mWaveform = math::EWaveform::SINE;	// Type of modulation
	};


	/**
	* Resource of the LineModulationComponent
	*/
	class LineModulationComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineModulationComponent, LineModulationComponentInstance)
	public:
		// property: link to the component that holds the mesh that we want to color
		ComponentPtr<nap::LineBlendComponent> mBlendComponent;

		// property: all modulation settings
		ModulationProperties mProperties;
	};


	/**
	 *	modulates a line based on it's normals and a waveform
	 */
	class LineModulationComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineModulationComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

		/**
		* Initializes this component
		*/
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		* Updates the line color
		*/
		virtual void update(double deltaTime) override;

		ModulationProperties mProperties;

	private:
		LineBlendComponentInstance* mBlendComponent = nullptr;
		float mCurrentTime = 0.0f;
	};
}
