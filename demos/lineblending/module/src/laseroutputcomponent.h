#pragma once

// Local Includes
#include "etherdreamdac.h"

// External Includes
#include <component.h>
#include <polyline.h>
#include <transformcomponent.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <nap/resourceptr.h>

namespace nap
{
	class LaserOutputComponentInstance;

	/**
	 * Laser output properties
	 * Note that the framerate controls the amount of points a single frame can have, together
	 * with the point rate of the laser output DAC. When a dac sends 30.000 points per second and the framerate
	 * is 60, the total number of points per frame will be 500: 30000.0 / 60.0. 
	 */
	struct NAPAPI LaserOutputProperties
	{
		glm::vec2	mFrustum = { 500.0f, 500.0f };		//< Frustrum of the laser in world space
		bool		mFlipHorizontal = false;			//< If the output should be flipped horizontal
		bool		mFlipVertical = false;				//< If the output should be flipped vertical
		int			mFrameRate = 60;					//< Preferred framerate
		float		mGapThreshold = 0.01f;				//< Threshold used to consider a gap between the begin and end vertex
	};


	/**
	 * Component that converts and sends data to ether-dream laser DAC
	 */
	class NAPAPI LaserOutputComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LaserOutputComponent, LaserOutputComponentInstance)

	public:
		// Link to the DAC
		ResourcePtr<EtherDreamDac> mDac;

		// Link to component that holds the line to send to the laser
		ResourcePtr<PolyLine> mLine;

		ComponentPtr<TransformComponent> mLineTransform;

		// Output properties
		LaserOutputProperties mProperties;
	};


	/**
	 * Converts and sends a polygon line to an ether-dream laser DAC
	 * This component re-samples the polyline and creates additional points for gaps
	 * between the beginning and ends of line segments. The final distribution
	 * depends on the line to gap ratio of the line that is updated and sent.
	 */
	class NAPAPI LaserOutputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		// Constructor
		LaserOutputComponentInstance(EntityInstance& entity, Component& resource) : 
			ComponentInstance(entity, resource)
		{
		}

		// Init
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Update will send the last converted line to the laser
		 */
		virtual void update(double deltaTime) override;

		// Properties
		LaserOutputProperties mProperties;

		// Sets the line to send to the laser
		void setPolyLine(nap::PolyLine& line)				{ mLine = &line; }

		// Sets the dac to send to the laser
		void setDac(nap::EtherDreamDac& dac)				{ mDac = &dac; }

	private:
		// Populate Laser Buffer
		void populateLaserBuffer(const PolyLine& line, const glm::mat4x4& lineXform);

		// Xform associated with the line
		ComponentInstancePtr<TransformComponent> mLineTransform = { this, &LaserOutputComponent::mLineTransform };

		// Lines will be uploaded to this laser DAC
		EtherDreamDac* mDac = nullptr;

		// Component that holds the lines to draw
		PolyLine* mLine = nullptr;

		// Converted laser points
		std::vector<nap::EtherDreamPoint> mPoints;			//< DAC points
		std::vector<glm::vec3> mVerts;						//< Converted vertex positions
		std::vector<glm::vec4> mColors;						//< Converted vertex colors
	};
}
