#include <rtti/pythonmodule.h>
#include "entity.h"
#include "core.h"

using namespace std;

RTTI_BEGIN_CLASS(nap::Entity)
	RTTI_PROPERTY("Components", &nap::Entity::mComponents, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Children", &nap::Entity::mChildren, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AutoSpawn", &nap::Entity::mAutoSpawn, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EntityInstance)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_FUNCTION("findComponent", (nap::ComponentInstance* (nap::EntityInstance::*)(const std::string &) const) &nap::EntityInstance::findComponent)
RTTI_END_CLASS

namespace nap
{
	EntityInstance::EntityInstance(Core& core) :
		mCore(&core)
	{
	}


	void EntityInstance::update(double deltaTime)
	{
		for (auto& component : mComponents)
			component->update(deltaTime);

		for (EntityInstance* child : mChildren)
			child->update(deltaTime);
	}


	void EntityInstance::addComponent(std::unique_ptr<ComponentInstance> component)
	{
		mComponents.emplace_back(std::move(component));
	}


	ComponentInstance* EntityInstance::findComponent(const std::string& type) const
	{
		return findComponent(rtti::TypeInfo::get_by_name(type));
	}

	ComponentInstance* EntityInstance::findComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck) const
	{
		ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) 
		{ 
			return isTypeMatch(element->get_type(), type, typeCheck); 
		});
		if (pos == mComponents.end())
			return nullptr;

		return pos->get();
	}

	void EntityInstance::getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components, ETypeCheck typeCheck) const
	{
		for (auto& component : mComponents)
			if (isTypeMatch(component->get_type(), type, typeCheck))
				components.push_back(component.get());
	}


	bool EntityInstance::hasComponentsOfType(const rtti::TypeInfo& type, ETypeCheck typeCheck) const
	{
		for (auto& component : mComponents)
			if (isTypeMatch(component->get_type(), type, typeCheck))
				return true;

		return false;
	}


	bool EntityInstance::hasComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck) const
	{
		return findComponent(type, typeCheck) != nullptr;
	}


	ComponentInstance& EntityInstance::getComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck) const
	{
		ComponentInstance* result = findComponent(type, typeCheck);
		assert(result != nullptr);
		return *result;
	}


	void EntityInstance::addChild(EntityInstance& child)
	{
		assert(child.mParent == nullptr);
		child.mParent = this;
		mChildren.push_back(&child);
	}


	void EntityInstance::clearChildren()
	{
		for (EntityInstance* child : mChildren)
			child->mParent = nullptr;

		mChildren.clear();
	}


	const EntityInstance::ChildList& EntityInstance::getChildren() const
	{
		return mChildren;
	}


	EntityInstance* EntityInstance::getParent() const
	{
		return mParent;
	}


	Core* EntityInstance::getCore() const
	{
		return mCore;
	}

	//////////////////////////////////////////////////////////////////////////

	ObjectPtr<Component> Entity::findComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck) const
	{
		ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) { return isTypeMatch(element->get_type(), type, typeCheck); });
		if (pos == mComponents.end())
			return nullptr;

		return pos->get();
	}


	bool Entity::hasComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck) const
	{
		return findComponent(type, typeCheck) != nullptr;
	}

}