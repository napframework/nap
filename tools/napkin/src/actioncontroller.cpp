#include "actioncontroller.h"

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


	ActionController::ActionController()
	{
		// Project actions
		mActions.reserve(1);
		registerAction<OpenProjectAction>(mActions, mProjectActions);
		mGroups.emplace_back(&mProjectActions);

		// File actions
		registerAction<NewFileAction>(mActions, mFileActions);
		registerAction<OpenFileAction>(mActions, mFileActions);
		registerAction<SaveFileAction>(mActions, mFileActions);
		registerAction<SaveFileAsAction>(mActions, mFileActions);
		registerAction<ReloadFileAction>(mActions, mFileActions);
		registerAction<UpdateDefaultFileAction>(mActions, mFileActions);
		mGroups.emplace_back(&mFileActions);

		// Service actions
		registerAction<NewServiceConfigAction>(mActions, mServiceActions);
		registerAction<OpenServiceConfigAction>(mActions, mServiceActions);
		registerAction<SaveServiceConfigAction>(mActions, mServiceActions);
		registerAction<SaveServiceConfigurationAs>(mActions, mServiceActions);
		registerAction<SetAsDefaultServiceConfigAction>(mActions, mServiceActions);
		mGroups.emplace_back(&mServiceActions);

		// Help actions
		registerAction<OpenURLAction>(mActions, mHelpActions, "NAP Documentation", QUrl("https://docs.nap.tech"));
		mGroups.emplace_back(&mHelpActions);
	}


	ActionController::~ActionController()
	{
		// Clear groups and delete actions
		for (auto* group : mGroups)
			group->clear();
		mActions.clear();
	}

}
