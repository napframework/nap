#pragma once

// Local Includes
#include "yoctosensor.h"
#include "parametermapping.h"

// External Includes
#include <component.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <parametervec.h>

namespace nap
{
	class ApplySensorComponentInstance;

	/**
	 *	applysensorcomponent
	 */
	class NAPAPI ApplySensorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ApplySensorComponent, ApplySensorComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		bool mEnabled = true;											///< Property: 'Enabled' if the component forwards sensor data to the selected parameter
		ResourcePtr<BaseYoctoSensor> mSensor;							///< Property: 'Sensor' the sensor to read and apply
		std::vector<ResourcePtr<ParameterMapping>> mParameters;			///< Property: 'Parameters' the parameter that you want to influence
		ResourcePtr<ParameterInt> mSelection;							///< Property: 'Index' Selection
	};


	/**
	 * applysensorcomponentInstance	
	 */
	class NAPAPI ApplySensorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ApplySensorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize applysensorcomponentInstance based on the applysensorcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applysensorcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update applysensorcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Selects a parameter to use by index
		 */
		void selectParameter(int index);

	private:
		BaseYoctoSensor* mSensor = nullptr;
		std::vector<ParameterMapping*> mParameters;
		int mCurrentIndex = 0;
		ParameterMapping* mCurrentParameter;
		nap::Slot<int> mIndexChangedSlot		= { this, &ApplySensorComponentInstance::selectParameter };
		bool mEnabled = true;
	};
}
