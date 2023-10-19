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
}


void MainWindow::showEvent(QShowEvent* event)
{
	BaseWindow::showEvent(event);
	if (!mShown)
	{
		QSettings settings;
		nap::Logger::debug("Using settings file: %s", settings.fileName().toStdString().c_str());
		getContext().restoreUI();
		rebuildRecentMenu();
		mShown = true;
	}
	connect(&getContext(), &AppContext::progressChanged, this, &MainWindow::onProgress, Qt::UniqueConnection);
}


void napkin::MainWindow::hideEvent(QHideEvent* event)
{
	mProgressDialog.reset(nullptr);
	disconnect(&getContext(), &AppContext::progressChanged, this, &MainWindow::onProgress);
	BaseWindow::hideEvent(event);
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!confirmSaveCurrentFile())
	{
		event->ignore();
		return;
	}
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
	addDock("Configuration", &mServiceConfigPanel);
}


void MainWindow::addMenu()
{
	// Project
	auto projectmenu = new QMenu("Project", menuBar());
	{
		projectmenu->addAction(new OpenProjectAction());
		mRecentProjectsMenu = projectmenu->addMenu("Recent Projects");
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), projectmenu);

	// File (Data)
	auto filemenu = new QMenu("File", menuBar());
	{
		filemenu->addAction(new NewFileAction());
		filemenu->addAction(new OpenFileAction());
		filemenu->addAction(new SaveFileAction());
		filemenu->addAction(new SaveFileAsAction());
		filemenu->addAction(new ReloadFileAction());
		filemenu->addAction(new UpdateDefaultFileAction());
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), filemenu);

	// Service Configuration menu
	auto config_menu = new QMenu("Configuration", menuBar());
	{
		config_menu->addAction(new NewServiceConfigAction());
		config_menu->addAction(new OpenServiceConfigAction());
		config_menu->addAction(new SaveServiceConfigAction());
		config_menu->addAction(new SaveServiceConfigurationAs());
		config_menu->addAction(new SetAsDefaultServiceConfigAction());
	}

	menuBar()->insertMenu(getWindowMenu()->menuAction(), config_menu);
	menuBar()->insertMenu(getWindowMenu()->menuAction(), &mThemeMenu);

	// Help
	auto help_menu = new QMenu("Help", menuBar());
	{
		auto open_url_action = new OpenURLAction("NAP Documentation", QUrl("https://docs.nap.tech"));
		help_menu->addAction(open_url_action);
	}
	menuBar()->insertMenu(getWindowMenu()->menuAction(), help_menu);
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
	QFileInfo fi(getContext().getDocument()->getFilename());
	QString changed = getContext().getDocument()->isDirty() ? "*" : "";
	setWindowTitle(QString("%1%2 %3 | %4 - %5").arg(QString::fromStdString(project_info->mTitle),
												changed, QString::fromStdString(project_info->mVersion),
												fi.exists() ? fi.fileName() : "No File",
												QApplication::applicationName()));
}

MainWindow::MainWindow() : BaseWindow(), mErrorDialog(this)
{
	setStatusBar(&mStatusBar);
	addMenu();
	addToolstrip();
	addDocks();
	bindSignals();
}

MainWindow::~MainWindow()
{
	unbindSignals();
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
		auto ob = rtti_cast<nap::math::FloatFCurve>(paths.first().getObject());
		if (ob != nullptr)
		{
			mCurvePanel.editCurve(ob);
		}
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

void MainWindow::onProgress(float fraction, const QString& message)
{
	if (mProgressDialog == nullptr)
	{
		mProgressDialog = std::make_unique<QProgressDialog>(this);
		mProgressDialog->setAutoReset(true);
		mProgressDialog->setAutoClose(true);
		mProgressDialog->setRange(0, 100);
		mProgressDialog->setWindowTitle("Working...");
		mProgressDialog->setCancelButton(nullptr);
		mProgressDialog->setModal(Qt::WindowModal);
		mProgressDialog->show();
		QApplication::processEvents();
	}

	// Set message and value
	if (!message.isEmpty())
		mProgressDialog->setLabelText(message);
	mProgressDialog->setValue(static_cast<int>(fraction * 100.0f));
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

	auto result = QMessageBox::question(this, "Save changes?",
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
{ }


AppContext& MainWindow::getContext() const
{
	return AppContext::get();
}


void napkin::MainWindow::addToolstrip()
{
	mToolbar = this->addToolBar("Toolbar");
	mToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
	mToolbar->setMovable(false);

	// Project Actions
	auto open_project_action = new OpenProjectAction();
	open_project_action->setText("Open Project");
	mToolbar->addAction(open_project_action);

	// File Actions
	mToolbar->addSeparator();
	mToolbar->addAction(new NewFileAction());
	mToolbar->addAction(new OpenFileAction());
	mToolbar->addAction(new SaveFileAction());
	mToolbar->addAction(new SaveFileAsAction());
	mToolbar->addAction(new ReloadFileAction());
	mToolbar->addAction(new UpdateDefaultFileAction());

	// Resource Actions
	mToolbar->addSeparator();
	mToolbar->addAction(new CreateResourceAction());
	mToolbar->addAction(new CreateEntityAction());
	mToolbar->addAction(new CreateGroupAction());
}


void napkin::MainWindow::onServiceConfigChanged(QList<PropertyPath> paths)
{
	auto sceneTreeSelection = mScenePanel.treeView().getTreeView().selectionModel();
	sceneTreeSelection->blockSignals(true);
	sceneTreeSelection->clearSelection();
	sceneTreeSelection->blockSignals(false);
	if (!paths.isEmpty())
	{
		auto path = paths.first();
		mInspectorPanel.setPath(paths.first());
	}
}
