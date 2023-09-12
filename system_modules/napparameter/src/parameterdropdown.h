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
	 *
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
		 * @param
		 */
		void setSelectedIndex(int selectedIndex);

		/**
		 * @return
		 */
		int getSelectedIndex(){ return mSelectedIndex; }

        /**
         *
         */
        void setItems(const std::vector<std::string>& items);

        // Signals
        Signal<int>				                indexChanged;
        Signal<const std::vector<std::string>&> itemsChanged;
	public:
        std::vector<std::string> mItems; ///<
        int mSelectedIndex = 0; ///<
	private:

	};
}
