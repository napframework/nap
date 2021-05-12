/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <fcurve.h>
#include <QtDebug>
#include <utility/fileutils.h>

using namespace napkin;

void MainWindow::bindSignals()
{
	auto ctx = &getContext();
	connect(ctx, &AppContext::documentOpened, this, &MainWindow::onDocumentOpened);
	connect(ctx, &AppContext::documentChanged, this, &MainWindow::onDocumentChanged);
	connect(&mResourcePanel, &ResourcePanel::selectionChanged, this, &MainWindow::onResourceSelectionChanged);
	connect(&mScenePanel, &ScenePanel::selectionChanged, this, &MainWindow::onSceneSelectionChanged);
	connect(&mServiceConfigPanel, &ServiceConfigPanel::selectionChanged, this, &MainWindow::onServiceConfigChanged);
	connect(&mInstPropPanel, &InstancePropPanel::selectComponentRequested, this, &MainWindow::onSceneComponentSelectionRequested);
	connect(ctx, &AppContext::selectionChanged, &mResourcePanel, &ResourcePanel::selectObjects);
	connect(ctx, &AppContext::logMessage, this, &MainWindow::onLog);
	connect(ctx, &AppContext::blockingProgressChanged, this, &MainWindow::onBlockingProgress);
	connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, &MainWindow::onDocked);
}


void MainWindow::unbindSignals()
{
	auto ctx = &getContext();
	disconnect(ctx, &AppContext::documentOpened, this, &MainWindow::onDocumentOpened);
	disconnect(ctx, &AppContext::documentChanged, this, &MainWindow::onDocumentChanged);
	disconnect(&mResourcePanel, &ResourcePanel::selectionChanged, this, &MainWindow::onResourceSelectionChanged);
	disconnect(&mScenePanel, &ScenePanel::selectionChanged, this, &MainWindow::onSceneSelectionChanged);
	disconnect(&mServiceConfigPanel, &ServiceConfigPanel::selectionChanged, this, &MainWindow::onServiceConfigChanged);
	disconnect(&mInstPropPanel, &InstancePropPanel::selectComponentRequested, this, &MainWindow::onSceneComponentSelectionRequested);
	disconnect(ctx, &AppContext::selectionChanged, &mResourcePanel, &ResourcePanel::selectObjects);
	disconnect(ctx, &AppContext::logMessage, this, &MainWindow::onLog);
	disconnect(ctx, &AppContext::blockingProgressChanged, this, &MainWindow::onBlockingProgress);
}


void MainWindow::showEvent(QShowEvent* event)
{
	BaseWindow::showEvent(event);

	if (mFirstShowEvent)
	{
		QSettings settings;
		nap::Logger::debug("Using settings file: %s", settings.fileName().toStdString().c_str());
		getContext().restoreUI();
		rebuildRecentMenu();
		mFirstShowEvent = false;
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!confirmSaveCurrentFile())
	{
		event->ignore();
		return;
	}

	unbindSignals();
	BaseWindow::closeEvent(event);
}

void MainWindow::addDocks()
{
//	addDock("History", &mHistoryPanel);
//	addDock("Path Browser", &mPathBrowser);
	addDock("Resources", &mResourcePanel);
	addDock("Inspector", &mInspectorPanel);
	addDock("Log", &mLogPanel);
	addDock("AppRunner", &mAppRunnerPanel);
    addDock("Scene", &mScenePanel);
	addDock("Curve", &mCurvePanel);
	addDock("Modules", &mModulePanel);
	addDock("Instance Properties", &mInstPropPanel);
	addDock("Services", &mServiceConfigPanel);
}


void MainWindow::addMenu()
{
	// Project
	auto projectmenu = new QMenu("Project", menuBar());
	{
		auto openFileAction = new OpenProjectAction();
		addAction(openFileAction);
		projectmenu->addAction(openFileAction);
		mRecentProjectsMenu = projectmenu->addMenu("Recent Projects");

		// Services sub menuy
		auto newServiceConfigAction = new NewServiceConfigAction();
		auto openServiceConfigAction = new OpenServiceConfigAction();
		auto saveServiceConfigACtion = new SaveServiceConfigAction();
		auto saveServiceConfigurationAs = new SaveServiceConfigurationAs();
		auto setDefaultServiceConfig = new SetAsDefaultServiceConfigAction();
		auto service_menu = new QMenu("Configuration", projectmenu);
		{
			service_menu->addAction(newServiceConfigAction);
			service_menu->addAction(openServiceConfigAction);
			service_menu->addAction(saveServiceConfigACtion);
			service_menu->addAction(saveServiceConfigurationAs);
			service_menu->addAction(setDefaultServiceConfig);
		}
		projectmenu->addMenu(service_menu);
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), projectmenu);

	// File (Data)
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

		auto reloadFileAction = new ReloadFileAction();
		addAction(reloadFileAction);
		filemenu->addAction(reloadFileAction);

		auto updateDefaultAction = new UpdateDefaultFileAction();
		addAction(updateDefaultAction);
		filemenu->addAction(updateDefaultAction);
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), filemenu);


	// General Options
	auto optionsMenu = new QMenu("Options", menuBar());
	{
		optionsMenu->addMenu(&mThemeMenu);
		optionsMenu->addAction("Save settings as...", [this]() {
			auto filename = QFileDialog::getSaveFileName(this, "Save Settings", QString(), "Settings file (*.ini)");
			if (filename.isEmpty())
				return;
			if (!QFile::copy(QSettings().fileName(), filename))
				nap::Logger::error("Failed to save settings to file: %s", filename.toStdString().c_str());
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
	// If there is not project information, set to default
	const nap::ProjectInfo* project_info = getContext().getProjectInfo();
	if (project_info == nullptr)
	{
		setWindowTitle(QApplication::applicationName());
		return;
	}

	// Otherwise display current project & file
	QFileInfo fi(getContext().getDocument()->getCurrentFilename());
	QString changed = getContext().getDocument()->isDirty() ? "*" : "";
	setWindowTitle(QString("%1%2 %3 | %4 - %5").arg(QString::fromStdString(project_info->mTitle),
												changed, QString::fromStdString(project_info->mVersion),
												fi.exists() ? fi.fileName() : "No File",
												QApplication::applicationName()));
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

void MainWindow::onResourceSelectionChanged(QList<PropertyPath> paths)
{
	auto sceneTreeSelection = mScenePanel.treeView().getTreeView().selectionModel();
	sceneTreeSelection->blockSignals(true);
	sceneTreeSelection->clearSelection();
	sceneTreeSelection->blockSignals(false);
	if (!paths.isEmpty())
	{
		auto path = paths.first();
		// Don't edit scenes
		if (!path.getType().is_derived_from<nap::Scene>())
			mInspectorPanel.setPath(paths.first());
	}

	// Edit curve?
	mCurvePanel.editCurve(nullptr);
	if (!paths.isEmpty())
	{
		auto ob = dynamic_cast<nap::math::FloatFCurve*>(paths.first().getObject());
		if (ob)
			mCurvePanel.editCurve(ob);
	}
}

void MainWindow::onSceneSelectionChanged(QList<PropertyPath> paths)
{
	auto resTreeSelection = mResourcePanel.treeView().getTreeView().selectionModel();
	resTreeSelection->blockSignals(true);
	resTreeSelection->clearSelection();
	resTreeSelection->blockSignals(false);
	if (!paths.isEmpty())
	{
		auto path = paths.first();

		// Don't edit scenes
		if (!path.getType().is_derived_from<nap::Scene>())
			mInspectorPanel.setPath(paths.first());
	}
}

void MainWindow::onSceneComponentSelectionRequested(nap::RootEntity* rootEntity, const QString& path)
{
	mScenePanel.select(rootEntity, path);
}

void MainWindow::onDocumentOpened(const QString filename)
{
	onDocumentChanged();
	rebuildRecentMenu();
}

void MainWindow::onLog(nap::LogMessage msg)
{
	statusBar()->showMessage(QString::fromStdString(msg.text()));

	if (msg.level().level() >= nap::Logger::fatalLevel().level())
		showError(msg);
}

void MainWindow::onBlockingProgress(float fraction, const QString& message)
{
	const int scale = 100;
	if (fraction >= 1)
	{
		// Done
		if (mProgressDialog)
			mProgressDialog.reset();
	}
	else
	{
		// In progress
		if (!mProgressDialog)
		{
			mProgressDialog = std::make_unique<QProgressDialog>(message, "Cancel", 0, 0, this);
            mProgressDialog->setWindowTitle("Working...");
			mProgressDialog->setCancelButton(nullptr);
		}

		if (fraction > 0)
		{
			mProgressDialog->setRange(0, scale);
			mProgressDialog->setValue((int) fraction * scale);
		}
		else
		{
			mProgressDialog->setRange(0, 0);
			mProgressDialog->setValue(0);
		}
		mProgressDialog->show();
	}
	QApplication::processEvents();
}

void MainWindow::showError(nap::LogMessage msg)
{
	mErrorDialog.addMessage(QString::fromStdString(msg.text()));
	mErrorDialog.show();
}

bool MainWindow::confirmSaveCurrentFile()
{
	if (!getContext().hasDocument())
		return true;

	if (!getContext().getDocument()->isDirty())
		return true;

	auto result = QMessageBox::question(this, "Confirm save unsaved change",
									"The current document has unsaved changes.\n"
									"Save the changes before exit?",
									QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (result == QMessageBox::Yes)
	{
		SaveFileAction action;
		action.trigger();
		return true;
	}
	return result == QMessageBox::No;
}

void MainWindow::rebuildRecentMenu()
{
	mRecentProjectsMenu->clear();

	auto recentFiles = getContext().getRecentlyOpenedProjects();
	for (const auto& filename : recentFiles)
	{
		auto action = mRecentProjectsMenu->addAction(filename);
		connect(action, &QAction::triggered, [this, filename]()
		{
			if (confirmSaveCurrentFile())
				getContext().loadProject(filename);
		});
	}

	mRecentProjectsMenu->setEnabled(!mRecentProjectsMenu->isEmpty());
}


void MainWindow::onDocked(QDockWidget *dockWidget)
{
}

AppContext& MainWindow::getContext() const
{
	return AppContext::get();
}


void napkin::MainWindow::onServiceConfigChanged(QList<PropertyPath> paths)
{
	/*
	auto sceneTreeSelection = mScenePanel.treeView().getTreeView().selectionModel();
	sceneTreeSelection->blockSignals(true);
	sceneTreeSelection->clearSelection();
	sceneTreeSelection->blockSignals(false);
	if (!paths.isEmpty())
	{
		auto path = paths.first();
		mInspectorPanel.setPath(paths.first());
	}
	*/
}
