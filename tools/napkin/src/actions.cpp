/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "actions.h"
#include "standarditemsobject.h"
#include "commands.h"
#include "naputils.h"
#include "napkinutils.h"
#include "napkin-resources.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include <rtti/rttiutilities.h>
#include <rtti/jsonwriter.h>
#include <utility/errorstate.h>
#include <entity.h>

using namespace napkin;

Action::Action(const char* text, const char* iconName) :
	QAction(), mIconName(iconName)
{
	setText(text);
	connect(this, &QAction::triggered, this, &Action::perform);
	connect(&AppContext::get().getThemeManager(), &ThemeManager::themeChanged, this, &Action::onThemeChanged);
	loadIcon();
}


void napkin::Action::loadIcon()
{
	if (!mIconName.isEmpty())
	{
		setIcon(AppContext::get().getResourceFactory().getIcon(mIconName));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NewFileAction::NewFileAction() : Action("New", QRC_ICONS_EDIT)
{
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

OpenProjectAction::OpenProjectAction() : Action("Open...", QRC_ICONS_FILE)
{ }


void OpenProjectAction::perform()
{
	QString filename = napkinutils::getOpenFilename(nullptr, "Select NAP Project", "", JSON_PROJECT_FILTER);
	if (filename.isNull())
		return;

	AppContext::get().loadProject(filename);
}


//////////////////////////////////////////////////////////////////////////

napkin::UpdateDefaultFileAction::UpdateDefaultFileAction() : Action("Set as project default", QRC_ICONS_CHANGE)
{ }


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

ReloadFileAction::ReloadFileAction() : Action("Reload", QRC_ICONS_RELOAD)
{ }


void ReloadFileAction::perform()
{
	AppContext::get().reloadDocument();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SaveFileAction::SaveFileAction() : Action("Save", QRC_ICONS_SAVE)
{
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

SaveFileAsAction::SaveFileAsAction() : Action("Save as...", QRC_ICONS_SAVE_AS)
{
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


napkin::OpenFileAction::OpenFileAction() : Action("Open...", QRC_ICONS_FILE)
{
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
	QString filename = napkinutils::getOpenFilename(nullptr, "Select NAP Data File", dir, JSON_DATA_FILTER);
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

CreateResourceAction::CreateResourceAction() : Action("Create Resource...", QRC_ICONS_RTTIOBJECT)
{ }


void CreateResourceAction::perform()
{
	auto parentWidget = AppContext::get().getMainWindow();
	auto type = napkin::showTypeSelector(parentWidget, [](auto t)
	{
		return t.is_derived_from(RTTI_OF(nap::Resource)) &&
			!t.is_derived_from(RTTI_OF(nap::Component))  &&
			!t.is_derived_from(RTTI_OF(nap::Entity));
	});

	if (type.is_valid() && !type.is_derived_from(RTTI_OF(nap::Component)))
		AppContext::get().executeCommand(new AddObjectCommand(type));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::CreateGroupAction::CreateGroupAction() : Action("Create Group...", QRC_ICONS_GROUP)
{ }


void napkin::CreateGroupAction::perform()
{
	auto parentWidget = AppContext::get().getMainWindow();
	auto type = napkin::showTypeSelector(parentWidget, [](auto t)
		{
			return t.is_derived_from(RTTI_OF(nap::IGroup));
		});

	if (type.is_valid())
		AppContext::get().executeCommand(new AddObjectCommand(type));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddNewResourceToGroupAction::AddNewResourceToGroupAction(nap::IGroup& group) :
	Action("Create Resource...", QRC_ICONS_RTTIOBJECT), mGroup(&group)
{ }

void AddNewResourceToGroupAction::perform()
{
	// Get path to resources array property
	PropertyPath array_path(*mGroup, mGroup->getMembersProperty(), *AppContext::get().getDocument());

	// Select type to add
	auto type = array_path.getArrayElementType();
	TypePredicate predicate = [type](auto t)
	{
		return t.is_derived_from(type) &&
			!t.is_derived_from(RTTI_OF(nap::Component)) &&
			!t.is_derived_from(RTTI_OF(nap::IGroup)) &&
			!t.is_derived_from(RTTI_OF(nap::Entity));
	};

	auto parentWidget = AppContext::get().getMainWindow();
	rttr::type elementType = showTypeSelector(parentWidget, predicate);
	if (elementType.is_valid())
	{
		AppContext::get().executeCommand(new ArrayAddNewObjectCommand(array_path, elementType));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::AddChildGroupAction::AddChildGroupAction(nap::IGroup& group) :
	Action("Create Group...", QRC_ICONS_GROUP), mGroup(&group)
{ }


void napkin::AddChildGroupAction::perform()
{
	// Get path to resources array property
	PropertyPath array_path(*mGroup, mGroup->getChildrenProperty(), *AppContext::get().getDocument());
	AppContext::get().executeCommand(new ArrayAddNewObjectCommand(array_path, mGroup->get_type()));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::MoveResourceToGroupAction::MoveResourceToGroupAction(nap::rtti::Object& resource, nap::IGroup* currentGroup) :
	Action("Move to Group...", QRC_ICONS_CHANGE), mObject(&resource), mCurrentGroup(currentGroup)
{ }



void napkin::MoveResourceToGroupAction::perform()
{
	// Get group to move to
	auto groups = AppContext::get().getDocument()->getObjects(RTTI_OF(nap::IGroup));

	// Filter out groups that cannot hold the item of the given type,
	// and the group that holds the item, if the item is parented.
	auto it = groups.begin();
	while(it != groups.end())
	{
		nap::IGroup* group = static_cast<nap::IGroup*>(*it);
		if (group == mObject || group == mCurrentGroup || !mObject->get_type().is_derived_from(group->getMemberType()))
		{
			it = groups.erase(it);
			continue;
		}
		it++;
	}

	// Let the user select the group
	auto parent_widget = AppContext::get().getMainWindow();
	auto selected_group = showObjectSelector(parent_widget, groups);

	// Operation canceled
	if (selected_group == nullptr)
		return;

	// Create origin path
	PropertyPath current_path = {};
	if (mCurrentGroup != nullptr)
		current_path = PropertyPath(*mCurrentGroup, mCurrentGroup->getMembersProperty(), *AppContext::get().getDocument());

	// Create target
	auto target_group = rtti_cast<nap::IGroup>(selected_group);
	PropertyPath target_path(*target_group, target_group->getMembersProperty(), *AppContext::get().getDocument());

	// Move
	AppContext::get().executeCommand(new GroupReparentCommand(*mObject, current_path, target_path));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::MoveGroupAction::MoveGroupAction(nap::IGroup& group, nap::IGroup* parentGroup) :
	Action("Move to Group...", QRC_ICONS_CHANGE), mGroup(&group), mParentGroup(parentGroup)
{ }


void napkin::MoveGroupAction::perform()
{
	// Get all groups of the same type
	auto groups = AppContext::get().getDocument()->getObjects(mGroup->get_type());

	// Moving the group to a child is not allowed
	PropertyPath group_path(*mGroup, mGroup->getChildrenProperty(), *AppContext::get().getDocument());
	auto sub_groups = group_path.getChildren(IterFlag::Resursive | IterFlag::FollowEmbeddedPointers);

	// Filter out groups that are not eligible. 
	// This includes child groups, itself and the current parent
	auto it = groups.begin();
	while (it != groups.end())
	{
		// Skip self and parent
		nap::IGroup* group = static_cast<nap::IGroup*>(*it);
		if (group == mParentGroup || group == mGroup)
		{
			it = groups.erase(it);
			continue;
		}

		// Check if the group is a child of the group to move
		PropertyPath current_path(*group, *AppContext::get().getDocument());
		auto child_it = std::find_if(sub_groups.begin(), sub_groups.end(), [&](const PropertyPath& sub_path)
			{
				return nap::utility::contains(sub_path.toString(), current_path.toString());
			});

		// If so, remove as option
		if (child_it != sub_groups.end())
		{
			it = groups.erase(it);
			continue;
		}

		it++;
	}

	// Let the user select the group
	auto parent_widget = AppContext::get().getMainWindow();
	nap::rtti::Object* selected_group = showObjectSelector(parent_widget, groups);

	// Operation canceled
	if (selected_group == nullptr)
		return;

	// Create origin path
	PropertyPath current_path = {};
	if (mParentGroup != nullptr)
		current_path = PropertyPath(*mParentGroup, mParentGroup->getChildrenProperty(), *AppContext::get().getDocument());

	// Create target path
	auto target_group = rtti_cast<nap::IGroup>(selected_group);
	PropertyPath target_path(*target_group, target_group->getChildrenProperty(), *AppContext::get().getDocument());

	// Move
	auto new_group = rtti_cast<nap::IGroup>(selected_group);
	AppContext::get().executeCommand(new GroupReparentCommand(*mGroup, current_path, target_path));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddExistingResourceToGroupAction::AddExistingResourceToGroupAction(nap::IGroup& group) :
	Action("Add Resource...", QRC_ICONS_ADD), mGroup(&group)
{ }


void AddExistingResourceToGroupAction::perform()
{
	// We know the group, find a resource to add
	PropertyPath members_path(*mGroup, mGroup->getMembersProperty(), *AppContext::get().getDocument());

	// Select type to add
	auto base_type = members_path.getArrayElementType();

	// Get objects to select from
	auto objects = topLevelObjects(AppContext::get().getDocument()->getObjectPointers());

	std::vector<nap::rtti::Object*> object_selection;
	object_selection.reserve(objects.size());
	for (const auto& object : objects)
	{
		nap::rtti::TypeInfo obj_type = object->get_type();
		if (obj_type.is_derived_from(base_type) &&
			!obj_type.is_derived_from(RTTI_OF(nap::Entity)) &&
			!obj_type.is_derived_from(RTTI_OF(nap::Component)) &&
			!obj_type.is_derived_from(RTTI_OF(nap::IGroup)))
		{
			object_selection.emplace_back(object);
		}
	}

	// Get object to add
	auto parent_widget = AppContext::get().getMainWindow();
	nap::rtti::Object* selected_object = showObjectSelector(parent_widget, object_selection);
	if (selected_object != nullptr)
	{
		// Move selected object from root to group
		AppContext::get().executeCommand(new GroupReparentCommand(*selected_object, {}, members_path));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::RemoveResourceFromGroupAction::RemoveResourceFromGroupAction(nap::IGroup& group, nap::rtti::Object& resource) :
	Action(nap::utility::stringFormat("Remove From '%s'", group.mID.c_str()).c_str(), QRC_ICONS_REMOVE),
	mGroup(&group), mObject(&resource)
{ }


void napkin::RemoveResourceFromGroupAction::perform()
{
	// Remove from group
	PropertyPath members_path(*mGroup, mGroup->getMembersProperty(), *AppContext::get().getDocument());
	AppContext::get().executeCommand(new GroupReparentCommand(*mObject, members_path, {}));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::RemoveGroupFromGroupAction::RemoveGroupFromGroupAction(nap::IGroup& group, nap::rtti::Object& resource) :
	Action(nap::utility::stringFormat("Remove From '%s'", group.mID.c_str()).c_str(), QRC_ICONS_REMOVE),
	mGroup(&group), mObject(&resource)
{ }


void napkin::RemoveGroupFromGroupAction::perform()
{
	// Remove from group
	PropertyPath group_path(*mGroup, mGroup->getChildrenProperty(), *AppContext::get().getDocument());
	AppContext::get().executeCommand(new GroupReparentCommand(*mObject, group_path, {}));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CreateEntityAction::CreateEntityAction() : Action("Create Entity", QRC_ICONS_ENTITY)
{ }


void CreateEntityAction::perform()
{
	AppContext::get().executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity), nullptr));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddChildEntityAction::AddChildEntityAction(nap::Entity& entity) :
	Action("Add Child Entity...", QRC_ICONS_ADD), mEntity(&entity)
{ }


void AddChildEntityAction::perform()
{
	auto doc = AppContext::get().getDocument();

	std::vector<nap::rtti::Object*> filteredEntities;
	for (auto e : doc->getObjects<nap::Entity>())
	{
		// Omit self and entities that have self as a child
		if (e == mEntity || doc->hasChild(*e, *mEntity, true))
			continue;
		filteredEntities.emplace_back(e);
	}

	auto parentWidget = AppContext::get().getMainWindow();
	auto child = rtti_cast<nap::Entity>(napkin::showObjectSelector(parentWidget, filteredEntities));
	if (!child)
		return;

	AppContext::get().executeCommand(new AddChildEntityCommand(*mEntity, *child));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddComponentAction::AddComponentAction(nap::Entity& entity) :
	Action("Add Component...", QRC_ICONS_ADD), mEntity(&entity)
{ }


void AddComponentAction::perform()
{
	auto parent = AppContext::get().getMainWindow();
	auto comptype = napkin::showTypeSelector(parent, [](auto t)
	{
		return t.is_derived_from(RTTI_OF(nap::Component));
	});

	if (comptype.is_valid())
		AppContext::get().executeCommand(new AddComponentCommand(*mEntity, comptype));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteObjectAction::DeleteObjectAction(nap::rtti::Object& object) :
	Action(nap::utility::stringFormat("Delete '%s'", object.mID.c_str()).c_str(), QRC_ICONS_DELETE),
	mObject(object)
{ }


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

napkin::DeleteGroupAction::DeleteGroupAction(nap::IGroup& group) :
	Action(nap::utility::stringFormat("Delete '%s'", group.mID.c_str()).c_str(), QRC_ICONS_DELETE),
	mGroup(group)
{ }


static void getObjectPointers(nap::IGroup& group, QList<PropertyPath>& outPointers)
{
	// Get members
	PropertyPath mem_property(group, group.getMembersProperty(), *AppContext::get().getDocument());
	int mem_count = mem_property.getArrayLength();
	for (int i = 0; i < mem_count; i++)
	{
		// Get links to member
		auto array_el = mem_property.getArrayElement(i);
		auto* member = array_el.getPointee();
		assert(member != nullptr);
		outPointers.append(AppContext::get().getDocument()->getPointersTo(*member, false, true));
	}

	// Do the same for every child group
	PropertyPath chi_property(group, group.getChildrenProperty(), *AppContext::get().getDocument());
	int chi_count = chi_property.getArrayLength();
	for (int i = 0; i < chi_count; i++)
	{
		auto array_el = chi_property.getArrayElement(i);
		auto child = rtti_cast<nap::IGroup>(array_el.getPointee());
		assert(child != nullptr);
		getObjectPointers(*child, outPointers);
	}
}



void napkin::DeleteGroupAction::perform()
{
	QList<PropertyPath> pointers;
	getObjectPointers(mGroup, pointers);
	if (!pointers.empty())
	{
		QString message = "The following properties are still pointing to members in this group,\n"
			"your data might end up in a broken state.\n\n"
			"Do you want to delete anyway?";
		if (!showPropertyListConfirmDialog(parentWidget(), pointers, "Warning", message))
			return;
	}
	AppContext::get().executeCommand(new DeleteObjectCommand(mGroup));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemoveChildEntityAction::RemoveChildEntityAction(EntityItem& entityItem) :
	Action(nap::utility::stringFormat("Remove '%s'", entityItem.getEntity()->mID.c_str()).c_str(), QRC_ICONS_REMOVE),
	mEntityItem(&entityItem)
{ }


void RemoveChildEntityAction::perform()
{
	// TODO: Move into Command
	auto parentItem = qobject_cast<EntityItem*>(mEntityItem->parentItem());
	auto doc = AppContext::get().getDocument();
	auto index = parentItem->childIndex(*mEntityItem);
	assert(index >= 0);

	// Grab all component paths for later instance property removal
	QStringList componentPaths;
	nap::qt::traverse(*parentItem->model(), [&componentPaths](QStandardItem* item)
	{
		auto compItem = qobject_cast<ComponentItem*>(static_cast<RTTIItem*>(item));
		if (compItem)
		{
			componentPaths << QString::fromStdString(compItem->componentPath());
		}
		return true;
	}, mEntityItem->index());

	doc->removeChildEntity(*parentItem->getEntity(), index);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemovePathAction::RemovePathAction(const PropertyPath& path) :
	Action("Remove", QRC_ICONS_REMOVE), mPath(path)
{ }


void RemovePathAction::perform()
{
	AppContext::get().getDocument()->remove(mPath);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SetThemeAction::SetThemeAction(const QString& themeName) :
	Action(themeName.isEmpty() ? napkin::TXT_THEME_DEFAULT : themeName.toStdString().c_str(), nullptr),
	mTheme(themeName)
{
    setCheckable(true);
}

void SetThemeAction::perform()
{
    AppContext::get().getThemeManager().setTheme(mTheme);
}


napkin::NewServiceConfigAction::NewServiceConfigAction() : Action("New", QRC_ICONS_EDIT)
{ }


void napkin::NewServiceConfigAction::perform()
{
	if (AppContext::get().hasServiceConfig())
	{
		AppContext::get().getServiceConfig()->create();
	}
}


napkin::SaveServiceConfigAction::SaveServiceConfigAction() : Action("Save", QRC_ICONS_SAVE)
{ }


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

 
napkin::SaveServiceConfigurationAs::SaveServiceConfigurationAs() : Action("Save as...", QRC_ICONS_SAVE_AS)
{ }


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


napkin::OpenServiceConfigAction::OpenServiceConfigAction() : Action("Open...", QRC_ICONS_FILE)
{ }


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
	QString filename = napkinutils::getOpenFilename(nullptr, "Select NAP Config File", dir, JSON_CONFIG_FILTER);
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


napkin::SetAsDefaultServiceConfigAction::SetAsDefaultServiceConfigAction() : Action("Set as project default", QRC_ICONS_CHANGE)
{ }


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
