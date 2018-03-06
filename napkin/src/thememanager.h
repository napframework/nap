#pragma once

#include <QObject>
#include <QtCore/QFileSystemWatcher>

namespace napkin
{
	static const QString sThemeFileExtension = "qss";
	static const QString sThemeSubDirectory = "resources/themes";
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
		const QString getThemeDir() const;

		/**
		 * Start watching the theme dir for changes and update when necessary.
		 */
		void watchThemeDir();

	Q_SIGNALS:
        /**
         * Will be fired when the theme has changed
         * @param theme The name of the theme
         */
        void themeChanged(const QString& theme);

	private:
		/**
		 * Reload and apply the current theme from file
		 */
		void reloadTheme();

		/**
		 * @return The theme filename based on the provided theme name
		 */
		const QString getThemeFilename(const QString& themeName) const;

		/**
		 * Invoked when a file in the theme dir has changed
		 */
		void onFileChanged(const QString& path);

		void loadFonts();

		QString mCurrentTheme; // The currently set theme
		QFileSystemWatcher mFileWatcher; // Watch the theme file and reload if it has changed
		bool mFontsLoaded = false;
	};
};