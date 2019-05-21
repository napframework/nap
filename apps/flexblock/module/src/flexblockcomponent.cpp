#include "FlexBlockComponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>

// nap::FlexBlockComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockComponent)
	RTTI_PROPERTY("mBoxRenderer", &nap::FlexBlockComponent::mBoxRenderer, nap::rtti::EPropertyMetaData::Required)
	
	RTTI_PROPERTY("mControlPoint1", &nap::FlexBlockComponent::mControlPoint1, nap::rtti::EPropertyMetaData::Required)
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
		SetControlPointOne(math::extractPosition(mControlPoint1Instance->getGlobalTransform()));

		//
		utility::computeNormals(mBoxRendererInstance->getMeshInstance(), *mVertexAttribute, *mNormalAttribute);

		mBoxRendererInstance->getMeshInstance().update(errorState);

		return true;
	}

	void FlexBlockComponentInstance::SetControlPointOne(glm::vec3 position)
	{
		// set control point position
		mControlPoint1Instance->setTranslate(position);

		// get vertices of the block mesh
		auto vertices = mVertexAttribute->getData();

		// calculate the position of the vertices in local space
		auto& transform = mBoxRendererInstance->getTransform();
		glm::vec3 pos = math::worldToObject(position, transform.getGlobalTransform());
		
		// assign positions
		vertices[0] = pos;

		//
		mVertexAttribute->setData(vertices);

		//
		utility::computeNormals(mBoxRendererInstance->getMeshInstance(), *mVertexAttribute, *mNormalAttribute);

		// and finally, update the mesh
		utility::ErrorState errorState;
		mBoxRendererInstance->getMeshInstance().update(errorState);
	}

	void FlexBlockComponentInstance::UpdateBox(glm::vec3 position)
	{
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{

	}
}