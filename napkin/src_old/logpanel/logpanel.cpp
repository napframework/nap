#include "logpanel.h"

#include "../appcontext.h"


LogPanel::LogPanel(QWidget *parent) :
	QDockWidget(parent)
{
	ui.setupUi(this);

	mLogModel = new QStandardItemModel(this);
	ui.treeView->setModel(mLogModel);

	mHeaderLabels.append("Level");
	mHeaderLabels.append("LogMessage");
	mLogModel->setHorizontalHeaderLabels(mHeaderLabels);
    nap::Logger::instance().log.connect(onLogSlot);
    nap::Logger::debug("LogPanel connected");
}

void LogPanel::onMessageReceived(nap::LogMessage msg)
{
    static QMap<nap::LogLevel, LogLevelStyle*> mStyleMap;

    auto levelItem = new QStandardItem(QString::fromStdString(msg.level().name()));
    auto icons = AppContext::get().iconStore();

    LogLevelStyle* style = nullptr;
    if (msg.level() == nap::Logger::debugLevel()) {
        style = &mLogStyleDebug;
    } else if (msg.level() == nap::Logger::infoLevel()) {
        style = &mLogStyleInfo;
    } else if (msg.level() == nap::Logger::warnLevel()) {
        style = &mLogStyleWarn;
    } else if (msg.level() == nap::Logger::fatalLevel()) {
        style = &mLogStyleFatal;
    } else {
        assert(false); // unsupported log level
    }

    levelItem->setBackground(style->color());
    levelItem->setIcon(*style->icon());

    QString message = QString::fromStdString(msg.text());
    auto messageItem = new QStandardItem(message);
    messageItem->setToolTip(message);
    messageItem->setBackground(levelItem->background());

    QList<QStandardItem*> items;
	items << levelItem;
	items << messageItem;
	mLogModel->appendRow(items);
	ui.treeView->scrollTo(mLogModel->index(mLogModel->rowCount() - 1, 0));
}

 
