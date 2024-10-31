/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "parameterbutton.h"

RTTI_DEFINE_CLASS(nap::ParameterButton)

namespace nap
{

	void ParameterButton::setValue(const Parameter& value)
	{
		const ParameterButton* derived_type = rtti_cast<const ParameterButton>(&value);
		assert(derived_type != nullptr);
		setPressed(derived_type->mPressed);
	}


	void ParameterButton::setPressed(bool pressed)
	{
		if (mPressed != pressed)
		{
			mPressed = pressed;
			mPressed ? press() : release();
		}
	}
}
