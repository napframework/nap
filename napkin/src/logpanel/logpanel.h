#pragma once

#include <QDockWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <nap/logger.h>

#include <ui_logpanel.h>
#include <src/appcontext.h>

 class LogLevelStyle
{
public:
	LogLevelStyle(const QString& iconName, const QColor& color) : mIconName(iconName), mColor(color.lighter(190)) {}

	const QIcon* icon() {
        if (!mIcon) {
            mIcon = AppContext::get().iconStore().get(mIconName);
        }
        return mIcon;
    }

    const QColor color() const { return mColor; }

private:
	const QString mIconName;
	const QIcon* mIcon = nullptr;
	QColor mColor;
};

class LogPanel : public QDockWidget
{
	Q_OBJECT
public:
	explicit LogPanel(QWidget* parent = 0);

private:
	Ui::LogPanelClass ui;
	QStandardItemModel* mLogModel;
	QStringList mHeaderLabels;

	nap::Slot<nap::LogMessage> onLogSlot = {this, &LogPanel::onMessageReceived};

	void onMessageReceived(nap::LogMessage msg);

	LogLevelStyle mLogStyleDebug = {"bug", Qt::green};
	LogLevelStyle mLogStyleInfo = {"information", Qt::blue};
	LogLevelStyle mLogStyleWarn = {"error", QColor(0xFF, 0x80, 0x00)};
	LogLevelStyle mLogStyleFatal = {"exclamation", Qt::red};
};
