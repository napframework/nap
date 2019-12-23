// Local Includes
#include "flexdevice.h"
#include "flexadapter.h"

// External Includes
#include <thread>
#include <nap/logger.h>
#include <nap/timer.h>
#include <nap/datetime.h>
#include <mathutils.h>

// nap::flexdevice run time class definition 
RTTI_BEGIN_CLASS(nap::FlexDevice)
	RTTI_PROPERTY("FlexBlockShape",		&nap::FlexDevice::mFlexBlockShape,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Frequency",			&nap::FlexDevice::mUpdateFrequency,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Scale",		&nap::FlexDevice::mSlackScale,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Min",			&nap::FlexDevice::mSlackMin,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Scale",		&nap::FlexDevice::mOverrideScale,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Min",		&nap::FlexDevice::mOverrideMin,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Frequency",	&nap::FlexDevice::mFrequencyRange,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Amplitude",	&nap::FlexDevice::mAmplitudeRange,		nap::rtti::EPropertyMetaData::Default)
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

		// concat points
		concatPoints(mPoints);

		// concat elements
		concatElements();

		// calc elements
		calcElements(mPoints);
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
		mTimer.reset();
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
		std::lock_guard<std::mutex> lock(mOutputMutex);
		outPoints = mPoints;
	}


	void FlexDevice::getFramePoints(std::vector<glm::vec3>& outPoints) const
	{
		outPoints = mPointsFrame;
	}


	void FlexDevice::getRopeLengths(std::vector<float>& outLengths) const
	{
		std::lock_guard<std::mutex> lock(mOutputMutex);
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
		std::vector<float> calc_ropes(8);
		std::vector<glm::vec3> calc_points(8);

		// Compute loop
		while (!mStopCompute)
		{
			// Compute loop delta-time in seconds
			double new_elapsed_time = mTimer.getElapsedTime();
			double delta_time = new_elapsed_time - mLastTimeStamp;
			mLastTimeStamp = new_elapsed_time;

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
			concatPoints(calc_points);
			calcElements(calc_points);

			// Calculate final algorithm output
			calcRopeLengths(delta_time, input, calc_ropes);

			// Update internal data, which can be used by external processes
			setData(calc_points, calc_ropes);
			
			// Now call adapters!
			for (auto& adapter: mAdapters)
				adapter->compute(*this, delta_time);

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


	void FlexDevice::calcElements(const std::vector<glm::vec3>& points)
	{
		for (int i = 0; i < mElements.size(); i++)
		{
			glm::vec3 p = points[mElements[i][1]] - points[mElements[i][0]];
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


	void FlexDevice::calcRopeLengths(double elapsed, const FlexInput& input, std::vector<float>& outLengths)
	{
		mSinTime += elapsed * (double)(input.mSinusFrequency * mFrequencyRange);

		outLengths.clear();
		for (int i = 12; i < 20; i++)
			outLengths.emplace_back(mElementsLength[i + 1] + input.mSlack * mSlackScale + mSlackMin);

		// Apply overrides
		for (auto i = 0; i < input.mOverrides.size(); i++)
			outLengths[i] += input.mOverrides[i] * mOverrideScale + mOverrideMin;

		// Apply sine wave using elapsed time in seconds
		float sin_value = static_cast<float>((((cos(mSinTime) * -1.0f)
			* 0.5f) 
			+ 0.5f) 
			* (double)(input.mSinusAmplitude));

		for (auto& length : outLengths)
			length += sin_value * mAmplitudeRange;

		//printf("%f		%f		%f		%f\n", mSinTime, outLengths[0], (double)(input.mSinusFrequency * mFrequencyRange), input.mSinusFrequency);
	}


	void FlexDevice::setData(const std::vector<glm::vec3>& points, const std::vector<float> lengths)
	{
		std::lock_guard<std::mutex> lock(mOutputMutex);
		mPoints = points;
		mRopes  = lengths;
	}


	void nap::FlexDevice::concatPoints(std::vector<glm::vec3>& outPoints)
	{
		// Create new points
		outPoints.clear();
		outPoints.insert(outPoints.end(), mPointsObject.begin(), mPointsObject.end());
		outPoints.insert(outPoints.end(), mPointsFrame.begin(), mPointsFrame.end());
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