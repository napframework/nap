#include "themeselectionmenu.h"

using namespace napkin;

ThemeSelectionMenu::ThemeSelectionMenu() : QMenu("Theme")
{
	refresh();
	connect(&AppContext::get().getThemeManager(), &ThemeManager::themeChanged, this,
			&ThemeSelectionMenu::onThemeChanged);
}

void ThemeSelectionMenu::refresh()
{
	clear();

	const auto& themes = AppContext::get().getThemeManager().getAvailableThemes();
	if (themes.empty())
	{
		setEnabled(false);
		return;
	}
	setEnabled(true);

	for (auto theme : AppContext::get().getThemeManager().getAvailableThemes())
	{
		auto action = new SetThemeAction(theme);
		addAction(action);
	}

	checkCurrentTheme();
}

void ThemeSelectionMenu::onThemeChanged(const QString& theme)
{
	checkCurrentTheme();
}

void ThemeSelectionMenu::checkCurrentTheme()
{
	const QString& theme = AppContext::get().getThemeManager().getCurrentTheme();
	for (auto action : actions())
		action->setChecked(action->text() == theme);
}
