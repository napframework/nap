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
	/**
	 * Theme Globals
	 */
	namespace theme
	{
		inline constexpr const char* filename = "theme.json";
		inline constexpr const char* directory = "resources/themes";

		namespace color
		{
			inline constexpr const char* highlight					= "highlight";
			inline constexpr const char* background1				= "background1";
			inline constexpr const char* background2				= "background2";
			inline constexpr const char* dark						= "dark";
			inline constexpr const char* front0						= "front0";
			inline constexpr const char* front1						= "front1";
			inline constexpr const char* front2						= "front2";
			inline constexpr const char* front3						= "front3";
			inline constexpr const char* componentoverride			= "componentWithOverrides";
			inline constexpr const char* instanceProperty			= "instanceProperty";
			inline constexpr const char* overriddenInstanceProperty = "overriddenInstanceProperty";
			inline constexpr const char* dimmedItem					= "dimmedItem";
		}
	}


	/**
	 * Fonts Globals
	 */
	namespace font
	{
		inline constexpr const char* directory = "resources/fonts";
		inline constexpr const char* extension = "*.ttf";
	}


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
		const QMap<QString, QColor>& getColors() const;
		const QMap<QString, QString>& getFonts() const;

	private:
		bool loadTheme();

		bool mIsValid = false;
		QString mStylesheetFilename;
		const QString mFilename;
		QString mName;
		QMap<int, QColor> mLogColors;
		QMap<QString, QColor> mColors;
		QMap<QString, QString> mFonts;
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
		 * @return The directory that contains all the font files
		 */
		const QString getFontDir() const;

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

		const Theme* mCurrentTheme = nullptr;			///< The currently set theme
		QFileSystemWatcher mFileWatcher;				///< Watch the theme file and reload if it has changed
		QSet<QString> mLoadedFonts;						///< All fonts that are loaded
		std::vector<std::unique_ptr<Theme>> mThemes;	///< All currently loaded themes
		QSet<QString> mWatchedFilenames;				///< Keep track of the files we're watching
	};
};
