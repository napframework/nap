#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>
#include <QMessageBox>

#include "mainwindow.h"
#include "napkin_utilities.h"


#include "4dsoundpanel/fourdsoundpanel.h"
#include "attributepanel/attributepanel.h"
#include "historypanel/historypanel.h"
#include "logpanel/logpanel.h"
#include "modulepanel/modulepanel.h"
#include "outlinepanel/outlinepanel.h"
#include "patchpanel/patchpanel.h"
#include "scenepanel/scenepanel.h"

#define WINDOW_GEOM "windowGeom"
#define WINDOW_STATE "windowState"

#define STYLESHEET "../../../napkin/resources/dark.qss"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	setDockNestingEnabled(true);
	centralWidget()->setHidden(true);

	initPanels();
	restoreGUI();

	// TODO: Temp
	QAction* refreshAction = new QAction("Refresh", this);
	refreshAction->setShortcut(tr("Ctrl+R"));
	connect(refreshAction, &QAction::triggered, [=]() {
		nap::Logger::info("Refresh");
		loadStyleSheet();
	});
	addAction(refreshAction);

	mResourceWatcher.addPath(STYLESHEET);
	connect(&mResourceWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::loadStyleSheet);
}

void MainWindow::initPanels()
{
	IconStore& icons = AppContext::get().iconStore();

	initPanel(new OutlinePanel(this), Qt::LeftDockWidgetArea);
	initPanel(new AttributePanel(this), Qt::RightDockWidgetArea);
	initPanel(new LogPanel(this), Qt::BottomDockWidgetArea);
	initPanel(new ModulePanel(this), Qt::BottomDockWidgetArea);
	initPanel(new PatchPanel(this), Qt::TopDockWidgetArea);
	initPanel(new HistoryPanel(this), Qt::RightDockWidgetArea);
	initPanel(new ScenePanel(this), Qt::TopDockWidgetArea);

	initPanel(new FourDSoundPanel(this), Qt::TopDockWidgetArea);
}


void MainWindow::initPanel(QDockWidget* panel, Qt::DockWidgetArea area, const QIcon* icon)
{
	panel->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetClosable |
					   QDockWidget::DockWidgetFeature::DockWidgetFloatable |
					   QDockWidget::DockWidgetFeature::DockWidgetMovable);
	if (icon) panel->setWindowIcon(*icon);
	addDockWidget(area, panel);

	QAction* showWindowAction = new QAction(panel->windowTitle(), ui.menuWindow);
	showWindowAction->setCheckable(true);
	connect(panel, &QDockWidget::visibilityChanged,
			[=](bool visible) { showWindowAction->setChecked(visible); });
	connect(showWindowAction, &QAction::triggered, [=]() { panel->show(); });
	ui.menuWindow->addAction(showWindowAction);
}


void MainWindow::restoreGUI()
{
	QSettings settings;
	restoreGeometry(settings.value(WINDOW_GEOM).toByteArray());
	restoreState(settings.value(WINDOW_STATE).toByteArray());
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	if (AppContext::get().isDirty()) {
		QMessageBox::StandardButton result = QMessageBox::question(
			this, "Save Unsaved Changes?", "There are unsaved changes,\nyou want to save?",
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		switch (result) {
		case QMessageBox::Yes:
			if (AppContext::get().actionStore().perform("Save")) {
				event->accept();
			} else {
				event->ignore();
			}
			break;
		case QMessageBox::No:
			event->accept();
			break;
		case QMessageBox::Cancel:
			event->ignore();
			break;
		default:
			assert(false);
		}
	}


	QSettings settings;
	settings.setValue(WINDOW_GEOM, saveGeometry());
	settings.setValue(WINDOW_STATE, saveState());
}

MainWindow::~MainWindow() {}


void MainWindow::loadStyleSheet()
{

	nap::Logger::info("Reloading stylesheet");
	QFile ssFile(STYLESHEET);
	ssFile.open(QFile::ReadOnly);
	QString stylesheet = QLatin1String(ssFile.readAll());
	setStyleSheet(stylesheet);
}

void MainWindow::showEvent(QShowEvent* event)
{
	initMenu();
	connect(&AppContext::get(), &AppContext::sceneChanged, this, &MainWindow::onSceneChanged);
	updateWindowTitle();
}

void MainWindow::initMenu()
{
	for (auto action : AppContext::get().actionStore().actions()) {
		QMenu* menu = getOrCreateMenu(action->category());
		menu->addAction(action);
	}
}

QMenu* MainWindow::getOrCreateMenu(QString name)
{
	if (name.isNull()) name = "General";
	QString objectName = "menu" + name;

	QMenu* menu = menuBar()->findChild<QMenu*>(objectName);
	if (!menu) {
		menu = new QMenu(name, menuBar());
		menu->setObjectName(objectName);
		menuBar()->addMenu(menu);
	}
	return menu;
}

void MainWindow::onSceneChanged() { updateWindowTitle(); }

void MainWindow::updateWindowTitle()
{
	QString title;
	QTextStream str(&title);

	if (AppContext::get().isDirty()) str << "*";
	str << AppContext::get().filename();

	setWindowTitle(title);
}
