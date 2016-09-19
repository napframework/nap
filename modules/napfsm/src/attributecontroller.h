#pragma once

#include <nap.h>
#include <nap/eventdispatcher.h>
#include "statecontroller.h"

namespace nap
{

	class AttrCtrl : public StateCtrl
	{
		RTTI_ENABLE_DERIVED_FROM(StateCtrl)
	public:
		AttrCtrl();
		AttrCtrl(AttributeBase& targetAttribute, const StateMode& trigger);

		void setTargetAttribute(nap::AttributeBase& attr);

		void activate() override;

        virtual void finish() override { activate(); }

        // The attribute that will be overwritten on activation
		virtual nap::AttributeBase* getTargetAttribute() const
		{
			return (nap::AttributeBase*)mTargetAttribute->getTarget();
		}

		// The attribute holding the potential value for the target attribute
		virtual nap::AttributeBase* getControlAttribute() const { return mControlAttribute; };

	private:
		nap::Link* mTargetAttribute = nullptr;
		nap::AttributeBase* mControlAttribute = nullptr;
	};
}
RTTI_DECLARE(nap::AttrCtrl)
