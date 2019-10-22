#pragma once

// Local Includes
#include "maccontroller.h"
#include "flexblockdata.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <mutex>

namespace nap
{
	// Forward Declares
	class FlexAdapter;

	/**
	 * The flexblock algorithm.
	 */
	class NAPAPI FlexDevice : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~FlexDevice() override;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the device
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the device
		 */
		virtual void stop() override;

		/**
		 * Returns the most recent calculated object points, this call is thread safe
		 * @param outPoints copy of the most recent calculated object points
		 */
		void getObjectPoints(std::vector<glm::vec3>& outPoints) const;

		/**
		 * Returns the computed rope lengths, this call is thread safe
		 * @param outLengths copy of the most recent calculated rope lengths
		 */
		void getRopeLengths(std::vector<float>& outLengths) const;

		/**
		 * Update individual motor inputs [0-8]. Thread safe.
		 * This call asserts if the input size doesn't match internally required length.
		 * @param inputs new motor inputs
		 */
		void setMotorInput(const std::vector<float>& inputs);

		/**
		 * Get currently used motor input values [0-8]. Thread safe.
		 * @param outInputs the currently used motor input values.
		 */
		void getMotorInput(std::vector<float>& outInputs) const;

		ResourcePtr<FlexBlockShape>				mFlexBlockShape;				///< Property: 'FlexBlockShape' Reference to the shape definition of the block.
		std::vector<ResourcePtr<FlexAdapter>>	mAdapters;						///< Property: 'Adapters' All flexblock adapters.

		// Properties
		int					mUpdateFrequency = 1000;		///< Property: 'Frequency' device operation frequency in hz.
		float				mOverrideMinimum = 0.0f;		///< Property: 'Override Minimum' minimum of override parameters in meters, we start to count from this value
		float				mSlack = 0.0f;					///< Property: 'Slack Range' 
		float				mSinusAmplitude = 0.0f;			///< Property: 'Sinus Amplitude'			
		float				mSinusFrequency = 0.0f;			///< Property: 'Sinus Frequency

	private:
		bool mStopCompute = false;							///< If the compute task should be stopped
		std::future<void> mComputeTask;						///< Compute background thread
		mutable std::mutex mPointMutex;						///< Mutex invoked when getting / setting points
		mutable std::mutex mMotorMutex;						///< Mutex invoked when getting / setting motor input
		mutable std::mutex mRopesMutes;						///< Mutex invoked when getting / setting final rope length

		/**
		 * Computes / Cooks the flexblock algorithm
		 * Runs on a separate thread and is managed by the mComputeTask future
		 */
		void compute();

		//////////////////////////////////////////////////////////////////////////
		// Flex-block compute specific variables
		//////////////////////////////////////////////////////////////////////////

		// statics
		static const float sForceObject;
		static const float sForceObject2Frame;
		static const float sForceObjectSpring;

		// Points
		std::vector<glm::vec3> mPointsObject;
		std::vector<glm::vec3> mPointsFrame;

		// Number of inputs
		int mCountInputs;
		std::vector<float> mMotorInput = std::vector<float>(8);

		// Elements
		std::vector<std::vector<int>> mElementsObject;
		std::vector<std::vector<int>> mElementsObject2Frame;
		std::vector<std::vector<int>> mElementsFrame;

		// What is computed
		std::vector<glm::vec3> mPoints;
		std::vector<float> mRopes;
		std::vector<std::vector<int>> mElements;

		// Intermediate storage results
		std::vector<glm::vec3>	mElementsVector;
		std::vector<float> mElementsLength;
		std::vector<float> mElementsLengthRef;
		std::vector<float> mElementsLengthDelta;
		std::vector<float> mElementsInput;
		std::vector<glm::vec3> mPointChange;
		std::vector<glm::vec3> mPointChangeCorr;
		float mMotorSpd = 0.0f;
		float mMotorAcc = 0.0f;
		glm::vec3 mPointForce = {0,0,0};
		glm::vec3 mPointForceCorr = {0,0,0};

		//////////////////////////////////////////////////////////////////////////
		// Flex-block logic
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Calculates the change of the element lengths in reference to start position
		 */
		void calcDeltaLengths();

		/**
		 * Calculates the new forces on the elements
		 */
		void calcElements();

		/**
		 * Makes a single list of all values of mElementsObject, mElementsObject2Frame and mElementsFrame
		 */
		void concatElements();

		/**
		 * Makes a single list of all values of mPointsObject and mPointsFrame
		 */
		void concatPoints();

		/**
		 * Applies motor input/force to elements
		 */
		void setMotorInputInternal(std::vector<float>& inputs);

		/**
		 * Collects the ids of elements connected to a point
		 * @param id point id
		 * @param outIDs a vector containing the element ids
		 */
		void getIdsOfSuspensionElementsOnPoint(int id, std::vector<int> &outIDs);

		/**
		 * Calculates force on opposite point of element
		 * @param elidx element id
		 * @param point point id
		 * @param outVec the calculated suspension force
		 */
		void getSuspensionForceOnPointOfElement(int elidx, int point, glm::vec3& outVec);

		/**
		 * Calculates force on opposite point of element
		 * @param object_element_id element id
		 * @param opposite_column the column opposite of the object_element_id
		 * @param outVec the calculated suspension force
		 */
		void getProjectedSuspensionForcesOnOppositePointOfElement(int object_element_id, int opposite_column, glm::vec3& outVec);

		/**
		 * Calculates force on opposite point of element
		 * @param object_element_id element id
		 * @param suspension_element_id suspension element id
		 * @param opposite_point the opposite point id of object_element_id
		 * @param outVec the calculated suspension force
		 */
		void getProjectedSuspensionForceOnOppositePointOfElement(int object_element_id, int suspension_element_id, int opposite_point, glm::vec3& outVec);

		/**
		 * Calculates vector of force applied to element
		 * @param elidx element index
		 * @param direction direction of force ( -1 or 1 )
		 * @param outVec the calculated vector
		 */
		void getObjectElementForceOfElement(int elidx, int direction, glm::vec3& outVec);

		/**
		 * Computes rope output length including slack
		 * @param outLengths the rope output length including slacks
		 */
		void calcRopeLengths(std::vector<float>& outLengths);
	};
}