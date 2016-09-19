#include "MessageCtrl.h"
#include "statemachinecomponent.h"

namespace nap
{
	void MessageCtrl::activate()
	{

		std::unique_ptr<Message> evt = std::move(mEventDispatcher.createEvent<Message>(mAction));
		mEventDispatcher.mDirection.setValue(mDispatchFlags);
		mEventDispatcher.dispatchEvent(*evt, *mSenderObject->getTarget());
        Logger::debug(*this, "Message sent: '%s'", mAction.c_str());
	}

    void MessageCtrl::finish() { activate(); }
}