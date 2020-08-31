#pragma once

// Local Includes
#include "lineblendcomponent.h"

// External includes
#include <component.h>
#include <rtti/objectptr.h>
#include <imagefromfile.h>
#include <glm/glm.hpp>
#include <smoothdamp.h>
#include <nap/resourceptr.h>
#include <color.h>
#include <parametercolor.h>

namespace nap
{
	class LineColorComponentInstance;

	/**
	 *	Resource of the line color component
	 */
	class NAPAPI LineColorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineColorComponent, LineColorComponentInstance)
	public:
		// property: Color of the line
		glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		// property: link to the component that holds the mesh that we want to color
		ComponentPtr<nap::LineBlendComponent> mBlendComponent;

		// property: intensity of the spline
		float mIntensity = 1.0f;

		// property: if the color values should be wrapped
		bool mWrap = false;

		// property: wrap blend intensity
		float mWrapPower = 1.0f;

		// property: if the colors are linked, so point2 receives the color of point1
		bool mLink = false;

		ResourcePtr<nap::ParameterRGBColorFloat> mColorOne;		///< First Color
		ResourcePtr<nap::ParameterRGBColorFloat> mColorTwo;		///< Second Color
	};


	/**
	 * Colors a line based on two colors
	 */
	class NAPAPI LineColorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineColorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

	   /**
		* Initializes this component
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	   /**
		* Updates the color of all the line vertices
		*/
		virtual void update(double deltaTime) override;

		/**
		 *	Set color smooth speed
		 */
		void setColorSmoothSpeed(float speed);

		/**
		 * Sets the intensity of the line, this is a global multiplier
		 * @param intensity the new intensity, will be clamped between 0-1
		 */
		void setIntensity(float intensity);

		/**
		 *	Sets the smooth speed for intensity
		 */
		void setIntensitySmoothSpeed(float speed)						{ mIntensitySmoother.mSmoothTime = math::max<float>(speed, 0.0f); }

		/**
		 *	If we want to link color 2 to color 1
		 * @param value if we want to link the colors
		 */
		void link(bool value)											{ mLink = value; }

	private:
		ComponentInstancePtr<LineBlendComponent> mBlendComponent = { this, &LineColorComponent::mBlendComponent };		// Holds the line we want to color
		float mIntensity = 1.0f;								// Final intensity
		bool mWrap = false;										// If the color values should be wrapped
		float mPower = 1.0f;									// Amount of blend power when computing the wrap
		bool mLink = false;										// If color 2 is linked to color one

		ParameterRGBColorFloat* mColorOne = nullptr;			// First color from parameter
		ParameterRGBColorFloat* mColorTwo = nullptr;			// Second color from parameter

		// Smooths the first color
		math::SmoothOperator<glm::vec3> mColorOneSmoother		{ {1.0f, 1.0f, 1.0f},  0.5f };

		// Smooths the second color
		math::SmoothOperator<glm::vec3> mColorTwoSmoother		{ { 1.0f, 0.0f, 0.0f}, 0.5f };

		// Smooths intensity
		math::SmoothOperator<float> mIntensitySmoother			{ 1.0f, 0.1f };
	};
}

