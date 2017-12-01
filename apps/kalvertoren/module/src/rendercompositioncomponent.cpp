#include "rendercompositioncomponent.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>

// nap::rendercompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderCompositionComponent)
	RTTI_PROPERTY("TargetOne",				&nap::RenderCompositionComponent::mTargetA,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TargetTwo",				&nap::RenderCompositionComponent::mTargetB,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CompositionComponent",	&nap::RenderCompositionComponent::mCompositionComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderComponent",		&nap::RenderCompositionComponent::mRenderableComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraComponent",		&nap::RenderCompositionComponent::mCameraComponent,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::rendercompositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderCompositionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderCompositionComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool RenderCompositionComponentInstance::init(utility::ErrorState& errorState)
	{
		mTargetA = getComponent<RenderCompositionComponent>()->mTargetA.get();
		mTargetB = getComponent<RenderCompositionComponent>()->mTargetB.get();

		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		if (!errorState.check(mRenderService != nullptr, "Unable to find render service: %s", this->mID.c_str()))
			return false;

		return true;
	}


	void RenderCompositionComponentInstance::render()
	{
		nap::Composition& selection = mCompositionComponent->getSelection();
	}

}