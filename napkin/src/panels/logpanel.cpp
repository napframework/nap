#include <appcontext.h>
#include "logpanel.h"

using namespace napkin;

class LogTextItem : public QStandardItem
{
public:
	LogTextItem(const QString& text) : QStandardItem(text) {}

	void setLink(const QString& link)
	{
		mLink = link;
	}

	bool hasLink() const { return !mLink.isEmpty(); }

	const QString& link() { return mLink; }
private:
	QString mLink;
};

LogModel::LogModel() : QStandardItemModel()
{
	mColors[nap::Logger::fineLevel()] = "#333";
	mColors[nap::Logger::debugLevel()] = "#000";
	mColors[nap::Logger::infoLevel()] = "#080";
	mColors[nap::Logger::warnLevel()] = "#00F";
	mColors[nap::Logger::fatalLevel()] = "#F00";

	// Register with nap::Logger, call the Qt signal in order to let the signal arrive on the Qt UI thread
	nap::Logger::instance().log.connect(mLogHandler);
	connect(this, &LogModel::napLogged, this, &LogModel::onLog);

	setHorizontalHeaderLabels({"Level", "Message"});
}

void LogModel::onLog(nap::LogMessage log)
{
	auto levelname = QString::fromStdString(log.level().name());
	auto logtext = QString::fromStdString(log.text());

	QRegularExpression re("([a-z]+:(\\/\\/)[^\\s&^'&^\"]+)");
	auto color = QColor(mColors[log.level()]);
	auto levelitem = new QStandardItem(levelname);
	levelitem->setForeground(color);
	levelitem->setEditable(false);
	auto textitem = new LogTextItem(logtext);
	textitem->setToolTip(logtext);
	textitem->setForeground(color);
	textitem->setEditable(false);

	auto match = re.match(QString::fromStdString(log.text()));
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

LogPanel::LogPanel() : QWidget()
{
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(&mTreeView);

	mTreeView.setModel(&mLogModel);

	connect(&mTreeView.getTreeView(), &QTreeView::doubleClicked, this, &LogPanel::onDoubleClicked);
}

void LogPanel::onDoubleClicked(const QModelIndex& index)
{
	auto sourceindex = mTreeView.getFilterModel().mapToSource(index);
	auto textitem = dynamic_cast<LogTextItem*>(mTreeView.getModel()->itemFromIndex(sourceindex));
	if (textitem == nullptr)
		return;

	AppContext::get().handleURI(textitem->link());
}
