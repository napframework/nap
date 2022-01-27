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
	class ThemeManager;

	/**
	 * Theme Globals
	 */
	namespace theme
	{
		inline constexpr const char* filename = "theme.json";
		inline constexpr const char* directory = "resources/themes";

		namespace color
		{
			inline constexpr const char* dark1						= "dark1";
			inline constexpr const char* dark2						= "dark2";
			inline constexpr const char* background1				= "background1";
			inline constexpr const char* background2				= "background2";
			inline constexpr const char* highlight1					= "highlight1";
			inline constexpr const char* highlight2					= "highlight2";
			inline constexpr const char* highlight3					= "highlight3";
			inline constexpr const char* front1						= "front1";
			inline constexpr const char* front2						= "front2";
			inline constexpr const char* front3						= "front3";
			inline constexpr const char* front4						= "front4";
			inline constexpr const char* instancePropertyOverride	= "instancePropertyOverride";
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
		friend class ThemeManager;
	public:
		Theme(const QString& filename);
		Theme(const Theme&) = delete;
		Theme& operator=(const Theme&) = delete;

		/**
		 * @return absolute path to theme file
		 */
		const QString& getFilePath() const { return mFilePath; }

		/**
		 * @return absolute path to style sheet
		 */
		const QString& getStylesheetFilePath() const;

		/**
		 * @return if the theme loaded
		 */
		bool isValid() const;

		/**
		 * @return theme name
		 */
		const QString& getName() const { return mName; }

		/**
		 * @return log color for the given level
		 */
		QColor getLogColor(const nap::LogLevel& lvl) const;

		/**
		 * @return specific color associated with given key
		 */
		QColor getColor(const QString& key) const;

		/**
		 * @return all color replacement ids
		 */
		const QMap<QString, QColor>& getColors() const;

		/**
		 * @return all font replacement ids
		 */
		const QMap<QString, QString>& getFonts() const;

	private:
		/**
		 * Attempts to reload the theme
		 */
		bool reload();

		/**
		 * Loads the theme
		 */
		bool loadTheme();

		bool mIsValid = false;
		QString mStylesheetFilePath;
		const QString mFilePath;
		QString mName;
		QMap<int, QColor> mLogColors;
		QMap<QString, QColor> mColors;
		QMap<QString, QString> mFonts;
		bool mValid = false;
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
         * @return The directory containing the themes
         */
		static QString getThemeDir();

		/**
		 * @return The directory that contains all the font files
		 */
		static QString getFontDir();

        /**
         * @return A list of available theme names
         */
		const std::vector<std::unique_ptr<Theme>>& getAvailableThemes();

		/**
		 * @param name the name of the theme
		 */
		void setTheme(const QString& name);

		/**
		 * Find the theme with the given name
		 * @param name the name of the theme to be found
		 * @return the theme with the given name or nullptr if no theme with that name could be found
		 */
		const Theme* findTheme(const QString& name) const;

        /**
         * @return The currently set theme
         */
		const Theme* getCurrentTheme() const;

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
         * Set apply the specified theme by name.
         * @param theme The name of the theme
         */
		void setTheme(Theme* theme);

		/**
		 * Find the theme with the given name
		 * @param name the name of the theme to be found
		 * @return the theme with the given name or nullptr if no theme with that name could be found
		 */
		Theme* findTheme(const QString& name);

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

		Theme* mCurrentTheme = nullptr;					///< The currently set theme
		QFileSystemWatcher mFileWatcher;				///< Watch the theme file and reload if it has changed
		QSet<QString> mLoadedFonts;						///< All fonts that are loaded
		std::vector<std::unique_ptr<Theme>> mThemes;	///< All currently loaded themes
		QSet<QString> mWatchedFilenames;				///< Keep track of the files we're watching
	};
};
