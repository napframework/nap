/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <rtti/typeinfo.h>

namespace nap
{
	/**
	 * Opaque object that can be given to core on construction to add platform specific functionality.
	 * The extension is available during initialization of core, before initialization of services and the application.
	 * Derive from this interface to add platform specific functionality, for example: (global) platform specific variables.
	 */
	class NAPAPI CoreExtension
	{
		RTTI_ENABLE()
	public:
		/**
		 * Default constructor	
		 */
		CoreExtension() = default;

		/**
		 * Destructor
		 */
		virtual ~CoreExtension() { }

		/**
		 * Copy is not allowed
		 */
		CoreExtension(CoreExtension&) = delete;
		CoreExtension& operator=(const CoreExtension&) = delete;

		/**
		 * Move is not allowed
		 */
		CoreExtension(CoreExtension&&) = delete;
		CoreExtension& operator=(CoreExtension&&) = delete;
	};
}