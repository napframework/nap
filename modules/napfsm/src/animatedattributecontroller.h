#pragma once

#include "attributeanimator.h"
#include "attributecontroller.h"
#include <nap.h>
#include <nap/eventdispatcher.h>

namespace nap
{

	class AnimAttrCtrl : public AttrCtrl
	{
		RTTI_ENABLE_DERIVED_FROM(AttrCtrl)
	public:
		AnimAttrCtrl() : AttrCtrl() {}
		AnimAttrCtrl(AttributeBase& attrib, AttributeAnimatorBase* animator) : AttrCtrl() {
			setName(attrib.getName() + "_Controller");
			setTargetAttribute(attrib);
			setAnimator(animator);
		}

		AnimAttrCtrl(AttributeBase& attrib, AttributeAnimatorBase* animator, const StateMode& mode) : AttrCtrl() {
			setTrigger(mode);
			setName(attrib.getName() + "_Controller");
			setTargetAttribute(attrib);
			setAnimator(animator);
		}

		void setAnimator(AttributeAnimatorBase* animator) {
			setAnimator(std::unique_ptr<AttributeAnimatorBase>(animator));
		}

		void setAnimator(std::unique_ptr<AttributeAnimatorBase> animator);
		AttributeAnimatorBase* getAnimator() const { return mAnimator.get(); }

		void activate() override;

		void update(const float& dt) override;

		void finish();

		bool isFinished() const override;

	private:
		std::unique_ptr<AttributeAnimatorBase> mAnimator;
		bool mIsActive;
	};
}
RTTI_DECLARE(nap::AnimAttrCtrl)
