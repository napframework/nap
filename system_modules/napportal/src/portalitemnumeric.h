/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalutils.h"

// External Includes
#include <apivalue.h>
#include <apievent.h>
#include <parameternumeric.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * Represents any numeric item in a NAP portal.
	 */
	template<typename T>
	class PortalItemNumeric : public PortalItem
	{
		RTTI_ENABLE(PortalItem)
	public:

		/**
		 * Subscribes to the parameter changed signal
		 * @param error contains the error message when initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;

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
		Slot<T> mParameterUpdateSlot = { this, &PortalItemNumeric::onParameterUpdate };

		ResourcePtr<ParameterNumeric<T>> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Numeric Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemSliderByte		= PortalItemNumeric<uint8_t>;
	using PortalItemSliderInt		= PortalItemNumeric<int>;
	using PortalItemSliderLong		= PortalItemNumeric<int64_t>;
	using PortalItemSliderFloat		= PortalItemNumeric<float>;
	using PortalItemSliderDouble	= PortalItemNumeric<double>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool PortalItemNumeric<T>::init(utility::ErrorState& error)
	{
		mParameter->valueChanged.connect(mParameterUpdateSlot);
		return true;
	}

	template<typename T>
	void PortalItemNumeric<T>::onDestroy()
	{
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
	}

	template<typename T>
	void PortalItemNumeric<T>::onParameterUpdate(T value)
	{
		updateSignal(*this);
	}

	template<typename T>
	bool PortalItemNumeric<T>::processUpdate(const APIEvent& event, utility::ErrorState& error)
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
	APIEventPtr PortalItemNumeric<T>::getDescriptor() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIValue<T>>(nap::portal::itemValueArgName, mParameter->mValue);
		event->addArgument<APIValue<T>>(nap::portal::itemMinArgName, mParameter->mMinimum);
		event->addArgument<APIValue<T>>(nap::portal::itemMaxArgName, mParameter->mMaximum);
		return event;
	}

	template<typename T>
	APIEventPtr PortalItemNumeric<T>::getValue() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIValue<T>>(nap::portal::itemValueArgName, mParameter->mValue);
		return event;
	}
}

/**
 * Helper macro that can be used to define the RTTI for a portal item numeric type
 */
#define DEFINE_PORTAL_ITEM_NUMERIC(Type)																				\
	RTTI_BEGIN_CLASS(Type, "Numeric portal item")																		\
		RTTI_PROPERTY("Parameter",	&Type::mParameter,	nap::rtti::EPropertyMetaData::Required, "Numeric parameter")	\
	RTTI_END_CLASS
