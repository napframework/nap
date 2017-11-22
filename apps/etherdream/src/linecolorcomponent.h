#pragma once

// Local Includes
#include "lineblendcomponent.h"

// External includes
#include "component.h"
#include <nap/objectptr.h>
#include <image.h>
#include <glm/glm.hpp>
#include <smoothdamp.h>

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

		// property: smoothing speed for start point
		glm::vec2 mStartSmoothTime = { 1.0f, 1.0f };

		// property: smoothing speed for end point
		glm::vec2 mEndSmoothTime = { 1.0f, 1.0f };

		// property: smoothing speed for intensity
		float mIntensitySmoothTime = 1.0f;

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
		virtual bool init(utility::ErrorState& errorState) override;

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
		 * @return the current start position in uv coordinates
		 */
		const glm::vec2& getStartPosition() const					{ return mStartPosition; }

		/**
		 * Sets the end uv coordinate of the line, will be clamped between 0-1
		 * @param endPosition end position in uv space of the line
		 */
		void setEndPosition(const glm::vec2& endPosition);

		/**
		 *	@return the current end position in uv coordinates
		 */
		const glm::vec2& getEndPosition() const						{ return mEndPosition; }

		/**
		 * Sets the intensity of the line, this is a global multiplier
		 * @param intensity the new intensity, will be clamped between 0-1
		 */
		void setIntensity(float intensity);

		/**
		 * Sets the smooth speed for the start point's x axis
		 * @param speed the time it takes in seconds
		 */
		void setStartSmoothSpeedX(float speed)						{ mStartSmootherX.mSmoothTime = math::max<float>(speed, 0.0f); }

		/**
		 * Sets the smooth speed for the start point's y axis
		 * @param speed the time it takes in seconds
		 */
		void setStartSmoothSpeedY(float speed)						{ mStartSmootherY.mSmoothTime = math::max<float>(speed, 0.0f); }

		/**
		 * Sets the smooth speed for end point's x axis
		 * @param speed the time it takes in seconds
		 */
		void setEndSmoothSpeedX(float speed)						{ mEndSmootherX.mSmoothTime = math::max<float>(speed, 0.0f); }

		/**
		* Sets the smooth speed for end point's x axis
		* @param speed the time it takes in seconds
		*/
		void setEndSmoothSpeedY(float speed)						{ mEndSmootherY.mSmoothTime = math::max<float>(speed, 0.0f); }

		/**
		 *	Sets the smooth speed for intensity
		 */
		void setIntensitySmoothSpeed(float speed)					{ mIntensitySmoother.mSmoothTime = math::max<float>(speed, 0.0f); }

		/**
		 *	If we want to link color 2 to color 1
		 * @param value if we want to link the colors
		 */
		void link(bool value)										{ mLink = value; }

		/**
		 * @return the pixel color as vector 3 at uv position x,y
		 * @param uvPos the uv position to get the color for
		 * @param outColor the pixel color as normalized float
		 */
		void getColor(const glm::vec2& uvPos, glm::vec3& outColor);

	private:
		ComponentInstancePtr<LineBlendComponent> mBlendComponent = { this, &LineColorComponent::mBlendComponent };		// Holds the line we want to color
		Image* mLookupImage = nullptr;								// Image used for color lookup
		glm::vec2 mStartPosition = { 0.5f, 0.5f };					// Start point lookup in uv space
		glm::vec2 mEndPosition = { 0.5f, 0.5f };					// End point lookup in uv space
		float mIntensity = 1.0f;									// Final intensity
		bool mWrap = false;											// If the color values should be wrapped
		float mPower = 1.0f;										// Amount of blend power when computing the wrap
		bool mLink = false;											// If color 2 is linked to color one

		// Smooths the start point
		math::SmoothOperator<float> mStartSmootherX					{ 0.5f, 1.0f };
		math::SmoothOperator<float> mStartSmootherY					{ 0.5f, 1.0f };

		// Smooths the end point
		math::SmoothOperator<float> mEndSmootherX					{ 0.5f, 1.0f };
		math::SmoothOperator<float> mEndSmootherY					{ 0.5f, 1.0f };

		// Smooths intensity
		math::SmoothOperator<float> mIntensitySmoother				{ 1.0f, 0.1f };
	};
}

