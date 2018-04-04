#include "mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>

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
	QSettings defaultSettings(DEFAULT_SETTINGS_FILE, QSettings::IniFormat);
	if (!QFileInfo::exists(DEFAULT_SETTINGS_FILE))
		nap::Logger::warn("Settings file not found: %1", DEFAULT_SETTINGS_FILE.toStdString().c_str());

	restoreSettings(defaultSettings);

	BaseWindow::showEvent(event);
	AppContext::get().restoreUI();
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
			saveSettings(s);
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
	mErrorDialog.addMessage(msg);
	mErrorDialog.show();
}

