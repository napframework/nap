/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/rtti.h>

namespace nap
{
	namespace SequenceGUIClipboards
	{
		class Clipboard
		{
			RTTI_ENABLE()
		public:
			virtual ~Clipboard(){}

			template<typename T>
			bool isClipboard()
			{
				return this->get_type() == RTTI_OF(T);
			}

			template<typename T>
			T* getDerived()
			{
				assert(isClipboard<T>());
				return static_cast<T*>(this);
			}
		};

		using SequenceClipboardPtr = std::unique_ptr<Clipboard>;

		// use this method to create an action
		template<typename T, typename... Args>
		static SequenceClipboardPtr createClipboard(Args&&... args)
		{
			return std::make_unique<T>(args...);
		}

		class Empty : public Clipboard { RTTI_ENABLE() };
	}
}
