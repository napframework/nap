#include "actions.h"
#include "commands.h"
#include <QFileDialog>
#include <QMenu>
#include <QSettings>


#define LAST_FILE_DIR "lastFileDir"



void NewAction::perform() const { AppContext::get().newFile(); }



void OpenAction::perform() const
{
	QSettings settings;
	auto lastFileDir = settings.value(LAST_FILE_DIR).toString();
	auto filename = QFileDialog::getOpenFileName(QApplication::activeWindow(), "Select file",
												 lastFileDir, fileTypesFilter());
	if (filename.isNull()) return;

	settings.setValue(LAST_FILE_DIR, QFileInfo(filename).absolutePath());

	AppContext::get().load(filename);
}



void SaveAsAction::perform() const
{
	QSettings settings;
	QString lastFileDir = settings.value(LAST_FILE_DIR).toString();

	QString filename = QFileDialog::getSaveFileName(
		QApplication::activeWindow(), "Select output file", lastFileDir, fileTypesFilter());
	if (filename.isNull()) return;

	settings.setValue(LAST_FILE_DIR, QFileInfo(filename).absolutePath());

	QString correctFilename =
		ensureExtension(filename, QString::fromStdString(defaultFileType()->extension()));

	AppContext::get().save(correctFilename);
}



void SaveAction::perform() const
{
	if (AppContext::get().isSaved()) {
		AppContext::get().save();
		return;
	}

	SaveAsAction action;
	action.trigger();
}



bool CutAction::isAvailable() { return AppContext::get().selection().size() > 0; }



void CutAction::perform() const
{
	selectionToClipboard();
	AppContext::get().execute(new RemoveObjectCmd(AppContext::get().selection()));
}



bool CopyAction::isAvailable() { return AppContext::get().selection().size() > 0; }



void CopyAction::perform() const { selectionToClipboard(); }



void PasteAction::perform() const
{
	nap::Object* parent = nullptr;
	QList<nap::Object*> selection = AppContext::get().selection();
	if (selection.size() > 0) {
		parent = selection[0];
	} else {
		parent = &AppContext::get().core().getRoot();
	}

	AppContext::get().execute(new PasteCmd(QApplication::clipboard()->text(), parent));
}



bool DeleteAction::isAvailable() { return AppContext::get().selection().size() > 0; }



void DeleteAction::perform() const
{
	auto selection = AppContext::get().selection();
	AppContext::get().execute(new RemoveObjectCmd(selection));
}



void QuitAction::perform() const
{
	if (!AppContext::get().isSaved()) {
		SaveAsAction().perform();
	}
	QApplication::exit(0);
}



bool UndoAction::isAvailable() { return AppContext::get().undoIndex() > 1; }



void UndoAction::perform() const { AppContext::get().undo(); }



bool RedoAction::isAvailable()
{
	return AppContext::get().undoIndex() > AppContext::get().undoStackSize();
}



void RedoAction::perform() const { AppContext::get().redo(); }



bool AddAttributeAction::isAvailable() { return AppContext::get().selection().size() == 1; }



void AddAttributeAction::perform() const
{
	nap::AttributeObject* object = (nap::AttributeObject*)AppContext::get().selectedObject();
	assert(object);
	AppContext::get().execute(new AddAttributeCmd(*object));
}



bool CreateEntityAction::isAvailable()
{
	return AppContext::get().selection().size() == 1 &&
		   AppContext::get().selectedObject()->getTypeInfo().isKindOf<nap::Entity>();
}



void CreateEntityAction::perform() const
{
	auto object = AppContext::get().selectedObject();
	assert(object);
	AppContext::get().execute(new CreateEntityCmd(*(nap::Entity*)object));
}



void CreateOperatorAction::perform() const
{
	QMenu menu;

	auto patch = AppContext::get().activePatch();
	QPoint mousePos = QCursor::pos();
	QWidget* widget = QApplication::activeWindow();
	QPointF dropPos = widget->childAt(widget->mapFromGlobal(mousePos))->mapFromGlobal(mousePos);

	assert(patch);

	for (const auto& opType : AppContext::get().core().getModuleManager().getOperatorTypes()) {
		auto action = menu.addAction(QString::fromStdString(opType.getName()));
		connect(action, &QAction::triggered, [=]() {
			AppContext::get().execute(new CreateOperatorCmd(*patch, opType, dropPos));
		});
	}

	if (menu.actions().size() > 0) menu.exec(QCursor::pos());
}



bool CreateComponentAction::isAvailable()
{
	return AppContext::get().selection().size() == 1 &&
		   AppContext::get().selectedObject()->getTypeInfo().isKindOf<nap::Entity>();
}



void CreateComponentAction::perform() const
{
	QMenu menu;
	auto entity = (nap::Entity*)AppContext::get().selectedObject();
	assert(entity);

	for (const auto& compType : AppContext::get().core().getModuleManager().getComponentTypes()) {
		auto action = menu.addAction(QString::fromStdString(compType.getName()));
		connect(action, &QAction::triggered,
				[=]() { AppContext::get().execute(new CreateComponentCmd(*entity, compType)); });
	}

	if (menu.actions().size() > 0) menu.exec(QCursor::pos());
}