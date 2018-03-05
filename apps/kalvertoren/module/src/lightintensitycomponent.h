#pragma once

// Local Includes
#include "yoctoluxsensor.h"

// External Includes
#include <component.h>
#include <glm/glm.hpp>
#include <smoothdamp.h>

namespace nap
{
	class LightIntensityComponentInstance;

	/**
	 * lightintensitycomponent
	 */
	class NAPAPI LightIntensityComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LightIntensityComponent, LightIntensityComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Properties
		std::vector<ObjectPtr<YoctoLuxSensor>> mLuxSensors;				///< Property: 'Sensors' all the available lux sensors
		float mSensorInfluence = 1.0f;									///< Property: 'SensorInfluence' the amount of influence the sensor has on the final light output
		float mMasterIntensity = 1.0f;									///< Property: 'MasterIntensity' the maximum number of allowed brightness
		glm::vec2 mLuxRange = { 10.0f, 25000.0f };						///< Property: "input lux range"
		glm::vec2 mLightRange = { 0.1f, 1.0f };							///< Property: "Output light range" 
		float mLuxPower = 1.0f;											///< Property: "LuxCurve" power of curve associated with min / max values
		float mSmoothTime = 1.0f;										///< Property: "SmoothTime" amount of time in seconds to blend between value
	};


	/**
	 * lightintensitycomponentInstance	
	 */
	class NAPAPI LightIntensityComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LightIntensityComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize lightintensitycomponentInstance based on the lightintensitycomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the lightintensitycomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update lightintensitycomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the final computed brightness (0-1)
		 */
		float getBrightness() const						{ return mBrightness; }

		/**
		 *	@return the average lux values from all sensors combined, -1.0f if no lux value is retrieved
		 */
		float getLuxAverage() const						{ return mLuxAverage; }

		/**
		 * Sets the master output range (0-1)
		 * @param value global brightness scale multiplier
		 */
		void setMasterBrightness(float value);

		/**
		 * Sets the lux power value for the interpolation curve
		 * @param value the power value applied to the lux curve
		 */
		void setLuxPower(float value)					{ mLuxPower = value; }

		/**	
		 * Sets the lux range
		 * @param range the min and max lux read-out values
		 */
		void setLuxRange(const glm::vec2 range);

		/**
		 *	@return the lux range
		 */
		glm::vec2 getLuxRange() const					{ return mLuxRange; }

		/**
		 * Sets the light output range
		 * @param range the min and max (normalized) light output values
		 */
		void setLightRange(const glm::vec2 range);

		/**
		 * Sets the influence of the sensors on the system
		 * @param value the normalized influence value
		 */
		void setSensorInfluence(float value);

		/**
		 * Sets the sensor smooth time in seconds
		 * @param value smooth time in seconds
		 */
		void setSmoothTime(float value);

		/**
		 *	@return all the sensors registered to the intensity component
		 */
		const std::vector<YoctoLuxSensor*>& getSensors();

	private:
		// All lux sensors
		std::vector<YoctoLuxSensor*> mSensors;

		// Average sensor lux value
		float mLuxAverage = -1.0f;

		// Output light value
		float mBrightness = 1.0f;

		// Component properties (could be turned in to a struct)
		float mSensorInfluence = 1.0f;
		float mMasterIntensity = 1.0f;									
		glm::vec2 mLuxRange = { 10.0f, 25000.0f };						
		glm::vec2 mLightRange = { 0.1f, 1.0f };							
		float mLuxPower = 1.0f;											

		 //This operator smooths the brightness value over time
		math::SmoothOperator<float> mIntensitySmoother = { mBrightness, 1.0f };
	};
}
