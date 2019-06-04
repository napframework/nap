#include "FlexBlockComponent.h"
#include "rtti/jsonreader.h"
#include "flexreader.h"

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

		// Read & parse json files
		std::vector<FlexblockShapePtr> shapes = flexreader::readShapes("shapes.json", errorState);

		// create flex logic
		mFlexLogic = std::make_unique<Flex>( shapes[0] );
		
		// start flex logic thread
		mFlexLogic->start();

		// calculate new frame
		const std::vector<glm::vec3>& framePoints = mFlexLogic->getFramePoints();
		std::vector<glm::vec3> frame = toNapPoints(framePoints);

		// set points
		mFrameMesh->setFramePoints(frame);

		//
		mFlexBlockSerialComponentInstance->start(errorState);

		return true;
	}

	void FlexBlockComponentInstance::SetMotorInput(int index, float value)
	{
		//
		mFlexLogic->setMotorInput(index, value);
	}

	void FlexBlockComponentInstance::SetInput(int index, int value)
	{
		static std::vector<int> inputs = { 0,0,0,0,0,0,0,0 };

		inputs[index] = value;
		
		std::string data = "<";
		for (int i : inputs)
		{
			data.append(std::to_string(i));
			data.append("|");
		}
		data.append(">");

		//printf(data.c_str());
		//printf("\n");
		//mFlexBlockSerialComponentInstance->write(data);
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		const std::vector<glm::vec3>& objectPoints = mFlexLogic->getObjectPoints();
		std::vector<glm::vec3>& points = toNapPoints(objectPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(points);
		
		// update the box
		mFlexBlockMesh->setControlPoints(points);

		//
		mUpdateSerialTime += deltaTime;
		if (mUpdateSerialTime > (double)mFlexBlockSerialComponentInstance->getUpdateIntervalMs() / 1000.0)
		{
			mUpdateSerialTime = 0.0;

			std::vector<float> ropeLengths = mFlexLogic->getRopeLengths();
			std::string data = "<";
			for (int i = 0; i < ropeLengths.size(); i++)
			{
				long c = (long)ropeLengths[i];
				data.append(std::to_string(c));
				if( i + 1 < ropeLengths.size())
					data.append("|");
			}
			data.append(">");

			//printf(data.c_str());
			//printf("\n");
			mFlexBlockSerialComponentInstance->write(data);
		}
	}

	std::vector<glm::vec3> FlexBlockComponentInstance::toNapPoints(const std::vector<glm::vec3>& points)
	{
		return std::vector<glm::vec3>
		{
			points[7],
			points[6],
			points[4],
			points[5],
			points[2],
			points[3],
			points[1],
			points[0]
		};
	}
}