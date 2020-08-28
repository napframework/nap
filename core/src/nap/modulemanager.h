#pragma once

// Local Includes
#include "projectinfo.h"

// External Includes
#include <utility/module.h>
#include <utility/errorstate.h>
#include <rtti/typeinfo.h>
#include <string>
#include <vector>
#include <assert.h>

namespace nap
{
	// Forward Declares
	class Core;
	class ModuleManager;

	/**
	 * Contains all data associated with a module, including handle and description.
	 * Created and managed by the nap::ModuleManager.
	 */
	class NAPAPI Module final
	{
		friend class ModuleManager;
	public:
		Module() = default;

		// Copy is not allowed
		Module(Module&) = delete;

		// Copy assignment is not allowed
		Module& operator=(const Module&) = delete;

		// Move is not allowed
		Module(Module&&) = delete;

		// Move assignment is not allowed
		Module& operator=(Module&&) = delete;

		/**
		 * @return module name
		 */
		const std::string& getName() const					{ return mName; }

		/**
		 * @return module descriptor
		 */
		const ModuleDescriptor& getDescriptor()	const		{ assert(mDescriptor != nullptr); return *mDescriptor; }

		/**
		 * @return module information
		 */
		const ModuleInfo& getInformation() const			{ assert(mInfo != nullptr); return *mInfo; }

		/**
		 * Returns type of service associated with this module, 
		 * @return type of service associated with this module, empty if module has no service.
		 */
		const rtti::TypeInfo getServiceType() const			{ return mService; }

	private:
		std::string						mName;									///< The canonical name of the module
		ModuleDescriptor*				mDescriptor = nullptr;					///< The descriptor that belongs to the module
		std::unique_ptr<ModuleInfo>		mInfo = nullptr;						///< Data that was loaded from the module json
		void*							mHandle = nullptr;						///< Handle to native module
		rtti::TypeInfo					mService = rtti::TypeInfo::empty();		///< Service associated with the module
	};


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
		std::vector<nap::Module*> getModules() const;

		/**
		 * Find a loaded module by its name as defined in its descriptor file
		 * @param moduleName The name of the module to find
		 * @return The object providing access to the Module, nullptr if not found.
		 */
		const Module* findModule(const std::string& moduleName);

	private:
		/**
		 * Loads a module into the current context.
		 * Needs the be implemented in an extension, platform specific.
		 * @param projectinfo project information resource
		 * @param moduleName name of the module to load
		 * @param err contains the error if loading operation fails
		 * @return if module loaded
		 */
		bool sourceModule(const ProjectInfo& projectinfo, const std::string& moduleName, utility::ErrorState& err);

		std::vector<std::unique_ptr<Module>> 			mModules;	// The loaded modules
		Core& 								 			mCore;		// Core
	};
}