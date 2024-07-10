/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "portalitem.h"
#include "portalutils.h"

// External Includes
#include <apievent.h>
#include <parameterbutton.h>

namespace nap
{
	/**
	 * Represents a button in a NAP portal.
	 */
	class NAPAPI PortalItemButton final : public PortalItem
	{
		RTTI_ENABLE(PortalItem)

	public:
        bool onInit(utility::ErrorState &error) override;

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
         * Sets the alignment of the portal item
         * @param alignment the new alignment
         */
        void setAlignment(EPortalItemAlignment alignment);

        /**
         * Gets the alignment of the portal item
         * @return the alignment
         */
        bool getAlignment() const;

		ResourcePtr<ParameterButton> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
        EPortalItemAlignment mDefaultAlignment = EPortalItemAlignment::Left; ///< Property: 'Alignment' the alignment of the portal item

    protected:
        void addStateArguments(nap::APIEventPtr &event) const override;
    private:
        EPortalItemAlignment mAlignment = EPortalItemAlignment::Left;
	};
}
