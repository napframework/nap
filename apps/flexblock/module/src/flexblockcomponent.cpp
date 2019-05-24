#include "FlexBlockComponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>

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

		// init control points
		auto box = mFlexBlockMesh->getBox();
		auto min = box.getMin();
		auto max = box.getMax();

		mControlPoints = std::vector<glm::vec3>(8, glm::vec3(0, 0, 0));

		mControlPoints[0] = { min.x, min.y, max.z };	//< Front Lower left
		mControlPoints[1] = { max.x, min.y, max.z };	//< Front Lower right
		mControlPoints[2] = { min.x, max.y, max.z };	//< Front Top left
		mControlPoints[3] = { max.x, max.y, max.z };	//< Front Top right

		mControlPoints[4] = { max.x, min.y, min.z };	//< Back Lower left
		mControlPoints[5] = { min.x, min.y, min.z };	//< Back lower right
		mControlPoints[6] = { max.x, max.y, min.z }; //< Back Top left
		mControlPoints[7] = { min.x, max.y, min.z };	//< Back Top right

		return true;
	}

	void FlexBlockComponentInstance::SetControlPoint(int index, glm::vec3 position)
	{
		//
		mControlPoints[index] = position;

		// update the control points mesh
		mControlPointsMesh->setControlPoints(mControlPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(mControlPoints);

		// update the box
		mFlexBlockMesh->setControlPoints(mControlPoints);
	}

	glm::vec3 FlexBlockComponentInstance::GetControlPoint(int index)
	{
		return mControlPoints[index];
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		// calculate points
		// this is where the magic happens
	}
}