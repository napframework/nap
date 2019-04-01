#pragma once

#include <utility/dllexport.h>
#include <rtti/typeinfo.h>

namespace nap
{
	/**
	 * Opaque object that can be given to a core instance to add platform specific functionality to a core.
	 * Derive from this interface to add platform specific functionality to core, for example: context specific variables.
	 */
	class NAPAPI CoreInterface
	{
		RTTI_ENABLE()
	public:
		/**
		 * No default constructor	
		 */
		CoreInterface() = default;

		/**
		 * Destructor
		 */
		virtual ~CoreInterface() { }

		/**
		 * Copy is not allowed
		 */
		CoreInterface(CoreInterface&) = delete;
		CoreInterface& operator=(const CoreInterface&) = delete;

		/**
		 * Move is not allowed
		 */
		CoreInterface(CoreInterface&&) = delete;
		CoreInterface& operator=(CoreInterface&&) = delete;
	};
}