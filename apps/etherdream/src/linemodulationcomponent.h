#pragma once

// Local Includes
#include "lineblendcomponent.h"

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <glm/glm.hpp>
#include <waveform.h>
#include <smoothdamp.h>

namespace nap
{
	class LineModulationComponentInstance;

	/**
	 *	Properties associated with the line modulation component
	 */
	struct ModulationProperties
	{
		float mFrequency = 1.0f;							// Frequency of the waveform
		float mFrequencySmoothTime = 0.1f;					// Freq smooth time
		float mSpeed = 0.0f;								// Speed in seconds to move the waveform
		float mSpeedSmoothTime = 0.1f;						// Speed smooth time
		float mOffset = 0.0f;								// Offset along the line
		float mOffsetSmoothTime = 0.1f;						// Offset smooth time
		float mAmplitude = 1.0f;							// Amplitude of the modulation
		float mAmplitudeSmoothTime = 0.1f;					// Amplitude smooth time
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
		* @param deltaTime the time in between frames in seconds
		*/
		virtual void update(double deltaTime) override;

		ModulationProperties mProperties;

	private:
		ComponentInstancePtr<LineBlendComponent> mBlendComponent = { this, &LineModulationComponent::mBlendComponent }; // Component that blends the lines
		float mCurrentTime = 0.0f;								// Current time
		
		// Smooths frequency over time
		math::SmoothOperator<float> mFreqSmoother				{ 1.0f, 0.1f };

		// Smooths amplitude over time
		math::SmoothOperator<float> mAmpSmoother				{ 1.0f, 0.1f };

		// Smooths Speed over time
		math::SmoothOperator<float> mSpeedSmoother				{ 0.0f, 0.1f };

		// Smooths offset over time
		math::SmoothOperator<float> mOffsetSmoother				{ 0.0f, 0.1f };

	};
}
