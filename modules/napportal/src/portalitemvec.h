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
#include <parametervec.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * Represents any vector item in a NAP portal.
	 */
	template<typename T, typename U>
	class PortalItemVec : public PortalItem
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
		Slot<T> mParameterUpdateSlot = { this, &PortalItemVec::onParameterUpdate };

		ResourcePtr<ParameterVec<T>> mParameter;		///< Property: 'Parameter' the parameter linked to this portal item

	private:

		/**
		 * @return the values of the linked vector parameter as a vector of the basic data type
		 */
		const std::vector<U> getVectorValues() const;

		/**
		 * Sets the values of the linked vector parameter from a vector of the basic data type
		 */
		bool setVectorValues(const std::vector<U>& values, utility::ErrorState& error);

		T mRetainedValue;								///< Retained value to check if the parameter has been updated externally
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Numeric Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemVec2		= PortalItemVec<glm::vec2, float>;
	using PortalItemVec3		= PortalItemVec<glm::vec3, float>;
	using PortalItemIVec2		= PortalItemVec<glm::ivec2, int>;
	using PortalItemIVec3		= PortalItemVec<glm::ivec3, int>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename U>
	bool PortalItemVec<T, U>::init(utility::ErrorState& error)
	{
		mRetainedValue = mParameter->mValue;
		mParameter->valueChanged.connect(mParameterUpdateSlot);
		return true;
	}

	template<typename T, typename U>
	void PortalItemVec<T, U>::onDestroy()
	{
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
	}

	template<typename T, typename U>
	void PortalItemVec<T, U>::onParameterUpdate(T value)
	{
		// No need to send update if retained value stays the same,
		// e.g. when we changed the parameter from a client update
		if (mRetainedValue == value)
			return;

		mRetainedValue = value;
		updateSignal(*this);
	}

	template<typename T, typename U>
	bool PortalItemVec<T, U>::processUpdate(const APIEvent& event, utility::ErrorState& error)
	{
		// Check for the portal item value argument
		const APIArgument* arg = event.getArgumentByName(nap::portal::itemValueArgName);
		if (!error.check(arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
			return false;

		// Ensure that the argument is an array
		if (!error.check(arg->isArray(), "%s: expected array for value argument, not %s", mID.c_str(), arg->getValueType().get_name().data()))
			return false;

		// Cast the argument to an array of the basic data type
		const std::vector<U>* values = arg->asArray<U>();
		if (!error.check(values != nullptr, "%s: expected array of %s for value argument, not %s", mID.c_str(), RTTI_OF(U).get_name().data(), arg->getValueType().get_name().data()))
			return false;

		return setVectorValues(*values, error);
	}

	template<typename T, typename U>
	APIEventPtr PortalItemVec<T, U>::getDescriptor() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIValue<std::vector<U>>>(nap::portal::itemValueArgName, getVectorValues());
		event->addArgument<APIValue<U>>(nap::portal::itemMinArgName, mParameter->mMinimum);
		event->addArgument<APIValue<U>>(nap::portal::itemMaxArgName, mParameter->mMaximum);
		event->addArgument<APIBool>(nap::portal::itemClampArgName, mParameter->mClamp);
		return event;
	}

	template<typename T, typename U>
	APIEventPtr PortalItemVec<T, U>::getValue() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIValue<std::vector<U>>>(nap::portal::itemValueArgName, getVectorValues());
		return event;
	}

	template<typename T, typename U>
	const std::vector<U> PortalItemVec<T, U>::getVectorValues() const
	{
		size_t length = mParameter->mValue.length();
		std::vector<U> values(length);
		std::memcpy(values.data(), &mParameter->mValue, sizeof(U) * length);
		return values;
	}

	template<typename T, typename U>
	bool PortalItemVec<T, U>::setVectorValues(const std::vector<U>& values, utility::ErrorState& error)
	{
		// Ensure we have enough data
		size_t length = mParameter->mValue.length();
		if (!error.check(values.size() == length, "%s: expected a vector of size %i, received %i", mID.c_str(), length, values.size()))
			return false;

		// Update the vector trough setValue so the
		// parameter will trigger an update signal
		T new_value;
		std::memcpy(&new_value, values.data(), sizeof(U) * length);
		mRetainedValue = new_value;
		mParameter->setValue(new_value);
		return true;
	}
}

/**
 * Helper macro that can be used to define the RTTI for a portal item numeric type
 */
#define DEFINE_PORTAL_ITEM_VEC(Type)															\
	RTTI_BEGIN_CLASS(Type)																		\
		RTTI_PROPERTY("Parameter",	&Type::mParameter,	nap::rtti::EPropertyMetaData::Required)	\
	RTTI_END_CLASS
