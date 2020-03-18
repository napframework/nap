#include "applyclassifycomponent.h"

// External Includes
#include <entity.h>

// nap::applydetectioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyClassifyComponent)
	RTTI_PROPERTY("ClassifyComponent",	&nap::ApplyClassifyComponent::mClassifyComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderComponent",	&nap::ApplyClassifyComponent::mRenderComponent,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::applydetectioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyClassifyComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool ApplyClassifyComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void ApplyClassifyComponentInstance::update(double deltaTime)
	{

	}
}