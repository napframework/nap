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
			constexpr const char* resources = "Resources";
			constexpr const char* object	= "Object";
		}
	}

	/**
	 * Basic action model - creates actions and groups them for repeated use.
	 * Use `getGroup()` in combination with the `napkin::action::groups` namespace to get
	 * all the actions associated with a specific group.
	 */
	class ActionModel final
	{
	public:
		/**
		 * Creates the actions
		 */
		ActionModel();

		/**
		 * Destroys the actions
		 */
		virtual ~ActionModel();

		/**
		 * Copy is not allowed
		 */
		ActionModel(ActionModel&) = delete;

		/**
		 * Copy assignment is not allowed
		 */
		ActionModel& operator=(const ActionModel&) = delete;

		/**
		 * Move is not allowed
		 */
		ActionModel(ActionModel&&) = delete;

		/**
		 * Move assignment is not allowed
		 */
		ActionModel& operator=(ActionModel&&) = delete;

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
	};
}
