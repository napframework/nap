#include <nap/entityinstance.h>
#include <nap/componentinstance.h>

using namespace std;

RTTI_BEGIN_CLASS(nap::EntityResource)
	RTTI_PROPERTY("Components", &nap::EntityResource::mComponents,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Children",	&nap::EntityResource::mChildren,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AutoSpawn",	&nap::EntityResource::mAutoSpawn,	nap::rtti::EPropertyMetaData::Default)
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


	ComponentInstance* EntityInstance::findComponent(const rtti::TypeInfo& type) const
	{
		ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) { return element->get_type() == type; });
		if (pos == mComponents.end())
			return nullptr;

		return pos->get();
	}

	void EntityInstance::getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components) const
	{
		for (auto& component : mComponents)
			if (component->get_type().is_derived_from(type))
				components.push_back(component.get());
	}


	bool EntityInstance::hasComponentsOfType(const rtti::TypeInfo& type) const
	{
		for (auto& component : mComponents)
			if (component->get_type().is_derived_from(type))
				return true;

		return false;
	}


	bool EntityInstance::hasComponent(const rtti::TypeInfo& type) const
	{
		return findComponent(type) != nullptr;
	}


	ComponentInstance& EntityInstance::getComponent(const rtti::TypeInfo& type) const
	{
		ComponentInstance* result = findComponent(type);
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

	ObjectPtr<ComponentResource> EntityResource::findComponent(const rtti::TypeInfo& type) const
	{
		ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) { return element->get_type() == type; });
		if (pos == mComponents.end())
			return nullptr;

		return pos->get();
	}


	bool EntityResource::hasComponent(const rtti::TypeInfo& type) const
	{
		return findComponent(type) != nullptr;
	}

}