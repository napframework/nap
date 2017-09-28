#pragma once

// Local Includes
#include "lineblendcomponent.h"

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <image.h>
#include <glm/glm.hpp>

namespace nap
{
	class LineColorComponentInstance;

	/**
	 *	Resource of the line color component
	 */
	class LineColorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineColorComponent, LineColorComponentInstance)
	public:
		// property: Color of the line
		glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		// property: link to the component that holds the mesh that we want to color
		ComponentPtr<nap::LineBlendComponent> mBlendComponent;

		// property: link to the image component that holds the lookup image
		ObjectPtr<nap::Image> mLookupImage;

		// property: start lookup color for spline
		glm::vec2 mStartPos = { 0.5f, 0.5f };

		// property: end lookup color for spline
		glm::vec2 mEndPos = { 0.5f, 0.5f };

		// property: intensity of the spline
		float mIntensity = 1.0f;

		// property: if the color values should be wrapped
		bool mWrap = false;

		// property: wrap blend intensity
		float mWrapPower = 1.0f;

		// property: if the colors are linked, so point2 receives the color of point1
		bool mLink = false;
	};


	/**
	 * Colors a line based on a bitmap
	 * The end and start vertices are given uv coordinates that are used to perform a bitmap color lookup
	 * The remaining vertices are assigned uv's based on those boundary uv coordinates
	 * This ensures the line only receives colors associated with a bitmap and no blended (interpolated) colors
	 */
	class LineColorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineColorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

	   /**
		* Initializes this component
		*/
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

	   /**
		* Updates the line color
		*/
		virtual void update(double deltaTime) override;

		/**
		 * Sets the start uv coordinate of the line, will be clamped between 0-1
		 * @param startPosition start position in uv space of the line
		 */
		void setStartPosition(const glm::vec2& startPosition);

		/**
		 * Sets the end uv coordinate of the line, will be clamped between 0-1
		 * @param endPosition end position in uv space of the line
		 */
		void setEndPosition(const glm::vec2& endPosition);

		/**
		 * Sets the intensity of the line, this is a global multiplier
		 * @param intensity the new intensity, will be clamped between 0-1
		 */
		void setIntensity(float intensity);

		/**
		 *	If we want to link color 2 to color 1
		 * @param value if we want to link the colors
		 */
		void link(bool value)										{ mLink = value; }

	private:
		LineBlendComponentInstance* mBlendComponent = nullptr;		// Holds the line we want to color
		Image* mLookupImage = nullptr;								// Image used for color lookup
		glm::vec2 mStartPosition = { 0.5f, 0.5f };					// Start point lookup in uv space
		glm::vec2 mEndPosition = { 0.5f, 0.5f };					// End point lookup in uv space
		float mIntensity = 1.0f;									// Final intensity
		bool mWrap = false;											// If the color values should be wrapped
		float mPower = 1.0f;										// Amount of blend power when computing the wrap
		bool mLink = false;											// If color 2 is linked to color one
	};
}
