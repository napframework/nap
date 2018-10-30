
#include <QApplication>
#include <QtWidgets/QPushButton>
#include <napqt/curveeditor/standardcurve.h>

#include "curveeditor/curveview.h"
#include "timeline/timelinepanel.h"

#include "basewindow.h"
#include "fileselector.h"
#include "errordialog.h"
#include "randomnames.h"
#include "filterpopup.h"
#include "autosettings.h"

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

		auto filterLayout = new QHBoxLayout();
		filterLayout->addWidget(new QLabel("Filter popup:"));
		filterLayout->addWidget(&mFilterResult, 1);
		mFilterResult.setPlaceholderText("Select text...");
		auto filterButton = new QPushButton("...", this);
		filterLayout->addWidget(filterButton);
		connect(filterButton, &QPushButton::clicked, this, &DemoPanel::onFilterPopup);
		mLayout.addLayout(filterLayout);

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
		namegen::NameGen gen;

		QStringList content;
		for (int i=0; i<100; i++)
			content << QString::fromStdString(gen.multiple(2, 5));

		auto result = FilterPopup::fromStringList(this, content);
		if (!result.isEmpty())
			mFilterResult.setText(result);
	}
	QVBoxLayout mLayout;
	QStringList mErrors;
	QTimer mErrorTimer;
	QLineEdit mFilterResult;
};

class MainWindow : public BaseWindow
{
public:
	MainWindow()
	{
		ErrorDialog::setDefaultParent(this);
		addDock("Timeline", &mTimelinePanel);
		addDock("Demo", &mDemoPanel);
		addDock("Curves", &mCurveView);
		addDock("Curves2", &mCurveView2);

		demoTimeline();
		demoCurves();
	}

	void demoCurves() {
		auto model = new StandardCurveModel(this);
//		{
//			auto curve = model->addCurve();
//			curve->setName("Fade In");
//			curve->addPoint(0.75, 0.00);
//			curve->addPoint(0.00, 0.25);
//			curve->addPoint(1.00, 0.50);
//			curve->addPoint(0.50, 0.75);
//			curve->addPoint(0.25, 1.00);
//		}
		mCurveView.setModel(model);
		mCurveView2.setModel(model);

		{
			auto curve = model->addCurve();
			curve->setName("Fade Out and In");
			curve->addPoint(0.0, 1.0);
			curve->addPoint(0.5, 0.0);
			curve->addPoint(1.0, 1.0);
			
		}

	}

	void demoTimeline() {
		namegen::NameGen gen;

		auto timeline = new Timeline(this);
		timeline->setMinEventLength(1.0/timeline->framerate());

		{
			auto track = timeline->addTrack("Event Track One");
			track->addEvent("FirstEvent", 0, 100);
			track->addEvent("Event Zwei", 100, 150);
			track->addEvent("Derde Event", 160, 250);
		}
		{
			auto track = timeline->addTrack("Tick Track");
			track->addTick(0);
			track->addTick(100);
			track->addTick(150);
			track->addTick(200);
		}
		{
			auto track = timeline->addTrack("Second Event Track");
			track->setHeight(20);
			auto child1 = track->addTrack("A Child Track");
			child1->addEvent("Pookie", 50, 90);
			child1->addEvent("Wookie", 100, 140);
			child1->addEvent("Dookie", 150, 190);
			auto child2 = track->addTrack("Another Child Track");
			child2->addEvent("Rob", 20, 65);
			child2->addEvent("Knob", 70, 115);
			child2->addEvent("Bob", 120, 135);
		}
		{
			auto track = timeline->addTrack("Track number Three");
			track->addEvent("0", 0, 10);
			track->addEvent("10", 10, 20);
			track->addEvent("20", 20, 30);
			track->addEvent("30", 30, 40);
			track->addEvent("40", 40, 50);
			track->addEvent("50", 50, 60);
			track->addEvent("60", 60, 70);
			track->addEvent("70", 70, 80);
			track->addEvent("80", 80, 90);
			track->addEvent("90", 90, 100);
		}
		{
			auto extraTrack = timeline->addTrack("Extra Tracks");
			extraTrack->setHeight(20);
			for (int i=0; i<10; i++) {
				int offset = 30 * i;
				auto track = extraTrack->addTrack(QString("Extra Track Whee %1").arg(i));
				track->addEvent(QString("Empty Event %1").arg(i), offset, 300 + offset);
			}
		}

//	int trackCount = 30;
//	int eventCount = 10;
//
//	auto framestep = 1.0 / timeline->framerate();
//
//	for (int i = 0; i < trackCount; i++)
//	{
//		auto trackname = QString::fromStdString(gen.multiple());
//		auto track = timeline->addTrack(trackname);
//
//		qreal t = 0;
//		for (int e = 0; e < eventCount; e++)
//		{
//			t += namegen::randint(0, 40) * framestep;
//			qreal len = namegen::randint(20, 300) * framestep;
//			auto eventname = QString::fromStdString(gen.multiple());
//			track->addEvent(eventname, t, t + len);
//			t += len;
//		}
//	}

		mTimelinePanel.setTimeline(timeline);
	}

private:
	TimelinePanel mTimelinePanel;
	Timeline mTimeline;
	CurveView mCurveView;
	CurveView mCurveView2;
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