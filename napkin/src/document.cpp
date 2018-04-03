#include "document.h"

#include <nap/logger.h>
#include <scene.h>

#include "generic/naputils.h"

using namespace napkin;
using namespace nap::rtti;

nap::Entity* Document::getParent(const nap::Entity& child)
{
	for (const auto& o : getObjectPointers())
	{
		if (!o->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* parent = rtti_cast<nap::Entity>(o);
		auto it = std::find_if(parent->mChildren.begin(), parent->mChildren.end(),
							   [&child](ObjectPtr<nap::Entity> e) -> bool { return &child == e.get(); });

		if (it != parent->mChildren.end())
			return parent;
	}
	return nullptr;
}


nap::Entity* Document::getOwner(const nap::Component& component)
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

	object.mID = getUniqueName(name);
	PropertyPath path(object, nap::rtti::sIDPropertyName);
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
	comp->mID = getUniqueName(type.get_name().data());

	mObjects.emplace_back(comp);
	entity.mComponents.emplace_back(comp);

	componentAdded(*comp, entity);

	return comp;
}


Object* Document::addObject(rttr::type type, Object* parent)
{
	Factory& factory = mCore.getResourceManager()->getFactory();
	assert(factory.canCreate(type));
	assert(type.is_derived_from<Object>());

	// Strip off namespace prefixes when creating new objects
	std::string base_name = type.get_name().data();
	size_t last_colon = base_name.find_last_of(':');
	if (last_colon != std::string::npos)
		base_name = base_name.substr(last_colon + 1);

	Object* obj = factory.create(type);
	obj->mID = getUniqueName(base_name);
	mObjects.emplace_back(obj);

	// Handle adding to a parent
	// TODO: Make this a little less hokey
	if (parent != nullptr)
	{
		auto parentEntity = rtti_cast<nap::Entity>(parent);

		auto newEntity = rtti_cast<nap::Entity>(obj);
		auto newComponent = rtti_cast<nap::Component>(obj);

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

	objectAdded(*obj, parent);

	return obj;
}


nap::Entity& Document::addEntity()
{
	auto e = addObject<nap::Entity>();
	assert(e != nullptr);
	return *e;
}


std::string Document::getUniqueName(const std::string& suggestedName)
{
	std::string newName = suggestedName;
	int i = 2;
	while (getObject(newName))
		newName = suggestedName + "_" + std::to_string(i++);
	return newName;
}


Object* Document::getObject(const std::string& name)
{
	auto it = std::find_if(mObjects.begin(), mObjects.end(),
						   [&name](std::unique_ptr<Object>& obj) { return obj->mID == name; });
	if (it == mObjects.end())
		return nullptr;
	return it->get();
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


ObjectList Document::getObjectPointers()
{
	ObjectList ret;
	for (auto& ob : getObjects())
		ret.emplace_back(ob.get());
	return ret;
}


void Document::removeObject(Object& object)
{
	// Emit signal first so observers can act before the change
	objectRemoved(object);

	// Start by cleaning up objects that depend on this one
	auto entity = rtti_cast<nap::Entity>(&object);
	if (entity != nullptr)
	{
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

		// Remove from parent
		nap::Entity* parent = getParent(*entity);
		if (parent)
			parent->mChildren.erase(std::remove(parent->mChildren.begin(), parent->mChildren.end(), &object));

	}

	auto component = rtti_cast<nap::Component>(&object);
	if (component != nullptr)
	{
		nap::Entity* owner = getOwner(*component);
		if (owner)
			owner->mComponents.erase(std::remove(owner->mComponents.begin(), owner->mComponents.end(), &object));
	}

	// All cleam. Remove our object
	auto filter = [&](auto& obj) { return obj.get() == &object; };
	mObjects.erase(std::remove_if(mObjects.begin(), mObjects.end(), filter), mObjects.end());
}


void Document::removeObject(const std::string& name)
{
	auto object = getObject(name);
	if (object != nullptr)
		removeObject(*object);
}


size_t Document::addEntityToScene(nap::Scene& scene, nap::Entity& entity)
{
	nap::RootEntity rootEntity;
	rootEntity.mEntity = &entity;
	size_t index = scene.mEntities.size();
	scene.mEntities.emplace_back(rootEntity);
	return index;
}


void Document::removeEntityFromScene(nap::Scene& scene, nap::Entity& entity)
{
	auto& v = scene.mEntities;
	auto filter = [&](nap::RootEntity& obj) { return obj.mEntity== &entity; };
	v.erase(std::remove_if(v.begin(), v.end(), filter), v.end());
}


size_t Document::arrayAddValue(const PropertyPath& path, size_t index)
{
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_array());
	VariantArray array_view = array.create_array_view();

	const TypeInfo element_type = array_view.get_rank_type(1);
	const TypeInfo wrapped_type = element_type.is_wrapper() ? element_type.get_wrapped_type() : element_type;
	assert(wrapped_type.can_create_instance());
	rttr::variant new_value = wrapped_type.create();
	assert(new_value.is_valid());
	assert(array_view.is_dynamic());

	bool inserted = array_view.insert_value(index, new_value);
	assert(inserted);

	resolved_path.setValue(array);

	propertyValueChanged(path);

	return index;
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
	Variant new_value = wrapped_type.create();
	assert(new_value.is_valid());
	assert(array_view.is_dynamic());

	size_t index = array_view.get_size();
	bool inserted = array_view.insert_value(index, new_value);
	assert(inserted);

	resolved_path.setValue(array);

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

	return index;
}



size_t Document::arrayAddNewObject(const PropertyPath& path, const TypeInfo& type, size_t index)
{
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	VariantArray array_view = array.create_array_view();

	Object* new_object = addObject(type);

	assert(index <= array_view.get_size());
	bool inserted = array_view.insert_value(index, new_object);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	propertyValueChanged(path);

	return index;
}

size_t Document::arrayAddNewObject(const PropertyPath& path, const nap::rtti::TypeInfo& type)
{
	ResolvedPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	VariantArray array_view = array.create_array_view();

	Object* new_object = addObject(type);

	size_t index = array_view.get_size();
	bool inserted = array_view.insert_value(index, new_object);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	propertyValueChanged(path);

	return index;
}


void Document::arrayRemoveElement(const PropertyPath& path, size_t index)
{
	ResolvedPath resolved_path = path.resolve();
	Variant value = resolved_path.getValue();
	VariantArray array = value.create_array_view();
	assert(index < array.get_size());

	bool ok = array.remove_value(index);
	assert(ok);

	ok = resolved_path.setValue(value);
	assert(ok);

	propertyValueChanged(path);
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

	ok = array.insert_value(toIndex, taken_value);
	assert(ok);

	ok = resolved_path.setValue(array_value);
	assert(ok);

	propertyValueChanged(path);

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

QList<PropertyPath> Document::getPointersTo(const nap::rtti::Object& obj, bool excludeArrays, bool excludeParent)
{
	QList<PropertyPath> properties;

	for (const std::unique_ptr<nap::rtti::Object>& object : mObjects)
	{
		std::vector<nap::rtti::ObjectLink> links;
		findObjectLinks(*object, links);
		for (const auto& link : links)
		{
			if (link.mTarget != &obj)
				continue;

			PropertyPath propPath(*object, link.mSourcePath);
			if (excludeArrays && propPath.isArray())
				continue;

			if (excludeParent) {
				auto entity = rtti_cast<nap::Entity>(object.get());
				if (std::find(entity->mChildren.begin(), entity->mChildren.end(), &obj) != entity->mChildren.end())
					continue;
				if (std::find(entity->mComponents.begin(), entity->mComponents.end(), &obj) != entity->mComponents.end())
					continue;
			}

			properties << propPath;
		}
	}
	return properties;
}

Document::~Document()
{
	
}



