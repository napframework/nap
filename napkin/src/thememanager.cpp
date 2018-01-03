#include "thememanager.h"
#include "appcontext.h"
#include "napkinglobals.h"
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <nap/logger.h>

using namespace napkin;

ThemeManager::ThemeManager() : mCurrentTheme(TXT_DEFAULT_THEME)
{
}


void ThemeManager::setTheme(const QString& themeName)
{
	if (themeName.isEmpty())
	{
		AppContext::get().getQApplication()->setStyleSheet(nullptr);
		mCurrentTheme = napkin::TXT_DEFAULT_THEME;
	}
	else
	{
		auto themeFile = QString("%1/%2.qss").arg(getThemeDir(), themeName);
		QFile f(themeFile);
		if (!f.open(QFile::ReadOnly | QFile::Text))
		{
			nap::Logger::warn("Could not load file: %s", themeFile.toStdString().c_str());
			return;
		}
		QTextStream in(&f);
		auto styleSheet = in.readAll();
		f.close();

		AppContext::get().getQApplication()->setStyleSheet(styleSheet);
		mCurrentTheme = themeName;
	}
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
		if (filename.suffix() == "qss")
			names << filename.baseName();
	}
	return names;
}

QString ThemeManager::getThemeDir()
{
	// TODO: This probably needs to be configurable
	return QString("%1/resources/themes").arg(QCoreApplication::applicationDirPath());
}
