#include "actions.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include "standarditemsobject.h"
#include "commands.h"
#include "naputils.h"
#include "napkinutils.h"

using namespace napkin;

Action::Action() : QAction() { connect(this, &QAction::triggered, this, &Action::perform); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NewFileAction::NewFileAction()
{
	setText("New");
	setShortcut(QKeySequence::New);
}

void NewFileAction::perform()
{
	if (AppContext::get().hasDocument() && AppContext::get().getDocument()->isDirty()) 
	{
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OpenProjectAction::OpenProjectAction()
{
	setText("Open Project...");
	setShortcut(QKeySequence::Open);
}

void OpenProjectAction::perform()
{
	auto lastFilename = AppContext::get().getLastOpenedProjectFilename();
    auto topLevelWidgets = QApplication::topLevelWidgets();

	QString filename = napkinutils::getOpenFilename(nullptr, "Open NAP Project", "", JSON_FILE_FILTER);
	if (filename.isNull())
		return;

	AppContext::get().loadProject(filename);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ReloadFileAction::ReloadFileAction()
{
	setText("Reload");
}

void ReloadFileAction::perform()
{
	AppContext::get().reloadDocument();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		prevFilename = "untitled.json";

	QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0], "Save NAP Data File",
													prevFilename, JSON_FILE_FILTER);

	if (filename.isNull())
		return;

	if (!filename.endsWith("." + JSON_FILE_EXT))
		filename = filename + "." + JSON_FILE_EXT;

	ctx.saveDocumentAs(filename);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CreateResourceAction::CreateResourceAction()
{
	setText("Create Resource...");
}

void CreateResourceAction::perform()
{
	auto parentWidget = AppContext::get().getMainWindow();

	auto type = napkin::showTypeSelector(parentWidget, [](auto t)
	{
		if (t.is_derived_from(RTTI_OF(nap::Component)))
			return false;
		return t.is_derived_from(RTTI_OF(nap::Resource));
	});

	if (type.is_valid() && !type.is_derived_from(RTTI_OF(nap::Component)))
		AppContext::get().executeCommand(new AddObjectCommand(type));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CreateEntityAction::CreateEntityAction()
{
	setText("Create Entity");
}

void CreateEntityAction::perform()
{
	AppContext::get().executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity), nullptr));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddChildEntityAction::AddChildEntityAction(nap::Entity& entity) : entity(&entity)
{
	setText("Add Child Entity...");
}

void AddChildEntityAction::perform()
{
	auto doc = AppContext::get().getDocument();

	std::vector<nap::rtti::Object*> filteredEntities;
	for (auto e : doc->getObjects<nap::Entity>())
	{
		// Omit self and entities that have self as a child
		if (e == entity || doc->hasChild(*e, *entity, true))
			continue;
		filteredEntities.emplace_back(e);
	}

	auto parentWidget = AppContext::get().getMainWindow();
	auto child = dynamic_cast<nap::Entity*>(napkin::showObjectSelector(parentWidget, filteredEntities));
	if (!child)
		return;

	AppContext::get().executeCommand(new AddChildEntityCommand(*entity, *child));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddComponentAction::AddComponentAction(nap::Entity& entity) : entity(&entity)
{
	setText("Add Component...");
}

void AddComponentAction::perform()
{
	auto parent = AppContext::get().getMainWindow();

	auto comptype = napkin::showTypeSelector(parent, [](auto t)
	{
		return t.is_derived_from(RTTI_OF(nap::Component));
	});

	if (comptype.is_valid())
		AppContext::get().executeCommand(new AddComponentCommand(*entity, comptype));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteObjectAction::DeleteObjectAction(nap::rtti::Object& object) : mObject(object)
{
    setText("Delete");
}

void DeleteObjectAction::perform()
{
	auto pointers = AppContext::get().getDocument()->getPointersTo(mObject, false, true);

	if (!pointers.empty())
	{
		QString message = "The following properties are still pointing to this object,\n"
			"your data might end up in a broken state.\n\n"
			"Do you want to delete anyway?";
		if (!showPropertyListConfirmDialog(parentWidget(), pointers, "Warning", message))
			return;
	}
    AppContext::get().executeCommand(new DeleteObjectCommand(mObject));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemoveChildEntityAction::RemoveChildEntityAction(EntityItem& entityItem) : entityItem(&entityItem)
{
	setText("Remove");
}

void RemoveChildEntityAction::perform()
{
	// TODO: Move into Command
	auto parentItem = dynamic_cast<EntityItem*>(entityItem->parentItem());

	auto doc = AppContext::get().getDocument();
	auto index = parentItem->childIndex(*entityItem);
	assert(index >= 0);

	// Grab all component paths for later instance property removal
	QStringList componentPaths;
	nap::qt::traverse(*parentItem->model(), [&componentPaths](QStandardItem* item)
	{
		auto compItem = dynamic_cast<ComponentItem*>(item);
		if (compItem)
			componentPaths << QString::fromStdString(compItem->componentPath());

		return true;
	}, entityItem->index());

	doc->removeChildEntity(*parentItem->getEntity(), index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemovePathAction::RemovePathAction(const PropertyPath& path)
	: mPath(path)
{
	setText("Remove");
}

void RemovePathAction::perform()
{
	AppContext::get().getDocument()->remove(mPath);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SetThemeAction::SetThemeAction(const QString& themeName) : Action(), mTheme(themeName)
{
    setText(themeName.isEmpty() ? napkin::TXT_THEME_DEFAULT : themeName);
    setCheckable(true);
}

void SetThemeAction::perform()
{
    AppContext::get().getThemeManager().setTheme(mTheme);
}

