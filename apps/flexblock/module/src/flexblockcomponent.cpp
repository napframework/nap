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
	RTTI_PROPERTY("FlexBlockShape", &nap::FlexBlockComponent::mFlexBlockShape, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockDevice",&nap::FlexBlockComponent::mFlexBlockDevice, nap::rtti::EPropertyMetaData::Required)
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

		mSlackMinimum = resource->mSlackMinimum;
		mSlackRange = resource->mSlackRange;;
		mSinusAmplitudeRange = resource->mSinusAmplitudeRange;
		mSinusFrequencyRange = resource->mSinusFrequencyRange;
		mOverrideMinimum = resource->mOverrideMinimum;
		mOverrideRange = resource->mOverrideRange;

		// calculate new frame
		mFlexblockDevice->getFramePoints(mFramePoints);

		// set points
		mFrameMesh->setFramePoints(mFramePoints, mObjectPoints);

		return true;
	}


	void FlexBlockComponentInstance::setInput(int index, float value)
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		flexInput.setInput(index, value);
		mFlexblockDevice->setInput(flexInput);
	}
	

	void FlexBlockComponentInstance::setOverride(int index, const float value)
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		flexInput.setOverride(index, (value * mOverrideRange) + mOverrideMinimum);
		mFlexblockDevice->setInput(flexInput);
	}


	void FlexBlockComponentInstance::setSinusAmplitude(float value)
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		flexInput.mSinusAmplitude = value * mSinusAmplitudeRange;
		mFlexblockDevice->setInput(flexInput);
	}


	float FlexBlockComponentInstance::getMotorOverride(int index) const
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		assert(index < flexInput.mOverrides.size());
		return flexInput.mOverrides[index];
	}


	void FlexBlockComponentInstance::setSinusFrequency(float value)
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		flexInput.mSinusFrequency = value * mSinusFrequencyRange;
		mFlexblockDevice->setInput(flexInput);
	}


	void FlexBlockComponentInstance::setSlack(float value)
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		flexInput.mSlack = value * mSlackRange + mSlackMinimum;
		mFlexblockDevice->setInput(flexInput);
	}


	void FlexBlockComponentInstance::getRopeLengths(std::vector<float>& output)
	{
		mFlexblockDevice->getRopeLengths(output);
	}
	

	float FlexBlockComponentInstance::getSlack() const
	{
		FlexInput flexInput;
		mFlexblockDevice->getInput(flexInput);
		return flexInput.mSlack;
	}


	void FlexBlockComponentInstance::update(double deltaTime)
	{
		// get points of objects
		mFlexblockDevice->getObjectPoints(mObjectPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(mObjectPoints);
		
		// update the box
		mFlexBlockMesh->setControlPoints(mObjectPoints);
	}
}
