#pragma once

#include <memory>
#include <vector>
#include <stddef.h>
#include <glm/glm.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

#include "flexblockdata.h"

namespace nap
{
	class Flex
	{
	public:
		Flex(FlexBlockShape* flexblockShape);
		~Flex();

		/**
		 * Update motor speed [0..8]
		 * @param index index of motor [0..8]
		 * @param value motor input 0-1
		 */
		void setMotorInput(const std::vector<float>& inputs);

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
		const bool getIsRunning() const  { return mIsRunning; }

		/**
		 * @return a vector containing calculated rope lengths
		 */
		const std::vector<float> getRopeLengths() const;

		/**
		 * @return reference to to latest calculate object points
		 */
		const std::vector<glm::vec3>& getObjectPoints() const { return mPoints; }

		/**
		 * @return reference to of frame points
		 */
		const std::vector<glm::vec3>& getFramePoints() const { return mPointsFrame; }
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
		void setMotorInputInternal(std::vector<float>& inputs);

		/**
		 * Update thread
		 */
		void update();

		/**
		 * Copies motor input from another thread to given reference
		 */
		void copyMotorInput(std::vector<float>& outputs);
	protected:

        std::atomic_bool mIsRunning = { false };
		std::thread mUpdateThread;
		std::mutex mMotorInputMutex;

		float mForceObject			= 10.0f;
		float mForceObjectSpring	= 0.02f;
		float mForceObject2Frame	= 2.0f;

		float mLengthError = 0;

		long mFrequency;

		FlexBlockShape* mObjShape;
		int mInputs;
		int mCountInputs;

		std::vector<glm::vec3> mPointsObject;
		std::vector<glm::vec3> mPointsFrame;

		std::vector<std::vector<int>> mElementsObject;
		std::vector<std::vector<int>> mElementsObject2Frame;
		std::vector<std::vector<int>> mElementsFrame;

		float mEndTime = 0.0f;
		std::vector<glm::vec3> mPoints;
		std::vector<std::vector<int>> mElements;
		std::vector<std::vector<int>> mElementsAll;
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

		std::vector<float> mOverride;
		float mSlack = 0.0f;

		float mMotorSpd = 0.0f;
		float mMotorAcc = 0.0f;

		std::vector<float> mElementsLengthDelta;
		std::vector<float> mMotorInput;
	};
}
