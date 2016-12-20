// External Includes
#include <ofMain.h>
#include <nap/logger.h>

// Local Includes
#include "napofspritecomponent.h"

namespace nap
{
	OFSpriteComponent::OFSpriteComponent() : OFImageComponent()
	{
		imageChanged.connect([&](ofImage& im) { spriteSheetChanged(); });
	}

	// update the animation
	void OFSpriteComponent::onUpdate()
	{
		if (!isPlaying.getValue()) return;

		if (!isAllocated()) return;

		float frameRate = getFrameRate();
		int startFrame = getStartFrame();
		float animLength = getAnimationLength();
		bool loop = getLoop();

		// Calculate time
		float animTime = animationTime.getValue();
		animTime += ofGetLastFrameTime();

		bool is_finished = false;

		if (loop) 
			animTime = fmod(animTime, animLength);
		else 
		{ 
			// Trigger finished when not looping
			if (animTime >= getAnimationLength())
				is_finished = true;

			// clamp time
			animTime = min(animTime, animLength);
		}
		animationTime.setValue(animTime);

		// Calculate current frame
		int currentFrame = startFrame + (int)roundf(animTime * frameRate);
		if (currentFrame != index.getValue()) index.setValue(currentFrame);

		// Trigger finished if animation completed
		if (is_finished)
			finished(*mCurrentAnimation);
	}


	bool OFSpriteComponent::isAnimationFinished() const {
		return animationTime.getValue() >= getAnimationLength();
	}


	int OFSpriteComponent::getStartFrame() const
	{
		if (mCurrentAnimation) 
			return mCurrentAnimation->startFrame.getValue();
		return 0;
	}


	int OFSpriteComponent::getEndFrame() const
	{
		if (mCurrentAnimation) return mCurrentAnimation->endFrame.getValue();
		return getSpriteCount() - 1;
	}


	bool OFSpriteComponent::getLoop() const
	{
		if (mCurrentAnimation) return mCurrentAnimation->loop.getValue();
		return true;
	}


	void OFSpriteComponent::setCurrentFrame(int frame) 
	{
		index.setValue(frame);
	}


	float OFSpriteComponent::getAnimationLength() const
	{
		float frameRate = getFrameRate();
		assert(frameRate != 0);
		return float(getEndFrame() - getStartFrame()) / getFrameRate();
	}


	void OFSpriteComponent::spriteSizeChanged(const ofVec2i& size) { spriteSheetChanged(); }


	void OFSpriteComponent::indexChanged(const int& newIndex)
	{
		if (index.getValue() >= mSpriteCount) {
			Logger::warn(*this, "Sprite index (%d) exceeds sprite count (%d)", 
				index.getValue(), mSpriteCount);
			index.getValueRef() = mSpriteCount - 1;
		}

		if (index.getValue() < 0) index.getValueRef() = 0;

		// conversion to int rounds down to nearest whole number
		int column = index.getValue() % mColumnCount;
		int row = index.getValue() / mColumnCount;

		normalizedSpriteSize.setValue(
			{spriteSize.getValue().x / getImage().getWidth(), spriteSize.getValue().y / getImage().getHeight()});
		normalizedSpriteOffset.setValue(
			{column * normalizedSpriteSize.getValue().x, row * normalizedSpriteSize.getValue().y});
	}


	void OFSpriteComponent::spriteSheetChanged()
	{
		// image must be allocated
		if (!isAllocated()) return;

		// sprite size must be greater than zero
		if (spriteSize.getValue().x == 0) spriteSize.getValueRef().x = getImage().getWidth();
		if (spriteSize.getValue().y == 0) spriteSize.getValueRef().y = getImage().getHeight();

		mColumnCount = getImage().getWidth() / spriteSize.getValue().x;
		mRowCount = getImage().getHeight() / spriteSize.getValue().y;

		mSpriteCount = mColumnCount * mRowCount;
	}


	SpriteAnimation& OFSpriteComponent::addAnimation(int startFrame, int endFrame, float frameRate, bool loop,
													 const std::string& name)
	{
		SpriteAnimation& anim = addChild<SpriteAnimation>(name);
		anim.startFrame.setValue(startFrame);
		anim.endFrame.setValue(endFrame);
		anim.frameRate.setValue(frameRate);
		anim.loop.setValue(loop);
		if (!mCurrentAnimation) setCurrentAnimation(&anim);
		return anim;
	}


	void OFSpriteComponent::setCurrentAnimation(SpriteAnimation* anim)
	{
		mCurrentAnimation = anim;
		animationTime.setValue(0);
		isPlaying.setValue(true);
	}


	void OFSpriteComponent::setCurrentAnimation(const std::string& animName)
	{
		auto anim = getAnimation(animName);
		if (!anim) return;
		setCurrentAnimation(anim);
	}


	SpriteAnimation* OFSpriteComponent::getAnimation(const std::string& name)
	{
		return getChild<SpriteAnimation>(name);
	}



}

RTTI_DEFINE(nap::OFSpriteComponent)
RTTI_DEFINE(nap::SpriteAnimation)