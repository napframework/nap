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

	for (const auto& theme : themes)
		addAction(new SetThemeAction(theme->getName()));

	checkCurrentTheme();
}

void ThemeSelectionMenu::onThemeChanged(const Theme* theme)
{
	checkCurrentTheme();
}

void ThemeSelectionMenu::checkCurrentTheme()
{
	const auto theme = AppContext::get().getThemeManager().getCurrentTheme();
	if (theme == nullptr)
		return;
	for (auto action : actions())
		action->setChecked(action->text() == theme->getName());
}
