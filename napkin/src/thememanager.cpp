#include "thememanager.h"

#include <QDir>
#include <QFontDatabase>
#include <QSettings>
#include <QStyleFactory>
#include <QTextStream>

#include <nap/logger.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <utility/fileutils.h>

#include "appcontext.h"
#include "napkinresources.h"
#include "napkinglobals.h"

using namespace napkin;


Theme::Theme(const QString& filename) : mFilename(filename)
{
	loadTheme();
}

bool Theme::isValid() const
{
	if (!QFileInfo::exists(mFilename))
		return false;

	return true;
}

const QString& Theme::getStylesheetFilename() const
{
	return mStylesheetFilename;
}

bool Theme::loadTheme()
{
	rapidjson::Document doc;
	std::string data;
	nap::utility::ErrorState err;
	nap::utility::readFileToString(mFilename.toStdString(), data, err);
	rapidjson::ParseResult ok = doc.Parse(data.c_str());
	if (!ok)
	{
		nap::Logger::error("JSON Parse error in %s: %s, offset: %s", mFilename.toStdString().c_str(),
						   rapidjson::GetParseError_En(ok.Code()), ok.Offset());
		return false;
	}

	// Read properties

	mName = QString::fromStdString(doc["name"].GetString());

	if (doc.HasMember("stylesheet"))
	{
		// Stylesheet is relative to style json file
		mStylesheetFilename = QFileInfo(mFilename).absolutePath() + "/" + doc["stylesheet"].GetString();
	}

	mLogColors.clear();
	auto logColors = doc["logColors"].GetObject();
	for (const auto logLevel : nap::Logger::getLevels())
	{
		std::string colname = logColors[logLevel->name().c_str()].GetString();
		QColor col(QString::fromStdString(colname));
		mLogColors.insert(logLevel, col);
	}

	return true;
}

QColor Theme::getLogColor(const nap::LogLevel& lvl) const
{
	if (mLogColors.contains(&lvl))
		return mLogColors[&lvl];
	return QColor();
}


ThemeManager::ThemeManager()
{
	connect(&mFileWatcher, &QFileSystemWatcher::directoryChanged, this, &ThemeManager::onFileChanged);
	connect(&mFileWatcher, &QFileSystemWatcher::fileChanged, this, &ThemeManager::onFileChanged);
}


void ThemeManager::setTheme(const Theme* theme)
{
	loadFonts();
	mCurrentTheme = theme;
	applyTheme();
	QString themeName = "";
	if (mCurrentTheme != nullptr)
		themeName = mCurrentTheme->getName();

	QSettings().setValue(settingsKey::LAST_THEME, themeName);
	themeChanged(mCurrentTheme);
}

void ThemeManager::setTheme(const QString& name)
{
	nap::Logger::fine("Setting theme: %s", name.toStdString().c_str());
	setTheme(getTheme(name));
}


const Theme* ThemeManager::getTheme(const QString& name)
{
	for (const auto& theme : mThemes)
		if (theme->getName() == name)
			return theme.get();
	return nullptr;
}

const Theme* ThemeManager::getCurrentTheme() const
{
	return mCurrentTheme;
}

const std::vector<std::unique_ptr<Theme>>& ThemeManager::getAvailableThemes()
{
	if (mThemes.empty())
		loadThemes();

	return mThemes;
}

const QString ThemeManager::getThemeDir() const
{
	// TODO: This probably needs to be configurable
	return QString("%1/%2").arg(QCoreApplication::applicationDirPath(), sThemeSubDirectory);
}

void ThemeManager::applyTheme()
{
	auto app = AppContext::get().getQApplication();

	if (!mCurrentTheme || !QFileInfo::exists(mCurrentTheme->getStylesheetFilename()))
	{
		app->setStyleSheet(nullptr);
		QApplication::setStyle(QStyleFactory::create("Fusion"));
		return;
	}

	auto stylesheetFile = mCurrentTheme->getStylesheetFilename();
	QFile file(stylesheetFile);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		nap::Logger::warn("Could not load file: %s", stylesheetFile.toStdString().c_str());
		return;
	}

	QTextStream in(&file);
	auto styleSheet = in.readAll();
	file.close();

	// Start watching for file changes
	mWatchedFilenames.clear();
	mWatchedFilenames << stylesheetFile;
	mWatchedFilenames << mCurrentTheme->getFilename();
	mWatchedFilenames << QFileInfo(mCurrentTheme->getFilename()).absolutePath();
	watchThemeFiles();

	QApplication::setStyle(QStyleFactory::create("Fusion"));
	app->setStyleSheet(styleSheet);
}

void ThemeManager::watchThemeFiles()
{
	for (const auto& filename : mWatchedFilenames)
	{
		mFileWatcher.addPath(filename);
	}
}


void ThemeManager::onFileChanged(const QString& path)
{
	auto theme_filename = getCurrentTheme()->getStylesheetFilename();

	if (QFileInfo(path).filePath() == theme_filename)
	{
		nap::Logger::info("Reloading: %s", path.toStdString().c_str());
		applyTheme();
	}

	watchThemeFiles();
}

void ThemeManager::loadFonts()
{
	if (mFontsLoaded)
		return;

	// TODO: Move this to the theme
	QStringList fonts;
	fonts << QRC_FONTS_MONTSERRAT_EXTRABOLD;
	fonts << QRC_FONTS_MONTSERRAT_LIGHT;
	fonts << QRC_FONTS_MONTSERRAT_MEDIUM;
	fonts << QRC_FONTS_MONTSERRAT_SEMIBOLD;
	fonts << QRC_FONTS_NUNITOSANS_EXTRABOLD;
	fonts << QRC_FONTS_NUNITOSANS_REGULAR;

	for (const auto& font : fonts)
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


void ThemeManager::loadThemes()
{
	for (const auto& dir : QDir(getThemeDir()).entryInfoList(QDir::AllDirs))
	{
		auto filename = QString("%1/%2").arg(dir.absoluteFilePath(), sThemeFilename);
		if (!QFileInfo::exists(filename))
			continue;
		nap::Logger::fine("Loading theme: %s", filename.toStdString().c_str());
		auto theme = std::make_unique<Theme>(filename);

		// Check for duplicated
		const auto existingTheme = getTheme(theme->getName());
		if (existingTheme != nullptr)
		{
			nap::Logger::error("Duplicate theme name '%s' while loading '%s', original: '%s'",
							   theme->getName().toStdString().c_str(), filename.toStdString().c_str(),
							   existingTheme->getFilename().toStdString().c_str());
			continue;
		}

		if (theme->isValid())
			mThemes.emplace_back(std::move(theme));
	}
}

QColor ThemeManager::getLogColor(const nap::LogLevel& lvl) const
{
	if (mCurrentTheme == nullptr)
		return {};

	return mCurrentTheme->getLogColor(lvl);
}

