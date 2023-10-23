#pragma once

// Local Includes
#include "actions.h"

namespace napkin
{
	// Action groups
	namespace actions
	{
		constexpr const char* project	= "Project";
		constexpr const char* file		= "File";
		constexpr const char* config	= "Configuration";
		constexpr const char* help		= "Help";
		constexpr const char* create	= "Create";
	}

	/**
	 * Creates and manages all actions in Napkin.
	 */
	class ActionController final
	{
	public:
		/**
		 * Creates the actions
		 */
		ActionController();

		/**
		 * Destroys the actions
		 */
		virtual ~ActionController();

		/**
		 * Copy is not allowed
		 */
		ActionController(ActionController&) = delete;

		/**
		 * Copy assignment is not allowed
		 */
		ActionController& operator=(const ActionController&) = delete;

		/**
		 * Move is not allowed
		 */
		ActionController(ActionController&&) = delete;

		/**
		 * Move assignment is not allowed
		 */
		ActionController& operator=(ActionController&&) = delete;

		/**
		 * @return project actions
		 */
		const std::vector<Action*>& getProjectActions() const	{ return mProjectActions; }

		/**
		 * @return file actions
		 */
		const std::vector<Action*>& getFileActions() const		{ return mFileActions; }

		/**
		 * @return service actions
		 */
		const std::vector<Action*>& getServiceActions() const	{ return mServiceActions; }

		/**
		 * @return help actions
		 */
		const std::vector<Action*>& getHelpActions() const		{ return mHelpActions; }

	private:
		std::vector<std::unique_ptr<Action>> mActions;	///< All registered actions

		std::vector<Action*> mProjectActions;			///< All project actions
		std::vector<Action*> mFileActions;				///< All file actions
		std::vector<Action*> mServiceActions;			///< All service actions
		std::vector<Action*> mHelpActions;				///< All help actions
		std::vector<std::vector<Action*>*> mGroups;		///< All groups
	};
}
