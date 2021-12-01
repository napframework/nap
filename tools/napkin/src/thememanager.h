/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QtCore/QFileSystemWatcher>
#include <memory>
#include <nap/logger.h>
#include <QtCore/QMap>
#include <QtCore/QSet>

namespace napkin
{
	inline constexpr const char* themeFilename = "theme.json";
	inline constexpr const char* themeSubDirectory = "resources/themes";
	inline constexpr const char* themeColComponentWidthOverrides = "componentWithOverrides";
	inline constexpr const char* themeColInstanceProperty = "instanceProperty";
	inline constexpr const char* themeColOverriddenInstanceProperty = "overriddenInstanceProperty";
	inline constexpr const char* themeColDimmedItem = "dimmedItem";

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
		QColor getColor(const QString& key) const;
	private:
		bool loadTheme();

		bool mIsValid = false;
		QString mStylesheetFilename;
		const QString mFilename;
		QString mName;
		QMap<int, QColor> mLogColors;
		QMap<QString, QColor> mColors;
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
		 * @return A color defined by the theme, by name.
		 */
		QColor getColor(const QString& key) const;

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
