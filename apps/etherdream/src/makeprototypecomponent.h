#pragma once

// Local Includes
#include "lasercompound.h"
#include "lineblendcomponent.h"
#include "updatenormalscomponent.h"
#include "linetracecomponent.h"
#include "laseroutputcomponent.h"
#include "linenoisecomponent.h"

// External Includes
#include <nap/component.h>
#include <nap/entity.h>
#include <renderablemeshcomponent.h>
#include <oscinputcomponent.h>

namespace nap
{
	class MakePrototypeComponentInstance;

	/**
	 *	makeprototypecomponent
	 */
	class MakePrototypeComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(MakePrototypeComponent, MakePrototypeComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// property: link to the spline entity to spawn
		nap::ObjectPtr<nap::Entity> mSplineEntity = nullptr;

		// property: link to the output entity to spawn
		nap::ObjectPtr<nap::Entity> mLaserOutputEntity = nullptr;

		// property: all the osc messages associated with the laser entity
		std::vector<std::string> mOSCAddresses;
	};


	/**
	 * makeprototypecomponentInstance	
	 */
	class MakePrototypeComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		MakePrototypeComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize makeprototypecomponentInstance based on the makeprototypecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the makeprototypecomponentInstance is initialized successfully
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Called by the controller, sets up the laser resources based on the compound settings
		 */
		bool setup(LaserCompound& settings, nap::utility::ErrorState& error);

		// Utilities
		nap::RenderableComponentInstance& getLineRenderer()			{ return *mLineRenderer; }
		nap::RenderableComponentInstance& getNormalRenderer()		{ return *mNormalsRenderer; }

	private:
		// Holds the created spline entity
		nap::EntityInstance* mSplineEntity = nullptr;

		// Holds the created laser output entity
		nap::EntityInstance* mLaserOutputEntity = nullptr;

		// Holds the Line Blend Component
		nap::LineBlendComponentInstance* mLineBlender = nullptr;

		// Holds the line renderable mesh component
		nap::RenderableMeshComponentInstance* mLineRenderer = nullptr;

		// Holds the normals renderable mesh component
		nap::RenderableMeshComponentInstance* mNormalsRenderer = nullptr;

		// Holds the update normals component
		nap::UpdateNormalsComponentInstance* mUpdateNormalsComponent = nullptr;

		// Holds the line trace component
		nap::LineTraceComponentInstance* mTraceComponent = nullptr;

		// Holds the laser output component
		nap::LaserOutputComponentInstance* mOutputComponent = nullptr;

		// Holds the osc receiver
		nap::OSCInputComponentInstance* mOSCInputComponent = nullptr;

		// Holds the line noise component
		nap::LineNoiseComponentInstance* mNoiseComponent = nullptr;

		// Holds the template osc address pattern
		std::vector<std::string> mOSCAddressPattern;
	};
}
