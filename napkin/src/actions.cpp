#include "actions.h"

using namespace napkin;

Action::Action() : QAction() { connect(this, &QAction::triggered, this, &Action::perform); }

NewFileAction::NewFileAction()
{
	setText("New");
	setShortcut(QKeySequence::New);
}

void NewFileAction::perform() { AppContext::get().newFile(); }


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

	AppContext::get().loadFile(filename);
}

SaveFileAction::SaveFileAction()
{
	setText("Save");
	setShortcut(QKeySequence::Save);
}

void SaveFileAction::perform()
{
	if (AppContext::get().getCurrentFilename().isNull())
	{
		SaveFileAsAction().trigger();
		return;
	}
	AppContext::get().saveFile();
}

SaveFileAsAction::SaveFileAsAction()
{
	setText("Save as...");
	setShortcut(QKeySequence::SaveAs);
}

void SaveFileAsAction::perform()
{
	auto& ctx = AppContext::get();
	auto prevFilename = ctx.getCurrentFilename();
	if (prevFilename.isNull())
		prevFilename = ctx.getLastOpenedFilename();

	QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0], "Save NAP Data File",
													prevFilename, JSON_FILE_FILTER);

	if (filename.isNull())
		return;

	ctx.saveFileAs(filename);
}

AddObjectAction::AddObjectAction(const rttr::type& type) : Action(), mType(type)
{
    setText(QString(type.get_name().data()));
}

void AddObjectAction::perform()
{
    AppContext::get().addObject(mType, true);
}

DeleteObjectAction::DeleteObjectAction(nap::rtti::RTTIObject& object) : Action(), mObject(object)
{
    setText("Delete");
}

void DeleteObjectAction::perform()
{
    AppContext::get().deleteObject(mObject);
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
    AppContext::get().addComponent(mEntity, mComponentType);
}

AddComponentAction::AddComponentAction(nap::Entity& entity, nap::rtti::TypeInfo type)
        : Action(), mEntity(entity), mComponentType(type)
{
    setText(QString(type.get_name().data()));
}

void AddEntityAction::perform()
{
    AppContext::get().createEntity(mParent);
}

AddEntityAction::AddEntityAction(nap::Entity* parent) : Action(), mParent(parent)
{
    setText("Add Entity");
}

