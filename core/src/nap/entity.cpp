#include <algorithm>
#include <nap/entity.h>
#include <nap/component.h>
#include <nap/serializer.h>

using namespace std;

// Define Entity in Type Registry
RTTI_DEFINE_BASE(nap::Entity)

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::ComponentInstance, nap::EntityInstance&)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::ComponentResource)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::EntityResource)
	RTTI_PROPERTY("Components", &nap::EntityResource::mComponents,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Children",	&nap::EntityResource::mChildren,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AutoSpawn",	&nap::EntityResource::mAutoSpawn,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	Entity* Entity::getRoot()
	{
		Entity* parent = getParent();
		if (!parent) return this;
		while (parent->hasParent())
			parent = parent->getParent();
		return parent;
	}
    
    
    // Add a new component of @componentType
    Component& Entity::addComponent(const rtti::TypeInfo& componentType)
    {
        assert(componentType.is_derived_from<Component>());
        return *static_cast<Component*>(&addChild("", componentType));
    }
    
    // Add a component from somewhere else, forwarding parentship to this entity
    Component& Entity::addComponent(std::unique_ptr<Component> component)
    {
        return *static_cast<Component*>(&addChild(std::move(component)));
    }
    
    
    // Remove a component from this entity
    bool Entity::removeComponent(Component& comp)
    {
        return removeChild(comp);
    }
    
    
    Component* Entity::getComponentOfType(const rtti::TypeInfo& componentType)
    {
        return dynamic_cast<Component*>(getChildOfType(componentType));
    }
    
 
    Entity& Entity::addEntity(const std::string& name)
    {
        std::unique_ptr<Entity> e = std::unique_ptr<Entity>(new Entity(mCore));
        e->mName = name;
        return static_cast<Entity&>(addChild(std::move(e)));
    }
    
    
}