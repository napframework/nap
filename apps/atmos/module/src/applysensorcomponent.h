#pragma once

// Local Includes
#include "yoctosensor.h"
#include "parametersimple.h"

// External Includes
#include <component.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <smoothdamp.h>

namespace nap
{
	class ApplySensorComponentInstance;

	/**
	 * Maps the output of a sensor to a selected parameter.
	 * Smoothing is applied on the input of the sensor data.
	 */
	class NAPAPI ApplySensorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ApplySensorComponent, ApplySensorComponentInstance)
	public:
		enum class SensorCalcTypes : uint8_t
		{
			AVERAGE_VALUE = 0,
			CUMULATIVE_VALUE = 1,
			CHOOSE_MAXIMUM_VALUE = 2,
			CHOOSE_MINIMUM_VALUE = 3
		};

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		/**
		 * Updates available range of parameters to select from
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		ResourcePtr<ParameterBool> mEnabled;							///< Property: 'Enabled' if events are forwarded											///< Property: 'Enabled' if the component forwards sensor data to the selected parameter
		std::vector<ResourcePtr<BaseYoctoSensor>> mSensors;							///< Property: 'Sensors' the sensors to read and apply
		std::vector<ResourcePtr<ParameterFloat>> mParameters;			///< Property: 'Parameters' the parameter that you want to influence
		ResourcePtr<ParameterVec2> mInputRange;							///< Property: 'InputRange' sensor input range
		ResourcePtr<ParameterVec2> mOutputRange;						///< Property: 'OutputRange' applied parameter output range
		ResourcePtr<ParameterInt> mSelection;							///< Property: 'Index' Selection
		ResourcePtr<ParameterFloat> mSmoothTime;						///< Property: 'SmoothTime' smoothing time applied to sensor input in seconds
		SensorCalcTypes mSensorCalcType;							///< Property: 'Calc Type' describes method of handling data input from multiple sensors
		double mMaxSensorValue = 1200;									///< Property: 'Max Sensor Value' sensor value will be clamped to this value
	};


	/**
	 * Maps the output of a sensor to a selected parameter.
	 * Smoothing is applied on the input of the sensor data.
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
		std::vector<BaseYoctoSensor*> mSensors;
		std::vector<ParameterFloat*> mParameters;
		int mCurrentIndex = 0;
		ParameterFloat* mCurrentParameter;
		nap::Slot<int> mIndexChangedSlot		= { this, &ApplySensorComponentInstance::selectParameter };
		ParameterBool* mEnabled = nullptr;
		ParameterVec2* mInputRange = nullptr;
		ParameterVec2* mOutputRange = nullptr;
		ParameterFloat* mSmoothTime = nullptr;
		math::FloatSmoothOperator mSmoother = { 0.0f, 0.1f };
		ApplySensorComponent::SensorCalcTypes mCalcType;
		double mMaxSensorValue;
	private:
		// private methods
		double calcValueAverage();
		double calcValueMax();
		double calcValueMin();
		double calcValueCumulative();

		double (ApplySensorComponentInstance::*mCalcFunc)();
	};
}
