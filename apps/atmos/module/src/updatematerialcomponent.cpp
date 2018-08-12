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
static const std::string sColorTexOneName = "colorTextureOne";
static const std::string sColorTexTwoName = "colorTextureTwo";

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
		MaterialInstance& scan_material = mScanMeshComponent->getMaterialInstance();
		MaterialInstance& norm_material = mNormalsMeshComponent->getMaterialInstance();

		UniformTexture2D& scan_tex_one = scan_material.getOrCreateUniform<UniformTexture2D>("colorTextureOne");
		UniformTexture2D& norm_tex_one = norm_material.getOrCreateUniform<UniformTexture2D>("colorTextureOne");
		scan_tex_one.setTexture(mTileableImageSelectComponent->getImage());
		norm_tex_one.setTexture(mTileableImageSelectComponent->getImage());
	}
}