/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "logpanel.h"

#include <napqt/autosettings.h>
#include <QPainter>
#include <QScrollBar>
#include <QSettings>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QtWidgets/QStylePainter>
#include <appcontext.h>

using namespace napkin;

const nap::LogMessage& LogEntryItem::getMessage() const
{
	return mMessage;
}


LogTextItem::LogTextItem(const nap::LogMessage& msg) : LogEntryItem(msg)
{
	setText(QString::fromStdString(msg.text()));
}


LevelItem::LevelItem(const nap::LogMessage& msg) : LogEntryItem(msg)
{
	setText(QString::fromStdString(msg.level().name()));
}


LogModel::LogModel() : QStandardItemModel()
{
	// Register with nap::Logger, call the Qt signal in order to let the signal arrive on the Qt UI thread
	connect(&AppContext::get(), &AppContext::logMessage, this, &LogModel::onLog);
	setHorizontalHeaderLabels({"Level", "Message"});
}


void LogModel::onLog(nap::LogMessage msg)
{
	auto levelname = QString::fromStdString(msg.level().name());
	auto logtext = QString::fromStdString(msg.text());


	QRegularExpression re("([a-z]+:(\\/\\/)[^\\s&^'&^\"]+)");
	auto levelitem = new LevelItem(msg);
	//	levelitem->setForeground(col);
	levelitem->setEditable(false);
	auto textitem = new LogTextItem(msg);
	//	textitem->setForeground(col);
	textitem->setToolTip(logtext);
	textitem->setEditable(false);

	auto match = re.match(QString::fromStdString(msg.text()));
	if (match.hasMatch())
	{
		textitem->setLink(match.captured(1));
		auto font = textitem->font();
		font.setUnderline(true);
		textitem->setFont(font);
	}

	appendRow({levelitem, textitem});

	// Keep maximum amount of rows
	while (rowCount() > mMaxRows)
		removeRow(0);
}


QVariant LogModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::ForegroundRole)
	{
		auto item = qitem_cast<LogEntryItem*>(itemFromIndex(index));
		assert(item != nullptr);
		auto col = AppContext::get().getThemeManager().getLogColor(item->getMessage().level());
		return QVariant(QColor(col));
	}
	return QStandardItemModel::data(index, role);
}


LogPanel::LogPanel() : QWidget()
{
	nap::qt::AutoSettings::get().registerStorer(std::make_unique<LogPanelWidgetStorer>());

	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);

	mLayout.addWidget(&mTreeView);

	QWidget& cornerWidget = mTreeView.getCornerWidget();
	mCornerLayout.setContentsMargins(0,0,0,0);
	mCornerLayout.setSpacing(0);
	cornerWidget.setLayout(&mCornerLayout);
	mCornerLayout.addWidget(&mFilterCombo);

	// Let the treeview filter log messages (Look at these beautiful placeholders)
	mTreeView.getProxyModel().addExtraFilter(std::bind(&LogPanel::levelFilter, this,
														std::placeholders::_1,
														std::placeholders::_2,
														std::placeholders::_3));
	populateFilterCombo();

	mTreeView.setModel(&mLogModel);


	connect(&mFilterCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &LogPanel::onLevelChanged);
	connect(&mTreeView.getTreeView(), &QTreeView::doubleClicked, this, &LogPanel::onDoubleClicked);
	connect(mTreeView.getModel(), &QAbstractItemModel::rowsInserted, this, &LogPanel::onRowInserted);
	connect(mTreeView.getModel(), &QAbstractItemModel::rowsAboutToBeInserted, this, &LogPanel::onRowsAboutToBeInserted);
}

void LogPanel::populateFilterCombo()
{
	for (const auto level : nap::Logger::getLevels())
		mFilterCombo.addItem(QString::fromStdString(level->name()), level->level());
}


void LogPanel::onLevelChanged(int index)
{
	mTreeView.getProxyModel().refreshFilter();
}


void LogPanel::onDoubleClicked(const QModelIndex& index)
{
	auto sourceindex = mTreeView.getProxyModel().mapToSource(index);
	auto textitem = qitem_cast<LogTextItem*>(mTreeView.getModel()->itemFromIndex(sourceindex));
	if (textitem == nullptr)
		return;

	AppContext::get().handleURI(textitem->link());
}


void LogPanel::onRowsAboutToBeInserted(const QModelIndex& parent, int first, int last)
{
	auto scrollBar = mTreeView.getTreeView().verticalScrollBar();
	wasMaxScroll = scrollBar->value() == scrollBar->maximum();
}


void LogPanel::onRowInserted(const QModelIndex &parent, int first, int last)
{
	auto scrollBar = mTreeView.getTreeView().verticalScrollBar();
	if (wasMaxScroll)
	{
		QTimer::singleShot(0, [scrollBar]()
		{
			scrollBar->setValue(scrollBar->maximum());
		});
	}
}


void LogPanel::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}


void LogPanel::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
}


bool LogPanel::levelFilter(const nap::qt::LeafFilterProxyModel& model, int sourceRow,
						   const QModelIndex& sourceParent)
{
	auto index = mLogModel.index(sourceRow, 0, sourceParent);
	auto levelItem = qitem_cast<LevelItem*>(mLogModel.itemFromIndex(index));
	assert(levelItem != nullptr);

	const auto& itemLevel = levelItem->level();
	return levelItem->level() >= getCurrentLevel();
}


const nap::LogLevel& LogPanel::getCurrentLevel() const
{
	const auto& levels = nap::Logger::getLevels();
	int idx = mFilterCombo.currentIndex();
	const auto level = levels[idx];
	assert(level);
	return *level;
}


void LogPanel::setCurrentLevel(const nap::LogLevel& level)
{
	int idx = getLevelIndex(level);
	assert(idx > 0);
	mFilterCombo.setCurrentIndex(idx);
}


int LogPanel::getLevelIndex(const nap::LogLevel& level) const
{
	const auto& levels = nap::Logger::getLevels();
	return static_cast<int>(std::find(levels.begin(), levels.end(), &level) - levels.begin());
}


void LogPanelWidgetStorer::store(const LogPanel& widget, const QString& key, QSettings& s) const
{
	s.setValue(key + "_LOGLEVEL", widget.getLevelIndex(widget.getCurrentLevel()));
}
void LogPanelWidgetStorer::restore(LogPanel& widget, const QString& key, const QSettings& s) const
{
	const auto levels = nap::Logger::getLevels();
	const auto& defaultLevel = nap::Logger::infoLevel();
	int defaultIndex = widget.getLevelIndex(defaultLevel);
	int index = s.value(key + "_LOGLEVEL", defaultIndex).toInt();

	if (index < 0 || index >= levels.size())
	{
		nap::Logger::warn("Wrong log level in settings (%s), using default", std::to_string(index).c_str());
		index = defaultIndex;
	}
	widget.setCurrentLevel(*levels[index]);
}
