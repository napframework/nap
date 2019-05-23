#include "FlexBlockComponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>

// nap::FlexBlockComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockComponent)
	RTTI_PROPERTY("BoxRenderer", &nap::FlexBlockComponent::mBoxRenderer, nap::rtti::EPropertyMetaData::Required)
	
	RTTI_PROPERTY("ControlPointsMesh", &nap::FlexBlockComponent::mControlPointsMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameMesh", &nap::FlexBlockComponent::mFrameMesh, nap::rtti::EPropertyMetaData::Required)
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
		assert(resource->mBoxRenderer != nullptr);

		//
		mVertexAttribute = &(mBoxRendererInstance->getMeshInstance().getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName()));
		mNormalAttribute = &(mBoxRendererInstance->getMeshInstance().getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName()));

		//
		mControlPointsMesh = resource->mControlPointsMesh;
		mControlPoints = mControlPointsMesh->getControlPoints();

		mFrameMesh = resource->mFrameMesh;

		//
		utility::computeNormals(mBoxRendererInstance->getMeshInstance(), *mVertexAttribute, *mNormalAttribute);

		mBoxRendererInstance->getMeshInstance().update(errorState);

		return true;
	}

	void FlexBlockComponentInstance::SetControlPoint(int index, glm::vec3 position)
	{
		//
		mControlPoints[index] = position;

		// update the box
		updateBox();

		// update the control points mesh
		mControlPointsMesh->setControlPoints(mControlPoints);

		// update ropes of frame
		mFrameMesh->setControlPoints(mControlPoints);
	}

	glm::vec3 FlexBlockComponentInstance::GetControlPoint(int index)
	{
		return mControlPoints[index];
	}

	void FlexBlockComponentInstance::updateBox()
	{
		// get vertices of the block mesh
		auto vertices = mVertexAttribute->getData();

		// assign positions
		// m1
		vertices[0] = mControlPoints[0]; //< Front Lower left
		vertices[13] = mControlPoints[0]; //< Left Lower right
		vertices[16] = mControlPoints[0];	//< Bottom Lower left

		// m2
		vertices[1] = mControlPoints[1];	//< Front Lower right
		vertices[4] = mControlPoints[1];	//< Right Lower Left
		vertices[17] = mControlPoints[1]; //< Bottom Lower right

		// m3
		vertices[2] = mControlPoints[2];	//< Front Top left
		vertices[15] = mControlPoints[2]; //< Left Top right
		vertices[20] = mControlPoints[2]; //< Top Lower left

		// m4
		vertices[3] = mControlPoints[3];	//< Front Top right
		vertices[6] = mControlPoints[3];	//< Right Top left
		vertices[21] = mControlPoints[3];	//< Top Lower right

		// m5
		vertices[8]  = mControlPoints[4];	//< Back Lower left
		vertices[19] = mControlPoints[4];	//< Bottom Top right
		vertices[5] = mControlPoints[4]; //< Right Lower Right

		// m6
		vertices[9] = mControlPoints[5];	//< Back lower right
		vertices[12] = mControlPoints[5]; //< Left Lower left
		vertices[18] = mControlPoints[5]; //< Bottom Top left

		// m7
		vertices[10] = mControlPoints[6]; //< Back Top left
		vertices[23] = mControlPoints[6]; //< Top Top right
		vertices[7] = mControlPoints[6]; //< Right Top right

		// m8
		vertices[11] = mControlPoints[7]; //< Back Top right
		vertices[14] = mControlPoints[7]; //< Left Top left
		vertices[22] = mControlPoints[7]; //< Top Top left

		// set the new verts
		mVertexAttribute->setData(vertices);

		// calc normals
		utility::computeNormals(mBoxRendererInstance->getMeshInstance(), *mVertexAttribute, *mNormalAttribute);

		// and finally, update the mesh
		utility::ErrorState errorState;
		mBoxRendererInstance->getMeshInstance().update(errorState);
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{

	}
}