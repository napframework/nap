#include "compoundcontrollers.h"

RTTI_DEFINE(nap::CompoundCtrl)
RTTI_DEFINE(nap::QueuedCtrl)
RTTI_DEFINE(nap::StaggeredCtrl)
RTTI_DEFINE(nap::ParallelCtrl)


namespace nap
{
	StateCtrl& CompoundCtrl::addController(const std::string& name, const RTTI::TypeInfo& type)
	{
		assert(type.is_derived_from<StateCtrl>());
		return *static_cast<StateCtrl*>(&addChild(name, type));
	}

	void QueuedCtrl::activate()
	{
		mQueue.clear();
		auto controllers = getControllers();
		mQueue.insert(mQueue.end(), controllers.begin(), controllers.end());
		activateNextController();

	}

	void QueuedCtrl::update(const float& dt)
	{
		if (!mCurrentController) return;
		mCurrentController->update(dt);

		if (mCurrentController->isFinished()) activateNextController();
	}

	void QueuedCtrl::activateNextController()
	{
		if (mQueue.size() <= 0) { // End of queue?
			mCurrentController = nullptr;
			return;
		}

		mCurrentController = mQueue.front();
		mQueue.pop_front();
		mCurrentController->activate();
        Logger::debug("Activated controller: %s", ObjectPath(mCurrentController).toString().c_str());
    }

    void QueuedCtrl::finish() {
		while (!mQueue.empty()) {
			auto ctrl = mQueue.front();
			mQueue.pop_front();
			ctrl->finish();
		}
    }

    void StaggeredCtrl::activate()
	{
		mLocalTime = 0;

		mInactiveQueue.clear();
		mActiveQueue.clear();

		auto controllers = getControllers();
		mInactiveQueue.insert(mInactiveQueue.end(), controllers.begin(), controllers.end());

		activateNextController();
	}

	void StaggeredCtrl::update(const float& dt)
	{
		// Activate next controller if time is right
		while (!mInactiveQueue.empty() && mLocalTime >= offsetTime.getValueRef() * mActiveQueue.size())
			activateNextController();

		// Update active controllers
		for (auto ctrl : mActiveQueue) {
			ctrl->update(dt);
			if (ctrl->isFinished()) mEraseList.push_back(ctrl);
		}

		// Cleanup potentially finished controllers
		for (auto ctrl : mEraseList)
			mActiveQueue.remove(ctrl);
		mEraseList.clear();

		mLocalTime += dt;
	}

	bool StaggeredCtrl::isFinished() const
	{
		return mInactiveQueue.empty() && mActiveQueue.empty();
	}

	void StaggeredCtrl::activateNextController()
	{
		if (mInactiveQueue.empty())
			return;
		auto ctrl = mInactiveQueue.front();
		mInactiveQueue.pop_front();
		mActiveQueue.push_back(ctrl);
		ctrl->activate();
		Logger::debug("Activated controller: %s", ObjectPath(ctrl).toString().c_str());

//         if (ctrl->isFinished())
//             activateNextController();
	}

	void StaggeredCtrl::finish() {
		while (!mActiveQueue.empty()) {
			auto ctrl = mActiveQueue.front();
			mActiveQueue.pop_front();
			ctrl->finish();
		}
		while (!mInactiveQueue.empty()) {
			auto ctrl = mInactiveQueue.front();
			mInactiveQueue.pop_front();
			ctrl->finish();
		}
	}

    void ParallelCtrl::activate() {
        mControllers.clear();
        for (auto ctrl : getControllers()) {
            mControllers.push_back(ctrl);
            ctrl->activate();
        }
    }

    void ParallelCtrl::update(const float &dt) {
        for (auto ctrl : mControllers)
            ctrl->update(dt);
    }

    bool ParallelCtrl::isFinished() const {
        for (auto ctrl : mControllers)
            if (!ctrl->isFinished())
                return false;
        return true;
    }

    void ParallelCtrl::finish() {
        for (auto ctrl : mControllers)
            ctrl->finish();
    }
}