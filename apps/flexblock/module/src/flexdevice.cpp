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
	RTTI_PROPERTY("Slack Minimum",		&nap::FlexDevice::mSlack,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Amplitude",	&nap::FlexDevice::mSinusAmplitude,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Frequency",	&nap::FlexDevice::mSinusFrequency,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Minimum",	&nap::FlexDevice::mOverrideMinimum,		nap::rtti::EPropertyMetaData::Default)
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

		// points
		mPointsObject = mFlexBlockShape->mPoints->mObject;
		mPointsFrame  = mFlexBlockShape->mPoints->mFrame;

		// elements
		mElementsObject = mFlexBlockShape->mElements->mObject;
		mElementsObject2Frame = mFlexBlockShape->mElements->mObject2Frame;
		mElementsFrame = mFlexBlockShape->mElements->mFrame;

		// convert zero indexed elements
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

		for (auto i = 0; i < mPointsFrame.size(); i++)
		{
			mPointsFrame[i].x *= mFlexBlockShape->mSize->mValues->mFrame.x * 0.5f;
			mPointsFrame[i].y *= mFlexBlockShape->mSize->mValues->mFrame.y * 0.5f;
			mPointsFrame[i].z *= mFlexBlockShape->mSize->mValues->mFrame.z * 0.5f;
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


	void FlexDevice::getObjectPoints(std::vector<glm::vec3>& outPoints)
	{
		// Copy points thread-safe
		std::lock_guard<std::mutex> lock(mPointMutex);
		outPoints = mPoints;
	}


	void FlexDevice::setMotorInput(const std::vector<float>& inputs)
	{
		// Copy motor input thread-safe
		std::lock_guard<std::mutex> lock(mMotorMutex);
		assert(inputs.size() == mMotorInput.size());
		mMotorInput = inputs;
	}


	void FlexDevice::getMotorInput(std::vector<float>& outInputs)
	{
		std::lock_guard<std::mutex> lock(mMotorMutex);
		outInputs = mMotorInput;
	}


	void FlexDevice::compute()
	{
		// Compute sleep time in microseconds 
		float sleep_time_microf = 1000.0f / static_cast<float>(mUpdateFrequency);
		long  sleep_time_micro = static_cast<long>(sleep_time_microf * 1000.0f);

		// Some variables that get updated regularly.
		std::vector<float> inputs(8);
		std::vector<int> suspensionElementIds;

		// Compute loop
		while (!mStopCompute)
		{
			// Update motor input internal algorithm state
			setMotorInputInternal(inputs);

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

			}

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
		// Get current input
		getMotorInput(inputs);

		// Convert
		for (int i = 0; i < inputs.size(); i++)
			mElementsInput[i] = (inputs[i] + 0.2f) * sForceObject2Frame;
	}


	void FlexDevice::getIdsOfSuspensionElementsOnPoint(int id, std::vector<int> &outIDs)
	{
		outIDs.clear();

		// Find the INDEX of the point_id in the array of elements_object2frame
		// Add the length of the elements_object array(because the arrays are concatenated)
		for (int i = 0; i < mElementsObject2Frame.size(); i++)
		{
			if (mElementsObject2Frame[i][0] == id)
				outIDs.emplace_back(i + mElementsObject.size());
		}
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
}