// Local Includes
#include "flex.h"

// External Includes
#include <mathutils.h>
#include <glm/geometric.hpp>

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	Flex::Flex(
		FlexBlockShape* flexblockShape,
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
		bool enableDigitalPin)
	{
		mObjShape = flexblockShape;

		// 
		mFrequency = frequency;
		mObjShape = flexblockShape;
		mOverrideMinimum = overrideMinimum;
		mOverrideRange = overrideRange;
		mSlackRange = slackRange;
		mSinusAmplitude = sinusAmplitude;
		mSinusFrequency = sinusFrequency;
		mMotorStepsPerMeter = motorStepsPerMeter;
		mMotorStepOffset = motorStepOffset;
		mEnableMacController = enableMacController;
		mMacController = macController;
		mMotorMapping = motorMapping;
		mEnableDigitalPin = enableDigitalPin;

		//
		mCountInputs = mObjShape->mMotorCount;

		//
		mForceObject = 10;
		mForceObjectSpring = 0.02;
		mForceObject2Frame = 2;

		mLengthError = 0;

		// points
		mPointsObject = mObjShape->mPoints->mObject;
		mPointsFrame = mObjShape->mPoints->mFrame;

		// elements
		mElementsObject = mObjShape->mElements->mObject;
		mElementsObject2Frame = mObjShape->mElements->mObject2Frame;
		mElementsFrame = mObjShape->mElements->mFrame;

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
			mPointsObject[i].x *= mObjShape->mSize->mValues->mObject.x * 0.5f;
			mPointsObject[i].y *= mObjShape->mSize->mValues->mObject.y * 0.5f;
			mPointsObject[i].z *= mObjShape->mSize->mValues->mObject.z * 0.5f;
		}
		for (int i = 0; i < mPointsFrame.size(); i++)
		{
			mPointsFrame[i].x *= mObjShape->mSize->mValues->mFrame.x * 0.5f;
			mPointsFrame[i].y *= mObjShape->mSize->mValues->mFrame.y * 0.5f;
			mPointsFrame[i].z *= mObjShape->mSize->mValues->mFrame.z * 0.5f;
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
		calcDeltaLengths();
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


	void Flex::setMotorInput(const std::vector<float>& inputs)
	{
		assert(inputs.size() == mMotorInput.size());
		for (int i = 0; i < inputs.size(); i++)
		{
			mMotorInput[i] = inputs[i];
		}
	}

	void Flex::setMotorOverrides(const std::vector<float>& overrides)
	{
		assert(overrides.size() == mMotorOverrides.size());
		for (int i = 0; i < overrides.size(); i++)
		{
			mMotorOverrides[i] = overrides[i];
		}
	}

	void Flex::setSlack(const float value)
	{
		mSlack = value;
	}

	void Flex::setSinusAmplitude(const float value)
	{
		mSinusAmplitude = value;
	}

	void Flex::setSinusFrequency(const float value)
	{
		mSinusFrequency = value;
	}

	void Flex::copyMotorInput(std::vector<float>& outputs)
	{
		for (int i = 0; i < mMotorInput.size(); i++)
		{
			outputs[i] = mMotorInput[i];
		}
	}


	void Flex::update()
	{
		auto sleepTime = std::chrono::milliseconds(1000 / mFrequency);
		auto prevTime = std::chrono::steady_clock::now();
		double time = 0.0;

		while (mIsRunning)
		{
			std::vector<float> inputs(8);
			copyMotorInput(inputs);

			setMotorInputInternal(inputs);

			calcDeltaLengths();

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
				mPointsObject[j] += mPointChange[j] * 0.01f + mPointChangeCorr[j] * 0.2f;
			}

			concatPoints();
			calcElements();

			// now handle motors
			// get copy of the ropelengths
			const std::vector<float> ropeLengths = getRopeLengths();

			// copy them to motorsteps
			std::vector<double> motorSteps(8);
			for (int i = 0; i < ropeLengths.size(); i++)
			{
				motorSteps[i] = ropeLengths[i];
			}

			// overrides
			for (int i = 0; i < mMotorOverrides.size(); i++)
			{
				motorSteps[i] += mMotorOverrides[i] + mOverrideMinimum;
			}

			// sinus
			auto timeNow = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(timeNow - prevTime).count();
			prevTime = timeNow;
			time += (double)((double)elapsed / 100000.0);
			float sinusValue = (((cos(time * mSinusFrequency) * -1.0f) * 0.5f) + 0.5f) * mSinusAmplitude;

			//printf("%f\n", time);

			for (int i = 0; i < motorSteps.size(); i++)
			{
				motorSteps[i] += sinusValue;
			}

			// convert meters to motorsteps
			for (int i = 0; i < motorSteps.size(); i++)
			{
				double a = motorSteps[i];
				a *= mMotorStepsPerMeter;
				a -= mMotorStepOffset;
				motorSteps[i] = a;
			}

			for (int i = 0; i < motorSteps.size(); i++)
			{
				mMotorSteps[i] = motorSteps[i];
			}

			//
			if (mEnableMacController)
			{
				// are we running ?
				if (mMacController->isRunning())
				{
					// check for all slaves to be operational and without errors
					// if not, stop immediatly
					bool allSlavesOperational = true;
					for (int i = 0; i < mMacController->getSlaveCount(); i++)
					{
						if (mMacController->getSlaveState(i) != EtherCATMaster::ESlaveState::Operational)
						{
							allSlavesOperational = false;
							printf("Slave %i not operational! Stopping MACController... \n", i);
							mMacController->stop();
							break;
						}
					}

					// we are ok, continue
					if (allSlavesOperational)
					{
						// Create the new position data array
						std::vector<MacPosition> position_data;
						mMacController->copyPositionData(position_data);

						// check if we have as much slaves as motors
						for (int i = 0; i < mMacController->getSlaveCount(); i++)
						{
							if (i < mMotorMapping.size())
							{
								// get the mapped motor index
								int mapped = mMotorMapping[i];
								if (mapped < mMacController->getSlaveCount())
								{
									// Update target position
									position_data[i].setTargetPosition(motorSteps[mapped]);

									// when digital pin enable, only enable it when we are giving meters
									// this occurs when targetmeters is higher then the motors curent position
									if (mEnableDigitalPin)
									{
										double targetMeters = (double)position_data[i].mTargetPosition / mMotorStepsPerMeter;
										double currentMeters = (double)mMacController->getActualPosition(i) / mMotorStepsPerMeter;

										bool activateDigitalPin = targetMeters - currentMeters > 0.02;
										if (activateDigitalPin != position_data[i].getDigitalPin(0))
										{
											position_data[i].setDigitalPin(0, activateDigitalPin);
											//printf("%s\n", activateDigitalPin ? "on" : "off");
										}
									}
								}
							}
						}
						mMacController->setPositionData(position_data);
					}
				}
			}

			std::this_thread::sleep_for(sleepTime);
		}
	}


	void Flex::setMotorInputInternal(std::vector<float>& inputs)
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


	void Flex::calcDeltaLengths()
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

		std::vector<float> elementsLength(mElementsLength.size());
		for (int i = 0; i < mElementsVector.size(); i++)
		{
			elementsLength[i] = glm::length(mElementsVector[i]);
		}

		float motorSpd = 0.0f;
		float a = 0.0f;
		for (int i = 12; i < 19; i++)
		{
			float new_a = math::abs(mElementsLength[i] - elementsLength[i]);
			if (new_a > a)
				a = new_a;
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


	const std::vector<float> Flex::getRopeLengths() const
	{
		std::vector<float> ropes;
		for (int i = 12; i < 20; i++)
		{
			ropes.push_back(mElementsLength[i + 1] + mSlack);
		}

		return ropes;
	}
}