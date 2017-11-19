#pragma once

#include <QObject>

class ThemeManager : public QObject {
    Q_OBJECT
public:
    ThemeManager();

    QStringList availableThemes();
    void setTheme(const QString& themeName);
    const QString& currentTheme() const;
    QString themeDir();

Q_SIGNALS:
    void themeChanged(const QString& theme);

private:
    QString mCurrentTheme;

};