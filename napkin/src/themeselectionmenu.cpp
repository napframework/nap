#include "themeselectionmenu.h"

ThemeSelectionMenu::ThemeSelectionMenu() : QMenu("Theme")
{
	refresh();
	connect(&AppContext::get().themeManager(), &ThemeManager::themeChanged, this, &ThemeSelectionMenu::onThemeChanged);
}

void ThemeSelectionMenu::refresh()
{
	clear();
	auto defaultThemeAction = new SetThemeAction(nullptr);
	addAction(defaultThemeAction);

	for (auto theme : AppContext::get().themeManager().availableThemes())
	{
		auto action = new SetThemeAction(theme);
		addAction(action);
	}

	checkCurrentTheme();
}

void ThemeSelectionMenu::onThemeChanged(const QString& theme) { checkCurrentTheme(); }

void ThemeSelectionMenu::checkCurrentTheme()
{
	const QString& theme = AppContext::get().themeManager().currentTheme();
	for (auto action : actions())
		action->setChecked(action->text() == theme);
}
