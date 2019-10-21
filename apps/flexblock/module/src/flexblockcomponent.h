#pragma once

// internal includes
#include "framemesh.h"
#include "flexblockmesh.h"
#include "flexblockdata.h"
#include "flex.h"
#include "visualizenormalsmesh.h"
#include "flexblockserialcomponent.h"

// external includes
#include <component.h>
#include <boxmesh.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>
#include <maccontroller.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class FlexBlockComponentInstance;

	/**
	 * FlexBlockComponent controls a flexblock mesh, frame mesh and optionally writes
	 * data to serial motors. 
	 * FlexBlockComponent needs a flexblock shape definition. See FlexBlockShape and flexblockdata.h
	 */
	class NAPAPI FlexBlockComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockComponent, FlexBlockComponentInstance)
	public:
		
		/**
		 * Resource pointer to the mesh of the frame
		 */
		ResourcePtr<FrameMesh> mFrameMesh; ///< Property: 'FrameMesh' Reference to the frame mesh

		/**
		 * Resource pointer to the flexblock mesh 
		 */
		ResourcePtr<FlexBlockMesh> mFlexBlockMesh; ///< Property: 'FlexBlockMesh' Reference to the FlexBlockMesh 

		/**
		 * Resource pointer to the shape definition
		 */
		ResourcePtr<FlexBlockShape> mFlexBlockShape; ///< Property: 'FlexBlockShape' Reference to the shape definition of the block 
	
		/**
		 * Reference to mac controller
		 */
		nap::ResourcePtr<MACController>		mMacController;

		bool mEnableMacController = true;

		/**
		 * Mapping of ethercat motors to flexblock motors
		 */
		std::vector<int> mMotorMapping = { 5, 1, 2, 6, 3, 7, 0, 4 };

		double mMotorStepsPerMeter = 12.73239f; ///< Property: 'Counts per meter' value that we need to calculate how much steps we need the motor(s) to make

		int mMotorOffset = 7542; ///< Property: 'Motor Offset' value that we need to calculate the zero position of the motor

		float mSlackRange = 1.0f; ///< Property: 'Slack Range' 

		float mSlackMinimum = -0.5f; ///< Property: 'Slack Minimum' slack minimum 

		float mSinusAmplitudeRange = 0.5f; ///< Property: 'Amplitude Range' range of the sinusoide in meters

		float mSinusFrequencyRange = 100.0f; ///< Property: 'Frequency Range' range of frequency in Hz

		float mOverrideRange = 24.0f; ///< Property: 'Override Range' range of override parameters in meters

		float mOverrideMinimum = 0.0f; ///< Property: 'Override Minimum' minimum of override parameters in meters, we start to count from this value

		bool mEnableSerial = false;///< Property: 'Use Serial' use serial or not

		int mFlexFrequency = 1000;

		bool mEnableDigitalPin = false;

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	* FlexBlockComponentInstance
	* Instantiated flexblock component
	*/
	class NAPAPI FlexBlockComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FlexBlockComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
		
		virtual ~FlexBlockComponentInstance();

		/**
		 * Initialize FlexBlockComponentInstance based on the FlexBlockComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FlexBlockComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update FlexBlockComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		* Update motor speed [0..8]
		* @param index index of motor [0..8]
		* @param value motor input 0-1
		*/
		void setMotorInput(const int index, float value);

		/**
		 * Set slack parameter
		 * @param value slack value, typically between -0.5 and 0.5
		 */
		void setSlack(const float value);

		/**
		 * Set override
		 * @param override range index, motor index will be in flexblock space
		 * @param value between 0..1 , will be multiplied by override range 
		 */
		void setOverrides(const int index, const float value);

		/**
		 * Sets frequency of override
		 * @param value between 0..1 , will be multiplied with sinus frequency
		 */
		void setSinusFrequency(const float value);

		/**
		 * Sets sinus amplitude
		 * @param value between 0..1 , will be multiplied with amplitude
		 */
		void setSinusAmplitude(const float value);

		/**
		 * Enable/disable mac controller
		 * @param enable bool true of false
		 */
		void setEnableMotorController(const bool enable) { mEnableMacController = enable; }

		/**
		 * @return returns object points in local space
		 */
		const std::vector<glm::vec3>& getObjectPoints() const { return mObjectPoints; }

		/**
		 * @return returns frame points in local space
		 */
		const std::vector<glm::vec3>& getFramePoints() const { return mFramePoints; }

		/**
		 * @return returns const reference of calculated motorsteps
		 */
		const std::vector<double>& getMotorSteps() const { return mMotorSteps; }

		/**
		 * @return returns slack range
		 */
		const float getSlackRange() const { return mSlackRange; }

		/**
		 * @return returns slack minimum
		 */
		const float getSlackMinimum() const { return mSlackMinimum; }

		/**
		 * @param index of motor in flexblock space
		 * @return returns motor override
		 */
		const float getMotorOverride(const int index) const { return mMotorOverrides[index]; }

		/**
		 * @return returns motor override minimum
		 */
		const float getMotorOverrideMinimum() const { return mOverrideMinimum; }

		/**
		 * @return returns motor override range
		 */
		const float getMotorOverrideRange() const { return mOverrideRange; }

		/**
		 * @return returns motor amplitude range
		 */
		const float getSinusAmplitudeRange() const { return mSinusAmplitudeRange; }

		/**
		 * @return returns motor frequency range
		 */
		const float getSinusFrequencyRange() const { return mSinusFrequencyRange; }

		/**
		 * @return returns motor steps per meter
		 */
		const double getMotorStepsPerMeter() const { return mMotorStepsPerMeter; }

		/**
		 * @return returns reference to motor mapping vector
		 */
		const std::vector<int>& getMotorMapping() const { return mMotorMapping; }

		/**
		 * Returns true if motor is enabled
		 */
		const bool getEnableMotorController() const { return mEnableMacController; }
	protected:
		FrameMesh* mFrameMesh = nullptr;
		FlexBlockMesh* mFlexBlockMesh = nullptr;
		
		bool mEnableSerial;

		// Initialize flexblock unique ptr to null
		std::unique_ptr<Flex> mFlexLogic = nullptr;

		//
		double mUpdateSerialTime = 0.0;
		
		std::vector<glm::vec3> mObjectPoints = std::vector<glm::vec3>(8);

		std::vector<glm::vec3> mFramePoints = std::vector<glm::vec3>(8);

		std::vector<float> mMotorInputs = std::vector<float>(8);

		std::vector<float> mMotorOverrides = std::vector<float>(8);

		std::vector<double> mMotorSteps;

		std::vector<int> mMotorMapping;

		double mMotorStepsPerMeter;

		int mMotorStepOffset;

		float mSlackRange;

		float mSlackMinimum;

		float mOverrideRange = 1000.0f;

		float mOverrideMinimum = 0.0f;

		float mSinusAmplitudeRange = 1.0f;

		float mSinusFrequencyRange = 100.0f;

		float mSinusAmplitude = 0.0f;

		float mSinusFrequency = 0.0f;

		int mFlexFrequency = 1000;

		double mTime = 0.0;

		MACController* mMacController;

		bool mEnableMacController;

		bool mEnableDigitalPin;
	};
}
