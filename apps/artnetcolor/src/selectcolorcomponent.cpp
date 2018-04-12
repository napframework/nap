#include "selectcolorcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::selectcolorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectColorComponent)
	RTTI_PROPERTY("Mesh",		&nap::SelectColorComponent::mMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color",		&nap::SelectColorComponent::mColor,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("White",		&nap::SelectColorComponent::mWhite,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::selectcolorcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectColorComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}

	bool SelectColorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get the resource we are instantiated from
		nap::SelectColorComponent* resource = getComponent<SelectColorComponent>();
		
		// Get the renderable mesh material we use to update the color values for
		mMaterial = &mRenderableMeshComponent->getMaterialInstance();

		// Get color
		mColor = resource->mColor;
		mWhite = resource->mWhite;

		mDirty = true;

		return true;
	}


	void SelectColorComponentInstance::update(double deltaTime)
	{
		if (mDirty)
		{
			// Update material values
			nap::UniformVec3&  ucolor = mMaterial->getOrCreateUniform<nap::UniformVec3>("mColor");
			nap::UniformFloat& uwhite = mMaterial->getOrCreateUniform<nap::UniformFloat>("mWhite");

			ucolor.setValue({mColor.getRed(), mColor.getGreen(), mColor.getBlue()});
			uwhite.setValue(mWhite.getRed());

			mDirty = false;
		}
	}


	void SelectColorComponentInstance::setColor(const RGBColorFloat& color)
	{
		mColor = color;
		mDirty  = true;
	}


	void SelectColorComponentInstance::setWhite(float white)
	{
		mWhite.setValue(EColorChannel::Red, white);
		mDirty = true;
	}


	RGBColorFloat SelectColorComponentInstance::getColor() const
	{
		return mColor;
	}


	float SelectColorComponentInstance::getWhite() const
	{
		return mWhite.getValue(EColorChannel::Red);
	}

}