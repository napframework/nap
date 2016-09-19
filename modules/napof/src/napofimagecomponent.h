#pragma once

// NAP Includes
#include <nap/attribute.h>
#include <nap/component.h>
#include <nap/coremodule.h>
#include <nap/signalslot.h>

#include <napofattributes.h>

// OF Includes
#include "napofupdatecomponent.h"
#include <ofImage.h>

namespace nap
{
	/**
	@brief Wraps an of image, holds both the image and hardware texture
	**/
	class OFImageComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:
		// Default constructor
		OFImageComponent();
		virtual ~OFImageComponent() override;

		// Attributes
		Attribute<std::string> mFile = {this, "Filename"};

		Attribute<bool> useImageCache = {this, "UseImageCache", true};

		// TODO: Populate this output texture
		Attribute<ofTexture*> texture = {this, "TexturePointer", nullptr};

		// Always returns a valid ofImage instance
		ofImage& getImage();

		float getWidth() const { return mImage->getWidth(); }
		float getHeight() const { return mImage->getHeight(); }
		float getRatio() const { return getWidth() / getHeight(); }
		bool isAllocated() { return getImage().isAllocated(); }

		void onUpdate() override {}

		// Slots
		NSLOT(mFileChanged, const std::string&, fileChanged)

		Signal<ofImage&> imageChanged;

	private:
		// Local storage
		std::shared_ptr<ofImage> mImage = nullptr;

		// Image cache shared across all OFImageComponent instances
		static std::unordered_map<std::string, std::shared_ptr<ofImage>> mImageCache;

		// Occurs when the file changes
		void fileChanged(const std::string& inName) { loadImage(); }

		// Load
		void loadImage();
	};
}

RTTI_DECLARE(nap::OFImageComponent)
