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
	RTTI_PROPERTY("ControlPointsMesh", &nap::FlexBlockComponent::mControlPointsMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameMesh", &nap::FlexBlockComponent::mFrameMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockMesh", &nap::FlexBlockComponent::mFlexBlockMesh, nap::rtti::EPropertyMetaData::Required)
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

		// assign meshes
		mControlPointsMesh = resource->mControlPointsMesh;
		mFlexBlockMesh = resource->mFlexBlockMesh;
		mFrameMesh = resource->mFrameMesh;

		// Read & parse json files
		// TODO : handle errors...
		auto sizes = flexreader::readSizes("sizes.json", errorState);
		auto shapes = flexreader::readShapes("shapes.json", errorState);

		mFlexLogic = std::make_shared<Flex>( shapes[0], sizes[0] );

		const std::vector<glm::vec3>& framePoints = mFlexLogic->getFramePoints();
		std::vector<glm::vec3> frame = toNapPoints(framePoints);

		mFrameMesh->setFramePoints(frame);

		return true;
	}

	void FlexBlockComponentInstance::SetMotorInput(int index, float value)
	{
		mFlexLogic->setMotorInput(index, value);
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		mFlexLogic->update(deltaTime);

		const std::vector<glm::vec3>& objectPoints = mFlexLogic->getObjectPoints();
		std::vector<glm::vec3> points = toNapPoints(objectPoints);

		// update the control points mesh
		mControlPointsMesh->setControlPoints(points);

		// update ropes of frame
		mFrameMesh->setControlPoints(points);
		
		// update the box
		mFlexBlockMesh->setControlPoints(points);
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