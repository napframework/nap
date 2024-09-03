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
			constexpr const char* dark1							= "dark1";
			constexpr const char* dark2							= "dark2";
			constexpr const char* background1					= "background1";
			constexpr const char* background2					= "background2";
			constexpr const char* highlight1					= "highlight1";
			constexpr const char* highlight2					= "highlight2";
			constexpr const char* highlight3					= "highlight3";
			constexpr const char* front1						= "front1";
			constexpr const char* front2						= "front2";
			constexpr const char* front3						= "front3";
			constexpr const char* front4						= "front4";
			constexpr const char* instancePropertyOverride		= "instancePropertyOverride";
			constexpr const char* dimmedItem					= "dimmedItem";
		}

		namespace font
		{
			constexpr const char* regular						= "regularFont";
			constexpr const char* mono							= "monoFont";
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
		 * @return specific color associated with given key, invalid if key doesn't exist
		 */
		QColor getColor(const QString& key) const;

		/**
		 * Returns the font associated with the given key, default font if key is invalid or font isn't loaded 
		 * @param key font key id (monoSpace, etc..)
		 * @param style font style (QFont::StyleNormal etc..)
		 * @param pointSize font size
		 * @return font for the given key, default font when key is invalid or font isn't loaded
		 */
		QFont getFont(const QString& key, const QString& style, int pointSize) const;

		/**
		 * Returns the font family name associated with the given key, invalid font name if key not available
		 * @param key font identifier
		 * @return font name for given key, invalid if not available
		 */
		QString getFontName(const QString& key) const;

		/**
		 * Updates the font (family) for the given widget, whilst maintaining font properties
		 * @param widget the widget to change the font for
		 * @param key font key identifier
		 */
		void changeWidgetFont(QWidget& widget, const QString& key) const;

		/**
		 * @return if the icons should be inverted or not
		 */
		bool invertIcons() const;

		/**
		 * @return all color replacement ids
		 */
		const QMap<QString, QColor>& getColors() const;

		/**
		 * @return all font replacement ids
		 */
		const QMap<QString, QString>& getFonts() const;

		/**
		 * @return regular or inverted icon, based on theme settings
		 */
		QIcon getIcon(const QString& path);

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
		bool mInvertIcons = false;
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
		 * @param key color key (front, back etc...) 
		 * @return A color defined by the theme, invalid when color does not exist
		 */
		QColor getColor(const QString& key) const;

		/**
		 * Returns the font family name associated with the given key, invalid font name if key not available
		 * @param key font identifier
		 * @return font name for given key, invalid if not available
		 */
		QString getFontName(const QString& key) const;

		/**
		 * Returns the font associated with the given key, returns the default font if key is invalid or font isn't loaded
		 * @param key font identifier (monoSpace, etc..)
		 * @param style font style (QFont::StyleNormal etc..)
		 * @param pointSize font size
		 * @return font for the given key, default font when key is invalid or font isn't loaded
		 */
		QFont getFont(const QString& key, const QString& style, int pointSize) const;

		/**
		 * Updates the font (family) for the given widget, whilst maintaining font properties
		 * @param widget the widget to change the font for
		 * @param key font key identifier
		 */
		void changeWidgetFont(QWidget& widget, const QString& key) const;

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
