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
		ModuleManager(Core& core);

		// Destructor
		~ModuleManager();

		// Copy is not allowed
		ModuleManager(ModuleManager&) = delete;

		// Copy assignment is not allowed
		ModuleManager& operator=(const ModuleManager&) = delete;

		// Move is not allowed
		ModuleManager(ModuleManager&&) = delete;
		
		// Move assignment is not allowed
		ModuleManager& operator=(ModuleManager&&) = delete;

		/**
		 * Load all modules that are required by the project
		 * @param projectInfo The descriptor providing the module dependencies
		 * @param error Any errors will be stored here
		 * @return True on success, false otherwise
		 */
		bool loadModules(const ProjectInfo& projectInfo, utility::ErrorState& error);

		/**
		 * @return All currently loaded modules
		 */
		const std::vector<std::unique_ptr<Module>>& getModules() const { return mModules; }

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
		Core& 								 			mCore;		// Core
	};
}
