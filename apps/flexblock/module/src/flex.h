#pragma once

#include <memory>
#include <vector>
#include <stddef.h>
#include <glm/glm.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <maccontroller.h>

#include "flexblockdata.h"

namespace nap
{
	class Flex
	{
	public:
		Flex(FlexBlockShape* flexblockShape,
			int frequency,
			float overrideMinimum,
			float slackRange,
			float overrideRange,
			float sinusAmplitude,
			float sinusFrequency,
			float motorStepsPerMeter,
			float motorStepOffset,
			bool enableMacController,
			MACController* macController,
			std::vector<int> motorMapping,
			bool enableDigitalPin);

		~Flex();

		/**
		 * Update motor speed [0..8]
		 * @param index index of motor [0..8]
		 * @param value motor input 0-1
		 */
		void setMotorInput(const std::vector<float>& inputs);

		/**
		* Set slack
		* @param value slack value, typically between -0.5 and 0.5
		*/
		void setSlack(const float value);

		/**
		*/
		void setSinusAmplitude(const float value);

		/**
		*/
		void setSinusFrequency(const float value);

		/**
		*
		*
		*/
		void setMotorOverrides(const std::vector<float>& inputs);

		/**
		* Starts the flex logic thread
		*/
		void start();

		/**
		* Stops the flex logic thread
		*/
		void stop();

		/**
		* @return true if logic thread is running
		*/
		const bool getIsRunning() const { return mIsRunning; }

		/**
		 * @return reference to to latest calculate object points
		 */
		const std::vector<glm::vec3>& getObjectPoints() const { return mPoints; }

		/**
		* @return reference to of frame points
		*/
		const std::vector<glm::vec3>& getFramePoints() const { return mPointsFrame; }

		/**
		*
		*/
		const std::vector<double> getMotorSteps() const
		{
			std::vector<double> motorSteps(8);
			for (int i = 0; i < motorSteps.size(); i++)
			{
				motorSteps[i] = mMotorSteps[i];
			}
			return motorSteps;
		}
	protected:
		/**
		* Calculates vector of force applied to element
		* @param elidx element index
		* @param direction direction of force ( -1 or 1 )
		* @param outVec the calculated vector
		*/
		void getObjectElementForceOfElement(int elidx, int direction, glm::vec3& outVec);

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
		* Calculates force on opposite point of element
		* @param elidx element id
		* @param point point id
		* @param outVec the calculated suspension force
		*/
		void getSuspensionForceOnPointOfElement(int elidx, int point, glm::vec3& outVec);

		/**
		* Collects the ids of elements connected to a point
		* @param id point id
		* @param outIDs a vector containing the element ids
		*/
		void getIdsOfSuspensionElementsOnPoint(int id, std::vector<int> &outIDs);

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
		void setMotorInputInternal(const std::vector<float>& inputs);

		/**
		* Update thread
		*/
		void update();

		/**
		* Copies motor input from another thread to given reference
		*/
		void copyMotorInput(std::vector<float>& outputs);

		/**
		 * @return a vector containing calculated rope lengths
		 */
		void getRopeLengths(std::vector <float> & outLengths) const;

	protected:

		std::atomic_bool mIsRunning = { false };
		std::thread mUpdateThread;

		float mForceObject = 10.0f;
		float mForceObjectSpring = 0.02f;
		float mForceObject2Frame = 2.0f;

		long mFrequency;

		FlexBlockShape* mObjShape;
		int mCountInputs;

		std::vector<glm::vec3> mPointsObject;
		std::vector<glm::vec3> mPointsFrame;

		std::vector<std::vector<int>> mElementsObject;
		std::vector<std::vector<int>> mElementsObject2Frame;
		std::vector<std::vector<int>> mElementsFrame;

		float mEndTime = 0.0f;
		std::vector<glm::vec3> mPoints;
		std::vector<std::vector<int>> mElements;
		std::vector<glm::vec3> mElementsVector;
		std::vector<float> mElementsLength;
		std::vector<float> mElementsLengthRef;
		std::vector<float> mElementsObjectLength;
		std::vector<float> mElementsInput;
		std::vector<glm::vec3> mPointChange;
		std::vector<glm::vec3> mPointChangeCorr;
		glm::vec3 mPointForce;
		glm::vec3 mPointForceCorr;
		std::vector<int> mElementIndices;

		float mMotorSpd = 0.0f;
		float mMotorAcc = 0.0f;

		std::vector<float> mElementsLengthDelta;

		// atomics
		std::atomic<float> mSlack;
		std::vector<std::atomic<float>> mMotorInput = std::vector<std::atomic<float>>(8);
		std::vector<std::atomic<float>> mMotorOverrides = std::vector<std::atomic<float>>(8);
		std::vector<std::atomic<float>> mMotorSteps = std::vector<std::atomic<float>>(8);
		std::atomic<float> mSinusAmplitude;
		std::atomic<float> mSinusFrequency;

		//
		float mOverrideMinimum;
		float mSlackRange;

		float mMotorStepsPerMeter;
		float mMotorStepOffset;
		bool mEnableMacController = false;
		bool mEnableDigitalPin = false;
		MACController* mMacController = nullptr;
		std::vector<int> mMotorMapping;
	};
}