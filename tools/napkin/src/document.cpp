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

using namespace napkin;
using namespace nap::rtti;

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

Document::Document(nap::Core& core, const QString& filename, nap::rtti::OwnedObjectList objects)
	: QObject(), mCore(core), mCurrentFilename(filename), mObjects(std::move(objects))
{
}

nap::Entity* Document::getParent(const nap::Entity& child) const
{
	for (const auto& o : getObjectPointers())
	{
		if (!o->get_type().is_derived_from<nap::Entity>())
			continue;

		auto parent = rtti_cast<nap::Entity>(o);
		auto it = std::find_if(parent->mChildren.begin(), parent->mChildren.end(),
							   [&child](ObjectPtr<nap::Entity> e) -> bool { return &child == e.get(); });

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
	for (const auto& obj : getObjects())
	{
		if (!obj->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* owner = static_cast<nap::Entity*>(obj.get());
		auto filter = [&component](ObjectPtr<nap::Component> comp) -> bool
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
	const auto& objects = getObjects();
	PropertyPath group_children_path = {};
	for(const auto& obj : objects)
	{
		if(!obj->get_type().is_derived_from(group.get_type()))
			continue;

		if(obj.get() == &group)
			continue;

		// Resolve child group property against current object
		auto property_path = Path::fromString(nap::IGroup::childrenPropertyName());
		ResolvedPath resolved_path;
		property_path.resolve(obj.get(), resolved_path);
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
				return static_cast<nap::IGroup*>(obj.get());
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
	for (const auto& obj : objects)
	{
		if (!obj->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
			continue;

		// Resolve child group property against current object
		auto property_path = Path::fromString(nap::IGroup::membersPropertyName());
		ResolvedPath resolved_path;
		property_path.resolve(obj.get(), resolved_path);
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
				return static_cast<nap::IGroup*>(obj.get());
			}
		}
	}

	outIndex = -1;
	return nullptr;
}

const std::string& Document::setObjectName(nap::rtti::Object& object, const std::string& name)
{
	if (name.empty())
		return object.mID;

	auto newName = getUniqueName(name, object, false);
	if (newName == object.mID)
		return object.mID;

	auto oldName = object.mID;
	object.mID = newName;

	// Update pointers to this object
	for (auto propPath : getPointersTo(object, false, false, false))
		propPath.setPointee(&object);

	PropertyPath path(object, Path::fromString(nap::rtti::sIDPropertyName), *this);
	assert(path.isValid());
	propertyValueChanged(path);

	return object.mID;
}


const std::string& Document::forceSetObjectName(nap::rtti::Object& object, const std::string& name)
{
	auto newName = getUniqueName(name, object, false);

	auto oldName = object.mID;
	object.mID = newName;

	// Update pointers to this object
	for (auto propPath : getPointersTo(object, false, false, false))
		propPath.setPointee(&object);

	PropertyPath path(object, Path::fromString(nap::rtti::sIDPropertyName), *this);
	assert(path.isValid());

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

	nap::rtti::Variant compVariant = factory.create(type);
	auto comp = compVariant.get_value<nap::Component*>();
	comp->mID = getUniqueName(type.get_name().data(), *comp, true);

	mObjects.emplace_back(comp);
	entity.mComponents.emplace_back(comp);
	componentAdded(comp, &entity);
	return comp;
}


nap::rtti::Object* Document::addObject(rttr::type type, nap::rtti::Object* parent, bool selectNewObject, const std::string& name)
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
	std::unique_ptr<Object> obj = std::unique_ptr<Object>(factory.create(type));
	assert(obj != nullptr);
	obj->mID = getUniqueName(base_name, *obj, true);

	// Add to managed object list
	Object* obj_ptr = obj.get();
	mObjects.emplace_back(std::move(obj));

	// Notify listeners
	objectAdded(obj_ptr, parent, selectNewObject);
	return obj_ptr;
}

void Document::reparentEntity(nap::Entity& entity, nap::Entity* newParent)
{
	nap::Entity* oldParent = getParent(entity);

	if (oldParent == newParent)
		return; // nothing to do, already parented

	if (oldParent)
		oldParent->mChildren.erase(std::remove(oldParent->mChildren.begin(), oldParent->mChildren.end(), &entity));

	if (newParent)
		newParent->mChildren.emplace_back(&entity);

	entityReparented(&entity, oldParent, newParent);
}

nap::Entity& Document::addEntity(nap::Entity* parent, const std::string& name)
{
	auto e = addObject<nap::Entity>(parent, name);
	assert(e != nullptr);
	return *e;
}

std::string Document::getUniqueName(const std::string& suggestedName, const nap::rtti::Object& object, bool useUUID)
{
	std::string newName = suggestedName;
	if (useUUID)
		newName += "_" + createSimpleUUID();
	int i = 2;
	auto obj = getObject(newName);
	while (obj != nullptr && obj != &object)
	{
		if (useUUID)
			newName = suggestedName + "_" + createSimpleUUID();
		else
			newName = suggestedName + std::to_string(i++);

		obj = getObject(newName);
	}
	return newName;
}


Object* Document::getObject(const std::string& name)
{
	for (auto obj : getObjectPointers())
		if (obj->mID == name)
			return obj;
	return nullptr;
}


Object* Document::getObject(const std::string& name, const rttr::type& type)
{
	auto object = getObject(name);

	if (object == nullptr)
		return nullptr;

	if (object->get_type().is_derived_from(type))
		return object;

	return nullptr;
}


ObjectList Document::getObjectPointers() const
{
	ObjectList ret;
	for (auto& ob : getObjects())
		ret.emplace_back(ob.get());
	return ret;
}


void Document::removeObject(Object& object)
{
	// Emit signal first so observers can act before the change
	objectRemoved(&object);

	// Remove instance properties for this object
	if (!object.get_type().is_derived_from<nap::InstancePropertyValue>())
		removeInstanceProperties(object);

	// Remove embedded objects under the given owner
	nap::rtti::ObjectList embedded_objs = getEmbeddedObjects(object);
	for (auto embedded_obj : embedded_objs)
		removeObject(*embedded_obj);

	// Start by cleaning up objects that depend on this one
	if (object.get_type().is_derived_from<nap::Entity>())
	{
		// Remove from every scene
		auto entity = static_cast<nap::Entity*>(&object);
		auto scenes = getObjects(RTTI_OF(nap::Scene));
		for (auto& scene : scenes)
			removeEntityFromScene(static_cast<nap::Scene&>(*scene), *entity);

		// Delete components of this Entity
		std::vector<nap::rtti::ObjectPtr<nap::Component>> comps = entity->getComponents();
		for (auto compptr : comps)
			removeObject(*compptr.get());

		// Remove from parent
		reparentEntity(*entity, nullptr);
	}

	// Remove the component from entities that reference it
	if (object.get_type().is_derived_from(RTTI_OF(nap::Component)))
	{
		auto component = static_cast<nap::Component*>(&object);
		auto owner = getOwner(*component);
		if (owner != nullptr)
		{
			auto it = std::remove(owner->mComponents.begin(), owner->mComponents.end(), &object);
			owner->mComponents.erase(it, owner->mComponents.end());
		}
	}

	// Remove the object from every group as a member
	int child_index = -1;
	auto parent_group = getGroup(object, child_index);
	if (parent_group != nullptr)
	{
		groupRemoveElement(*parent_group, parent_group->getMembersProperty(), child_index);
	}

	// Remove the group from parent groups, if any
	if (object.get_type().is_derived_from(RTTI_OF(nap::IGroup)))
	{
		// Get parent group
		auto group = static_cast<nap::IGroup*>(&object);
		int child_index = -1;
		auto owner =  getOwner(*group, child_index);

		// Remove from parent if found
		if (owner != nullptr)
		{
			groupRemoveElement(*owner, owner->getChildrenProperty(), child_index);
		}
	}

	// All clean. Remove our object
	auto filter = [&](const auto& obj) { return obj.get() == &object; };
	mObjects.erase(std::remove_if(mObjects.begin(), mObjects.end(), filter), mObjects.end());
}


void Document::removeObject(const std::string& name)
{
	auto object = getObject(name);
	if (object != nullptr)
		removeObject(*object);
}

void Document::removeInstanceProperties(nap::rtti::Object& object)
{
	for (auto scene : getObjects<nap::Scene>())
		removeInstanceProperties(*scene, object);
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
						prop.mTargetComponent.assign(newPath, *prop.mTargetComponent.get());

						if (!changedScenes.contains(scene))
							changedScenes.append(scene);
					}
				}
				++i;
			}
		}
	}

	for (auto scene : changedScenes)
		objectChanged(scene);
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
	objectChanged(&scene);
	return index;
}

size_t Document::addChildEntity(nap::Entity& parent, nap::Entity& child)
{
	auto index = parent.mChildren.size();
	parent.mChildren.emplace_back(&child);
	entityAdded(&child, &parent);
	return index;
}

void Document::removeChildEntity(nap::Entity& parent, size_t childIndex)
{
	// WARNING: This will NOT take care of removing and patching up instance properties
	auto obj = parent.mChildren[childIndex];
	parent.mChildren.erase(parent.mChildren.begin() + childIndex);
	objectChanged(&parent);

	PropertyPath childrenProp(parent, nap::rtti::Path::fromString("Children"), *this);
	assert(childrenProp.isValid());
	propertyValueChanged(childrenProp);
}

void Document::remove(const PropertyPath& path)
{
	auto parent = path.getParent();

	if (parent.getType().is_derived_from<nap::Entity>() && path.getType().is_derived_from<nap::Entity>())
	{
		// Removing child Entity from parent Entity
		auto parentEntity = rtti_cast<nap::Entity>(parent.getObject());
		assert(parentEntity);
		auto childEntity = rtti_cast<nap::Entity>(path.getObject());
		assert(childEntity);

		// Remove all instanceproperties that refer to this Entity:0 under ParentEntity
		auto realIndex = path.getRealChildEntityIndex();
		removeInstanceProperties(path);
		removeChildEntity(*parentEntity, realIndex);
		return;
	}

	auto _p1 = parent.toString();
	auto _p2 = path.toString();

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
					objectChanged(scene);
					return;
				}
				++idx;
			}

		}
	}

}

void Document::removeEntityFromScene(nap::Scene& scene, nap::RootEntity& entity)
{
	removeInstanceProperties(scene, *entity.mEntity);

	auto& v = scene.mEntities;
	auto filter = [&](nap::RootEntity& obj) { return &obj == &entity; };
	v.erase(std::remove_if(v.begin(), v.end(), filter), v.end());
	objectChanged(&scene);
}

void Document::removeEntityFromScene(nap::Scene& scene, nap::Entity& entity)
{
	removeInstanceProperties(scene, entity);

	auto& v = scene.mEntities;
	auto filter = [&](nap::RootEntity& obj) { return obj.mEntity == &entity; };
	v.erase(std::remove_if(v.begin(), v.end(), filter), v.end());
	objectChanged(&scene);
}

void Document::removeEntityFromScene(nap::Scene& scene, size_t index)
{
	removeInstanceProperties(scene, *scene.mEntities[index].mEntity);

	scene.mEntities.erase(scene.mEntities.begin() + index);
	objectChanged(&scene);
}

int Document::arrayAddValue(const PropertyPath& path)
{

	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_array());
	VariantArray array_view = array.create_array_view();

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
	if (wrapped_type == RTTI_OF(std::string))
		new_value = std::string();
	else
		new_value = wrapped_type.create();

	assert(new_value.is_valid());
	assert(array_view.is_dynamic());

	size_t index = array_view.get_size();
	bool inserted = array_view.insert_value(index, new_value);
	assert(inserted);
	resolved_path.setValue(array);

	propertyChildInserted(path, index);
	propertyValueChanged(path);

	return index;
}


size_t Document::arrayAddExistingObject(const PropertyPath& path, Object* object, size_t index)
{
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

	Variant new_item = object;
	bool convert_ok = new_item.convert(wrapped_type);
	assert(convert_ok);

	assert(index <= array_view.get_size());
	bool inserted = array_view.insert_value(index, new_item);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	propertyValueChanged(path);
	propertyChildInserted(path, index);

	// HACK? fixes: object->mID will somehow get invalidated, this makes it stick...
	forceSetObjectName(*object, object->mID);

	return index;
}

size_t Document::arrayAddExistingObject(const PropertyPath& path, nap::rtti::Object* object)
{
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

	Variant new_item = object;
	bool convert_ok = new_item.convert(wrapped_type);
	assert(convert_ok);

	size_t index = array_view.get_size();
	bool inserted = array_view.insert_value(index, new_item);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

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
	Object* new_object = addObject(type, parent_object, parent_object->get_type().is_derived_from(RTTI_OF(nap::IGroup)) );
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


size_t Document::arrayMoveElement(const PropertyPath& path, size_t fromIndex, size_t toIndex)
{
	ResolvedPath resolved_path = path.resolve();
	Variant array_value = resolved_path.getValue();
	VariantArray array = array_value.create_array_view();
	assert(fromIndex <= array.get_size());
	assert(toIndex <= array.get_size());

	if (fromIndex < toIndex)
		toIndex--;

	Variant taken_value = array.get_value(fromIndex);
	bool ok = array.remove_value(fromIndex);
	assert(ok);
	propertyChildRemoved(path, fromIndex);
	ok = array.insert_value(toIndex, taken_value);
	assert(ok);
	ok = resolved_path.setValue(array_value);
	assert(ok);
	propertyValueChanged(path);
	propertyChildInserted(path, toIndex);

	return toIndex;
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
	for (const std::unique_ptr<nap::rtti::Object>& sourceObject : mObjects)
	{
		std::vector<nap::rtti::ObjectLink> links;
		findObjectLinks(*sourceObject, links);
		for (const auto& link : links)
		{
			// Link is target
			assert(link.mSource == sourceObject.get());
			if (link.mTarget != &targetObject)
				continue;

			// Construct path
			PropertyPath propPath(*sourceObject, link.mSourcePath, *this);
			auto proppathstr = propPath.toString();
			assert(propPath.isPointer());

			if (excludeInstanceProperties && sourceObject->get_type().is_derived_from<nap::Scene>())
				continue;

			if (excludeArrays && propPath.isArray())
				continue;

			if (propPath.getParent().getType().is_derived_from<nap::ComponentInstanceProperties>())
				continue;

			if (excludeParent)
			{
				if (sourceObject->get_type().is_derived_from(RTTI_OF(nap::Entity)))
				{
					auto* entity = static_cast<nap::Entity*>(sourceObject.get());
					if (std::find(entity->mChildren.begin(), entity->mChildren.end(), &targetObject) != entity->mChildren.end())
						continue;

					if (std::find(entity->mComponents.begin(), entity->mComponents.end(), &targetObject) !=
						entity->mComponents.end())
						continue;
				}
				else if (sourceObject->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
				{
					// Check if item is part of group
					auto* group = static_cast<nap::IGroup*>(sourceObject.get());
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
	for (auto& object : getObjects())
	{
		if (object->get_type().is_derived_from(type))
			result.emplace_back(object.get());
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

std::string Document::createSimpleUUID()
{
	auto uuid = QUuid::createUuid().toString();
	// just take the last couple of characters
	int charCount = 8;
	auto shortuuid = uuid.mid(uuid.size() - 2 - charCount, charCount);
	return shortuuid.toStdString();
}

