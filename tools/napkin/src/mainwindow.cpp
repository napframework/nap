#include "mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <fcurve.h>
#include <utility/fileutils.h>

using namespace napkin;

void MainWindow::bindSignals()
{
	connect(&AppContext::get(), &AppContext::documentOpened, this, &MainWindow::onDocumentOpened);
	connect(&AppContext::get(), &AppContext::documentChanged, this, &MainWindow::onDocumentChanged);
	connect(&mResourcePanel, &ResourcePanel::selectionChanged, this, &MainWindow::onResourceSelectionChanged);
	connect(&AppContext::get(), &AppContext::selectionChanged, &mResourcePanel, &ResourcePanel::selectObjects);
	connect(&AppContext::get(), &AppContext::logMessage, this, &MainWindow::onLog);
}


void MainWindow::unbindSignals()
{
	disconnect(&AppContext::get(), &AppContext::documentOpened, this, &MainWindow::onDocumentOpened);
	disconnect(&AppContext::get(), &AppContext::documentChanged, this, &MainWindow::onDocumentChanged);
	disconnect(&mResourcePanel, &ResourcePanel::selectionChanged, this, &MainWindow::onResourceSelectionChanged);
	disconnect(&AppContext::get(), &AppContext::selectionChanged, &mResourcePanel, &ResourcePanel::selectObjects);
	disconnect(&AppContext::get(), &AppContext::logMessage, this, &MainWindow::onLog);
}


void MainWindow::showEvent(QShowEvent* event)
{
	BaseWindow::showEvent(event);

	if (mFirstShowEvent)
	{
		QSettings settings;
		nap::Logger::debug("Using settings file: %s", settings.fileName().toStdString().c_str());
		AppContext::get().restoreUI();
		mFirstShowEvent = false;
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (AppContext::get().getDocument()->isDirty())
	{
		auto result = QMessageBox::question(this, "Save before exit",
											"The current document has unsaved changes.\n"
													"Save the changes before exit?",
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			SaveFileAction action;
			action.trigger();
		}
		else if (result == QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}
	unbindSignals();
	BaseWindow::closeEvent(event);
}

void MainWindow::addDocks()
{
//	addDock("Available Types", &mHierarchyPanel);
//	addDock("History", &mHistoryPanel);

	addDock("Resources", &mResourcePanel);
	addDock("Inspector", &mInspectorPanel);
	addDock("Log", &mLogPanel);
	addDock("AppRunner", &mAppRunnerPanel);
    addDock("Scene", &mScenePanel);
	addDock("Curve", &mCurvePanel);
}


void MainWindow::addMenu()
{
	auto filemenu = new QMenu("File", menuBar());
	{
		auto newFileAction = new NewFileAction();
		addAction(newFileAction);
		filemenu->addAction(newFileAction);

		auto openFileAction = new OpenFileAction();
		addAction(openFileAction);
		filemenu->addAction(openFileAction);

		auto saveFileAction = new SaveFileAction();
		addAction(saveFileAction);
		filemenu->addAction(saveFileAction);

		auto saveFileAsAction = new SaveFileAsAction();
		addAction(saveFileAction);
		filemenu->addAction(saveFileAsAction);
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), filemenu);

	auto optionsMenu = new QMenu("Options", menuBar());
	{
		optionsMenu->addMenu(&mThemeMenu);
		optionsMenu->addAction("Save settings as...", [this]() {
			auto filename = QFileDialog::getSaveFileName(this, "Save Settings", QString(), "Settings file (*.ini)");
			if (filename.isEmpty())
				return;
			QSettings s(filename, QSettings::IniFormat);

		});
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), optionsMenu);
}


void MainWindow::onDocumentChanged()
{
	updateWindowTitle();
}


void MainWindow::updateWindowTitle()
{
	QString filename = AppContext::get().getDocument()->getCurrentFilename();
	if (filename.isEmpty()) {
		filename = napkin::TXT_UNTITLED_DOCUMENT;
	} else {
		filename = QFileInfo(filename).fileName();
	}

	QString changed = AppContext::get().getDocument()->isDirty() ? "*" : "";

	setWindowTitle(QString("%1%2 - %3").arg(filename, changed, QApplication::applicationName()));
}

MainWindow::MainWindow() : BaseWindow(), mErrorDialog(this)
{
	setStatusBar(&mStatusBar);

	addDocks();
	addMenu();
	bindSignals();
}

MainWindow::~MainWindow()
{
}

void MainWindow::onResourceSelectionChanged(QList<nap::rtti::Object*> objects)
{
	mInspectorPanel.setObject(objects.isEmpty() ? nullptr : objects.first());

	mCurvePanel.editCurve(nullptr);

	if (!objects.isEmpty()) {
		auto ob = dynamic_cast<nap::math::FloatFCurve*>(objects.at(0));
		if (ob) {
			mCurvePanel.editCurve(ob);
		}
	}

}

void MainWindow::onDocumentOpened(const QString filename)
{
	onDocumentChanged();
}

void MainWindow::onLog(nap::LogMessage msg)
{
	statusBar()->showMessage(QString::fromStdString(msg.text()));

	if (msg.level().level() >= nap::Logger::fatalLevel().level())
		showError(msg);
}

void MainWindow::showError(nap::LogMessage msg)
{
	mErrorDialog.addMessage(QString::fromStdString(msg.text()));
	mErrorDialog.show();
}

