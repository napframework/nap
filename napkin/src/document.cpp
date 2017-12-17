#include <nap/logger.h>
#include "document.h"


using namespace napkin;

nap::Entity* Document::getParent(const nap::Entity& child)
{
	for (const auto& o : getObjectPointers())
	{
		if (!o->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* parent = rtti_cast<nap::Entity>(o);
		auto it = std::find_if(parent->mChildren.begin(), parent->mChildren.end(),
							   [&child](nap::ObjectPtr<nap::Entity> e) -> bool { return &child == e.get(); });

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

		nap::Entity* owner = *rtti_cast<nap::Entity*>(o.get());
		auto it = std::find_if(
				owner->mComponents.begin(), owner->mComponents.end(),
				[&component](nap::ObjectPtr<nap::Component> comp) -> bool { return &component == comp.get(); });

		if (it != owner->mComponents.end())
			return owner;
	}
	return nullptr;
}

nap::Entity* Document::createEntity(nap::Entity* parent)
{
	auto e = std::make_unique<nap::Entity>();
	e->mID = getUniqueName("New Entity");
	auto ret = e.get();
	mObjects.emplace_back(std::move(e));

	if (parent != nullptr)
	{
		parent->mChildren.emplace_back(ret);
	}

	entityAdded(ret, parent);
	return ret;
}

nap::Component* Document::addComponent(nap::Entity& entity, rttr::type type)
{
	assert(type.can_create_instance());
	assert(type.is_derived_from<nap::Component>());

	auto compVariant = type.create();
	auto comp = compVariant.get_value<nap::Component*>();
	comp->mID = getUniqueName(type.get_name().data());
	mObjects.emplace_back(comp);
	entity.mComponents.emplace_back(comp);

	componentAdded(*comp, entity);

	return comp;
}

nap::rtti::RTTIObject* Document::addObject(rttr::type type, nap::rtti::RTTIObject* parent)
{
	nap::rtti::Factory& factory = mCore.getResourceManager()->getFactory();
	assert(factory.canCreate(type));
	assert(type.is_derived_from<nap::rtti::RTTIObject>());

	// Strip off namespace prefixes when creating new objects
	std::string base_name = type.get_name().data();
	size_t last_colon = base_name.find_last_of(':');
	if (last_colon != std::string::npos)
		base_name = base_name.substr(last_colon + 1);

	nap::rtti::RTTIObject* obj = factory.create(type);
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

std::string Document::getUniqueName(const std::string& suggestedName)
{
	std::string newName = suggestedName;
	int i = 2;
	while (getObject(newName))
		newName = suggestedName + "_" + std::to_string(i++);
	return newName;
}

nap::rtti::RTTIObject* Document::getObject(const std::string& name)
{
	auto it = std::find_if(mObjects.begin(), mObjects.end(),
						   [&name](std::unique_ptr<nap::rtti::RTTIObject>& obj) { return obj->mID == name; });
	if (it == mObjects.end())
		return nullptr;
	return it->get();
}

nap::rtti::RTTIObject* Document::getObject(const std::string& name, const rttr::type& type)
{
	auto object = getObject(name);

	if (object == nullptr)
		return nullptr;

	if (object->get_type().is_derived_from(type))
		return object;

	return nullptr;
}


std::vector<nap::rtti::RTTIObject*> Document::getObjectsOfType(const nap::rtti::TypeInfo& type) const
{
	std::vector<nap::rtti::RTTIObject*> result;
	for (auto& object : mObjects)
		if (object->get_type().is_derived_from(type))
			result.push_back(object.get());

	return result;
}

nap::rtti::ObjectList Document::getObjectPointers()
{
	nap::rtti::ObjectList ret;
	for (auto& ob : getObjects())
		ret.emplace_back(ob.get());
	return ret;
}


void Document::removeObject(nap::rtti::RTTIObject& object)
{
	objectRemoved(object);

	if (object.get_type().is_derived_from<nap::Entity>())
	{
		nap::Entity* entity = *rtti_cast<nap::Entity*>(&object);
		nap::Entity* parent = getParent(*entity);
		if (parent)
			parent->mChildren.erase(std::remove(parent->mChildren.begin(), parent->mChildren.end(), &object));
	}
	else if (object.get_type().is_derived_from<nap::Component>())
	{
		nap::Component* component = *rtti_cast<nap::Component*>(&object);
		nap::Entity* owner = getOwner(*component);
		if (owner)
			owner->mComponents.erase(std::remove(owner->mComponents.begin(), owner->mComponents.end(), &object));
	}


	mObjects.erase(
			std::remove_if(mObjects.begin(), mObjects.end(),
						   [&object](std::unique_ptr<nap::rtti::RTTIObject>& obj) { return obj.get() == &object; }),
			mObjects.end());
}

void Document::removeObject(const std::string& name)
{
	auto object = getObject(name);
	if (object != nullptr)
		removeObject(*object);
}


long Document::addArrayElement(const PropertyPath& path)
{
	nap::rtti::ResolvedRTTIPath resolved_path = path.resolve();
	assert(resolved_path.isValid());

	nap::rtti::Variant array = resolved_path.getValue();
	assert(array.is_array());
	nap::rtti::VariantArray array_view = array.create_array_view();

	//auto array_view = path.getArrayView();
	const nap::rtti::TypeInfo element_type = array_view.get_rank_type(array_view.get_rank());
	const nap::rtti::TypeInfo wrapped_type = element_type.is_wrapper() ? element_type.get_wrapped_type() : element_type;

	rttr::variant new_value = wrapped_type.create();
	assert(new_value.is_valid());
	assert(array_view.is_dynamic());
	long index = array_view.get_size();
	if (!array_view.insert_value(index, new_value))
	{
		nap::Logger::fatal("Failed to add array element to: %s", path.toString().c_str());
		return -1;
	}

	resolved_path.setValue(array);

	propertyValueChanged(path);
	return index;
}

void Document::executeCommand(QUndoCommand* cmd)
{
	mUndoStack.push(cmd);
}


