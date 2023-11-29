#include "updatetransformcomponent.h"

// External Includes
#include <transformcomponent.h>
#include <entity.h>

// nap::UpdateTransformComponent run time class defini4tion 
RTTI_BEGIN_CLASS(nap::UpdateTransformComponent)
	RTTI_PROPERTY("Position",			&nap::UpdateTransformComponent::mPosition,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Orientation",		&nap::UpdateTransformComponent::mOrientation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UniformScale",		&nap::UpdateTransformComponent::mUniformScale,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::UpdateTransformComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateTransformComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateTransformComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool UpdateTransformComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<UpdateTransformComponent>();
		mTransformComponent = &getEntityInstance()->getComponent<TransformComponentInstance>();

		if (mResource->mUniformScale != nullptr)
			mTransformComponent->setUniformScale(mResource->mUniformScale->mValue);

		//if (mResource->mOrientation != nullptr)
		//	mTransformComponent->setRotate(mResource->mOrientation->getValue());

		if (mResource->mPosition != nullptr)
			mTransformComponent->setTranslate(mResource->mPosition->mValue);

		return true;
	}


	void UpdateTransformComponentInstance::update(double deltaTime)
	{
		if (mResource->mUniformScale != nullptr)
			mTransformComponent->setUniformScale(mResource->mUniformScale->mValue);

		//if (mResource->mOrientation != nullptr)
		//	mTransformComponent->setRotate(mResource->mOrientation->getValue());

		if (mResource->mPosition != nullptr)
			mTransformComponent->setTranslate(mResource->mPosition->mValue);
	}
}
