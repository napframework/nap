#include "mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <fcurve.h>
#include <QtDebug>
#include <utility/fileutils.h>

using namespace napkin;

void MainWindow::bindSignals()
{
	connect(&getContext(), &AppContext::documentOpened, this, &MainWindow::onDocumentOpened);
	connect(&getContext(), &AppContext::documentChanged, this, &MainWindow::onDocumentChanged);
	connect(&mResourcePanel, &ResourcePanel::selectionChanged, this, &MainWindow::onResourceSelectionChanged);
	connect(&mScenePanel, &ScenePanel::selectionChanged, this, &MainWindow::onSceneSelectionChanged);
	connect(&mInstPropPanel, &InstancePropPanel::selectComponentRequested, this, &MainWindow::onSceneComponentSelectionRequested);
	connect(&getContext(), &AppContext::selectionChanged, &mResourcePanel, &ResourcePanel::selectObjects);
	connect(&getContext(), &AppContext::logMessage, this, &MainWindow::onLog);
	connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, &MainWindow::onDocked);
}


void MainWindow::unbindSignals()
{
	disconnect(&getContext(), &AppContext::documentOpened, this, &MainWindow::onDocumentOpened);
	disconnect(&getContext(), &AppContext::documentChanged, this, &MainWindow::onDocumentChanged);
	disconnect(&mResourcePanel, &ResourcePanel::selectionChanged, this, &MainWindow::onResourceSelectionChanged);
	disconnect(&mScenePanel, &ScenePanel::selectionChanged, this, &MainWindow::onSceneSelectionChanged);
	disconnect(&mInstPropPanel, &InstancePropPanel::selectComponentRequested, this, &MainWindow::onSceneComponentSelectionRequested);
	disconnect(&getContext(), &AppContext::selectionChanged, &mResourcePanel, &ResourcePanel::selectObjects);
	disconnect(&getContext(), &AppContext::logMessage, this, &MainWindow::onLog);
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
//	addDock("Available Types", &mHierarchyPanel);
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

		mRecentProjectsMenu = filemenu->addMenu("Recent Projects");

		auto reloadFileAction = new ReloadFileAction();
		addAction(reloadFileAction);
		filemenu->addAction(reloadFileAction);

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
	auto& ctx = getContext();

	auto project = ctx.getProject();
	if (!project)
		setWindowTitle(QApplication::applicationName());

	QString changed = getContext().getDocument()->isDirty() ? "*" : "";
	setWindowTitle(QString("%1%2 %3 - %4").arg(QString::fromStdString(project->mTitle),
											   changed, QString::fromStdString(project->mVersion),
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

void MainWindow::showError(nap::LogMessage msg)
{
	mErrorDialog.addMessage(QString::fromStdString(msg.text()));
	mErrorDialog.show();
}

bool MainWindow::confirmSaveCurrentFile()
{
	if (!getContext().getDocument())
		return false;

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


