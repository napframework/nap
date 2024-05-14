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
	 * Button parameter
	 */
	class NAPAPI ParameterButton : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		/**
		 * Set the value for this parameter from another parameter
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * Set the pressed state of this parameter.
		 * Raises the "press" signal if the state changed to pressed.
		 * Raises the "release" signal if the state changed to not pressed.
		 * @param pressed The pressed state to set
		 */
		void setPressed(bool pressed);

		/**
		 * @return whether the button is currently pressed
		 */
		bool isPressed() const { return mPressed; }

	public:
		Signal<>	press;		///< Signal that's raised when the button is pressed
		Signal<>	release;	///< Signal that's raised when the button is released

	private:
		bool		mPressed = false;	///< Stores whether the button is currently pressed
	};
}
