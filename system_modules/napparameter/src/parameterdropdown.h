/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "parameter.h"

// External Includes
#include <nap/signalslot.h>

namespace nap
{
    /**
     * Parameter that acts as a dropdown menu, holds an array of string values and a selected index
     */
	class NAPAPI ParameterDropDown : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		/**
		 * Set the value for this parameter from another parameter
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * Sets the selected index
		 * @param selectedIndex the selected index
		 */
		void setSelectedIndex(int selectedIndex);

		/**
		 * Gets the selected index
		 * @return the selected index
		 */
		int getSelectedIndex(){ return mSelectedIndex; }

        /**
         * Sets the items of the dropdown menu
         */
        void setItems(const std::vector<std::string>& items);

        // Signals
        Signal<int>				                indexChanged;
        Signal<const std::vector<std::string>&> itemsChanged;
	public:
        std::vector<std::string> mItems; ///< Property: 'Items' items of this dropdown
        int mSelectedIndex = 0; ///< Property: 'SelectedIndex' selected index of this dropdown
	private:

	};
}
