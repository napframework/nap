/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "projectinfo.h"

// External Includes
#include <utility/module.h>
#include <assert.h>
#include <vector>
#include <string>

namespace nap
{
	// Forward declares
	class ModuleManager;
	class Module;

	/**
	 * NAP dynamic library module cache.
	 * Provides an interface for thread-safe loading of NAP modules in the host process.
	 * This class is a singleton, exposed only to the module manager.
	 * 
	 * Every NAP project depends on a set of (system / user) modules.
	 * This cache ensures that modules with the same module name are sourced only once, which is of importance when
	 * multiple NAP projects are executed within the same host process, such as applets within Napkin.
	 */
	class NAPAPI ModuleCache final
	{
		friend class ModuleManager;
	public:

		// Copy and move is not allowed
		ModuleCache(ModuleCache&) = delete;
		ModuleCache& operator=(const ModuleCache&) = delete;
		ModuleCache(ModuleCache&&) = delete;
		ModuleCache& operator=(ModuleCache&&) = delete;

	private:
		/**
		 * Returns this process module cache
		 * @return this process module cache
		 */
		static ModuleCache& get();

		/**
		 * Loads a module into the current context.
		 * Needs the be implemented in an extension, platform specific.
		 * @param projectinfo project information resource
		 * @param moduleName name of the module to load
		 * @param err contains the error if loading operation fails
		 * @return if module loaded
		 */
		nap::Module* sourceModule(const ProjectInfo& projectinfo, const std::string& moduleName, utility::ErrorState& err);

		// Hide constructor
		ModuleCache() = default;

		// All modules loaded in current host process
		std::vector<std::unique_ptr<Module>> mModules;
	};


	//////////////////////////////////////////////////////////////////////////
	// NAP Module
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Contains all data associated with a module, including handle and description.
	 * Created and managed by the nap::ModuleManager.
	 */
	class NAPAPI Module final
	{
		friend class ModuleManager;
	public:
		Module() = default;

		// Copy and move is not allowed
		Module(Module&) = delete;
		Module& operator=(const Module&) = delete;
		Module(Module&&) = delete;
		Module& operator=(Module&&) = delete;

		/**
		 * @return module name
		 */
		const std::string& getName() const				{ return mName; }

		/**
		 * @return module descriptor
		 */
		const ModuleDescriptor& getDescriptor()	const	{ assert(mDescriptor != nullptr); return *mDescriptor; }

		/**
		 * @return module information
		 */
		const ModuleInfo& getInformation() const		{ assert(mInfo != nullptr); return *mInfo; }

		/**
		 * Attempts to find a (module specific) asset with the given name and extension.
		 * The asset must reside in one of the locations declared by the 'nap::ModuleInfo::DataSearchPaths' property.
		 * @return full path to the asset, empty string if not found
		 */
		std::string findAsset(const std::string& name) const;

		/**
		 * Returns type of service associated with this module,
		 * @return type of service associated with this module, empty if module has no service.
		 */
		const rtti::TypeInfo getServiceType() const		{ return mService; }

	private:
		std::string						mName;									///< The canonical name of the module
		ModuleDescriptor*				mDescriptor = nullptr;					///< The descriptor that belongs to the module
		std::unique_ptr<ModuleInfo>		mInfo = nullptr;						///< Data that was loaded from the module json
		void*							mHandle = nullptr;						///< Handle to native module
		rtti::TypeInfo					mService = rtti::TypeInfo::empty();		///< Service associated with the module
	};
}

