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
#include <parametercolor.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * Represents any color item in a NAP portal.
	 */
	template<typename T>
	class PortalItemColor : public PortalItem
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
		Slot<T> mParameterUpdateSlot = { this, &PortalItemColor::onParameterUpdate };

		ResourcePtr<ParameterSimple<T>> mParameter;		///< Property: 'Parameter' the parameter linked to this portal item
    protected:
        /**
         * Subscribes to the parameter changed signal
         * @param error contains the error message when initialization fails
         * @return if initialization succeeded.
         */
        virtual bool onInit(utility::ErrorState& error) override;
	private:

		/**
		 * @return the values of the color channels as a vector of the channel data type
		 */
		const std::vector<typename T::value_type> getColorValues() const;

		/**
		 * Sets the values of the color channels from a vector of the channel data type
		 */
		bool setColorValues(const std::vector<typename T::value_type>& values, utility::ErrorState& error);

		T mRetainedValue;								///< Retained value to check if the parameter has been updated externally
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Color Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemRGBColor8		= PortalItemColor<RGBColor8>;
	using PortalItemRGBAColor8		= PortalItemColor<RGBAColor8>;
	using PortalItemRGBColorFloat	= PortalItemColor<RGBColorFloat>;
	using PortalItemRGBAColorFloat	= PortalItemColor<RGBAColorFloat>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool PortalItemColor<T>::onInit(utility::ErrorState& error)
	{
		mRetainedValue = mParameter->mValue;
		mParameter->valueChanged.connect(mParameterUpdateSlot);
        mDisplayName = mParameter->getDisplayName();
		return true;
	}

	template<typename T>
	void PortalItemColor<T>::onDestroy()
	{
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
	}

	template<typename T>
	void PortalItemColor<T>::onParameterUpdate(T value)
	{
		// No need to send update if retained value stays the same,
		// e.g. when we changed the parameter from a client update
		if (mRetainedValue == value)
			return;

		mRetainedValue = value;
		valueUpdate(*this);
	}

	template<typename T>
	bool PortalItemColor<T>::processUpdate(const APIEvent& event, utility::ErrorState& error)
	{
		// Check for the portal item value argument
		const APIArgument* arg = event.getArgumentByName(nap::portal::itemValueArgName);
		if (!error.check(arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
			return false;

		// Ensure that the argument is an array
		if (!error.check(arg->isArray(), "%s: expected array for value argument, not %s", mID.c_str(), arg->getValueType().get_name().data()))
			return false;

		// Cast the argument to an array of the channel value type
		const std::vector<typename T::value_type>* values = arg->asArray<typename T::value_type>();
		if (!error.check(values != nullptr, "%s: expected array of %s for value argument, not %s", mID.c_str(), RTTI_OF(typename T::value_type).get_name().data(), arg->getValueType().get_name().data()))
			return false;

		return setColorValues(*values, error);
	}

	template<typename T>
	APIEventPtr PortalItemColor<T>::getDescriptor() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIValue<std::vector<typename T::value_type>>>(nap::portal::itemValueArgName, getColorValues());
        addStateArguments(event);
        return event;
	}

	template<typename T>
	APIEventPtr PortalItemColor<T>::getValue() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIValue<std::vector<typename T::value_type>>>(nap::portal::itemValueArgName, getColorValues());
        return event;
	}

	template<typename T>
	const std::vector<typename T::value_type> PortalItemColor<T>::getColorValues() const
	{
		return std::vector<typename T::value_type>(mParameter->mValue.getValues().begin(), mParameter->mValue.getValues().end());
	}

	template<typename T>
	bool PortalItemColor<T>::setColorValues(const std::vector<typename T::value_type>& values, utility::ErrorState& error)
	{
		// Ensure we have enough channels
		int channels = mParameter->mValue.getNumberOfChannels();
		if (!error.check(values.size() == channels, "%s: expected %i color channels, received %i", mID.c_str(), channels, values.size()))
			return false;

		// Update the color trough setValue so the
		// parameter will trigger an update signal
		T color;
		color.setData(values.data());
		mRetainedValue = color;
		mParameter->setValue(color);
		return true;
	}
}

/**
 * Helper macro that can be used to define the RTTI for a portal item numeric type
 */
#define DEFINE_PORTAL_ITEM_COLOR(Type)															\
	RTTI_BEGIN_CLASS(Type)																		\
		RTTI_PROPERTY("Parameter",	&Type::mParameter,	nap::rtti::EPropertyMetaData::Required)	\
	RTTI_END_CLASS
