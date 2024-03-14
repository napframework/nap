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
	template<typename T>
	class PortalItemVec : public PortalItem
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
		Slot<T> mParameterUpdateSlot = { this, &PortalItemVec::onParameterUpdate };

		ResourcePtr<ParameterVec<T>> mParameter;		///< Property: 'Parameter' the parameter linked to this portal item
    protected:
        /**
         * Subscribes to the parameter changed signal
         * @param error contains the error message when initialization fails
         * @return if initialization succeeded.
         */
        virtual bool onInit(utility::ErrorState& error) override;
	private:

		/**
		 * @return the values of the linked vector parameter as a vector of the basic data type
		 */
		const std::vector<typename T::value_type> getVectorValues() const;

		/**
		 * Sets the values of the linked vector parameter from a vector of the basic data type
		 */
		bool setVectorValues(const std::vector<typename T::value_type>& values, utility::ErrorState& error);
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Numeric Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemVec2		= PortalItemVec<glm::vec2>;
	using PortalItemVec3		= PortalItemVec<glm::vec3>;
	using PortalItemIVec2		= PortalItemVec<glm::ivec2>;
	using PortalItemIVec3		= PortalItemVec<glm::ivec3>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool PortalItemVec<T>::onInit(utility::ErrorState& error)
	{
        mDisplayName = mParameter->getDisplayName();
		mParameter->valueChanged.connect(mParameterUpdateSlot);
		return true;
	}

	template<typename T>
	void PortalItemVec<T>::onDestroy()
	{
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
	}

	template<typename T>
	void PortalItemVec<T>::onParameterUpdate(T value)
	{
		valueUpdate(*this);
	}

	template<typename T>
	bool PortalItemVec<T>::processUpdate(const APIEvent& event, utility::ErrorState& error)
	{
		// Check for the portal item value argument
		const APIArgument* arg = event.getArgumentByName(nap::portal::itemValueArgName);
		if (!error.check(arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
			return false;

		// Ensure that the argument is an array
		if (!error.check(arg->isArray(), "%s: expected array for value argument, not %s", mID.c_str(), arg->getValueType().get_name().data()))
			return false;

		// Cast the argument to an array of the basic data type
		const std::vector<typename T::value_type>* values = arg->asArray<typename T::value_type>();
		if (!error.check(values != nullptr, "%s: expected array of %s for value argument, not %s", mID.c_str(), RTTI_OF(typename T::value_type).get_name().data(), arg->getValueType().get_name().data()))
			return false;

		return setVectorValues(*values, error);
	}

	template<typename T>
	APIEventPtr PortalItemVec<T>::getDescriptor() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIValue<std::vector<typename T::value_type>>>(nap::portal::itemValueArgName, getVectorValues());
		event->addArgument<APIValue<typename T::value_type>>(nap::portal::itemMinArgName, mParameter->mMinimum);
		event->addArgument<APIValue<typename T::value_type>>(nap::portal::itemMaxArgName, mParameter->mMaximum);
		event->addArgument<APIBool>(nap::portal::itemClampArgName, mParameter->mClamp);

        addStateArguments(event);

        return event;
	}

	template<typename T>
	APIEventPtr PortalItemVec<T>::getValue() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIValue<std::vector<typename T::value_type>>>(nap::portal::itemValueArgName, getVectorValues());
        return event;
	}

	template<typename T>
	const std::vector<typename T::value_type> PortalItemVec<T>::getVectorValues() const
	{
		size_t length = mParameter->mValue.length();
		std::vector<typename T::value_type> values(length);
		std::memcpy(values.data(), &mParameter->mValue, sizeof(typename T::value_type) * length);
		return values;
	}

	template<typename T>
	bool PortalItemVec<T>::setVectorValues(const std::vector<typename T::value_type>& values, utility::ErrorState& error)
	{
		// Ensure we have enough data
		size_t length = mParameter->mValue.length();
		if (!error.check(values.size() == length, "%s: expected a vector of size %i, received %i", mID.c_str(), length, values.size()))
			return false;

		// Update the vector trough setValue so the parameter will trigger an update signal
		T new_value;
		std::memcpy(&new_value, values.data(), sizeof(typename T::value_type) * length);
		mParameter->valueChanged.disconnect(mParameterUpdateSlot);
		mParameter->setValue(new_value);
		mParameter->valueChanged.connect(mParameterUpdateSlot);
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
