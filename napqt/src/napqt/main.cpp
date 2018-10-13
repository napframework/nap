
#include <QApplication>
#include <QtWidgets/QPushButton>
#include "timeline/timelinepanel.h"
#include "basewindow.h"
#include "fileselector.h"
#include "errordialog.h"

using namespace napqt;

class DemoPanel : public QWidget
{
public:
	DemoPanel(QWidget* parent = nullptr) : QWidget(parent)
	{
		setLayout(&mLayout);

		auto errorButton = new QPushButton("Single Error", this);
		mLayout.addWidget(errorButton);
		connect(errorButton, &QPushButton::clicked, this, &DemoPanel::onSingleError);

		auto errorButton2 = new QPushButton("Multiple Errors", this);
		mLayout.addWidget(errorButton2);
		connect(errorButton2, &QPushButton::clicked, this, &DemoPanel::onMultipleErrors);

		auto filterButton = new QPushButton("Filter Popup", this);
		mLayout.addWidget(filterButton);
		connect(filterButton, &QPushButton::clicked, this, &DemoPanel::onFilterPopup);

		auto fileSelector = new FileSelector(this);
		mLayout.addWidget(fileSelector);
		mLayout.addStretch(1);
	}

private:
	void onSingleError() {
		ErrorDialog::showMessage("Annoying Error Message,\ndelivered especially for you!");
	}

	void onMultipleErrors() {
		int errCount = 10;
		int interval = 100;
		mErrors.clear();
		mErrors << QString("Showing %1 errors with an interval of %2 ms.").arg(QString::number(errCount), QString::number(interval));
		for (int i=0; i<errCount; i++)
			mErrors << QString("Error message number %1").arg(i);

		mErrorTimer.setInterval(interval);
		connect(&mErrorTimer, &QTimer::timeout, [this, &errCount]() {
			if (mErrors.isEmpty())
			{
				mErrorTimer.stop();
				return;
			}
			QString msg = mErrors.front();
			mErrors.pop_front();
			ErrorDialog::showMessage(msg);
		});
		mErrorTimer.start(interval);
	}

	void onFilterPopup() {

	}
	QVBoxLayout mLayout;
	QStringList mErrors;
	QTimer mErrorTimer;
};

class MainWindow : public BaseWindow
{
public:
	MainWindow()
	{
		ErrorDialog::setDefaultParent(this);
		addDock("Timeline", &mTimelinePanel);
		addDock("Demo", &mDemoPanel);
	}

private:
	TimelinePanel mTimelinePanel;
	Timeline mTimeline;
	DemoPanel mDemoPanel;
};


int main(int argc, char* argv[])
{
	QApplication::setApplicationName("baseoneclib_test");
	QApplication::setOrganizationName("CoreSmith");
	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	app.exec();
}