/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "projectinfo.h"
#include "modulecache.h"

// External Includes
#include <utility/module.h>
#include <utility/errorstate.h>
#include <rtti/typeinfo.h>
#include <assert.h>

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 * Responsible for dynamically loading NAP modules.
	 */
	class NAPAPI ModuleManager final
	{
		friend class Core;
	public:
		// Constructor
		ModuleManager() = default;

		// Copy is not allowed
		ModuleManager(ModuleManager&) = delete;
		ModuleManager& operator=(const ModuleManager&) = delete;
		ModuleManager(ModuleManager&&) = delete;
		ModuleManager& operator=(ModuleManager&&) = delete;

		/**
		 * Load all modules required by the project
		 * @param projectInfo project descriptor that provides top level module dependencies
		 * @param error contains the error if loading fails
		 * @return if loading succeeded
		 */
		bool loadModules(const ProjectInfo& projectInfo, utility::ErrorState& error);

		/**
         * @return All currently loaded modules
         */
        const std::vector<const Module*>& getModules() const { return mModules; }

		/**
		 * Find a loaded module by its name as defined in its descriptor file
		 * @param moduleName The name of the module to find
		 * @return The object providing access to the Module, nullptr if not found.
		 */
		const Module* findModule(const std::string& moduleName) const;

		/**
		 * Find the module associated with the given service
		 * @param serviceType The type of the service associated with the module
		 * @return The object providing access to the Module, nullptr if not found.
		 */
		const Module* findModule(const nap::rtti::TypeInfo& serviceType) const;

		/**
		 * Find the module associated with the given service of type T.
		 * @return The object providing access to the Module, nullptr if not found.
		 */
		template<typename T>
		const Module* findModule() const	{ return findModule(RTTI_OF(T)); }

	private:
		std::vector<const nap::Module*> mModules;	///< All sourced nap project modules

	};
}
