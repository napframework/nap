#pragma once

#include "statecontroller.h"
#include "attributecontroller.h"
#include <nap.h>


namespace nap
{

	// Abstract base class to combine multiple controllers into one
	class CompoundCtrl : public StateCtrl
	{
		RTTI_ENABLE_DERIVED_FROM(StateCtrl)
	public:
		CompoundCtrl() : StateCtrl() {}
		CompoundCtrl(const StateMode& mode) : StateCtrl(mode) {}

		StateCtrl& addController(const std::string& name, const RTTI::TypeInfo& type);

        template<typename T, typename... Args>
		T& addCtrl(Args &&... args) {
			RTTI::TypeInfo type = RTTI_OF(T);
			assert(type.isKindOf<StateCtrl>());
			return *static_cast<T*>(&addChild(std::make_unique<T>(std::forward<Args>(args)...)));
		}

		// Add a regular attribute controller that will immediately set the value upon activation
		template <typename T>
		AttrCtrl& addAttrCtrl(
                Attribute<T> &targetAttribute, T targetvalue, const StateMode &trigger = Entering)
		{
			assert(targetAttribute.getValueType() == RTTI_OF(T));

			AttrCtrl& ctrl = addCtrl<AttrCtrl>(targetAttribute, trigger);
			Attribute<T>* ctrlAttrib = static_cast<Attribute<T>*>(ctrl.getControlAttribute());
			ctrlAttrib->setValue(targetvalue);

			return ctrl;
		}

	protected:
		std::vector<StateCtrl*> getControllers() { return getChildrenOfType<StateCtrl>(); }
	};



	// Each child controller will be activated when the previous one has finished
	class QueuedCtrl : public CompoundCtrl
	{
		RTTI_ENABLE_DERIVED_FROM(CompoundCtrl)
	public:
		QueuedCtrl() : CompoundCtrl() {}
		QueuedCtrl(const StateMode& mode) : CompoundCtrl(mode) {}

		void activate() override;
		void update(const float& dt) override;
		bool isFinished() const override { return mCurrentController == nullptr; }

        virtual void finish() override;

    private:
		void activateNextController();
        bool mActivated = false;
		StateCtrl* mCurrentController = nullptr;
		std::list<StateCtrl*> mQueue;
	};



	// Each child controller will be activated one after another, separated by offset time
	class StaggeredCtrl : public CompoundCtrl
	{
		RTTI_ENABLE_DERIVED_FROM(CompoundCtrl)
	public:
		StaggeredCtrl() : CompoundCtrl() {}
		StaggeredCtrl(const StateMode& mode) : CompoundCtrl(mode) {}

		void activate() override;
		void update(const float& dt) override;
		bool isFinished() const override;
		Attribute<float> offsetTime = {this, "OffsetTime", 0.1f};

        virtual void finish() override;

    private:
		void activateNextController();

		float mLocalTime = 0;
		std::list<StateCtrl*> mInactiveQueue;
		std::list<StateCtrl*> mActiveQueue;
		std::list<StateCtrl*> mEraseList;
	};

    class ParallelCtrl : public CompoundCtrl
    {
        RTTI_ENABLE_DERIVED_FROM(CompoundCtrl)
    public:
        ParallelCtrl() : CompoundCtrl() {}
        ParallelCtrl(const StateMode& mode) : CompoundCtrl(mode) {}

        virtual void activate() override;
        virtual void update(const float &dt) override;
        virtual bool isFinished() const override;
        virtual void finish() override;

    private:
        std::vector<StateCtrl*> mControllers;
    };
}

RTTI_DECLARE_BASE(nap::CompoundCtrl)
RTTI_DECLARE(nap::QueuedCtrl)
RTTI_DECLARE(nap::StaggeredCtrl)
RTTI_DECLARE(nap::ParallelCtrl)