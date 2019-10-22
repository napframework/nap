// Local Includes
#include "flexdevice.h"
#include "flexadapter.h"

// External Includes
#include <thread>
#include <nap/logger.h>
#include <mathutils.h>

// nap::flexdevice run time class definition 
RTTI_BEGIN_CLASS(nap::FlexDevice)
	RTTI_PROPERTY("FlexBlockShape",		&nap::FlexDevice::mFlexBlockShape,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Frequency",			&nap::FlexDevice::mUpdateFrequency,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack",				&nap::FlexDevice::mSlack,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Adapters",			&nap::FlexDevice::mAdapters,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


// Statics
const float nap::FlexDevice::sForceObject		= 10.0f;
const float nap::FlexDevice::sForceObjectSpring = 0.02f;
const float nap::FlexDevice::sForceObject2Frame = 2.0f;

namespace nap
{
	FlexDevice::~FlexDevice()			{ }


	bool FlexDevice::init(utility::ErrorState& errorState)
	{
		// Number of inputs
		mCountInputs = mFlexBlockShape->mMotorCount;

		// elements
		mElementsObject = mFlexBlockShape->mElements->mObject;
		mElementsObject2Frame = mFlexBlockShape->mElements->mObject2Frame;
		mElementsFrame = mFlexBlockShape->mElements->mFrame;

		// convert zero indexed elements
		mPointsObject = mFlexBlockShape->mPoints->mObject;
		for (auto i = 0; i < mElementsObject2Frame.size(); i++)
			mElementsObject2Frame[i][1] += mPointsObject.size();

		for (auto  i = 0; i < mElementsFrame.size(); i++)
		{
			mElementsFrame[i][0] += mPointsObject.size();
			mElementsFrame[i][1] += mPointsObject.size();
		}

		// convert unit points to real size
		for (auto i = 0; i < mPointsObject.size(); i++)
		{
			mPointsObject[i].x *= mFlexBlockShape->mSize->mValues->mObject.x * 0.5f;
			mPointsObject[i].y *= mFlexBlockShape->mSize->mValues->mObject.y * 0.5f;
			mPointsObject[i].z *= mFlexBlockShape->mSize->mValues->mObject.z * 0.5f;
		}

		mPointsFrame = mFlexBlockShape->mPoints->mFrame;
		for (auto& point : mPointsFrame)
		{
			point.x *= mFlexBlockShape->mSize->mValues->mFrame.x * 0.5f;
			point.y *= mFlexBlockShape->mSize->mValues->mFrame.y * 0.5f;
			point.z *= mFlexBlockShape->mSize->mValues->mFrame.z * 0.5f;
		}

		// init elements
		auto element_count = mElementsObject.size() + mElementsObject2Frame.size();
		mElements = std::vector<std::vector<int>>(element_count, std::vector<int>(2));
		mElementsVector = std::vector<glm::vec3>(element_count);
		mElementsLength = std::vector<float>(element_count);
		mElementsLengthRef = std::vector<float>(element_count);
		mElementsInput = std::vector<float>(mCountInputs);
		
		// init points
		mPoints = std::vector<glm::vec3>(mPointsObject.size() + mPointsFrame.size());
		mPointChange = std::vector<glm::vec3>(mPointsObject.size());
		mPointChangeCorr = std::vector<glm::vec3>(mPointsObject.size());

		// Update slack
		mInput.mSlack = mSlack;

		// concat points
		concatPoints();

		// concat elements
		concatElements();

		// calc elements
		calcElements();
		mElementsLengthRef = mElementsLength;

		// calc input
		calcDeltaLengths();

		return true;
	}


	bool FlexDevice::start(utility::ErrorState& errorState)
	{
		assert(!mComputeTask.valid());
		mStopCompute = false;
		mComputeTask = std::async(std::launch::async, std::bind(&FlexDevice::compute, this));
		return true;
	}


	void FlexDevice::stop()
	{
		mStopCompute = true;
		if (mComputeTask.valid())
		{
			mComputeTask.wait();
		}
	}


	void FlexDevice::getObjectPoints(std::vector<glm::vec3>& outPoints) const
	{
		// Copy points thread-safe
		std::lock_guard<std::mutex> lock(mPointMutex);
		outPoints = mPoints;
	}


	void FlexDevice::getFramePoints(std::vector<glm::vec3>& outPoints) const
	{
		outPoints = mPointsFrame;
	}


	void FlexDevice::getRopeLengths(std::vector<float>& outLengths) const
	{
		std::lock_guard<std::mutex> lock(mRopesMutes);
		outLengths = mRopes;
	}


	void FlexDevice::setInput(const FlexInput& input)
	{
		// Copy motor input thread-safe
		std::lock_guard<std::mutex> lock(mInputMutex);
		mInput = input;
	}


	void FlexDevice::getInput(FlexInput& outInput) const
	{
		std::lock_guard<std::mutex> lock(mInputMutex);
		outInput = mInput;
	}


	void FlexDevice::compute()
	{
		// Compute sleep time in microseconds 
		float sleep_time_microf = 1000.0f / static_cast<float>(mUpdateFrequency);
		long  sleep_time_micro = static_cast<long>(sleep_time_microf * 1000.0f);

		// Some variables that get updated regularly.
		FlexInput input;
		std::vector<int> suspensionElementIds;
		glm::vec3 force;
		std::vector<int> objectElementIds0;
		std::vector<int> objectElementIds1;
		std::vector<float> rope_lengths(8);

		// Compute loop
		while (!mStopCompute)
		{
			// Get current input arguments
			getInput(input);

			// Update motor input internal algorithm state
			setMotorInputInternal(input.mInputs);

			// Calculate delta length
			calcDeltaLengths();

			// Calculate output
			for (auto i = 0; i < mPointsObject.size(); i++)
			{
				// Init
				mPointForce		= {0,0,0};
				mPointForceCorr = {0,0,0};

				// external forces
				getIdsOfSuspensionElementsOnPoint(i, suspensionElementIds);
				for (int j = 0; j < suspensionElementIds.size(); j++)
				{
					getSuspensionForceOnPointOfElement(suspensionElementIds[j], i, force);
					mPointForce += force;
				}

				// Internal forces + suspension forces on the other side of connected elements
				// Get the connected elements(both directions)
				objectElementIds0.clear();
				for (int j = 0; j < mElementsObject.size(); j++)
				{
					if (mElementsObject[j][0] == i)
						objectElementIds0.emplace_back(j);
				}

				objectElementIds1.clear();
				for (int j = 0; j < mElementsObject.size(); j++)
				{
					if (mElementsObject[j][1] == i)
						objectElementIds1.emplace_back(j);
				}

				// Forces due to external force on other points
				for (int j = 0; j < objectElementIds0.size(); j++)
				{
					getProjectedSuspensionForcesOnOppositePointOfElement(objectElementIds0[j], 1, force);
					mPointForce += force;

					getObjectElementForceOfElement(objectElementIds0[j], 1, force);
					mPointForceCorr += force;
				}

				for (int j = 0; j < objectElementIds1.size(); j++)
				{
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
			for (auto& j : mPointChangeCorr)
				j *= 0.95f;

			// update position
			for (int j = 0; j < mPointsObject.size(); j++)
				mPointsObject[j] += mPointChange[j] * 0.01f + mPointChangeCorr[j] * 0.2f;

			// Concatenate points and calculate length of elements
			concatPoints();
			calcElements();

			// Calculate final algorithm output
			calcRopeLengths(input.mSlack, rope_lengths);

			// Now call adapters!
			for (auto& adapter: mAdapters)
				adapter->compute(*this);

			// Wait update time
			// TODO: Do we need to consider the actual compute time of the algorithm here?
			// TODO: If so we need to subtract computation time from update frequency.
			std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_micro));
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Flex-block logic
	//////////////////////////////////////////////////////////////////////////

	void FlexDevice::calcDeltaLengths()
	{
		mElementsLengthDelta = std::vector<float>(mElementsLength.size());
		for (int i = 0; i < mElementsLengthDelta.size(); i++)
		{
			mElementsLengthDelta[i] = mElementsLength[i] - mElementsLengthRef[i];
		}
	}


	void FlexDevice::calcElements()
	{
		for (int i = 0; i < mElements.size(); i++)
		{
			glm::vec3 p = mPoints[mElements[i][1]] - mPoints[mElements[i][0]];
			mElementsVector[i] = p;
		}

		std::vector<float> elementsLength(mElementsLength.size());
		for (int i = 0; i < mElementsVector.size(); i++)
			elementsLength[i] = glm::length(mElementsVector[i]);

		float motorSpd = 0.0f;
		float a = 0.0f;
		for (int i = 12; i < 19; i++)
		{
			float new_a = math::abs(mElementsLength[i] - elementsLength[i]);
			a = new_a > a ? new_a : a;
		}

		motorSpd = a * static_cast<float>(mUpdateFrequency);
		mMotorAcc = (mMotorSpd - motorSpd) * static_cast<float>(mUpdateFrequency);

		mElementsLength = elementsLength;
		for (int i = 0; i < mElementsVector.size(); i++)
			mElementsVector[i] /= mElementsLength[i];
	}


	void FlexDevice::concatElements()
	{
		mElements = mElementsObject;
		mElements.reserve(mElementsObject.size() + mElementsObject2Frame.size());
		mElements.insert(mElements.end(), mElementsObject2Frame.begin(), mElementsObject2Frame.end());
	}


	void FlexDevice::setMotorInputInternal(std::vector<float>& inputs)
	{
		// Convert
		for (int i = 0; i < inputs.size(); i++)
			mElementsInput[i] = (inputs[i] + 0.2f) * sForceObject2Frame;
	}


	void FlexDevice::getIdsOfSuspensionElementsOnPoint(int id, std::vector<int> &outIDs)
	{
		// Find the INDEX of the point_id in the array of elements_object2frame
		// Add the length of the elements_object array(because the arrays are concatenated)
		outIDs.clear();
		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			if (mElementsObject2Frame[i][0] == id)
				outIDs.emplace_back(i + mElementsObject.size());
		}
	}


	void FlexDevice::getSuspensionForceOnPointOfElement(int elidx, int point, glm::vec3& outVec)
	{
		outVec = mElementsLength[elidx] * mElementsVector[elidx] * mElementsInput[point];
	}


	void FlexDevice::getProjectedSuspensionForcesOnOppositePointOfElement(int objectElementId, int oppositeColumn, glm::vec3& outVec)
	{
		// Predicted force due to force on other point
		int oppositePoint = mElementsObject[objectElementId][oppositeColumn];

		std::vector<int> suspensionElementIds;
		getIdsOfSuspensionElementsOnPoint(oppositePoint, suspensionElementIds);

		int suspensionElementId = suspensionElementIds[0];
		getProjectedSuspensionForceOnOppositePointOfElement(objectElementId, suspensionElementId, oppositePoint, outVec);
	}


	void FlexDevice::getProjectedSuspensionForceOnOppositePointOfElement(int objectElementId, int suspensionElementId, int opposite_point, glm::vec3& outVec)
	{
		glm::vec3 v1 = mElementsVector[objectElementId];
		glm::vec3 v2;
		getSuspensionForceOnPointOfElement(suspensionElementId, opposite_point, v2);

		float d = glm::dot(v1, v2);
		outVec = d * mElementsVector[objectElementId];
	}
	

	void FlexDevice::getObjectElementForceOfElement(int elidx, int direction, glm::vec3& outVec)
	{
		outVec = mElementsLengthDelta[elidx] * sForceObject * (float)direction * mElementsVector[elidx];
	}


	void FlexDevice::calcRopeLengths(float slack, std::vector<float>& outLengths)
	{
		outLengths.clear();
		for (int i = 12; i < 20; i++)
			outLengths.emplace_back(mElementsLength[i + 1] + slack);
		
		// Copy lengths as output value
		std::lock_guard<std::mutex> lock(mRopesMutes);
		mRopes = outLengths;
	}


	void nap::FlexDevice::concatPoints()
	{
		// Create new points
		std::vector<glm::vec3> newPoints = mPointsObject;
		newPoints.insert(newPoints.end(), mPointsFrame.begin(), mPointsFrame.end());

		// Copy points thread-safe
		std::lock_guard<std::mutex> lock(mPointMutex);
		mPoints = newPoints;
	}


	void FlexInput::setInput(int index, float value)
	{
		assert(index < mInputs.size());
		mInputs[index] = value;
	}


	void FlexInput::setOverride(int index, float value)
	{
		assert(index < mOverrides.size());
		mOverrides[index] = value;
	}

}