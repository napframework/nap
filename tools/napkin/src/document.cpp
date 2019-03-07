#include "document.h"

#include <nap/logger.h>
#include <QList>
#include <QtDebug>
#include <QStack>
#include <utility/fileutils.h>

#include "naputils.h"

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
	for (const auto& o : getObjects())
	{
		if (!o->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* owner = rtti_cast<nap::Entity>(o.get());
		auto filter = [&component](ObjectPtr<nap::Component> comp) -> bool
		{
			return &component == comp.get();
		};

		if (std::find_if(owner->mComponents.begin(), owner->mComponents.end(), filter) != owner->mComponents.end())
			return owner;
	}
	return nullptr;
}


const std::string& Document::setObjectName(nap::rtti::Object& object, const std::string& name)
{
	if (name.empty())
		return object.mID;

	object.mID = getUniqueName(name, object);
	PropertyPath path(object, Path::fromString(nap::rtti::sIDPropertyName));
	assert(path.isValid());
	propertyValueChanged(path);
	return object.mID;
}


nap::Component* Document::addComponent(nap::Entity& entity, rttr::type type)
{
	auto& factory = mCore.getResourceManager()->getFactory();
	bool canCreate = factory.canCreate(type);

	if (!canCreate)
		nap::Logger::fatal("Cannot create instance of '%s'", type.get_name().data());
	assert(canCreate);
	assert(type.is_derived_from<nap::Component>());

	nap::rtti::Variant compVariant = factory.create(type);
	auto comp = compVariant.get_value<nap::Component*>();
	comp->mID = getUniqueName(type.get_name().data(), *comp);

	mObjects.emplace_back(comp);
	entity.mComponents.emplace_back(comp);

	componentAdded(comp, &entity);

	return comp;
}


nap::rtti::Object* Document::addObject(rttr::type type, nap::rtti::Object* parent, bool selectNewObject, const std::string& name)
{
	Factory& factory = mCore.getResourceManager()->getFactory();

	if (!type.is_derived_from<Object>())
	{
		nap::Logger::error("Cannot create object of type: %s, type does not derive from",
				type.get_name().data(), RTTI_OF(Object).get_name().data());
		return nullptr;
	}

	if (!factory.canCreate(type))
	{
		nap::Logger::error("Cannot create object of type: %s", type.get_name().data());
		return nullptr;
	}

	std::string base_name = name.empty() ? friendlyTypeName(type) : name;

	std::unique_ptr<Object> obj = std::unique_ptr<Object>(factory.create(type));
	Object* objptr = obj.get();
	assert(objptr != nullptr);
	obj->mID = getUniqueName(base_name, *objptr);
	mObjects.emplace_back(std::move(obj));

	// Handle adding to a parent
	// TODO: Make this a little less hokey
	if (parent != nullptr)
	{
		auto parentEntity = rtti_cast<nap::Entity>(parent);

		auto newEntity = rtti_cast<nap::Entity>(objptr);
		auto newComponent = rtti_cast<nap::Component>(objptr);

		if (parentEntity != nullptr)
		{
			if (newEntity != nullptr)
			{
				parentEntity->mChildren.emplace_back(newEntity);
			}
			else if (newComponent != nullptr)
			{
				parentEntity->mComponents.emplace_back(newComponent);
			}
		}
	}

	objectAdded(objptr, selectNewObject);

	return objptr;
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

std::string Document::getUniqueName(const std::string& suggestedName, const nap::rtti::Object& object)
{
	std::string newName = suggestedName;
	int i = 2;
	auto obj = getObject(newName);
	while (obj != nullptr && obj != &object)
	{
		newName = suggestedName + "_" + std::to_string(i++);
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

	// Another special case for our snowflake: InstanceProperty
	if (!object.get_type().is_derived_from<nap::InstancePropertyValue>())
		removeInstanceProperties(object);

	// Start by cleaning up objects that depend on this one
	if (object.get_type().is_derived_from<nap::Entity>())
	{
		auto entity = rtti_cast<nap::Entity>(&object);
		// Remove from any scenes
		for (auto& obj : getObjects())
		{
			auto scene = rtti_cast<nap::Scene>(obj.get());
			if (scene != nullptr)
				removeEntityFromScene(*scene, *entity);
		}

		// Delete any components on this Entity
		std::vector<nap::rtti::ObjectPtr<nap::Component>> comps = entity->getComponents();
		for (auto compptr : comps)
			removeObject(*compptr.get());

		reparentEntity(*entity, nullptr);
	}

	auto component = rtti_cast<nap::Component>(&object);
	if (component != nullptr)
	{
		nap::Entity* owner = getOwner(*component);
		if (owner)
			owner->mComponents.erase(std::remove(owner->mComponents.begin(), owner->mComponents.end(), &object));
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
	auto allComponents = getComponentsRecursive(object);

	for (auto& rootEntity : scene.mEntities)
	{
		auto& props = rootEntity.mInstanceProperties;
		for (int i = static_cast<int>(props.size() - 1); i >= 0; --i)
		{
			if (allComponents.contains(props[i].mTargetComponent.get()))
			{
//				qInfo() << "Remove instance properties: " << QString::fromStdString(props[i].mTargetComponent.get()->mID);
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
	auto parentEntity = dynamic_cast<nap::Entity*>(&parent.getObject());
	assert(parentEntity);
	auto instIndex = path.getInstanceChildEntityIndex();

	auto parentID = parentEntity->mID;
	auto entityID = path.getObject().mID;

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
	QList<nap::Component*> components;

	auto component = dynamic_cast<nap::Component*>(&object);
	if (object.get_type().is_derived_from<nap::Component>())
	{
		components.append(component);
		return components;
	}

	auto entity = dynamic_cast<nap::Entity*>(&object);
	if (entity)
	{
		recurseChildren(*dynamic_cast<nap::Entity*>(&object), [&components](nap::Entity& child)
		{
			for (auto comp : child.mComponents)
				components << comp.get();
		});
		return components;
	}

	assert(false);
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

	PropertyPath childrenProp(parent, nap::rtti::Path::fromString("Children"));
	assert(childrenProp.isValid());
	propertyValueChanged(childrenProp);
}

void Document::remove(const PropertyPath& path)
{
	auto parent = path.getParent();

	if (parent.getType().is_derived_from<nap::Entity>() && path.getType().is_derived_from<nap::Entity>())
	{
		// Removing child Entity from parent Entity
		auto parentEntity = dynamic_cast<nap::Entity*>(&parent.getObject());
		assert(parentEntity);
		auto childEntity = dynamic_cast<nap::Entity*>(&path.getObject());
		assert(childEntity);
		auto realIndex = path.getRealChildEntityIndex();

		// Remove all instanceproperties that refer to this Entity:0 under ParentEntity
		removeInstanceProperties(path);

		removeChildEntity(*parentEntity, realIndex);
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

size_t Document::arrayAddValue(const PropertyPath& path)
{

	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_array());
	VariantArray array_view = array.create_array_view();

	const TypeInfo element_type = array_view.get_rank_type(1);
	const TypeInfo wrapped_type = element_type.is_wrapper() ? element_type.get_wrapped_type() : element_type;
	assert(wrapped_type.can_create_instance());

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

	propertyValueChanged(path);
	propertyChildInserted(path, index);

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
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	VariantArray array_view = array.create_array_view();

	Object* new_object = addObject(type, nullptr, false);
	if (!new_object)
	{
		nap::Logger::error("Did not create object at: %s", path.toString().c_str());
		return 0;
	}

	assert(index <= array_view.get_size());
	bool inserted = array_view.insert_value(index, new_object);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	propertyValueChanged(path);
	propertyChildInserted(path, index);

	return index;
}

size_t Document::arrayAddNewObject(const PropertyPath& path, const nap::rtti::TypeInfo& type)
{
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	VariantArray array_view = array.create_array_view();

	Object* new_object = addObject(type, nullptr, false);

	size_t index = array_view.get_size();
	bool inserted = array_view.insert_value(index, new_object);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	propertyValueChanged(path);
	propertyChildInserted(path, index);

	return index;
}

void Document::arrayRemoveElement(const PropertyPath& path, size_t index)
{
	// If embedded pointer, get pointee so we can delete that too
	auto elementPath = path.getArrayElement(index);
	nap::rtti::Object* pointee = nullptr;
	if (elementPath.isEmbeddedPointer())
		pointee = elementPath.getPointee();

	ResolvedPath resolved_path = path.resolve();
	Variant value = resolved_path.getValue();
	VariantArray array = value.create_array_view();
	assert(index < array.get_size());

	bool ok = array.remove_value(index);
	assert(ok);

	ok = resolved_path.setValue(value);
	assert(ok);

	if (pointee != nullptr)
		removeObject(*pointee);

	propertyValueChanged(path);
	propertyChildRemoved(path, index);
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

QList<PropertyPath> Document::getPointersTo(const nap::rtti::Object& targetObject,
		bool excludeArrays,
		bool excludeParent,
		bool excludeInstanceProperties)
{
	QList<PropertyPath> properties;

	for (const std::unique_ptr<nap::rtti::Object>& sourceObject : mObjects)
	{
		std::vector<nap::rtti::ObjectLink> links;
		findObjectLinks(*sourceObject, links);
		for (const auto& link : links)
		{
			assert(link.mSource == sourceObject.get());

			if (link.mTarget != &targetObject)
				continue;

			PropertyPath propPath(*sourceObject, link.mSourcePath);
			auto proppathstr = propPath.toString();
			assert(propPath.isPointer());

			if (excludeArrays && propPath.isArray())
				continue;

			if (excludeParent)
			{
				auto entity = rtti_cast<nap::Entity>(sourceObject.get());
				if (entity != nullptr)
				{
					if (std::find(entity->mChildren.begin(), entity->mChildren.end(), &targetObject) != entity->mChildren.end())
						continue;
					if (std::find(entity->mComponents.begin(), entity->mComponents.end(), &targetObject) !=
						entity->mComponents.end())
						continue;
				}
			}

			if (propPath.getParent().getType().is_derived_from<nap::ComponentInstanceProperties>())
				continue;

			properties << propPath;
		}
	}
	return properties;
}

QList<nap::RootEntity*> Document::getRootEntities(nap::Scene& scene, nap::rtti::Object& object)
{
	auto entity = dynamic_cast<nap::Entity*>(&object);
	if (entity == nullptr)
	{
		// must be component
		auto comp = dynamic_cast<nap::Component*>(&object);
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
	for (const auto& path : getPointersTo(obj, false, false))
	{
		//assert(path.isPointer());
		if (path.isEmbeddedPointer())
			return true;
	}

	return false;
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
	// This implementation is based off the description in componentptr.h
	// Notes:
	// - Sibling component paths start with a single period, but other paths (parent/child) do not.
	//   What is the meaning of the period, can we get rid of it? It would make it more consistent
	// - No speak of pointers to children, so making assumptions here.

	// Going for Entity -> Component path here.
	// In other words, we always start from an Entity

	// Grab the origin entity (ignore component if provided)
	auto originEntity = rtti_cast<const nap::Entity>(&origin);
	if (!originEntity) {
		auto originComponent = rtti_cast<const nap::Component>(&origin);
		assert(originComponent != nullptr);
		originEntity = getOwner(*originComponent);
	}
	assert(originEntity != nullptr);

	auto targetComponent = rtti_cast<const nap::Component>(&target);
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

	for (size_t i = commonidx, len=absTargetPath.size(); i<len; i++)
		result.push_back(absTargetPath[i]);
}

std::string Document::relativeObjectPath(const nap::rtti::Object& origin, const nap::rtti::Object& target) const
{
	std::deque<std::string> path;
	relativeObjectPathList(origin, target, path);
	return nap::utility::joinString(path, "/");
}

