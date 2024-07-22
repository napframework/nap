/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "entity.h"
#include "scene.h"
#include <nap/python.h>
#include <nap/core.h>

using namespace std;

RTTI_BEGIN_CLASS(nap::Entity, "General purpose object")
	RTTI_PROPERTY(nap::Entity::componentsPropertyName(), &nap::Entity::mComponents, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY(nap::Entity::childrenPropertyName(), &nap::Entity::mChildren, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EntityInstance)
	RTTI_CONSTRUCTOR(nap::Core&, const nap::Entity*)
	RTTI_FUNCTION("findComponent", (nap::ComponentInstance* (nap::EntityInstance::*)(const std::string &) const) &nap::EntityInstance::findComponent)
    RTTI_FUNCTION("findComponentByID", (nap::ComponentInstance* (nap::EntityInstance::*)(const std::string &) const) &nap::EntityInstance::findComponentByID)
RTTI_END_CLASS

namespace nap
{
	EntityInstance::EntityInstance(Core& core, const Entity* entity) :
		mCore(&core), mResource(entity)
	{ }


	bool EntityInstance::init(Scene& scene, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		for (int index = 0; index < mResource->mChildren.size(); ++index)
		{
			EntityInstance* child_entity_instance = scene.createChildEntityInstance(*mResource->mChildren[index], index, entityCreationParams, errorState);
			if (!errorState.check(child_entity_instance != nullptr, "Failed to spawn child entity %s for entity %s", mResource->mChildren[index]->mID.c_str(), mID.c_str()))
				return false; 

			addChild(*child_entity_instance);
		}

		return true;
	}


	void EntityInstance::update(double deltaTime)
	{
		for (auto& component : mComponents)
			component->update(deltaTime);

        // We need to work with an integer iterator control variable here because children can be added or removed from the list while iterating the loop.
        for (auto i = 0; i < mChildren.size(); ++i)
            mChildren[i]->update(deltaTime);
	}


	void EntityInstance::addComponent(std::unique_ptr<ComponentInstance> component)
	{
		mComponents.emplace_back(std::move(component));
	}


	ComponentInstance* EntityInstance::findComponent(const std::string& type) const
	{
		return findComponent(rtti::TypeInfo::get_by_name(type));
	}


    ComponentInstance* EntityInstance::findComponentByID(const std::string& ID) const
    {
        ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&, ID](auto& element)
        {
            return element->getComponent()->mID == ID;
        });
        if (pos == mComponents.end())
            return nullptr;
        
        return pos->get();
    }
    
    
	ComponentInstance* EntityInstance::findComponent(const rtti::TypeInfo& type) const
	{
		ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) 
		{ 
			return rtti::isTypeMatch(element->get_type(), type, rtti::ETypeCheck::IS_DERIVED_FROM);
		});
		if (pos == mComponents.end())
			return nullptr;

		return pos->get();
	}


	void EntityInstance::getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components) const
	{
		for (auto& component : mComponents)
			if (rtti::isTypeMatch(component->get_type(), type, rtti::ETypeCheck::IS_DERIVED_FROM))
				components.emplace_back(component.get());
	}


	bool EntityInstance::hasComponentsOfType(const rtti::TypeInfo& type) const
	{
		for (auto& component : mComponents)
			if (rtti::isTypeMatch(component->get_type(), type, rtti::ETypeCheck::IS_DERIVED_FROM))
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
		mChildren.emplace_back(&child);
	}


	void EntityInstance::clearChildren()
	{
		for (EntityInstance* child : mChildren)
			child->mParent = nullptr;

		mChildren.clear();
	}

	
	void EntityInstance::removeChild(const EntityInstance& entityInstance)
	{
		mChildren.erase(std::remove_if(mChildren.begin(), mChildren.end(), [&entityInstance](const EntityInstance* child)
		{
			return child == &entityInstance;
		}));
	}


	const EntityInstance::ChildList& EntityInstance::getChildren() const
	{
		return mChildren;
	}


	EntityInstance* EntityInstance::getParent() const
	{
		return mParent;
	}


	const nap::Entity* EntityInstance::getEntity() const
	{
		return mResource;
	}


	Core* EntityInstance::getCore() const
	{
		return mCore;
	}

	//////////////////////////////////////////////////////////////////////////

	rtti::ObjectPtr<Component> Entity::findComponent(const rtti::TypeInfo& type) const
	{
		ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) { return isTypeMatch(element->get_type(), type, rtti::ETypeCheck::IS_DERIVED_FROM); });
		if (pos == mComponents.end())
			return nullptr;

		return pos->get();
	}


	bool Entity::hasComponent(const rtti::TypeInfo& type) const
	{
		return findComponent(type) != nullptr;
	}
}
