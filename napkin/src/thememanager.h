#pragma once

#include <QObject>
#include <QtCore/QFileSystemWatcher>
#include <memory>
#include <nap/logger.h>
#include <QtCore/QMap>
#include <QtCore/QSet>

namespace napkin
{
	static const QString sThemeFilename = "theme.json";
	static const QString sThemeSubDirectory = "resources/themes";

	/**
	 * Represents one theme
	 */
	class Theme
	{
	public:
		Theme(const QString& filename);
		Theme(const Theme&) = delete;
		Theme& operator=(const Theme&) = delete;

		const QString& getFilename() const { return mFilename; }
		const QString& getStylesheetFilename() const;
		bool isValid() const;
		const QString& getName() const { return mName; }
		QColor getLogColor(const nap::LogLevel& lvl) const;
	private:
		bool loadTheme();

		bool mIsValid = false;
		QString mStylesheetFilename;
		const QString mFilename;
		QString mName;
		QMap<const nap::LogLevel*, QColor> mLogColors;
	};

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
		const std::vector<std::unique_ptr<Theme>>& getAvailableThemes();

        /**
         * Set apply the specified theme by name.
         * @param theme The name of the theme
         */
		void setTheme(const Theme* theme);

		/**
		 * @param name the name of the theme
		 */
		void setTheme(const QString& name);

		/**
		 * Find the theme with the given name
		 * @param name the name of the theme to be found
		 * @return the theme with the given name or nullptr if no theme with that name could be found
		 */
		const Theme* getTheme(const QString& name);

        /**
         * @return The name of the currently set theme
         */
		const Theme* getCurrentTheme() const;

        /**
         * @return The directory containing the themes
         */
		const QString getThemeDir() const;

		/**
		 * @return The color for the log level in the current theme
		 */
		QColor getLogColor(const nap::LogLevel& lvl) const;

		/**
		 * Start watching the theme dir for changes and update when necessary.
		 */
		void watchThemeDir();

	Q_SIGNALS:
        /**
         * Will be fired when the theme has changed
         * @param theme The name of the theme
         */
        void themeChanged(const Theme* theme);

	private:

		/**
		 * Load all themes from the theme directory
		 */
		void loadThemes();

		/**
		 * Reload and apply the current theme from file
		 */
		void applyTheme();

		/**
		 * Invoked when a file in the theme dir has changed
		 */
		void onFileChanged(const QString& path);

		/**
		 * Retrieve all fonts from the resource file
		 * TODO: Move into Theme
		 */
		void loadFonts();

		/**
		 * For live updating of themes
		 */
		void watchThemeFiles();

		const Theme* mCurrentTheme = nullptr; ///< The currently set theme
		QFileSystemWatcher mFileWatcher; ///< Watch the theme file and reload if it has changed
		bool mFontsLoaded = false; ///< keep track of loaded fonts
		std::vector<std::unique_ptr<Theme>> mThemes; ///< All currently loaded themes
		QSet<QString> mWatchedFilenames; ///< Keep track of the files we're watching
	};
};