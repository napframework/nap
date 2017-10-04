#pragma once

#include "lineblendcomponent.h"

#include <nap/component.h>
#include <nap/componentptr.h>

namespace nap
{
	class LineNoiseComponentInstance;

	/**
	 * Properties associated with the line noise modulation component
	 */
	struct NoiseProperties
	{
		float mFrequency = 1.0f;							// Frequency of the waveform
		float mSpeed = 0.0f;								// Speed in seconds to move the waveform
		float mOffset = 0.0f;								// Offset along the line
		float mAmplitude = 1.0f;							// Amplitude of the modulation
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
		ComponentPtr<nap::LineBlendComponent> mBlendComponent;

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
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		* Updates the line color
		* @param deltaTime the time in between frames in seconds
		*/
		virtual void update(double deltaTime) override;

		NoiseProperties mProperties;

	private:
		LineBlendComponentInstance* mBlendComponent = nullptr;
		float mCurrentTime = 0.0f;
	};
}
