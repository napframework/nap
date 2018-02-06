#include "actions.h"

#include <QMessageBox>
#include "commands.h"

using namespace napkin;

Action::Action() : QAction() { connect(this, &QAction::triggered, this, &Action::perform); }

NewFileAction::NewFileAction()
{
	setText("New");
	setShortcut(QKeySequence::New);
}

void NewFileAction::perform()
{
	if (AppContext::get().getDocument()->isDirty()) {
		auto result = QMessageBox::question(AppContext::get().getQApplication()->topLevelWidgets()[0],
											"Save before creating new document",
											"The current document has unsaved changes.\n"
													"Save the changes before creating a new document?",
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			SaveFileAction action;
			action.trigger();
		}
		else if (result == QMessageBox::Cancel)
		{
			return;
		}
	}
	AppContext::get().newDocument();
}


OpenFileAction::OpenFileAction()
{
	setText("Open...");
	setShortcut(QKeySequence::Open);
}

void OpenFileAction::perform()
{
	auto lastFilename = AppContext::get().getLastOpenedFilename();
	QString filename = QFileDialog::getOpenFileName(QApplication::topLevelWidgets()[0], "Open NAP Data File",
													lastFilename, JSON_FILE_FILTER);
	if (filename.isNull())
		return;

	AppContext::get().loadDocument(filename);
}

SaveFileAction::SaveFileAction()
{
	setText("Save");
	setShortcut(QKeySequence::Save);
}

void SaveFileAction::perform()
{
	if (AppContext::get().getDocument()->getCurrentFilename().isNull())
	{
		SaveFileAsAction().trigger();
		return;
	}
	AppContext::get().saveDocument();
}

SaveFileAsAction::SaveFileAsAction()
{
	setText("Save as...");
	setShortcut(QKeySequence::SaveAs);
}

void SaveFileAsAction::perform()
{
	auto& ctx = AppContext::get();
	auto prevFilename = ctx.getDocument()->getCurrentFilename();
	if (prevFilename.isNull())
		prevFilename = ctx.getLastOpenedFilename();

	QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0], "Save NAP Data File",
													prevFilename, JSON_FILE_FILTER);

	if (filename.isNull())
		return;

	ctx.saveDocumentAs(filename);
}

AddObjectAction::AddObjectAction(const rttr::type& type) : Action(), mType(type)
{
    setText(QString(type.get_name().data()));
}

void AddObjectAction::perform()
{
	AppContext::get().executeCommand(new AddObjectCommand(mType));
}

DeleteObjectAction::DeleteObjectAction(nap::rtti::RTTIObject& object) : Action(), mObject(object)
{
    setText("Delete");
}

void DeleteObjectAction::perform()
{
    AppContext::get().executeCommand(new DeleteObjectCommand(mObject));
}

SetThemeAction::SetThemeAction(const QString& themeName) : Action(), mTheme(themeName)
{
    setText(themeName.isEmpty() ? napkin::TXT_DEFAULT_THEME : themeName);
    setCheckable(true);
}

void SetThemeAction::perform()
{
    AppContext::get().getThemeManager().setTheme(mTheme);
}

void AddComponentAction::perform()
{
    AppContext::get().getDocument()->addComponent(mEntity, mComponentType);
}

AddComponentAction::AddComponentAction(nap::Entity& entity, nap::rtti::TypeInfo type)
        : Action(), mEntity(entity), mComponentType(type)
{
    setText(QString(type.get_name().data()));
}


AddEntityAction::AddEntityAction(nap::Entity* parent) : Action(), mParent(parent)
{
    setText("Add Entity");
}

void AddEntityAction::perform()
{
	AppContext::get().executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity), mParent));
}


