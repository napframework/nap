#pragma once
#include <nap.h>
#include "event.h"

namespace nap {

	// Look for component T in the provided entity,
	// if not found, go to its parent and look for its component there
	template<typename T>
	T* getFirstParentComponent(nap::Entity& parent) {
		T* comp = parent.getComponent<T>();
		if (comp)
			return comp;

		if (parent.getParent())
			return getFirstParentComponent<T>(*parent.getParent());

		return nullptr;
	}

	// Walk the object tree from the provided root.
	// For each ObjectType found, grab the provided attribute and set its value.
	template<typename ObjectType, typename AttributeType>
	void setAttributeRecursive(const std::string& attName, nap::Object& initialObject, const AttributeType& value) {
        std::vector<Object*> allObjects = initialObject.getChildren(true);
		allObjects.emplace_back(&initialObject);
		for (Object* ob : allObjects) {
			if (!ob->getTypeInfo().isKindOf<ObjectType>())
				continue;
			Attribute<AttributeType>* attrib = static_cast<ObjectType*>(ob)->template getAttribute<AttributeType>(attName);
			if (!attrib)
				continue;
			attrib->setValue(value);
		}
	}

	template<typename T>
	T* findFirstChildOfType(Object& initialObject) {
		std::vector<Object*> allObjects = initialObject.getChildren(true);
		allObjects.emplace_back(&initialObject);
		for (Object* ob : allObjects) {
			if (ob->getTypeInfo().isKindOf<T>())
				return static_cast<T*>(ob);
		}
		return nullptr;
	}


}
