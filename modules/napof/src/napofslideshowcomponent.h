#pragma once

// External Includes
#include <nap/signalslot.h>
#include <nap/component.h>
#include <nap/coreattributes.h>
#include <nap/arrayattribute.h>

// Local Includes
#include "napofupdatecomponent.h"

// OF Includes
#include <ofImage.h>

namespace nap 
{
	class OFSlideshowComponent : public OFUpdatableComponent 
	{
		RTTI_ENABLE_DERIVED_FROM(nap::OFUpdatableComponent)
	public:
		OFSlideshowComponent();
		virtual ~OFSlideshowComponent();

		// paths to all the images in the slide show
		ArrayAttribute<std::string> imageFilenames =				{ this, "imageFilenames", {} };

		// pointers to textures of the currently displayed slides
		Attribute<ofTexture*> leftTexture =							{ this, "leftTexture", nullptr };
		Attribute<ofTexture*> rightTexture =						{ this, "rightTexture", nullptr };

		// indices of the currently displayed images
		Attribute<int> leftImageIndex =								{ this, "leftImageIndex", -1  };
		Attribute<int> rightImageIndex =							{ this, "rightImageIndex", -1 };

		// the intersection indicates the fractional x value where the left hand side and the right hand side images meet
		Attribute<float> intersection = { this, "intersection", 0.f };

		// duration of animation from the current slide to the next
		Attribute<float> animationDuration = { this, "animationDuration", 0.4f };

		int getImageCount() const { return mImages.size(); }
		int getWidth() const { return mImages.empty() ? 0 : mImages[0]->getWidth(); };
		int getHeight() const { return mImages.empty() ? 0 : mImages[0]->getHeight(); };
		int getCurrentSlide() const { return currentSlide.getValue(); }

		int getLeftImageIndex();
		int getRightImageIndex();
        int getCenterImageIndex();

        void scroll(float distance);

		void previous();
		void next();
        void setTargetIndex(int index);

		// Derived from OFUpdatableComponent
		void onUpdate() override;

		bool isAnimating() const { return abs(mScrollVelocity) > mIsAnimatingThreshold; }

		// signal emitted when a different slide has been selected
		Signal<int> slideChanged;

		NSLOT(onFileNamesChanged, AttributeBase&, imageFilenamesChanged)
		NSLOT(onLeftIndexChanged, AttributeBase&, leftImageIndexChanged)
		NSLOT(onRightIndexChanged, AttributeBase&, rightImageIndexChanged)

	private:
		void imageFilenamesChanged(AttributeBase&  filenames);
		void leftImageIndexChanged(AttributeBase&  index);
		void rightImageIndexChanged(AttributeBase& index);

        Attribute<int> currentSlide = { this, "currentSlide", 0 };

		enum class State { LEFT, RIGHT, IDLE };

		float mIsAnimatingThreshold = 0.1f;
		int mTargetIndex = 0; // Virtual index // TODO: Check zero position
		float mScrollPosition = 0; // Virtual scroll position, spans all images' width
		float mScrollVelocity = 0;
		float mScrollTarget = 0;
		float mScrollSmoothTime = 0.3f;
		float mScrollMaxVelocity = 10000000;


		std::vector<std::shared_ptr<ofImage>> mImages;

		static std::unordered_map<std::string, std::shared_ptr<ofImage>> mImageCache;

		State mState = State::IDLE;
	};
}

RTTI_DECLARE(nap::OFSlideshowComponent)