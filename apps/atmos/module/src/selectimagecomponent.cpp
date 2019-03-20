#include "selectimagecomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::selectimagecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectImageComponent)
	RTTI_PROPERTY("Images",		&nap::SelectImageComponent::mImages,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",		&nap::SelectImageComponent::mIndex,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::selectimagecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectImageComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectImageComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool SelectImageComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over meshes
		nap::SelectImageComponent* resource = getComponent<SelectImageComponent>();

		if (!errorState.check(resource->mImages.size() > 0, "%s: no images found to select from", mID.c_str()))
			return false;

		// Copy all images
		mImages.clear();
		mImages.reserve(resource->mImages.size());
		for (auto& image : resource->mImages)
			mImages.emplace_back(image.get());

		resource->mIndex->valueChanged.connect(mImageIndexChangedSlot);

		// Select the right image (clamped)
		selectImage(resource->mIndex->mValue);

		return true;
	}


	void SelectImageComponentInstance::update(double deltaTime)
	{

	}


	void SelectImageComponentInstance::selectImage(int index)
	{
		mCurrentImage = mImages[index];
	}


	nap::ImageFromFile& SelectImageComponentInstance::getImage()
	{
		assert(mCurrentImage != nullptr);
		return *mCurrentImage;
	}


	const nap::ImageFromFile& SelectImageComponentInstance::getImage() const
	{
		assert(mCurrentImage != nullptr);
		return *mCurrentImage;
	}
}