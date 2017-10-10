#pragma once

// Local Includes
#include "etherdreamdac.h"

// External Includes
#include <nap/component.h>
#include <polyline.h>
#include <transformcomponent.h>
#include <nap/componentptr.h>
#include <nap/entityptr.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	class LaserOutputComponentInstance;

	/**
	 * Laser output properties
	 * Note that the framerate controls the amount of points a single frame can have, together
	 * with the point rate of the laser output DAC. When a dac sends 30.000 points per second and the framerate
	 * is 60, the total number of points per frame will be 500: 30000.0 / 60.0. 
	 */
	struct LaserOutputProperties
	{
		glm::vec2	mFrustrum = { 500.0f, 500.0f };		//< Frustrum of the laser in world space
		bool		mFlipHorizontal = false;			//< If the output should be flipped horizontal
		bool		mFlipVertical = false;				//< If the output should be flipped vertical
		int			mFrameRate = 60;					//< Preferred framerate
		float		mGapThreshold = 0.01f;				//< Threshold used to consider a gap between the begin and end vertex
	};


	/**
	 * Component that converts and sends data to ether-dream laser DAC
	 */
	class LaserOutputComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LaserOutputComponent, LaserOutputComponentInstance)

	public:
		// Link to the DAC
		ObjectPtr<EtherDreamDac> mDac;

		// Link to component that holds the line to send to the laser
		ObjectPtr<PolyLine> mLine;

		// Output properties
		LaserOutputProperties mProperties;
	};


	/**
	 * Converts and sends PolyLines to an ether-dream laser DAC
	 * This component re-samples the polyline and creates additional points for gaps
	 * between the beginning and ends of line segments. The final distribution
	 * depends on the line to gap ratio of the line that is updated and sent.
	 */
	class LaserOutputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		// Constructor
		LaserOutputComponentInstance(EntityInstance& entity, Component& resource) : 
			ComponentInstance(entity, resource)
		{
		}

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 *	Update will send the last converted line to the laser
		 */
		virtual void update(double deltaTime) override;

		// Lines will be uploaded to this laser DAC
		EtherDreamDac* mDac = nullptr;

		// Component that holds the lines to draw
		PolyLine* mLine = nullptr;

		// Properties
		LaserOutputProperties mProperties;

		// Sets the transform to use for the line
		void setTransform(nap::EntityInstance& entity);

	private:
		// Populate Laser Buffer
		void populateLaserBuffer(const PolyLine& line, const glm::mat4x4& laserXform, const glm::mat4x4& lineXform);

		// Xform associated with the line
		nap::TransformComponentInstance* mLineTransform = nullptr;

		// Converted laser points
		std::vector<nap::EtherDreamPoint> mPoints;			//< DAC points
		std::vector<glm::vec3> mVerts;						//< Converted vertex positions
		std::vector<glm::vec4> mColors;						//< Converted vertex colors 	
	};
}