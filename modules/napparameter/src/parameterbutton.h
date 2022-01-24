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
	 * Parameter that triggers down, up and click for button actions
	 */
	class ParameterButton : public Parameter
	{
		RTTI_ENABLE(Parameter)

	public:
		/**
		 * Set the value for this parameter from another parameter
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override
		{
			const ParameterButton* derived_type = rtti_cast<const ParameterButton>(&value);
			assert(derived_type != nullptr);
			setPressed(derived_type->mPressed);
		}

		/**
		 * Set the pressed state of this parameter.
		 * Raises the "press" signal if the state changed to pressed.
		 * Raises the "release" signal if the state changed to not pressed.
		 * @param pressed The pressed state to set
		 */
		void setPressed(const bool& pressed)
		{
			if (mPressed != pressed)
			{
				mPressed = pressed;
				mPressed ? press() : release();
			}
		}

		/**
		 * @return whether the button is currenty pressed
		 */
		const bool& isPressed() const
		{
			return mPressed;
		}

	public:
		Signal<>	click;		///< Signal that's raised when the button is clicked
		Signal<>	press;		///< Signal that's raised when the button is pressed
		Signal<>	release;	///< Signal that's raised when the button is released

	private:
		bool		mPressed;	///< Stores whether the button is currenty pressed
	};
}
