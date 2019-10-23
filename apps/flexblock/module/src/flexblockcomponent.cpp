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
	RTTI_PROPERTY("FrameMesh",		&nap::FlexBlockComponent::mFrameMesh,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockMesh",	&nap::FlexBlockComponent::mFlexBlockMesh,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockDevice",&nap::FlexBlockComponent::mFlexBlockDevice, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Enable Serial",	&nap::FlexBlockComponent::mEnableSerial,	nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("Mac Controller", &nap::FlexBlockComponent::mMacController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Enable Controller", &nap::FlexBlockComponent::mEnableMacController, nap::rtti::EPropertyMetaData::Required)

	RTTI_PROPERTY("FlexBlockShape", &nap::FlexBlockComponent::mFlexBlockShape, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Motor Steps Per Meter", &nap::FlexBlockComponent::mMotorStepsPerMeter, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Step Offset", &nap::FlexBlockComponent::mMotorOffset, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Range", &nap::FlexBlockComponent::mSlackRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Minimum", &nap::FlexBlockComponent::mSlackMinimum, nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("Sinus Amplitude", &nap::FlexBlockComponent::mSinusAmplitudeRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Frequency", &nap::FlexBlockComponent::mSinusFrequencyRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Range", &nap::FlexBlockComponent::mOverrideRange, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Minimum", &nap::FlexBlockComponent::mOverrideMinimum, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Mapping", &nap::FlexBlockComponent::mMotorMapping, nap::rtti::EPropertyMetaData::Default)

	RTTI_PROPERTY("Flex Thread Frequency", &nap::FlexBlockComponent::mFlexFrequency, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable Digital Pin", &nap::FlexBlockComponent::mEnableDigitalPin, nap::rtti::EPropertyMetaData::Default)
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

	}


	bool FlexBlockComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexBlockComponent* resource = getComponent<FlexBlockComponent>();

		// assign resources
		mFlexBlockMesh = resource->mFlexBlockMesh.get();
		mFrameMesh = resource->mFrameMesh.get();
		mFlexblockDevice = resource->mFlexBlockDevice.get();
		mMacController = resource->mMacController.get();

		mEnableMacController = resource->mEnableMacController;
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
		mMotorMapping = resource->mMotorMapping;
		mEnableSerial = resource->mEnableSerial;
		mFlexFrequency = resource->mFlexFrequency;
		mEnableDigitalPin = resource->mEnableDigitalPin;

		// calculate new frame
		mFlexblockDevice->getFramePoints(mFramePoints);

		// set points
		mFrameMesh->setFramePoints(mFramePoints, mObjectPoints);

		return true;
	}


	void FlexBlockComponentInstance::setInput(int index, float value)
	{
		mFlexInput.setInput(index, value);
	}
	

	void FlexBlockComponentInstance::setOverride(int index, const float value)
	{
		mFlexInput.setOverride(index, (value * mOverrideRange) + mOverrideMinimum);
	}


	void FlexBlockComponentInstance::setSinusAmplitude(float value)
	{
		mFlexInput.mSinusAmplitude = value * mSinusAmplitudeRange;
	}


	float FlexBlockComponentInstance::getMotorOverride(int index) const
	{
		assert(index < mFlexInput.mOverrides.size());
		return mFlexInput.mOverrides[index];
	}


	void FlexBlockComponentInstance::setSinusFrequency(float value)
	{
		mFlexInput.mSinusFrequency = value * mSinusFrequencyRange;
	}


	void FlexBlockComponentInstance::setSlack(float value)
	{
		mFlexInput.mSlack = value * mSlackRange + mSlackMinimum;
	}
	

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		// get points of objects
		mFlexblockDevice->getObjectPoints(mObjectPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(mObjectPoints);
		
		// update the box
		mFlexBlockMesh->setControlPoints(mObjectPoints);

		// update input of flex algorithm
		mFlexblockDevice->setInput(mFlexInput);
	}
}
