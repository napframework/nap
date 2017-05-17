#include "object.h"
#include "utility/stringutils.h"
#include "component.h"

#include <algorithm>

RTTI_BEGIN_BASE_CLASS(nap::Object)
RTTI_END_CLASS

namespace nap
{
    
    // Recursive function that notifies parents of possible child removal actions
    static void signalChildRemoval(nap::Object& child, nap::Object& parent)
    {
        parent.childRemoved.trigger(child);
        for (auto& new_child : child.getChildren()) {
            signalChildRemoval(*new_child, child);
        }
        child.removed.trigger(child);
    }
    
    
	const std::string& Object::getName() const { return mName; }

    
	const std::string& Object::setName(const std::string& name)
	{
		std::string objName = name;
		objName = utility::trim(objName);
		if (objName.empty()) objName = get_type().get_name().data();
		std::string sanitizedName = sanitizeName(objName);
		mName = getUniqueName(sanitizedName);
		nameChanged(mName);
		return mName;
	}


	std::string Object::getUniqueName(const std::string& suggestedName)
	{
		if (!getParentObject())
		{
			return suggestedName;
		}

		std::string newName = suggestedName;
		
		// Fetch possible child from parent
		Object* child = getParentObject()->getChild(newName);

		// If a child was found, try to find a child
		// that doesn't have this name
		int i = 0;
		while (child) 
		{
			if (child == this)
			{
				break;
			}
			newName = utility::stringFormat("%s_%d", suggestedName.c_str(), i++);
			child = getParentObject()->getChild(newName);
		}
		return newName;
	}


	std::string Object::sanitizeName(const std::string& name)
	{
		const char* nameCharP = name.c_str();
		std::ostringstream ss;

		char lastChar = -1;
		for (size_t i = 0; i < name.size(); i++) {
			char c = nameCharP[i];
			if (isalnum(c)) {
				ss << c;
			} else {
				c = '_';
				ss << c;
			}
			lastChar = c;
		}
		return ss.str();
	}
    
    
    void Object::addChild(Object& child)
    {
        child.setParent(*this);
        child.setName(child.getName()); // Ensures that the name is sanitized and unique
        mChildren.emplace_back(&child);
        childAdded(child);
    }
    
    
    nap::Object& Object::addChild(const std::string& name, const rtti::TypeInfo& type)
    {
        assert(type.can_create_instance());
        assert(type.is_derived_from<Object>());
        auto child = type.create<Object>();
        assert(child);
        child->mName = name;
        addChild(std::move(std::unique_ptr<Object>(child)));
        return *child;
    }
    
    
    Object& Object::addChild(const rtti::TypeInfo& type)
    {
        return addChild("", type); // Name will be sanitized later
    }
    
    
    nap::Object& Object::addChild(std::unique_ptr<Object> child)
    {
        auto ptr = child.get();
        addChild(*ptr);
        mOwnedObjects.emplace_back(std::move(child)); // ensure that the Object base class takes care of ownership and destruction of the child
        return *ptr;
    }


	// Removes a single child
	bool Object::removeChild(Object& child)
	{
        bool result = false;
        
        // look for the child in the children vector and erase
		for (auto it = mChildren.begin(); it != mChildren.end(); ++it) {
			if (*it != &child) continue;
			signalChildRemoval(**it, *this); // emit the removed signal
			mChildren.erase(it);
            result = true;
            break;
		}
        
        // erase the child from the owned object list, in case the Object base class handles ownership of the child
        for (auto it = mOwnedObjects.begin(); it != mOwnedObjects.end(); ++it)
            if (it->get() == &child)
            {
                mOwnedObjects.erase(it);
                break;
            }
        
        
		return result;
	}


	// Removes a single child based on it's name (case sensitive)
	bool Object::removeChild(const std::string& name)
	{
        auto child = getChild(name);
        if (child)
            return removeChild(*child);
        return false;
	}

    
    void Object::clearChildren()
    {
        mChildren.clear();
        mOwnedObjects.clear();
    }
    

	// Clears all children of a specific type
	void Object::clearChildren(const rtti::TypeInfo& type)
	{
        // look for the children of type type in the children vector and erase
        for (auto it = mChildren.begin(); it != mChildren.end();)
            if ((*it)->get_type().is_derived_from(type))
            {
                signalChildRemoval(**it, *this);
                mChildren.erase(it);
            }
            else
                it++;
        
        // erase the children of type from the owned object list, in case the Object base class handles ownership of the child
        for (auto it = mOwnedObjects.begin(); it != mOwnedObjects.end();)
            if ((*it)->get_type().is_derived_from(type))
                mOwnedObjects.erase(it);
            else
                it++;
	}


	std::vector<Object*> Object::getChildren(bool recursive)
	{
		std::vector<Object*> result;
        for (Object* child : mChildren)
        {
            result.emplace_back(child);
			if (recursive)
			{
				for (auto object : child->getChildren(true))
				{
					result.emplace_back(object);
				}
			}
        }
		return result;
	}
    

    const std::vector<Object*> Object::getChildren(bool recursive) const
    {
        std::vector<Object*> result;
        
        for (Object* child : mChildren)
        {
            result.emplace_back(child);
            if (recursive)
                for (auto object : child->getChildren(true))
                    result.emplace_back(object);
        }
        
        return result;
    }
    

	// Return direct number of children
	size_t Object::getNumberOfChildren() const
	{
		return mChildren.size();
	}


	Object* Object::getChild(const std::string& name)
	{
		for (auto child : mChildren)
			if (child->getName() == name)
                return child;

		return nullptr;
	}


	nap::Object* Object::getChild(size_t index)
	{
		return index >= mChildren.size() ? nullptr : mChildren[index];
	}
    
    
    bool Object::hasChild(const std::string& name) const
    {
        for (auto child : mChildren)
            if (child->getName() == name)
                return true;
        return false;
    }
    

    
	int Object::getChildIndex(Object& child)
	{
		auto children = getChildren();
		for (size_t i = 0; i < mChildren.size(); i++) {
			if (children[i] == &child) return i;
		}
		return -1;
	}
    

	Object* Object::getChildOfType(const rtti::TypeInfo& type)
	{
		for (auto& object : mChildren)
			if (object->get_type() == type) return object;
		return nullptr;
	}


	const Object* Object::getRootObject() const
	{
		Object* parent = getParentObject();
		if (parent == nullptr) return this;
		return parent->getRootObject();
	}


	Object* Object::getRootObject()
	{
		Object* parent = getParentObject();
		if (parent == nullptr) return this;
		return parent->getRootObject();
	}


	void Object::setParent(Object& inParent)
	{
		mParent = &inParent;
		added(*this);
	}

    
	bool Object::isChildOf(const Object& parent, bool recursive) const
	{
        if (!getParentObject())
            return false;

        if (getParentObject() == &parent)
            return true;

        if (!recursive)
            return false;

        return getParentObject()->isChildOf(parent);
	}
}
