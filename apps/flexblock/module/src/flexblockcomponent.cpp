#include "FlexBlockComponent.h"
#include "rtti/jsonreader.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>
#include <math.h>

// json
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>
#include <utility/fileutils.h>

#include <glm/geometric.hpp>

// nap::FlexBlockComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockComponent)
	RTTI_PROPERTY("FrameMesh", &nap::FlexBlockComponent::mFrameMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockMesh", &nap::FlexBlockComponent::mFlexBlockMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SerialComponent", &nap::FlexBlockComponent::mFlexBlockSerialComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockShape", &nap::FlexBlockComponent::mFlexBlockShape, nap::rtti::EPropertyMetaData::Required)
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

		// create flex logic
		mFlexLogic = std::make_unique<Flex>( resource->mFlexBlockShape.get() );
		
		// start flex logic thread
		mFlexLogic->start();

		// calculate new frame
		const std::vector<glm::vec3>& framePoints = mFlexLogic->getFramePoints();
		toNapPoints(framePoints, mFramePoints);

		// set points
		mFrameMesh->setFramePoints(mFramePoints);

		// start serial
		mFlexBlockSerialComponentInstance->start(errorState);

		return true;
	}

	void FlexBlockComponentInstance::SetMotorInput(int index, float value)
	{
		// 
		mMotorInputs[remapMotorInput(index)] = value;
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		// convert flex points to nap points
		const std::vector<glm::vec3>& objectPoints = mFlexLogic->getObjectPoints();
		toNapPoints(objectPoints, mObjectPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(mObjectPoints);
		
		// update the box
		mFlexBlockMesh->setControlPoints(mObjectPoints);

		// update motors of flex algorithm
		mFlexLogic->setMotorInput(mMotorInputs);

		// update serial
		mUpdateSerialTime += deltaTime;
		if (mUpdateSerialTime > (double)mFlexBlockSerialComponentInstance->getUpdateIntervalMs() / 1000.0)
		{
			mUpdateSerialTime = 0.0;

			std::vector<float> ropeLengths = mFlexLogic->getRopeLengths();
			std::string data = "<";
			for (int i = 0; i < ropeLengths.size(); i++)
			{
				long c = (long)ropeLengths[remapMotorInput(i)];
				data += std::to_string(c);
				if (i + 1 < ropeLengths.size())
					data += "|";
			}
			data += ">";

			mFlexBlockSerialComponentInstance->write(data);
		}
	}


	void FlexBlockComponentInstance::toNapPoints(const std::vector<glm::vec3>& inPoints, std::vector<glm::vec3>& outPoints )
	{
		outPoints[0] = inPoints[7];
		outPoints[1] = inPoints[6];
		outPoints[2] = inPoints[4];
		outPoints[3] = inPoints[5];

		outPoints[4] = inPoints[2];
		outPoints[5] = inPoints[3];
		outPoints[6] = inPoints[1];
		outPoints[7] = inPoints[0];
	}

	const int FlexBlockComponentInstance::remapMotorInput(const int index) const
	{
		const static std::vector<int> map =
		{
			7,
			6,
			2,
			3,
			4,
			5,
			1,
			0
		};

		return map[index];
	}
}