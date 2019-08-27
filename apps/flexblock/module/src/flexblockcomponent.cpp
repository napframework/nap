// local includes
#include "flexblockcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>
#include <math.h>
#include <glm/geometric.hpp>
#include <rtti/jsonreader.h>

// nap::FlexBlockComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockComponent)
	RTTI_PROPERTY("FrameMesh", &nap::FlexBlockComponent::mFrameMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockMesh", &nap::FlexBlockComponent::mFlexBlockMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SerialComponent", &nap::FlexBlockComponent::mFlexBlockSerialComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockShape", &nap::FlexBlockComponent::mFlexBlockShape, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Motor Steps Per Meter", &nap::FlexBlockComponent::mMotorStepsPerMeter, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Step Offset", &nap::FlexBlockComponent::mMotorOffset, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Range", &nap::FlexBlockComponent::mSlackRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Minimum", &nap::FlexBlockComponent::mSlackMinimum, nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("Sinus Amplitude", &nap::FlexBlockComponent::mSinusAmplitudeRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Frequency", &nap::FlexBlockComponent::mSinusFrequencyRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Range", &nap::FlexBlockComponent::mOverrideRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Minimum", &nap::FlexBlockComponent::mOverrideMinimum, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::FlexBlockComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexBlockComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void FlexBlockComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
	}


	FlexBlockComponentInstance::~FlexBlockComponentInstance()
	{
		//assert(mFlexLogic != nullptr);
		if (mFlexLogic != nullptr)
		{
			mFlexLogic->stop();
			mFlexLogic.reset(nullptr);
		}
	}


	bool FlexBlockComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexBlockComponent* resource = getComponent<FlexBlockComponent>();

		// assign resources
		mFlexBlockMesh = resource->mFlexBlockMesh.get();
		mFrameMesh = resource->mFrameMesh.get();
		mMotorStepsPerMeter = resource->mMotorStepsPerMeter;
		mMotorStepOffset = resource->mMotorOffset;
		mSlackMinimum = resource->mSlackMinimum;
		mSlackRange = resource->mSlackRange;
		mMotorStepOffset = resource->mMotorOffset;
		mSinusAmplitudeRange = resource->mSinusAmplitudeRange;
		mSinusFrequencyRange = resource->mSinusFrequencyRange;
		mSlackMinimum = resource->mSlackMinimum;
		mOverrideMinimum = resource->mOverrideMinimum;
		mOverrideRange = resource->mOverrideRange;

		// create flex logic
		mFlexLogic = std::make_unique<Flex>( resource->mFlexBlockShape.get() );
		
		// start flex logic thread
		mFlexLogic->start();

		// calculate new frame
		const std::vector<glm::vec3>& framePoints = mFlexLogic->getFramePoints();
		mFramePoints = framePoints;

		// set points
		mFrameMesh->setFramePoints(framePoints, mObjectPoints);

		// start serial
		mFlexBlockSerialComponentInstance->start(errorState);

		return true;
	}


	void FlexBlockComponentInstance::setMotorInput(const int index, float value)
	{
		// 
		mMotorInputs[index] = value;
	}


	void FlexBlockComponentInstance::setOverrides(const int index, const float value)
	{
		mMotorOverrides[index] = value * mOverrideRange;
	//	printf("%i : %f\n", index, value);
	}


	void FlexBlockComponentInstance::setSinusAmplitude(const float value)
	{
		mSinusAmplitude = value * mSinusAmplitudeRange;
	//	printf("%f\n", value);
	}


	void FlexBlockComponentInstance::setSinusFrequency(const float value)
	{
		mSinusFrequency = value * mSinusFrequencyRange;
	//	printf("%f\n", value);
	}


	void FlexBlockComponentInstance::setSlack(const float value)
	{
		mFlexLogic->setSlack(value * mSlackRange + mSlackMinimum);
	}


	void FlexBlockComponentInstance::update(double deltaTime)
	{
		// convert flex points to nap points
		const std::vector<glm::vec3>& objectPoints = mFlexLogic->getObjectPoints();
		mObjectPoints = objectPoints;

		// update ropes of frame
		mFrameMesh->setControlPoints(objectPoints);
		
		// update the box
		mFlexBlockMesh->setControlPoints(mObjectPoints);

		// update motors of flex algorithm
		mFlexLogic->setMotorInput(mMotorInputs);

		//
		const std::vector<float> ropeLengths = mFlexLogic->getRopeLengths();

		//
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
		mTime += deltaTime;
		float sinusValue = ((( cos(mTime * mSinusFrequency) * -1.0f ) * 0.5f ) + 0.5f ) * mSinusAmplitude ;
		//printf("%f\n", sinusValue);

		for (int i = 0; i < motorSteps.size(); i++)
		{
			motorSteps[i] += sinusValue;
		}

		//printf("%f\n", motorSteps[0]);

		for (int i = 0; i < motorSteps.size(); i++)
		{
			double a = motorSteps[i]; 
			a *= mMotorStepsPerMeter; 
			a -= mMotorStepOffset; 
			motorSteps[i] = a;
		}

		// update serial
		/*
		mUpdateSerialTime += deltaTime;
		if (mUpdateSerialTime > (double)mFlexBlockSerialComponentInstance->getUpdateIntervalMs() / 1000.0)
		{
			mUpdateSerialTime = 0.0;

			std::vector<float> motorSteps = mFlexLogic->getRopeLengths();

			for (int i = 0; i < motorSteps.size(); i++)
			{
				float a = motorSteps[i] * 1000.0f; // convert to millimeters
				a *= mMillimeterToMotorsteps; //  millimeters to motor steps
				a -= mMotorOffset; // step offset
				motorSteps[i] = a;
			}

			std::string data = "<";
			for (int i = 0; i < motorSteps.size(); i++)
			{
				long c = (long)motorSteps[i];
				data += std::to_string(c);
				if (i + 1 < motorSteps.size())
					data += "|";
			}
			data += ">";

			//printf("%s\n", data.c_str());
			mFlexBlockSerialComponentInstance->write(data);
		}
		*/
	}
}
