#include "selectcolorcomponent.h"

// External Includes
#include <nap/entity.h>
#include <mathutils.h>

// nap::selectcolorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectColorComponent)
	RTTI_PROPERTY("Controller", &nap::SelectColorComponent::mController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Mesh",		&nap::SelectColorComponent::mMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Channel",	&nap::SelectColorComponent::mStartChannel,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Red",		&nap::SelectColorComponent::mRed,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Green",		&nap::SelectColorComponent::mGreen,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Blue",		&nap::SelectColorComponent::mBlue,			nap::rtti::EPropertyMetaData::Required)
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


	bool SelectColorComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get the resource we are instantiated from
		nap::SelectColorComponent* resource = getComponent<SelectColorComponent>();

		// Get the artnet controller
		mArtnetController = resource->mController.get();
		
		// Get the renderable mesh material we use to update the color values for
		mMaterial = &mRenderableMeshComponent->getMaterialInstance();

		// Start channel needs to be a modulo of 4 (rgbw)
		mStartChannel = (math::min<int>(resource->mStartChannel,511) / 4) * 4;

		// Get color
		mRed	= static_cast<uint8>(math::clamp<int>(resource->mRed,	0, math::max<nap::uint8>()));
		mGreen	= static_cast<uint8>(math::clamp<int>(resource->mGreen, 0, math::max<nap::uint8>()));
		mBlue	= static_cast<uint8>(math::clamp<int>(resource->mBlue,  0, math::max<nap::uint8>()));
		mWhite	= static_cast<uint8>(math::clamp<int>(resource->mWhite, 0, math::max<nap::uint8>()));

		return true;
	}


	void SelectColorComponentInstance::update(double deltaTime)
	{
		// Clear buffer
		mArtnetController->clear();

		// Send dmx values
		std::vector<uint8> data
		{
			mRed,
			mGreen,
			mBlue,
			mWhite
		};
		mArtnetController->send(data, mStartChannel);

		// Update material values
		float fr = static_cast<float>(mRed)	 / static_cast<float>(math::max<nap::uint8>());
		float fg = static_cast<float>(mGreen)/ static_cast<float>(math::max<nap::uint8>());
		float fb = static_cast<float>(mBlue) / static_cast<float>(math::max<nap::uint8>());
		float fw = static_cast<float>(mWhite)/ static_cast<float>(math::max<nap::uint8>());

		nap::UniformVec3&  ucolor = mMaterial->getOrCreateUniform<nap::UniformVec3>("mColor");
		nap::UniformFloat& uwhite = mMaterial->getOrCreateUniform<nap::UniformFloat>("mWhite");

		ucolor.setValue({ fr, fg, fb });
		uwhite.setValue(fw);
	}
}