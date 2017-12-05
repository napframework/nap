#pragma once

#include <QObject>
namespace napkin
{
	/**
	 * Keep track of and allow changing the visual style of the application.
	 */
	class ThemeManager : public QObject
	{
		Q_OBJECT
	public:
		ThemeManager();

        /**
         * @return A list of available theme names
         */
		QStringList getAvailableThemes();

        /**
         * Set apply the specified theme by name.
         * @param themeName The name of the theme
         */
		void setTheme(const QString& themeName);

        /**
         * @return The name of the currently set theme
         */
		const QString& getCurrentTheme() const;

        /**
         * @return The directory containing the themes
         */
		QString getThemeDir();

	Q_SIGNALS:
        /**
         * Will be fired when the theme has changed
         * @param theme The name of the theme
         */
        void themeChanged(const QString& theme);

	private:
		QString mCurrentTheme; // The currently set theme
	};
};