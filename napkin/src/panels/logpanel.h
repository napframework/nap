#pragma once

#include <QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTreeView>
#include <QtGui/QStandardItemModel>
#include <nap/logger.h>
#include <generic/filtertreeview.h>


/**
 * Captures log messages and provides the LogPanel with the currently cached log messages.
 */
class LogModel : public QStandardItemModel {
Q_OBJECT
public:
    LogModel() : QStandardItemModel()
    {
        // Register with nap::Logger, call the Qt signal in order to let the signal arrive on the Qt UI thread
        nap::Logger::instance().log.connect([&](nap::LogMessage msg) { napLogged(msg); });
        connect(this, &LogModel::napLogged, this, &LogModel::onLog);

        setHorizontalHeaderLabels({"Level", "Message"});
    }

Q_SIGNALS:
    void napLogged(nap::LogMessage msg);

private:
    void onLog(nap::LogMessage log)
    {
        appendRow({new QStandardItem(QString::fromStdString(log.level().name())),
                   new QStandardItem(QString::fromStdString(log.text()))});

        // Keep maximum amount of rows
        while (rowCount() > maxRows)
            removeRow(0);
    }

    int maxRows = 1000;
};


class LogPanel : public QWidget {
public:
    explicit LogPanel() : QWidget()
    {
        setLayout(new QVBoxLayout());
        layout()->setContentsMargins(0,0,0,0);
        layout()->addWidget(&mTreeView);

        mTreeView.setModel(&mLogModel);
    }

private:
    FilterTreeView mTreeView;
    LogModel mLogModel;
};