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
	connect(ctx, &AppContext::projectLoaded, this, &MainWindow::onProjectLoaded);
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
	disconnect(ctx, &AppContext::projectLoaded, this, &MainWindow::onProjectLoaded);
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

	// Set status bar font
	auto sf = statusBar()->font();
	sf.setPixelSize(this->font().pixelSize());
	statusBar()->setFont(sf);

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
	menuBar()->addMenu(getWindowMenu());
}


void MainWindow::configureMenu()
{
	// Project
	mProjectMenu.setTitle(action::groups::project);
	const auto& p_actions = mActionModel.getGroup(action::groups::project);
	for (auto* action : p_actions)
		mProjectMenu.addAction(action);
	mRecentProjectsMenu.setTitle("Recent Projects");
	mProjectMenu.addMenu(&mRecentProjectsMenu);
	menuBar()->addMenu(&mProjectMenu);

	// File (Data)
	mFileMenu.setTitle(action::groups::file);
	const auto& f_actions = mActionModel.getGroup(action::groups::file);
	for (auto* action : f_actions)
		mFileMenu.addAction(action);
	menuBar()->addMenu(&mFileMenu);

	// Service Configuration menu
	mConfigMenu.setTitle(action::groups::config);
	const auto& s_actions = mActionModel.getGroup(action::groups::config);
	for (auto* action : s_actions)
		mConfigMenu.addAction(action);
	menuBar()->addMenu(&mConfigMenu);

	// Create menu
	mCreateMenu.setTitle(action::groups::object);
	const auto& c_actions = mActionModel.getGroup(action::groups::object);
	for (auto* action : c_actions)
		mCreateMenu.addAction(action);
	menuBar()->addMenu(&mCreateMenu);

	// Theme
	menuBar()->addMenu(&mThemeMenu);

	// Help & Additional Web Resources
	mHelpMenu.setTitle(action::groups::help);
	const auto& h_actions = mActionModel.getGroup(action::groups::help);
	for (auto* action : h_actions)
		mHelpMenu.addAction(action);

	const auto& r_actions = mActionModel.getGroup(action::groups::resources);
	for (auto* action : r_actions)
		mHelpMenu.addAction(action);

	menuBar()->addMenu(&mHelpMenu);

	// Panels
	menuBar()->addMenu(getWindowMenu());
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
	configureMenu();
	addToolstrip();
	addDocks();

	enableProjectDependentActions(AppContext::get().getProjectLoaded());
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

	QMessageBox msg(this);
	msg.setWindowTitle("Save Changes?");
	msg.setText("The current document has unsaved changes.\nSave changes before exit?");
	msg.setIconPixmap(AppContext::get().getResourceFactory().getIcon(
		QRC_ICONS_QUESTION).pixmap(32, 32)
	);
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	msg.setDefaultButton(QMessageBox::Yes);
	auto result = msg.exec();

	if (result == QMessageBox::Yes)
	{
		SaveFileAction action(nullptr);
		action.trigger();
		return true;
	}
	return result == QMessageBox::No;
}

void MainWindow::rebuildRecentMenu()
{
	mRecentProjectsMenu.clear();
	auto recentFiles = getContext().getRecentlyOpenedProjects();
	for (const auto& filename : recentFiles)
	{
		auto action = mRecentProjectsMenu.addAction(filename);
		connect(action, &QAction::triggered, [this, filename]()
		{
			if (confirmSaveCurrentFile())
				getContext().loadProject(filename);
		});
	}
	mRecentProjectsMenu.setEnabled(!mRecentProjectsMenu.isEmpty());
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
    mToolbar->setObjectName("MainToolbar");
	mToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
	mToolbar->setMovable(false);

	// Project Actions
	const auto& p_actions = mActionModel.getGroup(action::groups::project);
	for (const auto& action : p_actions)
		mToolbar->addAction(action);

	// File Actions
	mToolbar->addSeparator();
	const auto& f_actions = mActionModel.getGroup(action::groups::file);
	for (const auto& action : f_actions)
		mToolbar->addAction(action);

	// Create Actions
	mToolbar->addSeparator();
	const auto& c_actions  = mActionModel.getGroup(action::groups::object);
	for (const auto& action : c_actions)
		mToolbar->addAction(action);

	// Create help actions
	mToolbar->addSeparator();
	const auto& h_actions = mActionModel.getGroup(action::groups::help);
	for (const auto& action : h_actions)
		mToolbar->addAction(action);
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


void napkin::MainWindow::onProjectLoaded(const nap::ProjectInfo& projectInfo)
{
	enableProjectDependentActions(true);
}


void napkin::MainWindow::enableProjectDependentActions(bool enable)
{
	// Project aware action groups
	static const std::vector<std::string> project_groups
	{
		action::groups::file,
		action::groups::config,
		action::groups::object
	};

	// Disable / Enable based
	for (const auto& group_name : project_groups)
	{
		auto& group = mActionModel.getGroup(group_name);
		for (const auto& action : group)
		{
			action->setEnabled(enable);
		}
	}
}
