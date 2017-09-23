#pragma once

// Local Includes
#include "etherdreamdac.h"

// External Includes
#include <nap/component.h>
#include <polyline.h>
#include <transformcomponent.h>
#include <nap/componentptr.h>
#include <renderablemeshcomponent.h>

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
		DECLARE_COMPONENT(LaserOutputComponent, LaserOutputComponentInstance)

	public:
		// Link to the DAC
		ObjectPtr<EtherDreamDac> mDac;

		// Link to component that holds the line to send to the laser
		ComponentPtr<RenderableMeshComponent> mLine;

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
		RenderableMeshComponentInstance* mLine = nullptr;

		// Properties
		LaserOutputProperties mProperties;

	private:
		// Populate Laser Buffer
		void populateLaserBuffer(const std::vector<glm::vec3>& verts, const std::vector<glm::vec4>& colors, const glm::mat4x4& laserXform, const glm::mat4x4& lineXform);

		// Converted laser points
		std::vector<nap::EtherDreamPoint> mPoints;
	};
}