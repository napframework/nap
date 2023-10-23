#pragma once

// Local Includes
#include "actions.h"

// External Includes
#include <QObject>

namespace napkin
{
	namespace action
	{
		// All available action group names
		namespace groups
		{
			constexpr const char* project	= "Project";
			constexpr const char* file		= "File";
			constexpr const char* config	= "Configuration";
			constexpr const char* help		= "Help";
			constexpr const char* create	= "Create";
		}
	}

	/**
	 * Creates and groups actions in Napkin
	 */
	class ActionController : public QObject
	{
		Q_OBJECT
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
		 * Returns all actions in a certain group
		 * @param name the group name
		 * @return action group, nullptr if not available
		 */
		const std::vector<Action*>* findGroup(const std::string& name) const;

		/**
		 * Returns all actions in a certain group, asserts if group doesn't exist
		 * @param name the group name
		 * @return action group, asserts if group doesn't exist
		 */
		const std::vector<Action*>& getGroup(const std::string& name) const;

	private:
		std::vector<std::unique_ptr<Action>> mActions;					///< All registered actions
		std::unordered_map<std::string, std::vector<Action*>> mGroups;	///< All registered groups

		// Slots
		void onProjectLoaded(const nap::ProjectInfo& projectInfo);

		// Enable / Disable project related actions
		void enableProjectActions(bool enable);
	};
}
