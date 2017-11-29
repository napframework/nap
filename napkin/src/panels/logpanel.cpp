#include "logpanel.h"

using namespace napkin;

LogModel::LogModel() : QStandardItemModel()
{
	// Register with nap::Logger, call the Qt signal in order to let the signal arrive on the Qt UI thread
	nap::Logger::instance().log.connect([&](nap::LogMessage msg) { napLogged(msg); });
	connect(this, &LogModel::napLogged, this, &LogModel::onLog);

	setHorizontalHeaderLabels({"Level", "Message"});
}

void LogModel::onLog(nap::LogMessage log)
{
	appendRow({new QStandardItem(QString::fromStdString(log.level().name())),
			   new QStandardItem(QString::fromStdString(log.text()))});

	// Keep maximum amount of rows
	while (rowCount() > maxRows)
		removeRow(0);
}

LogPanel::LogPanel() : QWidget()
{
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(&mTreeView);

	mTreeView.setModel(&mLogModel);
}
