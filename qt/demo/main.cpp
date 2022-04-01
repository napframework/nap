/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <iostream>

#include <napqt/curveeditor/standardcurve.h>
#include <napqt/curveeditor/curvewidget.h>
#include <napqt/curveeditor/curveview.h>
#include <napqt/basewindow.h>
#include <napqt/fileselector.h>
#include <napqt/errordialog.h>
#include <napqt/randomnames.h>
#include <napqt/filterpopup.h>
#include <napqt/autosettings.h>
#include <napqt/colorpicker.h>

using namespace nap::qt;

/**
 * Simple demo panel including features of napqt
 */
class DemoPanel : public QWidget
{
public:
	explicit DemoPanel(QWidget* parent = nullptr) : QWidget(parent)
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

		auto result = FilterPopup::show(this, content);
		if (!result.isEmpty())
			mFilterResult.setText(result);
	}

	QVBoxLayout mLayout;
	QStringList mErrors;
	QTimer mErrorTimer;
	QLineEdit mFilterResult;
};

/**
 * Basic window containing panels that feature some napqt components
 */
class MainWindow : public BaseWindow
{
public:
	MainWindow()
	{
		ErrorDialog::setDefaultParent(this);
		addDock("Demo", &mDemoPanel);
		addDock("CurvesPanel", &mCurvePanel);
		addDock("CurveView", &mCurveView);
		addDock("ColorPicker", &mColorPicker);

		createExampleCurves();
	}

	void createExampleCurves() {
		// create a curve and add some points
		auto model = new StandardCurveModel(this);
		{
			auto curve = model->addCurve();
			curve->setName("Fade In");
			curve->addPoint(0.75, 0.00);
			curve->addPoint(0.00, 0.25);
			curve->addPoint(1.00, 0.50);
			curve->addPoint(0.50, 0.75);
			curve->addPoint(0.25, 1.00);
		}

		mCurveView.setModel(model);
		mCurvePanel.setModel(model);

		// add more points after setting the model to make sure it works
		{
			auto curve = model->addCurve();
			curve->setName("Fade Out and In");
			curve->addPoint(0.0, 1.0);
			curve->addPoint(0.5, 0.0);
			curve->addPoint(1.0, 1.0);
		}
	}

private:
	CurveEditor mCurveView;
	CurveWidget mCurvePanel;
	ColorPicker mColorPicker;
	DemoPanel mDemoPanel;
};




int main(int argc, char* argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication::setApplicationName("napqt");
	QApplication::setOrganizationName("nap-labs");
	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	QApplication::exec();
}
