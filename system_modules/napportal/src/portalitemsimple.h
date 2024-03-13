/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "portalitem.h"
#include "portalutils.h"
#include "color.h"
#include "portalitemlayout.h"

// External Includes
#include <apivalue.h>
#include <apievent.h>
#include <parametersimple.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * Represents any simple item in a NAP portal.
	 */
	template<typename T>
	class NAPAPI PortalItemSimple : public PortalItem
	{
		RTTI_ENABLE(PortalItem)
	public:
		/**
		 * Unsubscribes from the parameter changed signal
		 */
		virtual void onDestroy() override;

		/**
		 * Processes an update type API event.
		 * @param event The event to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		virtual bool processUpdate(const APIEvent& event, utility::ErrorState& error) override;

		/**
		 * @return the descriptor of the portal item as an API event
		 */
		virtual APIEventPtr getDescriptor() const override;

		/**
		 * @return the current value of the portal item as an API event
		 */
		virtual APIEventPtr getValue() const override;

		/**
		 * Called when the parameter value changes, sends the update as an API event
		 * @param value the updated value of the parameter
		 */
		virtual void onParameterUpdate(T value);

		/**
		* Slot which is called when the parameter value changes
		*/
		Slot<T> mParameterUpdateSlot = { this, &PortalItemSimple::onParameterUpdate };

		ResourcePtr<ParameterSimple<T>> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
    protected:
        /**
         * Subscribes to the parameter changed signal
         * @param error contains the error message when initialization fails
         * @return if initialization succeeded.
         */
        virtual bool onInit(utility::ErrorState& error) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Simple Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemToggle = PortalItemSimple<bool>;
	using PortalItemTextField = PortalItemSimple<std::string>;
	class NAPAPI PortalItemTextArea final : public PortalItemSimple<std::string>
	{
		RTTI_ENABLE(PortalItemSimple<std::string>)
	};

    class NAPAPI PortalItemLabel final : public PortalItemSimple<std::string>
    {
    RTTI_ENABLE(PortalItemSimple<std::string>)
    public:
        bool onInit(utility::ErrorState &error) override;

        void setTextLayout(const PortalItemTextLayout& layout);

        const PortalItemTextLayout& getTextLayout() const;

        PortalItemTextLayout mStartTextLayout; ///< Property: "StartTextLayout" The layout of the text
    protected:
        void addStateArguments(nap::APIEventPtr &event) const override;
    private:
        PortalItemTextLayout mTextLayout;
    };


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool PortalItemSimple<T>::onInit(utility::ErrorState& error)
	{
		mParameter->valueChanged.connect(mParameterUpdateSlot);
        mDisplayName = mParameter->getDisplayName();
		return true;
	}

	template<typename T>
	void PortalItemSimple<T>::onDestroy()
	{
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
	}

	template<typename T>
	void PortalItemSimple<T>::onParameterUpdate(T value)
	{
		valueUpdate(*this);
	}

	template<typename T>
	bool PortalItemSimple<T>::processUpdate(const APIEvent& event, utility::ErrorState& error)
	{
		// Check for the portal item value argument
		const APIArgument* arg = event.getArgumentByName(nap::portal::itemValueArgName);
		if (!error.check(arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
			return false;

		// Check the portal item value type
		const rtti::TypeInfo type = arg->getValueType();
		if (!error.check(type == RTTI_OF(T), "%s: cannot process value type %s", mID.c_str(), type.get_name().data()))
			return false;

		// Cast and set the value on the parameter
		T value = static_cast<const APIValue<T>*>(&arg->getValue())->mValue;
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
		mParameter->setValue(value);
		mParameter->valueChanged.connect(mParameterUpdateSlot);
		return true;
	}

	template<typename T>
	APIEventPtr PortalItemSimple<T>::getDescriptor() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIValue<T>>(nap::portal::itemValueArgName, mParameter->mValue);

        addStateArguments(event);

        return event;
	}

	template<typename T>
	APIEventPtr PortalItemSimple<T>::getValue() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIValue<T>>(nap::portal::itemValueArgName, mParameter->mValue);
        return event;
	}
}

/**
 * Helper macro that can be used to define the RTTI for a portal item simple type
 */
#define DEFINE_PORTAL_ITEM_SIMPLE(Type)															\
	RTTI_BEGIN_CLASS(Type)																		\
		RTTI_PROPERTY("Parameter",	&Type::mParameter,	nap::rtti::EPropertyMetaData::Required)	\
	RTTI_END_CLASS
