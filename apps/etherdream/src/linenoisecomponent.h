#pragma once

#include "lineblendcomponent.h"

#include <nap/component.h>
#include <nap/componentptr.h>
#include <smoothdamp.h>

namespace nap
{
	class LineNoiseComponentInstance;

	/**
	 * Properties associated with the line noise modulation component
	 */
	struct NoiseProperties
	{
		float mFrequency = 1.0f;							// Frequency of the waveform
		float mFrequencySmoothTime = 0.1f;					// Freq smooth time
		float mSpeed = 0.0f;								// Speed in seconds to move the waveform
		float mSpeedSmoothTime = 0.1f;						// Speed smooth time
		float mOffset = 0.0f;								// Offset along the line
		float mOffsetSmoothTime = 0.1f;						// Offset smooth time
		float mAmplitude = 1.0f;							// Amplitude of the modulation
		float mAmplitudeSmoothTime = 0.1f;					// Amplitude smooth time
	};


	/**
	 * Resource of the LineNoiseComponent
	 */
	class LineNoiseComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineNoiseComponent, LineNoiseComponentInstance)
	public:
		// property: link to the component that holds the mesh that we want to color
		ComponentPtr<LineBlendComponent> mBlendComponent;

		// property: all modulation settings
		NoiseProperties mProperties;
	};


	/**
	 * modulates a line based on it's normals and a noise pattern
	 * the noise is applied in the object's uv space
	 */
	class LineNoiseComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineNoiseComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

		/**
		* Initializes this component
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Updates the line color
		* @param deltaTime the time in between frames in seconds
		*/
		virtual void update(double deltaTime) override;

		NoiseProperties mProperties;

	private:
		ComponentInstancePtr<LineBlendComponent>	mBlendComponent = { this, &LineNoiseComponent::mBlendComponent };		// Component that holds the line we want to modulate
		float mCurrentTime = 0.0f;									// Current update time associated with this component

		// Smooths frequency over time
		math::SmoothOperator<float> mFreqSmoother					{ 1.0f, 0.1f };

		// Smooths amplitude over time
		math::SmoothOperator<float> mAmpSmoother					{ 1.0f, 0.1f };

		// Smooths Speed over time
		math::SmoothOperator<float> mSpeedSmoother					{ 0.0f, 0.1f };

		// Smooths offset over time
		math::SmoothOperator<float> mOffsetSmoother					{ 0.0f, 0.1f };
	};
}
