/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "document.h"
#include "naputils.h"

#include <QList>
#include <QtDebug>
#include <QStack>
#include <QUuid>

#include <nap/logger.h>
#include <mathutils.h>
#include <utility/fileutils.h>
#include <nap/group.h>
#include <nap/timer.h>
#include <entityptr.h>
#include <componentptr.h>

using namespace napkin;
using namespace nap::rtti;


static std::string createSimpleUUID()
{
	auto uuid = QUuid::createUuid().toString();
	// just take the last couple of characters
	int charCount = 8;
	auto shortuuid = uuid.mid(uuid.size() - 2 - charCount, charCount);
	return shortuuid.toStdString();
}


void splitNameIndex(const std::string& str, std::string& name, int& index)
{
	auto split = nap::utility::splitString(str, ':');
	if (split.size() == 2)
	{
		name = split[0];
		index = std::stoi(split[1]);
	}
	else
	{
		name = str;
		index = -1;
	}

}


Document::Document(nap::Core& core, const QString& filename, nap::rtti::OwnedObjectList&& objects)
	: QObject(), mCore(core), mCurrentFilename(filename)
{
	for (auto& obj : objects)
	{
		nap::rtti::Object* obj_ptr = obj.get();
		mObjects.emplace(std::make_pair(obj_ptr->mID, std::move(obj)));
	}
}


nap::Entity* Document::getParent(const nap::Entity& child) const
{
	for (const auto& obj : mObjects)
	{
		if (!obj.second->get_type().is_derived_from<nap::Entity>())
			continue;

		auto parent = rtti_cast<nap::Entity>(obj.second.get());
		auto it = std::find_if(parent->mChildren.begin(), parent->mChildren.end(), [&child](const auto& child_entity) -> bool
			{
				return &child == child_entity.get();
			});

		// Child is part of parent
		if (it != parent->mChildren.end())
			return parent;
	}
	return nullptr;
}


bool Document::hasChild(const nap::Entity& parentEntity, const nap::Entity& childEntity, bool recursive) const
{
	for (auto child : parentEntity.mChildren)
	{
		if (child == &childEntity)
			return true;
		if (recursive && hasChild(*child, childEntity, true))
			return true;
	}
	return false;
}


nap::Entity* Document::getOwner(const nap::Component& component) const
{
	for (const auto& obj : mObjects)
	{
		if (!obj.second->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* owner = static_cast<nap::Entity*>(obj.second.get());
		auto filter = [&component](const auto& comp) -> bool
		{
			return &component == comp.get();
		};

		if (std::find_if(owner->mComponents.begin(), owner->mComponents.end(), filter) != owner->mComponents.end())
			return owner;
	}
	return nullptr;
}


nap::IGroup* napkin::Document::getOwner(const nap::IGroup& group, int& outIndex) const
{
	PropertyPath group_children_path = {};
	for(const auto& obj : mObjects)
	{
		if(!obj.second->get_type().is_derived_from(group.get_type()))
			continue;

		if(obj.second.get() == &group)
			continue;

		// Resolve child group property against current object
		auto obj_group = static_cast<nap::IGroup*>(obj.second.get());
		auto property_path = Path::fromString(obj_group->childrenPropertyName());
		ResolvedPath resolved_path;
		property_path.resolve(obj_group, resolved_path);
		assert(resolved_path.isValid());

		// Get array value
		Variant value = resolved_path.getValue();
		assert(value.is_valid() && value.is_array());
		VariantArray view = value.create_array_view();
		assert(view.is_valid());

		// Check if the group is a child of this object
		for (int i = 0; i < view.get_size(); i++)
		{
			Variant child = view.get_value(i);
			assert(child.get_type().is_wrapper());
			auto child_obj = child.extract_wrapped_value().get_value<nap::IGroup*>();
			if (child_obj == &group)
			{
				outIndex = i;
				return obj_group;
			}
		}
	}

	outIndex = -1;
	return nullptr;
}


nap::IGroup* napkin::Document::getGroup(const nap::rtti::Object& object, int& outIndex) const
{
	const auto& objects = getObjects();
	PropertyPath group_children_path = {};
	for (const auto& obj : mObjects)
	{
		if (!obj.second->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
			continue;

		// Resolve child group property against current object
		nap::IGroup* obj_group = static_cast<nap::IGroup*>(obj.second.get());
		auto property_path = Path::fromString(obj_group->membersPropertyName());
		ResolvedPath resolved_path;
		property_path.resolve(obj.second.get(), resolved_path);
		assert(resolved_path.isValid());

		// Get array value
		Variant value = resolved_path.getValue();
		assert(value.is_valid() && value.is_array());
		VariantArray view = value.create_array_view();
		assert(view.is_valid());

		// Check if the group can hold the given type
		auto element_type = view.get_rank_type(1).get_wrapped_type();
		if (!object.get_type().is_derived_from(element_type))
			continue;

		// Check if the group is a child of this object
		for (int i = 0; i < view.get_size(); i++)
		{
			Variant child = view.get_value(i);
			assert(child.get_type().is_wrapper());
			auto child_obj = child.extract_wrapped_value().get_value<nap::IGroup*>();
			if (child_obj == &object)
			{
				outIndex = i;
				return static_cast<nap::IGroup*>(obj.second.get());
			}
		}
	}

	outIndex = -1;
	return nullptr;
}


void napkin::Document::patchLinks(const std::string& oldID, const std::string& newID)
{
	// Iterate over the properties of a component or component ptr override.
	// Find component and entity pointers that reference the old object and patch accordingly.
	auto objects = getObjects({ RTTI_OF(nap::Component), RTTI_OF(nap::ComponentPtrInstancePropertyValue), RTTI_OF(nap::Scene) });
	for (auto& object : objects)
	{
		// Recursively iterate over properties and patch paths
		auto props = object->get_type().get_properties();
		nap::rtti::Path prop_path;
		for (auto& ptr_prop : props)
		{
			prop_path.pushAttribute(ptr_prop.get_name().data());
			patchLinks(object, oldID, newID, prop_path);
			prop_path.popBack();
		}
	}
}


nap::rtti::Object* napkin::Document::duplicateObject(const nap::rtti::Object& src, const PropertyPath& parent)
{
	// Duplicate and add object
	auto* parent_obj = parent.getObject();
	auto* duplicate = duplicateObject(src, parent_obj); assert(duplicate != nullptr);

	// Add duplicate to parent
	if (duplicate != nullptr && parent_obj != nullptr)
	{
		// Resolve and get array
		auto resolved = parent.resolve(); assert(resolved.isValid());
		auto value = resolved.getValue(); assert(value.is_array());
		auto array_view = value.create_array_view();

		// Find source index
		int index = -1;
		for (auto i = 0; i < array_view.get_size(); i++)
		{
			if (array_view.get_value_as_ref(i) == &src)
			{
				index = arrayAddExistingObject(parent, duplicate, i+1); 
				break;
			}
		}

		// Assert when source missing from parent property (mismatch)
		NAP_ASSERT_MSG(index >= 0, nap::utility::stringFormat("Source '%s' not found in parent '%s'",
			src.mID.c_str(), parent.toString().c_str()).c_str());
	}
	return duplicate;
}


void napkin::Document::patchLinks(nap::rtti::Object* object, const std::string& oldID, const std::string& newID, nap::rtti::Path& propPath)
{
	// Resolve path
	nap::rtti::ResolvedPath resolved_path;
	propPath.resolve(object, resolved_path);

	// Continue searching if property not of type component or entity ptr
	auto prop_type = resolved_path.getType();
	if (!prop_type.is_derived_from(RTTI_OF(nap::ComponentPtrBase)) &&
		!prop_type.is_derived_from(RTTI_OF(nap::EntityPtr)))
	{
		// Recursively iterate over array elements
		if (resolved_path.getType().is_array())
		{
			auto array_value = resolved_path.getValue();
			auto array_view = array_value.create_array_view();
			for (auto i = 0; i < array_view.get_size(); i++)
			{
				propPath.pushArrayElement(i);
				patchLinks(object, oldID, newID, propPath);
				propPath.popBack();
			}
		}

		// Recursively iterate over nested components
		auto nested_properties = resolved_path.getType().get_properties();
		for (const auto& nested_prop : nested_properties)
		{
			propPath.pushAttribute(nested_prop.get_name().data());
			patchLinks(object, oldID, newID, propPath);
			propPath.popBack();
		}
		return;
	}

	// Get to string (path) method
	auto pointer_object = resolved_path.getValue();
	rttr::method string_method = nap::rtti::findMethodRecursive(prop_type, nap::rtti::method::toString);
	assert(string_method.is_valid());

	// Get path and check for ID inclusion - exclude partial names
	auto object_path = string_method.invoke(pointer_object).to_string();
	auto index = object_path.find(oldID);
	while (index != std::string::npos)
	{
		auto end_index = index + oldID.size();
		if (end_index >= object_path.size() ||
			object_path[end_index] == '/' ||
			object_path[end_index] == ':')
			break;

		index = object_path.find(oldID, index + 1);
	}

	// No match
	if (index == std::string::npos)
		return;

	// Update path and find target object
	object_path.replace(index, oldID.size(), newID);
	size_t obj_pos = object_path.find_last_of('/');
	nap::rtti::Object* target = getObject(obj_pos != std::string::npos ?
		object_path.substr(obj_pos + 1) : object_path);

	// Target doesn't exist
	if (target == nullptr)
	{
		assert(false);
		return;
	}

	// Create and assign new path
	rttr::method assign_method = nap::rtti::findMethodRecursive(prop_type, nap::rtti::method::assign);
	assert(assign_method.is_valid());
	assign_method.invoke(pointer_object, object_path, target);

	// Set as new property value
	if (!resolved_path.setValue(pointer_object))
	{
		std::string msg = nap::utility::stringFormat("Unable to update: %s", propPath.toString().c_str());
		NAP_ASSERT_MSG(false, msg.c_str());
	}
}


const std::string& Document::setObjectName(nap::rtti::Object& object, const std::string& name, bool appenUUID)
{
	if (name.empty())
		return object.mID;

	// Get name
	auto new_name = getUniqueName(name, object, appenUUID);
	if (new_name == object.mID)
		return object.mID;

	// Set name
	auto old_name = object.mID;
	object.mID = new_name;

	// Release item 
	auto it = mObjects.find(old_name);
	assert(it != mObjects.end());
	auto released_obj = it->second.release();

	// Erase old entry, add new entry
	mObjects.erase(it);
	mObjects.emplace(std::make_pair(new_name, std::unique_ptr<nap::rtti::Object>(released_obj)));

	// Patch entity and component ptr links.
	// This occurs when the name of an entity or a component changes, which invalidates existing links to those objects.
	if (object.get_type().is_derived_from(RTTI_OF(nap::Entity)) ||
		object.get_type().is_derived_from(RTTI_OF(nap::Component)))
	{
		patchLinks(old_name, new_name);
	}

	// Notify listeners
	PropertyPath path(object, Path::fromString(nap::rtti::sIDPropertyName), *this);
	assert(path.isValid());
	propertyValueChanged(path);
	objectRenamed(object, old_name, new_name);

	// New name
	return object.mID;
}


nap::Component* Document::addComponent(nap::Entity& entity, rttr::type type)
{
	auto& factory = mCore.getResourceManager()->getFactory();
	bool canCreate = factory.canCreate(type);

	if (!canCreate)
	{
		nap::Logger::fatal("Cannot create instance of '%s'", type.get_name().data());
	}

	assert(canCreate);
	assert(type.is_derived_from<nap::Component>());

	// Create and add component
	nap::rtti::Variant compVariant = factory.create(type);
	auto comp = compVariant.get_value<nap::Component*>();
	comp->mID = getUniqueName(type.get_name().data(), *comp, true);
	auto it = mObjects.emplace(std::make_pair(comp->mID, comp));
	assert(it.second);
	entity.mComponents.emplace_back(comp);

	// Notify others
	auto comp_path = PropertyPath(entity.mID, nap::Entity::componentsPropertyName(), *this);
	propertyValueChanged(comp_path);
	propertyChildInserted(comp_path, entity.mComponents.size()-1);
	return comp;
}


nap::rtti::Object* Document::addObject(rttr::type type, nap::rtti::Object* parent, const std::string& name)
{
	// Make sure it's an rtti Object
	Factory& factory = mCore.getResourceManager()->getFactory();
	if (!type.is_derived_from<Object>())
	{
		nap::Logger::error("Cannot create object of type: %s, type does not derive from",
				type.get_name().data(), RTTI_OF(Object).get_name().data());
		return nullptr;
	}

	// Ensure we can create the object using the factory
	if (!factory.canCreate(type))
	{
		nap::Logger::error("Cannot create object of type: %s", type.get_name().data());
		return nullptr;
	}

	// Get initial name
	std::string base_name = name.empty() ? friendlyTypeName(type) : name;
	nap::rtti::Object* obj = factory.create(type);
	assert(obj != nullptr);
	obj->mID = getUniqueName(base_name, *obj, true);

	// Add to managed object list
	mObjects.emplace(std::make_pair(obj->mID, obj));

	// Notify listeners
	objectAdded(obj, parent);
	return obj;
}


nap::Entity& Document::addEntity(nap::Entity* parent, const std::string& name)
{
	auto e = addObject<nap::Entity>(parent, name);
	assert(e != nullptr);
	return *e;
}


std::string Document::getUniqueName(const std::string& suggestedName, const nap::rtti::Object& object, bool useUUID)
{
	// Construct name
	std::string newName = useUUID ?
		nap::utility::stringFormat("%s_%s", suggestedName.c_str(), createSimpleUUID().c_str()) :
		suggestedName;

	// Ensure name is unique
	auto ex_obj = getObject(newName); int i = 2;
	while (ex_obj != nullptr && ex_obj != &object)
	{
		newName = useUUID ?
			nap::utility::stringFormat("%s_%s", suggestedName.c_str(), createSimpleUUID().c_str()) :
			nap::utility::stringFormat("%s_%d", suggestedName.c_str(), i++);
		ex_obj = getObject(newName);
	}
	return newName;
}


Object* Document::getObject(const std::string& name)
{
	auto it = mObjects.find(name);
	return it != mObjects.end() ? it->second.get() : nullptr;
}


Object* Document::getObject(const std::string& name, const rttr::type& type)
{
	auto object = getObject(name);
	return object != nullptr && object->get_type().is_derived_from(type) ? object : nullptr;
}


void Document::removeObject(Object& object)
{
	// Emit signal first so observers can act before the change
	removingObject(&object);

	// Remove instance properties for this object
	if (!object.get_type().is_derived_from<nap::InstancePropertyValue>())
		removeInstanceProperties(object);

	// Delete embedded objects under the given owner.
	nap::rtti::ObjectList embedded_objs = getEmbeddedObjects(object);
	for (auto embedded_obj : embedded_objs)
		removeObject(*embedded_obj);

	// Remove this object from every group as a member
	int child_index = -1;
	auto parent_group = getGroup(object, child_index);
	if (parent_group != nullptr)
	{
		groupRemoveElement(*parent_group, parent_group->getMembersProperty(), child_index);
	}

	// Handle specific objects.
	// By default, all items that reference the deleted item will have their handle set to null.
	// For some items we do a bit of cleanup, to ensure their children remain organized.
	// TODO: Check if the item that references this item is in an array and a regular pointer
	// If it is, we can erase that index. Note that that will be slower than handling specific cases.
	if (object.get_type().is_derived_from<nap::Entity>())
	{
		// Remove from every scene
		auto entity = static_cast<nap::Entity*>(&object);
		auto scenes = getObjects(RTTI_OF(nap::Scene));
		for (auto& scene : scenes)
			removeEntityFromScene(static_cast<nap::Scene&>(*scene), *entity);

		// Remove (every!) reference to the entity
		// Entities can have multiple references to the same child entity
		auto parent_entity = getParent(*entity);
		if (parent_entity != nullptr)
		{
			for (auto it = parent_entity->mChildren.begin(); it != parent_entity->mChildren.end(); )
			{
				if (it->get() == entity)
				{
					it = parent_entity->mChildren.erase(it);
					continue;
				}
				it++;
			}
		}
	}

	// Remove the component from entities that reference it
	else if (object.get_type().is_derived_from(RTTI_OF(nap::Component)))
	{
		auto component = static_cast<nap::Component*>(&object);
		auto owner = getOwner(*component);
		if (owner != nullptr)
		{
			auto it = std::remove(owner->mComponents.begin(), owner->mComponents.end(), &object);
			owner->mComponents.erase(it, owner->mComponents.end());
		}
	}

	// Remove the group from parent groups, if any
	else if (object.get_type().is_derived_from(RTTI_OF(nap::IGroup)))
	{
		// Get parent group
		auto group = static_cast<nap::IGroup*>(&object);
		int child_index = -1;
		auto owner =  getOwner(*group, child_index);

		// Remove from parent if found
		if (owner != nullptr)
			groupRemoveElement(*owner, owner->getChildrenProperty(), child_index);
	}

	// References to object have been removed, 
	auto found_it = std::find_if(mObjects.begin(), mObjects.end(), [&](const auto& obj) 
		{
			return obj.second.get() == &object;
		});
	assert(found_it != mObjects.end());

	// Erase
	auto released_obj = found_it->second.release();
	mObjects.erase(found_it);

	// Notify listeners this object has been removed
	objectRemoved(released_obj);
	delete released_obj;
}


void Document::removeObject(const std::string& name)
{
	auto object = getObject(name);
	if (object != nullptr)
	{
		removeObject(*object);
	}
}


void Document::removeInstanceProperties(nap::rtti::Object& object)
{
	for (auto scene : getObjects<nap::Scene>())
	{
		removeInstanceProperties(*scene, object);
	}
}


void Document::removeInstanceProperties(nap::Scene& scene, nap::rtti::Object& object)
{
	// Get components based on object
	auto allComponents = getComponentsRecursive(object);

	// If there's nothing to compare against return
	if (allComponents.isEmpty())
		return;

	// Otherwise iterate over all entities in the scene and see if it contains the given component
	for (auto& rootEntity : scene.mEntities)
	{
		auto& props = rootEntity.mInstanceProperties;
		for (int i = static_cast<int>(props.size() - 1); i >= 0; --i)
		{
			if (allComponents.contains(props[i].mTargetComponent.get()))
			{
				props.erase(props.begin() + i);
			}
		}
	}
}


void Document::removeInstanceProperties(PropertyPath path)
{
	// Remove instanceproperties for a parentEntity, childEntity combo
	// Path is expected to be a child of another entity
	auto parent = path.getParent();
	assert(parent.isValid());
	assert(parent.getType().is_derived_from<nap::Entity>());
	auto parentEntity = rtti_cast<nap::Entity>(parent.getObject());
	assert(parentEntity);
	auto instIndex = path.getInstanceChildEntityIndex();

	auto parentID = parentEntity->mID;
	auto entityID = path.getObject()->mID;

	QList<nap::Scene*> changedScenes;
	for (auto scene : getObjects<nap::Scene>())
	{
		for (auto& rootEntity : scene->mEntities)
		{
			auto& props = rootEntity.mInstanceProperties;
			for (int i=0; i < props.size();)
			{
				auto& prop = props.at(i);
				auto compPathStr = prop.mTargetComponent.toString();
				auto compID = nap::utility::getFileName(compPathStr);
				auto entPath = nap::utility::getFileDir(compPathStr);
				auto _entityID = nap::utility::getFileName(entPath);
				auto _parentID = nap::utility::getFileName(nap::utility::getFileDir(entPath));

				auto split = nap::utility::splitString(_entityID, ':');
				int _instIndex;
				splitNameIndex(_entityID, _entityID, _instIndex);

				int parentIndex;
				splitNameIndex(_parentID, _parentID, parentIndex);

				if (_parentID == ".")
					_parentID = rootEntity.mEntity->mID;

				// If entity and parentEntity match, we can delete it
				if (_parentID == parentID && _entityID == entityID)
				{
					if (_instIndex == instIndex)
					{
						// same index, remove
						props.erase(props.begin() + i);

						if (!changedScenes.contains(scene))
							changedScenes.append(scene);

						continue;
					}
					else if (_instIndex > instIndex)
					{
						// shift index and reconstruct path
						auto parentPath = nap::utility::getFileDir(entPath);
						auto newIndex = _instIndex - 1;
						auto newPath = nap::utility::stringFormat("%s/%s:%d/%s",
								parentPath.c_str(), _entityID.c_str(), newIndex, compID.c_str());
						prop.mTargetComponent.assign(newPath, prop.mTargetComponent.get());

						if (!changedScenes.contains(scene))
							changedScenes.append(scene);
					}
				}
				++i;
			}
		}
	}
}


QList<nap::Component*> Document::getComponentsRecursive(nap::rtti::Object& object)
{
	// List of components to return
	QList<nap::Component*> components;

	// Check if the incoming object is a component
	nap::Component* component = rtti_cast<nap::Component>(&object);
	if (component != nullptr)
	{
		components.append(component);
		return components;
	}

	nap::Entity* entity = rtti_cast<nap::Entity>(&object);
	if (entity != nullptr)
	{
		recurseChildren(*entity, [&components](nap::Entity& child)
		{
			for (auto comp : child.mComponents)
				components << comp.get();
		});
		return components;
	}

	// No component or entity
	return components;
}


void Document::recurseChildren(nap::Entity& entity, std::function<void(nap::Entity& child)> visitor)
{
	QStack<nap::Entity*> stack;
	stack.push(&entity);

	while (!stack.empty())
	{
		auto current = stack.pop();
		visitor(*current);

		for (auto child : current->mChildren)
			stack.push(child.get());
	}
}


size_t Document::addEntityToScene(nap::Scene& scene, nap::Entity& entity)
{
	nap::RootEntity rootEntity;
	rootEntity.mEntity = &entity;
	size_t index = scene.mEntities.size();
	scene.mEntities.emplace_back(rootEntity);
	propertyValueChanged(PropertyPath(scene, nap::rtti::Path::fromString("Entities"), *this));
	return index;
}


size_t Document::addChildEntity(nap::Entity& parent, nap::Entity& child)
{
	auto index = parent.mChildren.size();
	parent.mChildren.emplace_back(&child);
	childEntityAdded(&child, &parent);
	return index;
}


void Document::removeChildEntity(nap::Entity& parent, size_t childIndex)
{
	// WARNING: This will NOT take care of removing and patching up instance properties
	// TODO: Remove associated instance properties
	auto obj = parent.mChildren[childIndex];
	parent.mChildren.erase(parent.mChildren.begin() + childIndex);

	PropertyPath childrenProp(parent, nap::rtti::Path::fromString("Children"), *this);
	assert(childrenProp.isValid());
	propertyValueChanged(childrenProp);
}


void Document::remove(const PropertyPath& path)
{
	// TODO: Nuke this from orbit -> use regular array object removal
	auto parent = path.getParent();
	if (parent.getType().is_derived_from<nap::Entity>() && path.getType().is_derived_from<nap::Entity>())
	{
		// Removing child Entity from parent Entity
		auto parentEntity = rtti_cast<nap::Entity>(parent.getObject());
		assert(parentEntity);
		auto childEntity = rtti_cast<nap::Entity>(path.getObject());
		assert(childEntity);

		// Remove all instance properties that refer to this Entity:0 under ParentEntity
		auto realIndex = path.getEntityIndex();
		removeInstanceProperties(path);
		removeChildEntity(*parentEntity, realIndex);
		return;
	}

	// TODO: Nuke this from orbit -> use regular array object removal
	if (parent.getType().is_derived_from<nap::Scene>() && path.getType().is_derived_from<nap::Entity>())
	{
		auto entity = rtti_cast<nap::Entity>(path.getObject());
		auto scene = rtti_cast<nap::Scene>(parent.getObject());
		int idx = 0;
		int pathidx = path.getInstanceChildEntityIndex();
		for (int i=0; i<scene->mEntities.size(); i++)
		{
			auto& rootEntity = scene->mEntities[i];
			if (rootEntity.mEntity->mID == entity->mID) {
				if (idx == pathidx)
				{
					scene->mEntities.erase(scene->mEntities.begin() + i);
					propertyValueChanged(PropertyPath(*scene, nap::rtti::Path::fromString("Entities"), *this));
					return;
				}
				++idx;
			}
		}
	}
}


void Document::removeEntityFromScene(nap::Scene& scene, nap::Entity& entity)
{
	// Remove instance properties associated with this entity
	removeInstanceProperties(scene, entity);

	// Remove (every!) child entity from the scene
	bool changed = false;
	for (auto it = scene.mEntities.begin(); it != scene.mEntities.end(); )
	{
		if (it->mEntity == &entity)
		{
			it = scene.mEntities.erase(it);
			changed = true;
			continue;
		}
		it++;
	}
}


void Document::removeEntityFromScene(nap::Scene& scene, size_t index)
{
	removeInstanceProperties(scene, *scene.mEntities[index].mEntity);
	scene.mEntities.erase(scene.mEntities.begin() + index);
}


int Document::arrayAddValue(const PropertyPath& path)
{
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_array());
	VariantArray array_view = array.create_array_view();
	assert(array_view.is_dynamic());

	const TypeInfo element_type = array_view.get_rank_type(1);
	const TypeInfo wrapped_type = element_type.is_wrapper() ? element_type.get_wrapped_type() : element_type;
	if (!wrapped_type.can_create_instance())
	{
		nap::Logger::error("Cannot create instance of type '%s'", wrapped_type.get_name().data());
		return -1;
	}

	// HACK: In the case of a vector<string>, rttr::type::create() gives us a shared_ptr to a string,
	// hence, rttr::variant_array_view::insert_value will fail because it expects just a string.
	Variant new_value;
	new_value = wrapped_type == RTTI_OF(std::string) ? std::string() : wrapped_type.create();
	assert(new_value.is_valid());

	size_t index = array_view.get_size();
	bool inserted = array_view.insert_value(index, new_value);
	assert(inserted);
	resolved_path.setValue(array);

	propertyValueChanged(path);
	propertyChildInserted(path, index);

	return index;
}


size_t Document::arrayAddExistingObject(const PropertyPath& path, Object* object, size_t index)
{
	// Resolve path and fetch array
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());
	Variant array = resolved_path.getValue();
	assert(array.is_valid());
	assert(array.is_array());
	VariantArray array_view = array.create_array_view();
	assert(array_view.is_dynamic());
	assert(array_view.is_valid());

	// Convert the object to the wrapped type
	const TypeInfo array_type = array_view.get_rank_type(array_view.get_rank());
	const TypeInfo wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;

	// Insert item into array
	Variant new_item = object;
	bool convert_ok = new_item.convert(wrapped_type);
	assert(convert_ok);
	assert(index <= array_view.get_size());
	bool inserted = array_view.insert_value(index, new_item);
	assert(inserted);

	// If the item is a component or entity ptr -> assign relative object path
	if (array_type.is_derived_from(RTTI_OF(nap::ComponentPtrBase)) ||
		array_type.is_derived_from(RTTI_OF(nap::EntityPtr)))
	{
		// Get assign method
		Variant new_ptr = array_view.get_value_as_ref(index);
		rttr::method assign_method = nap::rtti::findMethodRecursive(array_type, nap::rtti::method::assign);
		assert(assign_method.is_valid());
		auto obj_path = relativeObjectPath(*path.getObject(), *object);

		// Assign
		array_type.is_derived_from(RTTI_OF(nap::ComponentPtrBase)) ?
			assign_method.invoke(new_ptr.get_wrapped_value<nap::ComponentPtrBase>(), obj_path, object) :
			assign_method.invoke(new_ptr.get_wrapped_value<nap::EntityPtr>(), obj_path, object);
	}

	// Set updated array
	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	// Notify listeners
	propertyValueChanged(path);
	propertyChildInserted(path, index);

	return index;
}


int Document::arrayAddNewObject(const PropertyPath& path, const TypeInfo& type, size_t index)
{
	// Resolve path
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	// Get parent
	PropertyPath parent_path = path.getParent();
	assert(parent_path.isValid());
	auto* parent_object = parent_path.getObject();

	// Create object
	Object* new_object = addObject(type, parent_object);
	if (!new_object)
	{
		nap::Logger::error("Did not create object at: %s", path.toString().c_str());
		return 0;
	}

	// Insert into array
	Variant array = resolved_path.getValue();
	VariantArray array_view = array.create_array_view();
	assert(index <= array_view.get_size());
	bool inserted = array_view.insert_value(index, new_object);
	assert(inserted);

	// Set array after modification
	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	// Call listeners
	propertyValueChanged(path);
	propertyChildInserted(path, index);

	return index;
}


void Document::arrayRemoveElement(const PropertyPath& path, size_t index)
{
	// If embedded pointer, get pointee 
	auto elementPath = path.getArrayElement(index);
	nap::rtti::Object* pointee = nullptr;
	if (elementPath.isEmbeddedPointer())
		pointee = elementPath.getPointee();

	// Get array from path
	ResolvedPath resolved_path = path.resolve();
	Variant value = resolved_path.getValue();
	VariantArray array = value.create_array_view();
	assert(index < array.get_size());

	// Remove from array and update
	bool ok = array.remove_value(index);
	assert(ok);
	ok = resolved_path.setValue(value);
	assert(ok);

	// Notify listeners that the array changed
	propertyValueChanged(path);
	propertyChildRemoved(path, index);

	// Delete pointee if the resource is embedded
	if (pointee != nullptr)
	{
		removeObject(*pointee);
	}
}


void napkin::Document::groupRemoveElement(nap::IGroup& group, rttr::property arrayProperty, size_t index)
{
	// Resolve path to property
	PropertyPath path(group, arrayProperty, *this);
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant value = resolved_path.getValue();
	VariantArray array = value.create_array_view();
	assert(index < array.get_size());

	// Remove from array and update
	bool ok = array.remove_value(index); assert(ok);
	ok = resolved_path.setValue(value);  assert(ok);

	// Notify listeners that the array changed
	propertyValueChanged(path);
	propertyChildRemoved(path, index);
}


void napkin::Document::reparentObject(nap::rtti::Object& object, const PropertyPath& currentPath, const PropertyPath& newPath)
{
	// Notify users about re-parent operation
	objectReparenting(object, currentPath, newPath);

	// remove from current parent if it has one
	if (currentPath.isValid())
	{
		// Get index
		int resource_idx = -1; int length = currentPath.getArrayLength();
		for (int i = 0; i < length; i++)
		{
			auto el_path = currentPath.getArrayElement(i);
			if (el_path.getPointee() == &object)
			{
				resource_idx = i;
				break;
			}
		}
		assert(resource_idx >= 0);

		// Remove from array
		ResolvedPath resolved_path = currentPath.resolve();
		Variant array_value = resolved_path.getValue();
		VariantArray view = array_value.create_array_view();
		bool ok = view.remove_value(resource_idx);
		assert(ok);
		ok = resolved_path.setValue(array_value);
		assert(ok);

		// Notify listeners that the array changed
		propertyValueChanged(currentPath);
		propertyChildRemoved(currentPath, resource_idx);
	}

	// Move to new parent
	if (newPath.isValid())
	{
		// Add to existing array
		ResolvedPath resolved_path = newPath.resolve();
		Variant target_array_value = resolved_path.getValue();
		VariantArray view = target_array_value.create_array_view();
		int index = view.get_size();
		bool ok = view.insert_value(index, &object);
		assert(ok);
		ok = resolved_path.setValue(target_array_value);
		assert(ok);

		// Call listeners
		propertyValueChanged(newPath);
		propertyChildInserted(newPath, index);
	}

	// Notify users the re-parent operation succeeded
	objectReparented(object, currentPath, newPath);
}


nap::rtti::Object* Document::duplicateObject(const nap::rtti::Object& src, nap::rtti::Object* parent)
{
	// Make sure we can create the object
	Factory& factory = mCore.getResourceManager()->getFactory();
	if (!factory.canCreate(src.get_type()))
	{
		nap::Logger::error("Cannot create object of type: %s", src.get_type().get_name().data());
		return nullptr;
	}

	// Create the object and give it a new name
	nap::rtti::Object* target = factory.create(src.get_type());
	assert(target != nullptr && !src.mID.empty());
	target->mID = getUniqueName(src.mID, *target, true);

	// Add to managed object list
	mObjects.emplace(std::make_pair(target->mID, target));
	
	// Copy properties
	auto properties = src.get_type().get_properties();
	for (const auto& property : properties)
	{
		// Skip ID
		if(property.get_name() == nap::rtti::sIDPropertyName)
			continue;

		// Get value (by copy)
		nap::rtti::Variant src_value = property.get_value(src);
		 
		// Check if it's an embedded pointer or embedded pointer array ->
		// In that case we need to duplicate the embedded object and set that
		if (nap::rtti::hasFlag(property, EPropertyMetaData::Embedded))
		{
			// Regular embedded pointer -> duplicate and set
			if (!property.is_array())
			{
				assert(src_value.get_type().is_wrapper() && src_value.get_type().get_wrapped_type().is_pointer());
				auto variant = src_value.extract_wrapped_value();
				assert(variant.get_type().is_derived_from(RTTI_OF(nap::rtti::Object)));
				auto src_obj = variant.get_value<nap::rtti::Object*>();
				src_value = src_obj != nullptr ? duplicateObject(*src_obj, target) : nap::rtti::Variant();
			}
			else
			{
				// Iterate over array, duplicate entries and set
				auto array_view  = src_value.create_array_view();
				auto array_type =  array_view.get_rank_type(array_view.get_rank());
				assert(array_type.is_wrapper() && array_type.get_wrapped_type().is_pointer());
				for (auto i = 0; i < array_view.get_size(); i++)
				{
					// Fetch src object
					auto src_array_value = array_view.get_value(i);
					auto variant = src_array_value.extract_wrapped_value();
					assert(variant.get_type().is_derived_from(RTTI_OF(nap::rtti::Object)));
					auto src_array_obj = variant.get_value<nap::rtti::Object*>();

					// Duplicate & set
					nap::rtti::Object* obj_handle = src_array_obj != nullptr ? 
						duplicateObject(*src_array_obj, target) : nullptr;
					array_view.set_value(i, obj_handle);
				}
			}
		}

		// Copy value if available
		if (src_value.is_valid() && !property.set_value(target, src_value))
		{
			NAP_ASSERT_MSG(false, nap::utility::stringFormat("Failed to copy: %s",
				property.get_name().data()).c_str());
		}
	}

	// Notify listeners object has been added and return
	objectAdded(target, parent);
	return target;
}


void Document::arraySwapElement(const PropertyPath& path, size_t fromIndex, size_t toIndex)
{
	// Resolve property path
	ResolvedPath resolved_path = path.resolve();
	Variant array_value = resolved_path.getValue();
	VariantArray array = array_value.create_array_view();

	// Swap & Set
	assert(fromIndex < array.get_size());
	Variant fr_value = array.get_value(fromIndex);
	array.set_value(fromIndex, array.get_value(toIndex));
	assert(toIndex < array.get_size());
	array.set_value(toIndex, fr_value);
	bool ok = resolved_path.setValue(array_value); assert(ok);
	propertyValueChanged(path);
	arrayIndexSwapped(path, fromIndex, toIndex);
}


nap::rtti::Variant Document::arrayGetElement(const PropertyPath& path, size_t index) const
{
	ResolvedPath resolved_path = path.resolve();
	Variant array_value = resolved_path.getValue();
	VariantArray array = array_value.create_array_view();
	return array.get_value(index);
}


void Document::executeCommand(QUndoCommand* cmd)
{
	mUndoStack.push(cmd);
}


QList<PropertyPath> Document::getPointersTo(const nap::rtti::Object& targetObject, bool excludeArrays, bool excludeParent, bool excludeInstanceProperties)
{
	QList<PropertyPath> properties;
	for (const auto& it : mObjects)
	{
		nap::rtti::Object* source_obj = it.second.get();
		std::vector<nap::rtti::ObjectLink> links;
		findObjectLinks(*source_obj, links);
		for (const auto& link : links)
		{
			// Link is target
			assert(link.mSource == source_obj);
			if (link.mTarget != &targetObject)
				continue;

			// Construct path
			PropertyPath propPath(*source_obj, link.mSourcePath, *this);
			auto proppathstr = propPath.toString();
			assert(propPath.isPointer());

			if (excludeInstanceProperties && source_obj->get_type().is_derived_from<nap::Scene>())
				continue;

			if (excludeArrays && propPath.isArray())
				continue;

			if (propPath.getParent().getType().is_derived_from<nap::ComponentInstanceProperties>())
				continue;

			if (excludeParent)
			{
				if (source_obj->get_type().is_derived_from(RTTI_OF(nap::Entity)))
				{
					auto* entity = static_cast<nap::Entity*>(source_obj);
					if (std::find(entity->mChildren.begin(), entity->mChildren.end(), &targetObject) != entity->mChildren.end())
						continue;

					if (std::find(entity->mComponents.begin(), entity->mComponents.end(), &targetObject) !=
						entity->mComponents.end())
						continue;
				}
				else if (source_obj->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
				{
					// Check if item is part of group
					auto* group = static_cast<nap::IGroup*>(source_obj);
					PropertyPath array_path(*group, group->getMembersProperty(), *this);
					bool part_of_group = false;
					array_path.iterateChildren([&](const PropertyPath& path)
						{
							// Cancel iteration if found 
							part_of_group = path.getPointee() == &targetObject;
							return !part_of_group;
						}, 0);

					// Don't add groups that directly reference the item
					if (part_of_group)
						continue;
				}
			}

			// This property references the item
			properties << propPath;
		}
	}
	return properties;
}


QList<nap::RootEntity*> Document::getRootEntities(nap::Scene& scene, nap::rtti::Object& object)
{
	auto entity = rtti_cast<nap::Entity>(&object);
	if (entity == nullptr)
	{
		// must be component
		auto comp = rtti_cast<nap::Component>(&object);
		entity = getOwner(*comp);
	}
	assert(entity);

	QList<nap::RootEntity*> rootEntities;
	for (nap::RootEntity& rootEntity : scene.mEntities)
	{
		if (rootEntity.mEntity == &object)
			rootEntities.append(&rootEntity);
		else if (hasChild(*rootEntity.mEntity.get(), *entity, true))
			rootEntities.append(&rootEntity);
	}
	return rootEntities;
}


Document::~Document()
{
	mUndoStack.disconnect();
}


std::vector<nap::rtti::Object*> Document::getObjects(const nap::rtti::TypeInfo& type)
{
	std::vector<nap::rtti::Object*> result;
	for (auto& object : mObjects)
	{
		if (object.second->get_type().is_derived_from(type))
		{
			result.emplace_back(object.second.get());
		}
	}
	return result;
}


std::vector<nap::rtti::Object*> napkin::Document::getObjects(const std::vector<nap::rtti::TypeInfo>& types)
{
	std::vector<nap::rtti::Object*> result;
	for (auto& object : mObjects)
	{
		const auto& found_it = std::find_if(types.begin(), types.end(), [&object](const auto& type)
		{
			return object.second->get_type().is_derived_from(type);
		});

		if (found_it != types.end())
		{
			result.emplace_back(object.second.get());
		}
	}
	return result;
}


bool Document::isPointedToByEmbeddedPointer(const nap::rtti::Object& obj)
{
	return bool(getEmbeddedObjectOwner(obj));
}


nap::rtti::Object* Document::getEmbeddedObjectOwner(const nap::rtti::Object& obj)
{
	auto path = getEmbeddedObjectOwnerPath(obj);
	if (path.isValid())
		return path.getObject();
	return nullptr;
}


PropertyPath Document::getEmbeddedObjectOwnerPath(const nap::rtti::Object& obj)
{
	for (const auto& path : getPointersTo(obj, false, false))
	{
		if (path.isEmbeddedPointer())
			return path;
	}
	return {};
}


std::vector<nap::rtti::Object*> Document::getEmbeddedObjects(nap::rtti::Object& owner)
{
	// Iterate over child properties, down from owner
	// Add every embedded object if not null
	PropertyPath prop_path(owner, *this);
	std::vector<nap::rtti::Object*> embedded_objects;
	prop_path.iterateChildren([&](const PropertyPath& curent_path)
		{
			if (curent_path.isEmbeddedPointer() && !curent_path.isArray())
			{
				auto pointee = curent_path.getPointee();
				if (pointee != nullptr)
				{
					embedded_objects.emplace_back(pointee);
				}
			}
			return true;
		}, IterFlag::Resursive);
	return embedded_objects;
}


nap::Component* Document::getComponent(nap::Entity& entity, rttr::type componenttype)
{
	for (auto comp : entity.getComponents())
	{
		if (comp.get()->get_type().is_derived_from(componenttype))
			return comp.get();
	}
	return nullptr;
}


void Document::removeComponent(nap::Component& comp)
{
	auto owner = getOwner(comp);
	auto& comps = owner->mComponents;
	comps.erase(std::remove_if(comps.begin(), comps.end(), [&comp](const auto& objptr) {
		return objptr == &comp;
	}));
}


void Document::absoluteObjectPathList(const nap::rtti::Object& obj, std::deque<std::string>& result) const
{
	const nap::Entity* entity = nullptr;

	// If we got a component, push that first, then get the owning entity
	auto component = rtti_cast<const nap::Component>(&obj);
	if (component != nullptr) {
		result.push_front(component->mID);
		entity = getOwner(*component);
		assert(entity != nullptr);
	}

	// Entity is null, so we must not have gotten a component
	if (entity == nullptr)
		entity = rtti_cast<const nap::Entity>(&obj);

	assert(entity != nullptr); // Unsupported object type received.

	while (entity != nullptr)
	{
		result.push_front(entity->mID);
		entity = getParent(*entity);
	}
}


std::string Document::absoluteObjectPath(const nap::rtti::Object& obj) const
{
	std::deque<std::string> path;
	absoluteObjectPathList(obj, path);
	return "/" + nap::utility::joinString(path, "/");
}


size_t findCommonStartingElements(const std::deque<std::string>& a, const std::deque<std::string>& b)
{
	size_t i = 0;
	size_t len = std::min(a.size(), b.size());
	for (; i < len; i++)
	{
		if (a[i] != b[i])
			return i;
	}
	return i;
}


void Document::relativeObjectPathList(const nap::rtti::Object& origin, const nap::rtti::Object& target,
									  std::deque<std::string>& result) const
{

	// Grab the origin entity (ignore component if provided)
	auto originEntity = rtti_cast<const nap::Entity>(&origin);
	if (!originEntity) {
		auto originComponent = rtti_cast<const nap::Component>(&origin);
		assert(originComponent != nullptr);
		originEntity = getOwner(*originComponent);
	}
	assert(originEntity != nullptr);

	// Componentptrs and EntityPtrs have to be handled differently. Check for entity first
	if (auto targetEntity = rtti_cast<const nap::Entity>(&target))
	{
		result.emplace_back(targetEntity->mID);
		return;
	}

	// This implementation is based off the description in componentptr.h
	// Notes:
	// - Sibling component paths start with a single period, but other paths (parent/child) do not.
	//   What is the meaning of the period, can we get rid of it? It would make it more consistent
	// - No speak of pointers to children, so making assumptions here.

	// Going for Entity -> Component path here.
	// In other words, we always start from an Entity
	if (auto targetComponent = rtti_cast<const nap::Component>(&target))
	{
		auto targetEntity = getOwner(*targetComponent);

		// Sibling component found? Return only one element
		if (targetEntity != nullptr && originEntity == targetEntity)
		{
			result.emplace_back("."); // TODO: Probably not necessary
			result.push_back(targetComponent->mID);
			return;
		}

		// Get absolute paths and compare
		std::deque<std::string> absOriginPath;
		std::deque<std::string> absTargetPath;

		absoluteObjectPathList(*originEntity, absOriginPath);
		absoluteObjectPathList(*targetComponent, absTargetPath);

		size_t commonidx = findCommonStartingElements(absOriginPath, absTargetPath);
		for (size_t i = commonidx, len = absOriginPath.size(); i < len; i++)
			result.emplace_back("..");

		if (result.empty()) // Add a period to be consistent with sibling path?
			result.emplace_back("."); // TODO: Probably not necessary

		for (size_t i = commonidx, len = absTargetPath.size(); i < len; i++)
			result.push_back(absTargetPath[i]);
	}
}


std::string Document::relativeObjectPath(const nap::rtti::Object& origin, const nap::rtti::Object& target) const
{
	std::deque<std::string> path;
	relativeObjectPathList(origin, target, path);
	return nap::utility::joinString(path, "/");
}
