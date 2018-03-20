#include "thememanager.h"

#include <QDir>
#include <QSettings>
#include <QTextStream>
#include <QFontDatabase>
#include <QStyleFactory>

#include <nap/logger.h>

#include "napkinglobals.h"
#include "appcontext.h"
#include "napkinresources.h"

using namespace napkin;

ThemeManager::ThemeManager() : mCurrentTheme(TXT_THEME_NATIVE)
{
	connect(&mFileWatcher, &QFileSystemWatcher::directoryChanged, this, &ThemeManager::onFileChanged);
	connect(&mFileWatcher, &QFileSystemWatcher::fileChanged, this, &ThemeManager::onFileChanged);
}


void ThemeManager::setTheme(const QString& themeName)
{
	loadFonts();

	if (themeName.isEmpty())
	{
		mCurrentTheme = napkin::TXT_THEME_NATIVE;
	}
	else
	{
		auto theme_filename = getThemeFilename(themeName);
		if (!QFileInfo::exists(theme_filename))
		{
			nap::Logger::warn("File not found: %s", theme_filename.toStdString().c_str());
			return;
		}

		mCurrentTheme = themeName;
	}

	reloadTheme();
	QSettings().setValue(settingsKey::LAST_THEME, mCurrentTheme);
	themeChanged(mCurrentTheme);
}

const QString& ThemeManager::getCurrentTheme() const
{
	return mCurrentTheme;
}

QStringList ThemeManager::getAvailableThemes()
{
	QStringList names;

	for (const auto& filename : QDir(getThemeDir()).entryInfoList())
	{
		if (filename.suffix() == sThemeFileExtension)
			names << filename.baseName();
	}
	return names;
}

const QString ThemeManager::getThemeDir() const
{
	// TODO: This probably needs to be configurable
	return QString("%1/%2").arg(QCoreApplication::applicationDirPath(), sThemeSubDirectory);
}

void ThemeManager::reloadTheme()
{
	auto app = AppContext::get().getQApplication();

	if (mCurrentTheme == TXT_THEME_NATIVE)
	{
		app->setStyleSheet(nullptr);
//		app->setStyle(nullptr);
		app->setStyle(QStyleFactory::create("Fusion"));
		return;

	}

	if (mCurrentTheme.isEmpty())
	{
		nap::Logger::warn("No theme set, not reloading");
		return;
	}

	auto theme_filename = getThemeFilename(mCurrentTheme);
	QFile file(theme_filename);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		nap::Logger::warn("Could not load file: %s", theme_filename.toStdString().c_str());
		return;
	}

	QTextStream in(&file);
	auto styleSheet = in.readAll();
	file.close();

	mFileWatcher.addPath(theme_filename);

	app->setStyle(QStyleFactory::create("Fusion"));
	app->setStyleSheet(styleSheet);

}

const QString ThemeManager::getThemeFilename(const QString& themeName) const
{
	return QString("%1/%2.%3").arg(getThemeDir(), themeName, sThemeFileExtension);
}

void ThemeManager::onFileChanged(const QString& path)
{
	auto theme_filename = getThemeFilename(mCurrentTheme);
	QFileInfo path_info(path);
	if (path_info.filePath() == theme_filename) {
		nap::Logger::info("Reloading: %s", path.toStdString().c_str());
		reloadTheme();
	}

	mFileWatcher.addPath(theme_filename);
}

void ThemeManager::loadFonts()
{
	if (mFontsLoaded)
		return;

	QStringList fonts;
	fonts << QRC_FONTS_MONTSERRAT_EXTRABOLD;
	fonts << QRC_FONTS_MONTSERRAT_LIGHT;
	fonts << QRC_FONTS_MONTSERRAT_MEDIUM;
	fonts << QRC_FONTS_MONTSERRAT_SEMIBOLD;
	fonts << QRC_FONTS_NUNITOSANS_EXTRABOLD;
	fonts << QRC_FONTS_NUNITOSANS_REGULAR;

	for (auto font : fonts)
	{
		if (QFontDatabase::addApplicationFont(font) < 0)
			nap::Logger::warn("Failed to load font: '%s'", font.toStdString().c_str());
	}

	mFontsLoaded = true;
}

void ThemeManager::watchThemeDir()
{
	mFileWatcher.addPath(getThemeDir());
}
