#pragma once

#include <memory>
#include <vector>
#include <stddef.h>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>
#include <atomic>

#include "flexblockdata.h"

namespace nap
{
	class Flex
	{
	public:
		Flex(FlexblockShapePtr flexblockShape);
		~Flex();

		/**
		* Update motor speed [0..8]
		* @param index index of motor [0..8]
		* @param value motor input 0-1
		*/
		void setMotorInput(int index, float value);

		/**
		* Starts the logic thread
		*/
		void start();

		/**
		* Stops the logic thread
		*/
		void stop();

		/**
		* Returns true if logic thread is running
		*/
		bool getIsRunning()
		{
			return mIsRunning;
		}

		std::vector<float> getRopeLengths();

		/**
		* Returns copy of latest calculate object points
		*/
		std::vector<glm::vec3> getObjectPoints()
		{
			return mPoints;
		}

		/**
		* Returns copy of frame points
		*/
		std::vector<glm::vec3> getFramePoints()
		{
			return mPointsFrame;
		}
	protected:
		void getObjectElementForceOfElement(int elidx, int direction, glm::vec3& outVec);
		
		void getProjectedSuspensionForcesOnOppositePointOfElement(int object_element_id, int opposite_column, glm::vec3& outVec);
		
		void getProjectedSuspensionForceOnOppositePointOfElement(int object_element_id, int suspension_element_id, int opposite_point, glm::vec3& outVec);

		void getSuspensionForceOnPointOfElement(int elidx, int point, glm::vec3& outVec);

		void getIdsOfSuspensionElementsOnPoint(int id, std::vector<int> &outIDs);

		void calcInput();

		void calcElements();

		void concatElements();

		void concatPoints();

		void setInput(std::vector<float> inputs);

		void update();
	protected:
		std::atomic_bool mIsRunning = false;
		std::thread mUpdateThread;
		std::mutex mMotorInputMutex;

		float mForceObject = 10.0f;
		float mForceObjectSpring = 0.02f;
		float mForceObject2Frame = 2.0f;

		float mLengthError = 0;

		long mFrequency;

		std::shared_ptr<FlexblockShape> mObjShape;
		FlexblockShapeSize mObjSize;
		int mInputs;
		int mCountInputs;

		std::vector<glm::vec3> mPointsObject;
		std::vector<glm::vec3> mPointsFrame;

		std::vector<std::vector<int>> mElementsObject;
		std::vector<std::vector<int>> mElementsObject2Frame;
		std::vector<std::vector<int>> mElementsFrame;

		std::vector<FlexblockShape> mShapes;
		std::vector<FlexblockSize> mSizes;

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
