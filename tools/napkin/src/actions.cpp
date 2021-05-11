/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "actions.h"
#include "standarditemsobject.h"
#include "commands.h"
#include "naputils.h"
#include "napkinutils.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include <rtti/rttiutilities.h>
#include <rtti/jsonwriter.h>
#include <utility/errorstate.h>

using namespace napkin;

Action::Action() : QAction() { connect(this, &QAction::triggered, this, &Action::perform); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NewFileAction::NewFileAction()
{
	setText("New");
	setShortcut(QKeySequence::New);
}


static bool continueAfterSavingChanges(const QString& reason, const QString& type)
{
	// No document
	if (!AppContext::get().hasDocument())
		return true;

	// Document not dirty
	if (!AppContext::get().getDocument()->isDirty())
		return true;

	// Document is dirty
	auto result = QMessageBox::question(AppContext::get().getQApplication()->topLevelWidgets()[0],
		QString("Save before %1 %2").arg(reason, type),
		QString("The current document has unsaved changes.\n"
		"Save the changes before %1 %2?").arg(reason, type),
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	// Handle result
	switch (result)
	{
		case QMessageBox::Yes:
		{
			SaveFileAction action;
			action.trigger();
			return true;
		}
		case QMessageBox::No:
			return true;
		case QMessageBox::Cancel:
			return false;
		default:
			assert(false);
			return false;
	}
}


void NewFileAction::perform()
{
	if (continueAfterSavingChanges("creating a new", "document"))
	{
		AppContext::get().newDocument();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OpenProjectAction::OpenProjectAction()
{
	setText("Open...");
}

void OpenProjectAction::perform()
{
	QString filename = napkinutils::getOpenFilename(nullptr, "Open NAP Project", "", JSON_PROJECT_FILTER);
	if (filename.isNull())
		return;

	AppContext::get().loadProject(filename);
}

//////////////////////////////////////////////////////////////////////////

napkin::UpdateDefaultAction::UpdateDefaultAction()
{
	setText("Set as project default");
}


void napkin::UpdateDefaultAction::perform()
{
	// No document, core is not initialized
	Document* doc = AppContext::get().getDocument();
	if (doc == nullptr)
		return;

	// Save if not saved yet
	if (doc->getCurrentFilename().isNull())
	{
		// Attempt to save document
		SaveFileAsAction().trigger();
		if (doc->getCurrentFilename().isNull())
			return;
	}

	// Clone current project information
	const auto* project_info = AppContext::get().getProjectInfo();
	assert(project_info != nullptr);
	std::unique_ptr<nap::ProjectInfo> new_info = nap::rtti::cloneObject(*project_info, AppContext::get().getCore().getResourceManager()->getFactory());
	
	// Get data directory and create relative path
	QDir data_dir(QString::fromStdString(project_info->getProjectDir()));
	QString new_path = data_dir.relativeFilePath(AppContext::get().getDocument()->getCurrentFilename());
	new_info->mDefaultData = new_path.toStdString();

	nap::rtti::JSONWriter writer;
	nap::utility::ErrorState error;
	if (!nap::rtti::serializeObject(*new_info, writer, error))
	{
		nap::Logger::error(error.toString());
		return;
	}

	// Open output file
	std::ofstream output(project_info->getFilename(), std::ios::binary | std::ios::out);
	if (!error.check(output.is_open() && output.good(), "Failed to open %s for writing", project_info->getFilename().c_str()))
	{
		nap::Logger::error(error.toString());
		return;
	}

	// Write to disk
	std::string json = writer.GetJSON();
	output.write(json.data(), json.size());
	nap::Logger::info("Updated project data: %s", new_path.toUtf8().constData());

	// Notify user app needs to restart if running
	auto result = QMessageBox::information(AppContext::get().getQApplication()->topLevelWidgets()[0],
		"Restart application if running",
		QString("Please restart %1 for the changes to take effect").arg(QString::fromStdString(project_info->mTitle)));
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
	// Get current document, nullptr when there is no document and document can't be created
	// This is the case when no project is loaded or core failed to initialize
	napkin::Document* doc = AppContext::get().getDocument();
	if (doc == nullptr)
	{
		nap::Logger::warn("Unable to save document to file, no document loaded");
		return;
	}

	if (doc->getCurrentFilename().isNull())
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
	napkin::Document* doc = ctx.getDocument();
	
	// Core not initialized
	if (doc == nullptr)
		return;

	// Get name and location to store
	auto cur_file_name = doc->getCurrentFilename();
	if (cur_file_name.isNull())
	{
		assert(AppContext::get().getProjectInfo() != nullptr);
		cur_file_name =  QString::fromStdString(AppContext::get().getProjectInfo()->getDataDirectory());
		cur_file_name += "/untitled.json";
	}
	QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0], "Save NAP Data File",
		cur_file_name, JSON_DATA_FILTER);

	if (filename.isNull())
		return;

	if (!filename.endsWith("." + JSON_FILE_EXT))
		filename = filename + "." + JSON_FILE_EXT;

	if (!ctx.saveDocumentAs(filename))
	{
		nap::Logger::error("Unable to save file: %s", filename.toUtf8().constData());
		return;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


napkin::OpenFileAction::OpenFileAction()
{
	setText("Open...");
	setShortcut(QKeySequence::Open);
}


void napkin::OpenFileAction::perform()
{
	auto& ctx = AppContext::get();

	// Doc exists, use current file or data directory as starting point
	napkin::Document* doc = ctx.getDocument();
	QString dir = "";
	if (doc != nullptr)
	{	
		dir = doc->getCurrentFilename().isNull() ?
			QString::fromStdString(ctx.getProjectInfo()->getDataDirectory()) :
			ctx.getDocument()->getCurrentFilename();
	}

	// Get file to open
	QString filename = napkinutils::getOpenFilename(nullptr, "Open NAP Data File", dir, JSON_DATA_FILTER);
	if (filename.isNull())
		return;

	if (continueAfterSavingChanges("opening", "document"))
	{ 
		AppContext::get().loadDocument(filename);
	}
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


napkin::NewServiceConfigAction::NewServiceConfigAction()
{
	setText("New Default");
}


void napkin::NewServiceConfigAction::perform()
{
	AppContext::get().newServiceConfig();
}


napkin::SaveServiceConfigAction::SaveServiceConfigAction()
{
	setText("Save");
}


void napkin::SaveServiceConfigAction::perform()
{
	if (AppContext::get().getServiceConfigFilename().isNull())
	{
		SaveServiceConfigurationAs().trigger();
		return;
	}
	AppContext::get().saveServiceConfig();
}

 
napkin::SaveServiceConfigurationAs::SaveServiceConfigurationAs()
{
	setText("Save as...");
}


void napkin::SaveServiceConfigurationAs::perform()
{
	// Get name and location to store
	auto& ctx = AppContext::get();
	auto cur_file_name = ctx.getServiceConfigFilename();
	if (cur_file_name.isNull())
	{
		assert(AppContext::get().getProjectInfo() != nullptr);
		cur_file_name = QString::fromStdString(AppContext::get().getProjectInfo()->getProjectDir());
		cur_file_name += "/service_config.json";
	}
	QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0], "Save NAP Config File",
		cur_file_name, JSON_CONFIG_FILTER);

	// Cancelled
	if (filename.isNull())
		return;

	// Ensure extension and save
	filename = !filename.endsWith("." + JSON_FILE_EXT) ? filename+"."+JSON_FILE_EXT : filename;
	if (!ctx.saveServiceConfigAs(filename))
	{
		nap::Logger::error("Unable to save config file: %s", filename.toUtf8().constData());
		return;
	}
}


napkin::OpenServiceConfigAction::OpenServiceConfigAction()
{
	setText("Open...");
}


void napkin::OpenServiceConfigAction::perform()
{
	// Ensure project is available
	auto& ctx = AppContext::get();
	if (ctx.getProjectInfo() == nullptr)
	{
		nap::Logger::error("No project loaded");
		return;
	}

	// Get directory
	QString dir = ctx.getServiceConfigFilename().isNull() ?
		QString::fromStdString(ctx.getProjectInfo()->getDataDirectory()) :
		ctx.getServiceConfigFilename();

	// Get file to open
	QString filename = napkinutils::getOpenFilename(nullptr, "Open NAP Config File", dir, JSON_CONFIG_FILTER);
	if (filename.isNull())
		return;
	
	// Load config
	ctx.loadServiceConfig(filename);
}


napkin::DefaultServiceConfigAction::DefaultServiceConfigAction()
{
	setText("Set as project default");
}


void napkin::DefaultServiceConfigAction::perform()
{
	// Save if not saved yet
	if (AppContext::get().getServiceConfigFilename().isNull())
	{
		// Attempt to save document
		SaveServiceConfigurationAs().trigger();
		if (AppContext::get().getServiceConfigFilename().isNull())
			return;
	}

	// Clone current project information
	const auto* project_info = AppContext::get().getProjectInfo();
	assert(project_info != nullptr);
	std::unique_ptr<nap::ProjectInfo> new_info = nap::rtti::cloneObject(*project_info, AppContext::get().getCore().getResourceManager()->getFactory());

	// Get data directory and create relative path
	QDir data_dir(QString::fromStdString(project_info->getProjectDir()));
	QString new_path = data_dir.relativeFilePath(AppContext::get().getServiceConfigFilename());
	new_info->mServiceConfigFilename = new_path.toStdString();

	nap::rtti::JSONWriter writer;
	nap::utility::ErrorState error;
	if (!nap::rtti::serializeObject(*new_info, writer, error))
	{
		nap::Logger::error(error.toString());
		return;
	}

	// Open output file
	std::ofstream output(project_info->getFilename(), std::ios::binary | std::ios::out);
	if (!error.check(output.is_open() && output.good(), "Failed to open %s for writing", project_info->getFilename().c_str()))
	{
		nap::Logger::error(error.toString());
		return;
	}

	// Write to disk
	std::string json = writer.GetJSON();
	output.write(json.data(), json.size());
	nap::Logger::info("Updated project configuration: %s", new_path.toUtf8().constData());

	// Notify user app needs to restart if running
	auto result = QMessageBox::information(AppContext::get().getQApplication()->topLevelWidgets()[0],
		"Restart application if running",
		QString("Please restart %1 for the changes to take effect").arg(QString::fromStdString(project_info->mTitle)));
}


napkin::ClearServiceConfigAction::ClearServiceConfigAction()
{
	setText("Clear");
}


void napkin::ClearServiceConfigAction::perform()
{
		
}
