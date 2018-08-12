#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>

// nap::updatematerialcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("ScanMeshComponent",				&nap::UpdateMaterialComponent::mScanMeshComponent,							nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalMeshComponent",			&nap::UpdateMaterialComponent::mNormalMeshComponent,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("TileableImageSelectComponent",	&nap::UpdateMaterialComponent::mTileableImageSelectComponent,		nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("SingleImageSelectComponent",		&nap::UpdateMaterialComponent::mSingleImageSelectComponent,			nap::rtti::EPropertyMetaData::Required);
RTTI_END_CLASS

// nap::updatematerialcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool UpdateMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void UpdateMaterialComponentInstance::update(double deltaTime)
	{

	}
}