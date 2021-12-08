/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalutils.h"

// External Includes
#include <parameternumeric.h>

namespace nap
{
	/**
	 * Represents an numeric slider in a NAP portal.
	 */
	template<typename T, typename U>
	class PortalItemSlider : public PortalItem
	{
		RTTI_ENABLE(PortalItem)

	public:

		/**
		 * Processes an update type API event.
		 * @param event The event to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		virtual bool processUpdateEvent(const APIEvent& event, utility::ErrorState& error) override;

		/**
		 * @return the current state of the portal item as an API event
		 */
		virtual APIEventPtr getState() override;

		ResourcePtr<T> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
	};


	//////////////////////////////////////////////////////////////////////////
	// Portal Item Slider Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using PortalItemSliderFloat		= PortalItemSlider<ParameterFloat, APIFloat>;
	using PortalItemSliderInt		= PortalItemSlider<ParameterInt, APIInt>;
	using PortalItemSliderChar		= PortalItemSlider<ParameterChar, APIChar>;
	using PortalItemSliderByte		= PortalItemSlider<ParameterByte, APIByte>;
	using PortalItemSliderDouble	= PortalItemSlider<ParameterDouble, APIDouble>;
	using PortalItemSliderLong		= PortalItemSlider<ParameterLong, APILong>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename U>
	bool PortalItemSlider<T, U>::processUpdateEvent(const APIEvent& event, utility::ErrorState& error)
	{
		return true;
	}

	template<typename T, typename U>
	APIEventPtr PortalItemSlider<T, U>::getState()
	{
		APIEventPtr state = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		state->addArgument<APIString>(nap::portal::itemTypeArgName, nap::portal::itemTypeSlider);
		state->addArgument<U>(nap::portal::itemValueArgName, mParameter->mValue);
		state->addArgument<U>(nap::portal::itemMinArgName, mParameter->mMinimum);
		state->addArgument<U>(nap::portal::itemMaxArgName, mParameter->mMaximum);
		return state;
	}



	/**
	 * Helper macro that can be used to define the RTTI for a portal item slider type
	 */
	#define DEFINE_PORTAL_ITEM_SLIDER(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Parameter",	&Type::mParameter,		nap::rtti::EPropertyMetaData::Required)				\
		RTTI_END_CLASS
}
