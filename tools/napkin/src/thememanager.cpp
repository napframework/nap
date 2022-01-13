/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "thememanager.h"
#include "appcontext.h"
#include "napkin-resources.h"
#include "napkinglobals.h"

#include <QDir>
#include <QFontDatabase>
#include <QSettings>
#include <QStyleFactory>
#include <QTextStream>

#include <nap/logger.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <utility/fileutils.h>

using namespace napkin;


Theme::Theme(const QString& filename) :
	mFilePath(filename)
{
	reload();
}


bool napkin::Theme::reload()
{
	return mValid = loadTheme();
}


bool Theme::isValid() const
{
	return mValid;
}


const QString& Theme::getStylesheetFilePath() const
{
	return mStylesheetFilePath;
}


bool Theme::loadTheme()
{
	rapidjson::Document doc;
	std::string data;
	nap::utility::ErrorState err;
	nap::utility::readFileToString(mFilePath.toStdString(), data, err);
	rapidjson::ParseResult ok = doc.Parse(data.c_str());
	if (!ok)
	{
		nap::Logger::error("JSON Parse error in %s: %s, offset: %d",
			mFilePath.toStdString().c_str(), 
			rapidjson::GetParseError_En(ok.Code()), ok.Offset());
		return false;
	}

	// Enure preset name element is present
	mName.clear();
	auto name_el = doc.FindMember("name");
	if (name_el == doc.MemberEnd())
	{
		nap::Logger::error("Missing 'name' element in: %s",
			mFilePath.toStdString().c_str());
		return false;
	}

	// Make sure name is not empty
	mName = QString::fromStdString(name_el->value.GetString());
	if (mName.isEmpty())
	{
		nap::Logger::error("Empty 'name' element in: %s",
			mFilePath.toStdString().c_str());
		return false;
	}

	// Get preset stylesheet
	mStylesheetFilePath.clear();
	auto style_el = doc.FindMember("stylesheet");
	if (style_el != doc.MemberEnd())
	{
		// Stylesheet is relative to style json file
		mStylesheetFilePath = QFileInfo(mFilePath).absolutePath() + "/" + style_el->value.GetString();
		if (!QFileInfo(mStylesheetFilePath).exists())
		{
			nap::Logger::warn("Missing 'stylesheet' %s, %s",
				style_el->value.GetString(), mFilePath.toStdString().c_str());
		}
	}

	// load log colors
	mLogColors.clear();
	auto itLogCols = doc.FindMember("logColors");
	if (itLogCols != doc.MemberEnd())
	{
		auto logColors = itLogCols->value.GetObject();
		for (const auto logLevel : nap::Logger::getLevels())
		{
			const auto& levelName = logLevel->name();
			auto log_color_it = logColors.FindMember(levelName.c_str());
			if (log_color_it != logColors.MemberEnd())
			{
				auto colname = log_color_it->value.GetString();
				QColor col(QString::fromStdString(colname));
				mLogColors.insert(logLevel->level(), col);
			}
			else
			{
				nap::Logger::warn("Missing log level: '%s' in '%s'",
					levelName.c_str(), mFilePath.toStdString().c_str());
			}
		}
	}
	else
	{
		nap::Logger::warn("Missing 'logColors' element in '%s'",
			mFilePath.toStdString().c_str());
	}

	// Load custom fonts
	mFonts.clear();
	auto itFonts = doc.FindMember("fonts");
	if (itFonts != doc.MemberEnd())
	{
		auto fonts = itFonts->value.GetObject();
		for (const auto& font : fonts)
		{
			auto key = QString::fromStdString(font.name.GetString());
			auto col(QString::fromStdString(font.value.GetString()));
			mFonts.insert(std::move(key), std::move(col));
		}
	}

	// load custom colors
	mColors.clear();
	auto itCols = doc.FindMember("colors");
	if (itCols != doc.MemberEnd())
	{
		auto colors = itCols->value.GetObject();
		for (const auto& color : colors)
		{
			auto key = QString::fromStdString(color.name.GetString());
			QColor col(QString::fromStdString(color.value.GetString()));
			mColors.insert(key, col);
		}
	}
	else
	{
		nap::Logger::warn("Missing 'colors' element in '%s'", mFilePath.toStdString().c_str());
	}
	return true;
}


QColor Theme::getLogColor(const nap::LogLevel& lvl) const
{
	if (mLogColors.contains(lvl.level()))
		return mLogColors[lvl.level()];

	std::cout << "warning: unable to find color for log level: " << lvl.name().c_str() << std::endl;
	return QColor(0,0,0);
}


QColor Theme::getColor(const QString& key) const
{
	if (mColors.contains(key))
		return mColors[key];

	nap::Logger::error("Color not found: %s", key.toStdString().c_str());
	return {};
}


const QMap<QString, QColor>& Theme::getColors() const
{
	return mColors;
}


const QMap<QString, QString>& Theme::getFonts() const
{
	return mFonts;
}


ThemeManager::ThemeManager()
{
	connect(&mFileWatcher, &QFileSystemWatcher::directoryChanged, this, &ThemeManager::onFileChanged);
	connect(&mFileWatcher, &QFileSystemWatcher::fileChanged, this, &ThemeManager::onFileChanged);
}


void ThemeManager::setTheme(Theme* theme)
{
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
	Theme* new_theme = findTheme(name);
	if (new_theme == nullptr)
	{
		nap::Logger::warn("Unable to find theme with name: %s", name.toStdString().c_str());
		return;
	}
	setTheme(new_theme);
}


const Theme* ThemeManager::findTheme(const QString& name) const
{
	for (const auto& theme : mThemes)
		if (theme->getName() == name)
			return theme.get();
	return nullptr;
}


Theme* ThemeManager::findTheme(const QString& name)
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
	{
		loadThemes();
	}
	return mThemes;
}


QString ThemeManager::getThemeDir()
{
	// TODO: This probably needs to be configurable
	return QString("%1/%2").arg(QCoreApplication::applicationDirPath(), theme::directory);
}


QString ThemeManager::getFontDir()
{
	return QString("%1/%2").arg(QCoreApplication::applicationDirPath(), font::directory);
}


void ThemeManager::applyTheme()
{
	auto app = AppContext::get().getQApplication();
	if (!mCurrentTheme || !QFileInfo::exists(mCurrentTheme->getStylesheetFilePath()))
	{
		app->setStyleSheet(nullptr);
		QApplication::setStyle(QStyleFactory::create("Fusion"));
		return;
	}

	auto stylesheetFile = mCurrentTheme->getStylesheetFilePath();
	QFile file(stylesheetFile);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		nap::Logger::warn("Could not load file: %s", stylesheetFile.toStdString().c_str());
		return;
	}

	QTextStream in(&file);
	QString styleSheet = in.readAll();
	file.close();

	// Replace colors from theme
	const auto& theme_colors = mCurrentTheme->getColors();
	for (auto it = theme_colors.begin(); it != theme_colors.end(); it++)
	{
		QString key = "@" + it.key();
		QString color = it.value().name();
		styleSheet.replace(key, color);
	}

	// Replace fonts from theme
	const auto& theme_fonts = mCurrentTheme->getFonts();
	for (auto it = theme_fonts.begin(); it != theme_fonts.end(); it++)
	{
		QString key = "@" + it.key();
		QString font_name = it.value();
		styleSheet.replace(key, font_name);
	}

	// Start watching for file changes (style and theme)
	mWatchedFilenames.clear();
	mWatchedFilenames << QFileInfo(mCurrentTheme->getFilePath()).absolutePath();
	mWatchedFilenames << stylesheetFile;
	mWatchedFilenames << mCurrentTheme->getFilePath();
	watchThemeFiles();

	QApplication::setStyle(QStyleFactory::create("Fusion"));
	app->setStyleSheet(styleSheet);
}


void ThemeManager::watchThemeFiles()
{
	for (const auto& filename : mWatchedFilenames)
	{
		std::string path = filename.toStdString();
		mFileWatcher.addPath(filename);
	}
}


void ThemeManager::onFileChanged(const QString& path)
{
	QString changed_file = QFileInfo(path).filePath();
	if (changed_file == getCurrentTheme()->getFilePath())
	{
		nap::Logger::info("Reloading theme: %s", path.toStdString().c_str());
		if (mCurrentTheme->reload())
		{
			applyTheme();
		}
		else
		{
			nap::Logger::error("Failed to load: %s", path.toStdString().c_str());
		}
		return;
	}

	if (changed_file == getCurrentTheme()->getStylesheetFilePath())
	{
		nap::Logger::info("Reloading style: %s", path.toStdString().c_str());
		applyTheme();
	}
}


void ThemeManager::loadFonts()
{
	QDir font_dir(getFontDir());
	QStringList fonts = font_dir.entryList(QStringList() << font::extension, QDir::Files);
	for (const auto& name : fonts)
	{
		QString font_file(font_dir.absolutePath() + "/" + name);
		if (QFontDatabase::addApplicationFont(font_file) < 0)
		{
			nap::Logger::warn("Failed to load font: '%s'", name.toStdString().c_str());
		}
	}
}


void ThemeManager::watchThemeDir()
{
	mFileWatcher.addPath(getThemeDir());
}


void ThemeManager::loadThemes()
{
	// Load fonts themes can reference
	loadFonts();

	// Load themes
	for (const auto& dir : QDir(getThemeDir()).entryInfoList(QDir::AllDirs))
	{
		auto filename = QString("%1/%2").arg(dir.absoluteFilePath(), theme::filename);
		if (!QFileInfo::exists(filename))
			continue;

		nap::Logger::fine("Loading theme: %s", filename.toStdString().c_str());
		auto theme = std::make_unique<Theme>(filename);

		// Check for duplicated
		const auto existingTheme = findTheme(theme->getName());
		if (existingTheme != nullptr)
		{
			nap::Logger::error("Duplicate theme name '%s' while loading '%s', original: '%s'",
							   theme->getName().toStdString().c_str(), filename.toStdString().c_str(),
							   existingTheme->getFilePath().toStdString().c_str());
			continue;
		}

		if (theme->isValid())
		{
			mThemes.emplace_back(std::move(theme));
		}
	}
}

QColor ThemeManager::getLogColor(const nap::LogLevel& lvl) const
{
	if (mCurrentTheme)
		return mCurrentTheme->getLogColor(lvl);
	return {};
}


QColor ThemeManager::getColor(const QString& key) const
{
	if (mCurrentTheme)
		return mCurrentTheme->getColor(key);
	return {};
}
