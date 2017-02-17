// Local Includes
#include "attribute.h"
#include "resourcemanager.h"
#include "attributeobject.h"
#include "entity.h"
#include "coreattributes.h"

// RTTI Define

namespace nap 
{
    /**
    @brief Attribute Constructor
    **/
    AttributeBase::AttributeBase(AttributeObject* parent, const std::string& name, bool atomic)
	{
        mName = name;
        parent->addChild(*this);
	}

    
	/**
	@brief Returns this attribute's parent, always an AttributeObject
	**/
	AttributeObject* AttributeBase::getParent() const
	{
		return static_cast<AttributeObject*>(getParentObject());
	}


    /**
     Set attribute value using fromString() method
     @param value: The value to set on this attribute
     **/
    void AttributeBase::setValue(const std::string& value)
    {
        fromString(value);
        valueChanged(*this);
    }
    
    
    /**
     * Converts this attribute to a string using the type converters registered in core
     */
    void AttributeBase::toString(std::string &outStringValue) const
    {
        const Entity* root = dynamic_cast<const Entity*>(getRootObject());
        if (!root)
            return;
        
        auto converter = root->getCore().getModuleManager().getTypeConverter(getValueType(), RTTI::TypeInfo::get<std::string>());
        if (!converter)
            return;
        
        Attribute<std::string> stringAttr;
        converter->convert(this, &stringAttr);
        outStringValue = stringAttr.getValue();
    }
    
    
    /**
     * Sets the value of this attribute using the type convertor registered in core
     */
    void AttributeBase::fromString(const std::string &stringValue)
    {
        const Entity* root = dynamic_cast<const Entity*>(getRootObject());
        if (!root)
            return;
        
        auto converter = root->getCore().getModuleManager().getTypeConverter(RTTI::TypeInfo::get<std::string>(), getValueType());
        if (!converter)
            return;
        
        Attribute<std::string> stringAttr;
        stringAttr.setValue(stringValue);
        
        converter->convert(&stringAttr, this);
    }
    

	//////////////////////////////////////////////////////////////////////////
	// ObjectLinkAttribute
	//////////////////////////////////////////////////////////////////////////

	/**
	 * constructor using a type as link type
	 */
	ObjectLinkAttribute::ObjectLinkAttribute(AttributeObject* parent, const std::string& name, const RTTI::TypeInfo& type) :
		AttributeBase(parent, name)
	{
		mLink.setTargetType(type);
		mLink.targetChanged.connect(onLinkTargetChangedSlot);
	}

	
	ObjectLinkAttribute::ObjectLinkAttribute()
	{
		mLink.targetChanged.connect(onLinkTargetChangedSlot);
	}

	/**
	 * Sets the path based on the current link target
	 */
	void ObjectLinkAttribute::getValue(AttributeBase& attribute) const
	{
		const ObjectPath& path = getPath();
		static_cast<ObjectLinkAttribute&>(attribute).setTarget(path);
	}


	/**
	 * Sets the path based on the incoming attribute
	 */
	void ObjectLinkAttribute::setValue(const AttributeBase& attribute)
	{
        assert(attribute.getTypeInfo() == getTypeInfo());
		const ObjectLinkAttribute& attr = static_cast<const ObjectLinkAttribute&>(attribute);
        this->setTarget(attr.getPath());
	}


	/**
	 * Set new link target, trigger change
	 */
	void ObjectLinkAttribute::setTarget(Object& target)
	{
		mLink.setTarget(target);
	}


	/**
	 * Set new target and forward change
	 */
	void ObjectLinkAttribute::setTarget(const std::string& targetPath)
	{
		mLink.setTarget(targetPath);
	}

	/**
	 * Always returns type of link
	 */
	const RTTI::TypeInfo ObjectLinkAttribute::getValueType() const
	{
		return RTTI_OF(nap::Link);
	}

	/**
	 * Trigger update when link target changes
	 */
	void ObjectLinkAttribute::onLinkTargetChanged(const Link& link)
	{
		valueChanged.trigger(*this);
	}
}

// RTTI Define
RTTI_DEFINE(nap::AttributeBase)
RTTI_DEFINE(nap::SignalAttribute)
RTTI_DEFINE(nap::ObjectLinkAttribute)

