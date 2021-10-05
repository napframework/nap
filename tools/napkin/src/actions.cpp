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
	auto result = QMessageBox::question
	(
		AppContext::get().getMainWindow(),
		QString("Save before %1 %2").arg(reason, type),
		QString("The current document has unsaved changes.\n"
		"Save the changes before %1 %2?").arg(reason, type),
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
	);

	// Handle
	if (result == QMessageBox::No || result == QMessageBox::Yes)
	{
		if (result == QMessageBox::Yes)
		{
			SaveFileAction action;
			action.trigger();
		}
		return true;
	}
	return false;
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

napkin::UpdateDefaultFileAction::UpdateDefaultFileAction()
{
	setText("Set as project default");
}


void napkin::UpdateDefaultFileAction::perform()
{
	// No document, core is not initialized
	Document* doc = AppContext::get().getDocument();
	if (doc == nullptr)
		return;

	// Save if not saved yet
	if (doc->getFilename().isNull())
	{
		// Attempt to save document
		SaveFileAsAction().trigger();
		if (doc->getFilename().isNull())
		{
			return;
		}
	}

	// Clone current project information
	auto* project_info = AppContext::get().getProjectInfo();
	assert(project_info != nullptr);
	
	// Get data directory and create relative path
	QDir proj_dir(QString::fromStdString(project_info->getProjectDir()));
	QString new_path = proj_dir.relativeFilePath(AppContext::get().getDocument()->getFilename());
	project_info->mDefaultData = new_path.toStdString();

	nap::rtti::JSONWriter writer;
	nap::utility::ErrorState error;
	if (!nap::rtti::serializeObject(*project_info, writer, error))
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
	auto& ctx = AppContext::get();
	napkin::Document* doc = AppContext::get().getDocument();
	if (doc == nullptr)
	{
		nap::Logger::warn("Unable to save document to file, no document loaded");
		return;
	}

	// No previous save
	if (doc->getFilename().isNull())
	{
		SaveFileAsAction().trigger();
		return;
	}

	// Override existing
	if (!ctx.saveDocument())
	{
		nap::Logger::error("Unable to save file: %s", doc->getFilename().toUtf8().constData());
	}
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
	auto cur_file_name = doc->getFilename();
	if (cur_file_name.isNull())
	{
		assert(AppContext::get().getProjectInfo() != nullptr);
		cur_file_name =  QString::fromStdString(AppContext::get().getProjectInfo()->getDataDirectory());
		cur_file_name += "/untitled.json";
	}

	QString filename = QFileDialog::getSaveFileName(ctx.getMainWindow(), "Save NAP Data File", 	cur_file_name, JSON_DATA_FILTER);
	if (filename.isNull())
		return;

	if (!filename.endsWith("." + QString(JSON_FILE_EXT)))
		filename = filename + "." + QString(JSON_FILE_EXT);

	// Save document
	if (ctx.saveDocumentAs(filename))
	{
		/// If the saved document is different from current project default, ask to update
		if (!ctx.documentIsProjectDefault())
		{
			auto result = QMessageBox::question(AppContext::get().getMainWindow(),
				"Set as Project Default?",
				QString("Set %1 as project default?").arg(QFileInfo(filename).fileName()));

			if (result == QMessageBox::StandardButton::Yes)
				UpdateDefaultFileAction().trigger();
		}
	}
	else
	{
		nap::Logger::error("Unable to save file: %s", filename.toUtf8().constData());
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
		dir = doc->getFilename().isNull() ?
			QString::fromStdString(ctx.getProjectInfo()->getDataDirectory()) :
			ctx.getDocument()->getFilename();
	}

	// Get file to open
	QString filename = napkinutils::getOpenFilename(nullptr, "Open NAP Data File", dir, JSON_DATA_FILTER);
	if (filename.isNull())
		return;

	// Bail if we don't want to continue
	if (!continueAfterSavingChanges("opening", "document"))
		return;

	// Load document
	if (ctx.loadDocument(filename) != nullptr)
	{
		/// If the saved document is different from current project default, ask to update
		if (!ctx.documentIsProjectDefault())
		{
			auto result = QMessageBox::question(AppContext::get().getMainWindow(),
				"Set as Project Default?", QString("Set %1 as project default?").arg(QFileInfo(filename).fileName()));

			if (result == QMessageBox::StandardButton::Yes)
				UpdateDefaultFileAction().trigger();
		}
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
	setText("New");
}


void napkin::NewServiceConfigAction::perform()
{
	if (AppContext::get().hasServiceConfig())
	{
		AppContext::get().getServiceConfig()->create();
	}
}


napkin::SaveServiceConfigAction::SaveServiceConfigAction()
{
	setText("Save");
}


void napkin::SaveServiceConfigAction::perform()
{
	auto& ctx = AppContext::get();
	if (ctx.hasServiceConfig())
	{
		// Save as if no file associated with config
		if (ctx.getServiceConfig()->getFilename().isNull())
		{
			SaveServiceConfigurationAs().trigger();
			return;
		}

		// Save config and ask to set as default if different
		if (!ctx.getServiceConfig()->save())
		{
			nap::Logger::error("Unable to save config file: %s", 
				ctx.getServiceConfig()->getFilename().toUtf8().constData());
		}
	}
}

 
napkin::SaveServiceConfigurationAs::SaveServiceConfigurationAs()
{
	setText("Save as...");
}


void napkin::SaveServiceConfigurationAs::perform()
{
	auto& ctx = AppContext::get();
	if (!ctx.hasServiceConfig())
		return;

	// Get name and location to store
	auto cur_file_name = ctx.getServiceConfig()->getFilename();
	if (cur_file_name.isNull())
	{
		assert(AppContext::get().getProjectInfo() != nullptr);
		cur_file_name = QString::fromStdString(AppContext::get().getProjectInfo()->getProjectDir());
		cur_file_name += "/service_config." + QString(JSON_FILE_EXT);
	}
	QString filename = QFileDialog::getSaveFileName(ctx.getMainWindow(), "Save NAP Config File",
		cur_file_name, JSON_CONFIG_FILTER);

	// Cancelled
	if (filename.isNull())
		return;

	// Ensure extension and save
	filename = !filename.endsWith("." + QString(JSON_FILE_EXT)) ? filename + "." + QString(JSON_FILE_EXT) : filename;
	if (!ctx.getServiceConfig()->saveAs(filename))
	{
		nap::Logger::error("Unable to save config file: %s", filename.toUtf8().constData());
		return;
	}

	// Set as project default if saved config is different from project default
	if(!ctx.getServiceConfig()->isProjectDefault())
	{
		auto result = QMessageBox::question(AppContext::get().getMainWindow(),
			"Set as Project Default?",
			QString("Set %1 as default configuration?").arg(QFileInfo(filename).fileName()));

		if (result == QMessageBox::StandardButton::Yes)
			SetAsDefaultServiceConfigAction().trigger();
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
	if (!ctx.hasServiceConfig())
		return;

	// Get directory
	QString dir = ctx.getServiceConfig()->getFilename().isNull() ?
		QString::fromStdString(ctx.getProjectInfo()->getProjectDir()) :
		ctx.getServiceConfig()->getFilename();

	// Get file to open
	QString filename = napkinutils::getOpenFilename(nullptr, "Open NAP Config File", dir, JSON_CONFIG_FILTER);
	if (filename.isNull())
		return;
	
	// Load config
	if (ctx.getServiceConfig()->load(filename))
	{
		// Set as project default if new config is different from project default
		if (!ctx.getServiceConfig()->isProjectDefault())
		{
			auto result = QMessageBox::question(AppContext::get().getMainWindow(),
				"Set as Project Default?", QString("Set %1 as default configuration?").arg(QFileInfo(filename).fileName()));

			if (result == QMessageBox::StandardButton::Yes)
				SetAsDefaultServiceConfigAction().trigger();
		}
	}
}


napkin::SetAsDefaultServiceConfigAction::SetAsDefaultServiceConfigAction()
{
	setText("Set as project default");
}


void napkin::SetAsDefaultServiceConfigAction::perform()
{
	auto& ctx = AppContext::get();
	if (!ctx.hasServiceConfig())
		return;

	// Save if not saved yet
	if (ctx.getServiceConfig()->getFilename().isNull())
	{
		// Attempt to save document
		SaveServiceConfigurationAs().trigger();
		if (ctx.getServiceConfig()->getFilename().isNull())
		{
			return;
		}
	}

	// Set as default in project
	ctx.getServiceConfig()->makeProjectDefault();
}


napkin::ClearServiceConfigAction::ClearServiceConfigAction()
{
	setText("Clear");
}


void napkin::ClearServiceConfigAction::perform()
{
		
}
