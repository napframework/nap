#include "animatedattributecontroller.h"

namespace nap {

	void AnimAttrCtrl::setAnimator(std::unique_ptr<AttributeAnimatorBase> animator)
	{
		mAnimator = std::move(animator);
		mAnimator->setAttribute(*getTargetAttribute());
	}

	void AnimAttrCtrl::activate()
	{
		//Logger::debug("Activate Controller: %s", ObjectPath(this).toString().c_str());
		mAnimator->activate();
		mIsActive = true;
	}

	void AnimAttrCtrl::update(const float& dt)
	{

		if (mIsActive) {
			mAnimator->update(dt);
			if (mAnimator->isFinished())
				mIsActive = false;
		}
	}

	void AnimAttrCtrl::finish()
	{
		mAnimator->finish();
		mIsActive = false;
	}

	bool AnimAttrCtrl::isFinished() const
	{
		return mAnimator->isFinished();
	}

}
