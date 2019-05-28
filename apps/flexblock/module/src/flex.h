#pragma once

#include <memory>
#include <vector>
#include <stddef.h>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>

#include "flexblockdata.h"

namespace nap
{
	class Flex
	{
	public:
		Flex(std::shared_ptr<FlexblockShape> flexblockShape, std::shared_ptr<FlexblockSize> flexblockSize);
	
		void setMotorInput(int index, float value);

		const std::vector<glm::vec3> getObjectPoints() const
		{
			return mPoints;
		}
		const std::vector<glm::vec3>& getFramePoints() const
		{
			return mPointsFrame;
		}

		void update(double deltaTime);
	protected:
		glm::vec3 getObjectElementForceOfElement(int elidx, int direction);
		
		glm::vec3 getProjectedSuspensionForcesOnOppositePointOfElement(int object_element_id, int opposite_column);
		
		glm::vec3 getProjectedSuspensionForceOnOppositePointOfElement(int object_element_id, int suspension_element_id, int opposite_point);

		glm::vec3 getSuspensionForceOnPointOfElement(int elidx, int point);

		std::vector<int> getIdsOfSuspensionElementsOnPoint(int id);

		void calcInput();

		void calcElements();

		void concatElements();

		void concatPoints();

		void setInput(std::vector<float> inputs);
	protected:
		std::thread updateThread;
		std::mutex updatedPointsMutex;
		std::vector<glm::vec3> updatedPoints;

		float mForceObject = 10.0f;
		float mForceObjectSpring = 0.02f;
		float mForceObject2Frame = 2.0f;

		float mMaxAcc;
		float mMaxSpeed;

		float mLengthError = 0;

		long mFrequency;

		std::shared_ptr<FlexblockShape> mObjShape;
		std::shared_ptr<FlexblockSize> mObjSize;
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
