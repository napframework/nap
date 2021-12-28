/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalutils.h"

// External Includes
#include <parameternumeric.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * Represents an numeric slider in a NAP portal.
	 */
	template<typename T>
	class PortalItemSlider : public PortalItem
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
		virtual APIEventPtr getDescriptor() override;

		/**
		 * @return the current value of the portal item as an API event
		 */
		virtual APIEventPtr getValue() override;

		/**
		 * Called when the parameter value changes, sends the update as an API event
		 * @param value the updated value of the parameter
		 */
		virtual void onParameterUpdate(T value);

		/**
		* Slot which is called when the parameter value changes
		*/
		Slot<T> mParameterUpdateSlot = { this, &PortalItemSlider::onParameterUpdate };

		ResourcePtr<ParameterNumeric<T>> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Slider Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemSliderByte		= PortalItemSlider<uint8_t>;
	using PortalItemSliderInt		= PortalItemSlider<int>;
	using PortalItemSliderLong		= PortalItemSlider<int64_t>;
	using PortalItemSliderFloat		= PortalItemSlider<float>;
	using PortalItemSliderDouble	= PortalItemSlider<double>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool PortalItemSlider<T>::init(utility::ErrorState& error)
	{
		mParameter->valueChanged.connect(mParameterUpdateSlot);
		return true;
	}

	template<typename T>
	void PortalItemSlider<T>::onDestroy()
	{
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
	}

	template<typename T>
	void PortalItemSlider<T>::onParameterUpdate(T value)
	{
		APIEventPtr event = getValue();
		updateSignal(*event);
	}

	template<typename T>
	bool PortalItemSlider<T>::processUpdate(const APIEvent& event, utility::ErrorState& error)
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
		mParameter->setValue(value);
		return true;
	}

	template<typename T>
	APIEventPtr PortalItemSlider<T>::getDescriptor()
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIValue<T>>(nap::portal::itemValueArgName, mParameter->mValue);
		event->addArgument<APIValue<T>>(nap::portal::itemMinArgName, mParameter->mMinimum);
		event->addArgument<APIValue<T>>(nap::portal::itemMaxArgName, mParameter->mMaximum);
		return event;
	}

	template<typename T>
	APIEventPtr PortalItemSlider<T>::getValue()
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIValue<T>>(nap::portal::itemValueArgName, mParameter->mValue);
		return event;
	}



	/**
	 * Helper macro that can be used to define the RTTI for a portal item slider type
	 */
	#define DEFINE_PORTAL_ITEM_SLIDER(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Parameter",	&Type::mParameter,		nap::rtti::EPropertyMetaData::Required)				\
		RTTI_END_CLASS
}
