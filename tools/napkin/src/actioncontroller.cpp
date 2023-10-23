// Local Includes
#include "actioncontroller.h"
#include "appcontext.h"

namespace napkin
{
	/**
	 * Creates and registers an actions of type T using the arguments provided.
	 * @param outActions global action container
	 * @param outGroup action group
	 * @param args Action construction arguments
	 */
	template<typename T, typename... Args>
	static void registerAction(std::vector<std::unique_ptr<Action>>& outActions, std::vector<Action*>& outGroup, Args&&... args)
	{
		auto& i = outActions.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		outGroup.emplace_back(i.get());
	}


	/**
	 * Creates and registers a new group with the given name
	 * @param name the group name
	 */
	std::vector<Action*>& registerGroup(std::string&& name, std::unordered_map<std::string, std::vector<Action*>>& outGroups)
	{
		auto group_it = outGroups.try_emplace(std::move(name), std::vector<Action*>()); assert(group_it.second);
		return group_it.first->second;
	}


	ActionController::ActionController()
	{
		// Project actions
		auto& project_group = registerGroup(action::groups::project, mGroups);
		registerAction<OpenProjectAction>(mActions, project_group);

		// File actions
		auto& file_group = registerGroup(action::groups::file, mGroups);
		registerAction<NewFileAction>(mActions, file_group);
		registerAction<OpenFileAction>(mActions, file_group);
		registerAction<SaveFileAction>(mActions, file_group);
		registerAction<SaveFileAsAction>(mActions, file_group);
		registerAction<ReloadFileAction>(mActions, file_group);
		registerAction<UpdateDefaultFileAction>(mActions, file_group);

		// Service actions
		auto& config_group = registerGroup(action::groups::config, mGroups);
		registerAction<NewServiceConfigAction>(mActions, config_group);
		registerAction<OpenServiceConfigAction>(mActions, config_group);
		registerAction<SaveServiceConfigAction>(mActions, config_group);
		registerAction<SaveServiceConfigurationAs>(mActions, config_group);
		registerAction<SetAsDefaultServiceConfigAction>(mActions, config_group);

		// Help actions
		auto& help_group = registerGroup(action::groups::help, mGroups);
		registerAction<OpenURLAction>(mActions, help_group, "NAP Documentation", QUrl("https://docs.nap.tech"));

		// Create actions
		auto& create_group = registerGroup(action::groups::create, mGroups);
		registerAction<CreateResourceAction>(mActions, create_group);
		registerAction<CreateEntityAction>(mActions, create_group);
		registerAction<CreateGroupAction>(mActions, create_group);

		// Enable / Disable actions based on project state
		enableProjectActions(false);
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &ActionController::onProjectLoaded);
	}


	ActionController::~ActionController()
	{
		// Clear groups and delete actions
		for (auto& group : mGroups)
			group.second.clear();
		mActions.clear();
	}


	const std::vector<Action*>* ActionController::findGroup(const std::string& name) const
	{
		auto it = mGroups.find(name);
		return it != mGroups.end() ? &(it->second) : nullptr;
	}


	const std::vector<Action*>& ActionController::getGroup(const std::string& name) const
	{
		auto* group = findGroup(name); assert(group != nullptr);
		return *group;
	}


	void ActionController::onProjectLoaded(const nap::ProjectInfo& projectInfo)
	{
		enableProjectActions(true);
	}


	void ActionController::enableProjectActions(bool enable)
	{
		// Project aware action groups
		static const std::vector<std::string> project_groups
		{
			action::groups::file,
			action::groups::config,
			action::groups::create
		};

		// Disable / Enable based
		for(const auto& group_name : project_groups)
		{
			auto& group = getGroup(group_name);
			for (const auto& action : group)
			{
				action->setEnabled(enable);
			}
		}
	}
}
