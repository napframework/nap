#pragma once

// NAP Includes
#include <nap/attribute.h>
#include <nap/component.h>
#include <nap/coremodule.h>
#include <nap/signalslot.h>

#include "napofimagecomponent.h"
#include <napofattributes.h>
#include <napofupdatecomponent.h>
#include <utils/ofVec2i.h>

// OF Includes
#include <ofImage.h>


namespace nap
{
	class SpriteAnimation : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		SpriteAnimation() {}

		int getFrameCount() const { return endFrame.getValue() - startFrame.getValue(); }
		// Total length in seconds
		float getLength() const { return (float)getFrameCount() / (float)frameRate.getValue(); }

		Attribute<bool> loop = {this, "loop", true};
		Attribute<int> startFrame = {this, "startFrame", 0};
		Attribute<int> endFrame = {this, "endFrame", 0};
		Attribute<float> frameRate = {this, "frameRate", 30};
	};



	/**
	@brief Wraps an of image, holds both the image and hardware texture
	**/
	class OFSpriteComponent : public OFImageComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFImageComponent)
	public:
		OFSpriteComponent();

		SpriteAnimation& addAnimation(int startFrame, int endFrame, float frameRate, bool loop,
									  const std::string& name);

		void setCurrentAnimation(SpriteAnimation* anim);
		void setCurrentAnimation(const std::string& animName);
		SpriteAnimation* getCurrentAnimation() { return mCurrentAnimation; }

		// Input attributes
		Attribute<ofVec2i> spriteSize = {this, "spriteSize", {0, 0}, &OFSpriteComponent::spriteSizeChanged};
		Attribute<int> index = {this, "index", 0, &OFSpriteComponent::indexChanged};
		Attribute<bool> isPlaying = {this, "IsPlaying", true};
		Attribute<float> animationTime = {this, "AnimationTime", 0.0f};

		// Output attributes that will be exposed to the material
		Attribute<ofVec2f> normalizedSpriteSize = {this, "normalizedSpriteSize", {0, 0}};
		Attribute<ofVec2f> normalizedSpriteOffset = {this, "normalizedSpriteOffset", {0, 0}};


		int getStartFrame() const;

		int getEndFrame() const;

		bool getLoop() const;

		float getFrameRate() const
		{
			if (mCurrentAnimation) return mCurrentAnimation->frameRate.getValue();
			return 30;
		}

		void setCurrentFrame(int frame);

		float getAnimationLength() const;

		bool isAnimationFinished() const;
		Signal<SpriteAnimation&> finished;

		// Utility
		int getSpriteCount() const { return mSpriteCount; }

		// Derived from OFUpdatableComponent
		void onUpdate() override;

	private:
		SpriteAnimation* getAnimation(const std::string& name);
		SpriteAnimation* mCurrentAnimation = nullptr;
		// number of sprites (frames) in the sprite sheet image
		int mSpriteCount = 0;

		// number of rows and columns in the sprite sheet
		int mRowCount = 0;
		int mColumnCount = 0;

		// attribute slots
		void spriteSizeChanged(const ofVec2i& size);
		void indexChanged(const int& index);

		void spriteSheetChanged();
	};
}

RTTI_DECLARE(nap::OFSpriteComponent)
RTTI_DECLARE(nap::SpriteAnimation)