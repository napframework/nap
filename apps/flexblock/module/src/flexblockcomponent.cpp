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
	RTTI_PROPERTY("NormalsMesh", &nap::FlexBlockComponent::mNormalsMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SerialPort", &nap::FlexBlockComponent::mSerialPort, nap::rtti::EPropertyMetaData::Default)
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


	bool FlexBlockComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexBlockComponent* resource = getComponent<FlexBlockComponent>();

		// assign resources
		mFlexBlockMesh = resource->mFlexBlockMesh;
		mFrameMesh = resource->mFrameMesh;
		mNormalsMesh = resource->mNormalsMesh;
		mSerialPort = resource->mSerialPort;

		// Read & parse json files
		std::vector<FlexblockSizePtr> sizes = flexreader::readSizes("sizes.json", errorState);
		std::vector<FlexblockShapePtr> shapes = flexreader::readShapes("shapes.json", errorState);

		// create flex logic
		mFlexLogic = std::make_shared<Flex>( shapes[0], sizes[0] );
		
		// start flex logic thread
		mFlexLogic->start();

		// calculate new frame
		const std::vector<glm::vec3>& framePoints = mFlexLogic->getFramePoints();
		std::vector<glm::vec3> frame = toNapPoints(framePoints);

		// set points
		mFrameMesh->setFramePoints(frame);

		return true;
	}

	void FlexBlockComponentInstance::SetMotorInput(int index, float value)
	{
		mFlexLogic->setMotorInput(index, value);
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		const std::vector<glm::vec3>& objectPoints = mFlexLogic->getObjectPoints();
		std::vector<glm::vec3>& points = toNapPoints(objectPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(points);
		
		// update the box
		mFlexBlockMesh->setControlPoints(points);

		// update normals mesh
		utility::ErrorState errorState;
		mNormalsMesh->calculateNormals(errorState, true);

		// do serial stuff
		std::vector<float> ropeLengths = mFlexLogic->getRopeLengths();
		mWriteBuffer = "<";
		for (const float& ropeLength : ropeLengths)
		{
			int l = (int)ropeLength;
			mWriteBuffer.append(std::to_string(l));
			mWriteBuffer.append("|");
		}
		mWriteBuffer.append(">");

		SerialPort::Error error;
		//mSerialPort->write(mWriteBuffer, error);
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
			points[0],
		};
	}
}