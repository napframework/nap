#pragma once

// Local Includes
#include "etherdreamdac.h"
#include "lasershapecomponent.h"

// External Includes
#include <nap/component.h>
#include <linemesh.h>
#include <transformcomponent.h>

namespace nap
{
	class LaserOutputComponentInstance;

	/**
	 *	Laser output properties
	 */
	struct LaserOutputProperties
	{
		glm::vec2	mFrustrum = { 500.0f, 500.0f };		//< Frustrum of the laser in world space
		bool		mFlipHorizontal = false;			//< If the output should be flipped horizontal
		bool		mFlipVertical = false;				//< If the output should be flipped vertical
	};


	/**
	 * Component that converts and sends data to the laser output DAC
	 * Converts and outputs lines to a specific laser DAC
	 */
	class LaserOutputComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(LaserOutputComponentInstance);
		}

		// Link to the DAC
		ObjectPtr<EtherDreamDac> mDac;

		// Output properties
		LaserOutputProperties mProperties;
	};


	/**
	 *	Sends lines to an etherdream laser DAC
	 */
	class LaserOutputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		LaserOutputComponentInstance(EntityInstance& entity) : ComponentInstance(entity)
		{ }

		// Init
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 *	Set the line to upload to the laser
		 * @param line: a line mesh to resample and upload
		 * @param xform: the line's global transform
		 */
		void setLine(PolyLine& line, const glm::mat4x4& xform);

		/**
		 *	Update will send the last converted line to the laser
		 */
		virtual void update(double deltaTime) override;

		// Lines will be uploaded to this laser DAC
		ObjectPtr<EtherDreamDac> mDac;

		// Properties
		LaserOutputProperties mProperties;

	private:
		// All the available shapes to draw
		std::vector<LaserShapeComponentInstance*> mShapes;

		// Populate Laser Buffer
		void populateLaserBuffer(std::vector<glm::vec3>& verts, const std::vector<glm::vec4>& colors, const glm::mat4x4& laserXform, const glm::mat4x4& lineXform);

		// Converted laser points
		std::vector<nap::EtherDreamPoint> mPoints;
	};
}