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
#include <mutex>

namespace nap
{
	// Forward declare
	class ModuleCache;

	/**
	 * Contains all data associated with a module, including handle and description.
	 * Created and managed by the nap::ModuleManager.
	 */
	class NAPAPI Module final
	{
		friend class ModuleCache;
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
		const std::string& getName() const { return mName; }

		/**
		 * @return module descriptor
		 */
		const ModuleDescriptor& getDescriptor()	const { assert(mDescriptor != nullptr); return *mDescriptor; }

		/**
		 * @return module information
		 */
		const ModuleInfo& getInformation() const { assert(mInfo != nullptr); return *mInfo; }

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
		const rtti::TypeInfo getServiceType() const { return mService; }

		/**
		 * Returns all NAP module dependencies, including child dependencies, in order from bottom to top.
		 * @return all NAP module dependencies, including child dependencies, in order from bottom to top.
		 */
		const std::vector<const nap::Module*> getDependencies() const { return mDependencies; }

	private:
		std::string						mName;									///< The canonical name of the module
		ModuleDescriptor*				mDescriptor = nullptr;					///< The descriptor that belongs to the module
		std::unique_ptr<ModuleInfo>		mInfo = nullptr;						///< Data that was loaded from the module json
		void*							mHandle = nullptr;						///< Handle to native module
		rtti::TypeInfo					mService = rtti::TypeInfo::empty();		///< Service associated with the module
		std::vector<const nap::Module*>	mDependencies;							///< Ordered (bottom to top) module dependency list
	};


	//////////////////////////////////////////////////////////////////////////
	// Module Cache
	//////////////////////////////////////////////////////////////////////////

	/**
	 * NAP dynamic library module cache.
	 * Provides an interface for thread-safe loading of NAP modules in a host process.
	 * Call getHandle() to get exclusive access to the cache until the handle is destroyed.
	 * 
	 * Every NAP project depends on a set of (system / user) modules.
	 * This cache ensures that modules with the same module name are sourced only once, which is of importance when
	 * multiple NAP projects are executed within the same host process, such as applets within Napkin.
	 */
	class NAPAPI ModuleCache final
	{
	public:
		/**
		 * Returns a unique handle to this process module.
		 * Any other thread that wants to access the module cache must wait
		 * until this handle is destroyed and the lock released.
		 * @return handle to host process module cache
		 */
		class  Handle;
		static Handle getHandle();

		// Copy and move is not allowed
		ModuleCache(ModuleCache&) = delete;
		ModuleCache& operator=(const ModuleCache&) = delete;
		ModuleCache(ModuleCache&&) = delete;
		ModuleCache& operator=(ModuleCache&&) = delete;

		/**
		 * Unique handle to this process module cache.
		 * Any other thread that wants to source a module must wait
		 * until this handle is destroyed and the lock released.
		 */
		class Handle final
		{
			friend class ModuleCache;
		public:
			/**
			 * Attempts to find and load a module into the current NAP process, including all module dependencies.
			 * @param projectinfo project information resource
			 * @param moduleName name of the module to load
			 * @param err contains the error if loading operation fails
			 * @return if loading the module succeeded
			 */
			const nap::Module* sourceModule(const ProjectInfo& projectinfo, const std::string& moduleName, utility::ErrorState& err);

			// Copy is not allowed
			Handle(Handle&) = delete;
			Handle& operator=(const Handle&) = delete;

			// Move is allowed
			Handle(Handle&& other) noexcept;
			Handle& operator=(Handle&& other) noexcept;

		private:
			Handle(nap::ModuleCache& cache, std::mutex& mutex);		///< Only the cache can create the handle
			std::unique_lock<std::mutex> mLock;						///< Cache lock, can be moved
			nap::ModuleCache* mCache = nullptr;						///< Cache handle
		};

	private:
		// Hide constructor
		ModuleCache() = default;

		// Attempts to load a NAP module into the current NAP process.
		const nap::Module* sourceModule(const ProjectInfo& projectinfo, const std::string& moduleName, utility::ErrorState& err);

		// Find a loaded module by its name as defined in its descriptor file
		const Module* findModule(const std::string& moduleName) const;

		// List of unique modules loaded in current host process
		std::vector<std::unique_ptr<Module>> mModules;
	};
}

