#pragma once
#include "statecontroller.h"

namespace nap
{
	class MessageCtrl : public StateCtrl
	{
		RTTI_ENABLE_DERIVED_FROM(StateCtrl)
	public:
		MessageCtrl() : StateCtrl() {
			mSenderObject = &addChild<Link>("senderObject");
		}

		void setDispatchFlags(DispatchMethod flags) { mDispatchFlags = flags; }
		void setMessage(const std::string& action) { mAction = action; }
		void setSenderObject(Object& sender) { mSenderObject->setTarget(sender); }
		virtual void activate() override;

		virtual void finish() override;

	private:
		Link* mSenderObject;
		std::string mAction;
		DispatchMethod mDispatchFlags;
		EventDispatcher mEventDispatcher = {this, "EventDispatcher"};
	};
}

RTTI_DECLARE(nap::MessageCtrl)
