#include "flex.h"

#include <mathutils.h>
#include <glm/geometric.hpp>

#define MOTORSTEPS 12.73239

namespace nap
{
	Flex::Flex(FlexblockShapePtr flexblockShape, FlexblockSizePtr flexblockSize)
	{
		mObjShape = flexblockShape;
		mObjSize = flexblockSize;

		mMotorInput = std::vector<float>(8);

		// 
		mFrequency = 200;
		mObjShape = flexblockShape;
		mObjSize = flexblockSize;
		mCountInputs = mObjShape->inputs;

		//
		mForceObject = 10;
		mForceObjectSpring = 0.02;
		mForceObject2Frame = 2;

		mLengthError = 0;

		// points
		mPointsObject = mObjShape->points.object;
		mPointsFrame = mObjShape->points.frame;

		// elements
		mElementsObject = mObjShape->elements.object;
		mElementsObject2Frame = mObjShape->elements.object2frame;
		mElementsFrame = mObjShape->elements.frame;

		// convert zero indexed elements
		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			mElementsObject2Frame[i][1] += mPointsObject.size();
		}
		for (int i = 0; i < mElementsFrame.size(); i++)
		{
			mElementsFrame[i][0] += mPointsObject.size();
			mElementsFrame[i][1] += mPointsObject.size();
		}

		// convert unit points to real size
		for (int i = 0; i < mPointsObject.size(); i++)
		{
			mPointsObject[i].x *= mObjSize->values.object.x * 0.5f;
			mPointsObject[i].y *= mObjSize->values.object.y * 0.5f;
			mPointsObject[i].z *= mObjSize->values.object.z * 0.5f;
		}
		for (int i = 0; i < mPointsFrame.size(); i++)
		{
			mPointsFrame[i].x *= mObjSize->values.frame.x * 0.5f;
			mPointsFrame[i].y *= mObjSize->values.frame.y * 0.5f;
			mPointsFrame[i].z *= mObjSize->values.frame.z * 0.5f;
		}

		// init
		mPoints = std::vector<glm::vec3>(mPointsObject.size() + mPointsFrame.size());
		mElements = std::vector<std::vector<int>>(mElementsObject.size() + mElementsObject2Frame.size());
		for (int i = 0; i < mElements.size(); i++)
		{
			mElements[i] = std::vector<int>(2);
		}

		mElementsAll = std::vector<std::vector<int>>(mElementsObject.size() + mElementsObject2Frame.size() + mElementsFrame.size());
		mElementsVector = std::vector<glm::vec3>(mElements.size());
		mElementsLength = std::vector<float>(mElements.size());
		mElementsLengthRef = std::vector<float>(mElements.size());
		mElementsObjectLength = std::vector<float>(mElements.size());
		mElementsInput = std::vector<float>(mCountInputs);
		mPointChange = std::vector<glm::vec3>(mPointsObject.size());
		mPointChangeCorr = std::vector<glm::vec3>(mPointsObject.size());
		mElementIndices = std::vector<int>(2);
		mElementIndices[0] = mElementsObject.size();
		mElementIndices[1] = mElementsObject.size() + mElementsObject2Frame.size();
		mOverride = std::vector<float>(4); 

		// concat points
		concatPoints();

		// concat elements
		concatElements();

		// calc elements
		calcElements();

		mElementsLengthRef = std::vector<float>(mElementsLength.size());
		for (int i = 0; i < mElementsLength.size(); i++)
		{
			mElementsLengthRef[i] = mElementsLength[i];
		}

		// calc input
		calcInput();
	}

	Flex::~Flex()
	{
		stop();
	}

	void Flex::start()
	{
		if (!mIsRunning)
		{
			mIsRunning = true;
			mUpdateThread = std::thread(&Flex::update, this);
		}
	}

	void Flex::stop()
	{
		if (mIsRunning)
		{
			mIsRunning = false;
			mUpdateThread.join();
		}
	}

	void Flex::setMotorInput(int index, float value)
	{
		std::lock_guard<std::mutex> l(mMotorInputMutex);
		mMotorInput[index] = value;
	}

	void Flex::update()
	{
		while (mIsRunning)
		{
			{
				std::lock_guard<std::mutex> l(mMotorInputMutex);
				setInput(mMotorInput);
			}

			calcInput();

			for (int i = 0; i < mPointsObject.size(); i++)
			{
				// init
				mPointForce = glm::vec3(0, 0, 0);
				mPointForceCorr = glm::vec3(0, 0, 0);

				// external forces
				std::vector<int> suspensionElementIds;
				getIdsOfSuspensionElementsOnPoint(i, suspensionElementIds);

				for (int j = 0; j < suspensionElementIds.size(); j++)
				{
					glm::vec3 force;
					getSuspensionForceOnPointOfElement(suspensionElementIds[j], i, force);
					mPointForce += force;
				}

				// Internal forces + suspension forces on the other side of connected elements
				// Get the connected elements(both directions) :

				std::vector<int> objectElementIds0;
				for (int j = 0; j < mElementsObject.size(); j++)
				{
					if (mElementsObject[j][0] == i)
					{
						objectElementIds0.push_back(j);
					}
				}

				std::vector<int> objectElementIds1;
				for (int j = 0; j < mElementsObject.size(); j++)
				{
					if (mElementsObject[j][1] == i)
					{
						objectElementIds1.push_back(j);
					}
				}

				// Forces due to external force on other points
				for (int j = 0; j < objectElementIds0.size(); j++)
				{
					glm::vec3 force;
					getProjectedSuspensionForcesOnOppositePointOfElement(objectElementIds0[j], 1, force);
					mPointForce += force;

					getObjectElementForceOfElement(objectElementIds0[j], 1, force);
					mPointForceCorr += force;
				}

				for (int j = 0; j < objectElementIds1.size(); j++)
				{
					glm::vec3 force;
					
					getProjectedSuspensionForcesOnOppositePointOfElement(objectElementIds1[j], 0, force);
					mPointForce += force;
					
					getObjectElementForceOfElement(objectElementIds1[j], -1, force);
					mPointForceCorr += force;
				}

				// add change to change
				mPointChange[i] = mPointForce;
				mPointChangeCorr[i] = mPointChangeCorr[i] + mPointForceCorr * 0.2f;
			}

			// add damping
			for (int j = 0; j < mPointChangeCorr.size(); j++)
			{
				mPointChangeCorr[j] *= 0.95f;
			}

			// update position
			for (int j = 0; j < mPointsObject.size(); j++)
			{
				mPointsObject[j] += mPointChange[j] * 0.01f + mPointChangeCorr[j] * 0.2f ;
			}

			concatPoints();
			calcElements();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / mFrequency));
		}
	}

	void Flex::setInput(std::vector<float> inputs)
	{
		for (int i = 0; i < inputs.size(); i++)
		{
			inputs[i] += 0.2f; // why ??
		}

		for (int i = 0; i < inputs.size(); i++)
		{
			mElementsInput[i] = inputs[i] * mForceObject2Frame;
		}
	}

	void Flex::getObjectElementForceOfElement(int elidx, int direction, glm::vec3& outVec)
	{
		outVec = mElementsLengthDelta[elidx] * mForceObject * (float)direction * mElementsVector[elidx];
	}

	void Flex::getProjectedSuspensionForcesOnOppositePointOfElement(int objectElementId, int oppositeColumn, glm::vec3& outVec)
	{
		// Predicted force due to force on other point
		int oppositePoint = mElementsObject[objectElementId][oppositeColumn];
		
		std::vector<int> suspensionElementIds;
		getIdsOfSuspensionElementsOnPoint(oppositePoint, suspensionElementIds);

		int suspensionElementId = suspensionElementIds[0];
		getProjectedSuspensionForceOnOppositePointOfElement(objectElementId, suspensionElementId, oppositePoint, outVec);
	}

	void Flex::getProjectedSuspensionForceOnOppositePointOfElement(int objectElementId, int suspensionElementId, int opposite_point, glm::vec3& outVec)
	{
		glm::vec3 v1 = mElementsVector[objectElementId];
		glm::vec3 v2;
		getSuspensionForceOnPointOfElement(suspensionElementId, opposite_point, v2);

		float d = glm::dot(v1, v2);
		outVec = d * mElementsVector[objectElementId];
	}

	void Flex::getSuspensionForceOnPointOfElement(int elidx, int point, glm::vec3& outVec)
	{
		outVec = mElementsLength[elidx] * mElementsVector[elidx] * mElementsInput[point];
	}

	void Flex::getIdsOfSuspensionElementsOnPoint(int pointId, std::vector<int> &outIDs)
	{
		outIDs.clear();

		// Find the INDEX of the point_id in the array of elements_object2frame
		// Add the length of the elements_object array(because the arrays are concatenated)
		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			if (mElementsObject2Frame[i][0] == pointId)
			{
				outIDs.push_back(i + mElementsObject.size());
			}
		}
	}

	void Flex::calcInput()
	{
		mElementsLengthDelta = std::vector<float>(mElementsLength.size());
		for (int i = 0; i < mElementsLengthDelta.size(); i++)
		{
			mElementsLengthDelta[i] = mElementsLength[i] - mElementsLengthRef[i];
		}
	}

	void Flex::calcElements()
	{
		for (int i = 0; i < mElements.size(); i++)
		{
			glm::vec3 p = mPoints[mElements[i][1]] - mPoints[mElements[i][0]];
			mElementsVector[i] = p;
		}

		float sum = 0.0f;
		std::vector<float> elementsLength(mElementsLength.size());
		for (int i = 0; i < mElementsVector.size(); i++)
		{
			elementsLength[i] = glm::length(mElementsVector[i]);
		}

		float motorSpd = 0.0f;
		float a = 0.0f;
		for (int i = 12; i < 19; i++)
		{
			float n_a = math::abs(mElementsLength[i] - elementsLength[i]);
			if (n_a > a)
				a = n_a;
		}
		motorSpd = a * mFrequency;

		mMotorAcc = (mMotorSpd - motorSpd) * mFrequency;

		mElementsLength = elementsLength;
		for (int i = 0; i < mElementsVector.size(); i++)
		{
			mElementsVector[i] /= mElementsLength[i];
		}
	}

	void Flex::concatElements()
	{
		// elements
		std::vector<std::vector<int>> newElements;
		for (int i = 0; i < mElementsObject.size(); i++)
		{
			newElements.push_back(mElementsObject[i]);
		}

		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			newElements.push_back(mElementsObject2Frame[i]);
		}
		mElements = newElements;

		// elements_all
		std::vector<std::vector<int>> newElementsAll;
		for (int i = 0; i < mElementsObject.size(); i++)
		{
			newElementsAll.push_back(mElementsObject[i]);
		}

		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			newElementsAll.push_back(mElementsObject2Frame[i]);
		}

		for (int i = 0; i < mElementsFrame.size(); i++)
		{
			newElementsAll.push_back(mElementsFrame[i]);
		}
		mElementsAll = newElementsAll;
	}

	void Flex::concatPoints()
	{
		std::vector<glm::vec3> newPoints;
		for (int i = 0; i < mPointsObject.size(); i++)
		{
			newPoints.push_back(mPointsObject[i]);
		}

		for (int i = 0; i < mPointsFrame.size(); i++)
		{
			newPoints.push_back(mPointsFrame[i]);
		}

		mPoints = newPoints;
	}

	std::vector<float> Flex::getRopeLengths()
	{
		std::vector<float> ropes;
		for (int i = 12; i < 20; i++)
		{
			ropes.push_back(mElementsLength[i]);
		}

		for (float& ropeLength : ropes)
		{
			ropeLength *= 1000.0f * MOTORSTEPS - 7542.0f;
		}
		
		return ropes;
	}
}