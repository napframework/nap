#include "flex.h"

#include <mathutils.h>
#include <glm/geometric.hpp>

#define REF_FREQ 200.0f

namespace nap
{
	Flex::Flex(std::shared_ptr<FlexblockShape> flexblockShape, std::shared_ptr<FlexblockSize> flexblockSize)
	{
		// Read & parse json files
		// TODO : handle errors...
		//readShapes();
		//readSizes();

		mObjShape = flexblockShape;
		mObjSize = flexblockSize;

		mMotorInput = std::vector<float>(8);

		// TODO : define this somewhere else
		mFrequency = 200;
		mObjShape = flexblockShape;
		mObjSize = flexblockSize;
		mCountInputs = mObjShape->inputs;

		//
		mForceObject = 10;
		mForceObjectSpring = 0.02;
		mForceObject2Frame = 2;

		mMaxAcc = 2.0 / mFrequency;
		mMaxSpeed = 5.0 / mFrequency;

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

	void Flex::setMotorInput(int index, float value)
	{
		mMotorInput[index] = value;
		setInput(mMotorInput);
	}

	void Flex::update(double deltaTime)
	{
		// calculate points
		// this is where the magic happens

		mFrequency = (int)(1.0 / deltaTime);

		printf("%i\n", mFrequency);

		mMaxAcc = 2.0 / mFrequency;
		mMaxSpeed = 5.0 / mFrequency;

		mForceObject = (10 * REF_FREQ) / mFrequency;
		mForceObjectSpring = (0.02f * REF_FREQ) / mFrequency;
		mForceObject2Frame = (2.0f * REF_FREQ) / mFrequency;

		/*
		mForceObject = 10;
		mForceObjectSpring = 0.02;
		mForceObject2Frame = 2;
		mChangeSpeed = 1;

		mMaxAcc = 2.0 / mFrequency;
		mMaxSpeed = 5.0 / mFrequency;
		*/

		calcInput();

		for (int i = 0; i < mPointsObject.size(); i++)
		{
			// init
			mPointForce = glm::vec3(0, 0, 0);
			mPointForceCorr = glm::vec3(0, 0, 0);

			// external forces
			std::vector<int> suspensionElementIds = getIdsOfSuspensionElementsOnPoint(i);

			for (int j = 0; j < suspensionElementIds.size(); j++)
			{
				mPointForce += getSuspensionForceOnPointOfElement(suspensionElementIds[j], i);
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
				mPointForce += getProjectedSuspensionForcesOnOppositePointOfElement(objectElementIds0[j], 1);
				mPointForceCorr += getObjectElementForceOfElement(objectElementIds0[j], 1);
			}

			for (int j = 0; j < objectElementIds1.size(); j++)
			{
				mPointForce += getProjectedSuspensionForcesOnOppositePointOfElement(objectElementIds1[j], 0);
				mPointForceCorr += getObjectElementForceOfElement(objectElementIds1[j], -1);
			}

			// add change to change
			mPointChange[i] = mPointForce;
			mPointChangeCorr[i] = mPointChangeCorr[i] + mPointForceCorr * ( (0.2f * REF_FREQ ) / mFrequency );
		}

		// add damping
		for (int j = 0; j < mPointChangeCorr.size(); j++)
		{
			mPointChangeCorr[j] *= ( 0.95f * REF_FREQ ) / mFrequency;
		}

		// update position
		for (int j = 0; j < mPointsObject.size(); j++)
		{
			mPointsObject[j] += mPointChange[j] * ( ( 0.01f * REF_FREQ ) / mFrequency ) 
				+ mPointChangeCorr[j] * ( ( 0.2f * REF_FREQ ) / mFrequency );
		}

		concatPoints();
		calcElements();
	}

	void Flex::setInput(std::vector<float> inputs)
	{
		float tot = 0.0f;
		for (int i = 0; i < inputs.size(); i++)
		{
			inputs[i] += 0.2f; // why ??
			tot += inputs[i];
		}

		for (int i = 0; i < inputs.size(); i++)
		{
			mElementsInput[i] = inputs[i] * mForceObject2Frame;
		}
	}

	glm::vec3 Flex::getObjectElementForceOfElement(int elidx, int direction)
	{
		return mElementsLengthDelta[elidx] * mForceObject * (float)direction * mElementsVector[elidx];
	}

	glm::vec3 Flex::getProjectedSuspensionForcesOnOppositePointOfElement(int object_element_id, int opposite_column)
	{
		// Predicted force due to force on other point
		int oppositePoint = mElementsObject[object_element_id][opposite_column];
		auto suspensionElementIds = getIdsOfSuspensionElementsOnPoint(oppositePoint);

		int suspensionElementId = suspensionElementIds[0];

		auto tot = getProjectedSuspensionForceOnOppositePointOfElement(object_element_id, suspensionElementId, oppositePoint);

		return tot;
	}

	glm::vec3 Flex::getProjectedSuspensionForceOnOppositePointOfElement(int object_element_id, int suspension_element_id, int opposite_point)
	{
		auto v1 = mElementsVector[object_element_id];
		auto v2 = getSuspensionForceOnPointOfElement(suspension_element_id, opposite_point);

		auto d = glm::dot(v1, v2);
		auto r = d * mElementsVector[object_element_id];

		return r;
	}

	glm::vec3 Flex::getSuspensionForceOnPointOfElement(int elidx, int point)
	{
		return mElementsLength[elidx] * mElementsVector[elidx] * mElementsInput[point];
	}

	std::vector<int> Flex::getIdsOfSuspensionElementsOnPoint(int point_id)
	{
		// Find the INDEX of the point_id in the array of elements_object2frame
		// Add the length of the elements_object array(because the arrays are concatenated)
		std::vector<int> returnVector;
		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			if (mElementsObject2Frame[i][0] == point_id)
			{
				returnVector.push_back(i + mElementsObject.size());
			}
		}

		return returnVector;
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
		std::vector<float> n_elements_length(mElementsLength.size());
		for (int i = 0; i < mElementsVector.size(); i++)
		{
			n_elements_length[i] = glm::length(mElementsVector[i]);
		}

		float n_motorspd = 0.0f;
		float a = 0.0f;
		for (int i = 12; i < 19; i++)
		{
			float n_a = math::abs(mElementsLength[i] - n_elements_length[i]);
			if (n_a > a)
				a = n_a;
		}
		n_motorspd = a * mFrequency;

		mMotorAcc = (mMotorSpd - n_motorspd) * mFrequency;

		mElementsLength = n_elements_length;
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
}