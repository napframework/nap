// Local Includes
#include "attribute.h"
#include <nap/attributeobject.h>
#include <nap/entity.h>
#include <nap/coreattributes.h>

// RTTI Define

namespace nap {

    /**
    @brief Attribute Constructor
    **/
    AttributeBase::AttributeBase(AttributeObject* parent, const std::string& name, bool atomic) : mAtomic(atomic)
	{
        mName = name;
        parent->addChild(*this);
	}


	/**
	 Set attribute value using fromString() method
	 @param value: The value to set on this attribute
	 **/
	void AttributeBase::setValue(const std::string& value)
	{
        if (mAtomic)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            fromString(value);
        }
		else
		{
			fromString(value);
		}

		valueChanged(*this);
        emitValueChanged();
	}


	/**
	@brief Returns this attribute's parent, always an AttributeObject
	**/
	AttributeObject* AttributeBase::getParent() const
	{
		return static_cast<AttributeObject*>(getParentObject());
	}


	/**
     @brief Returns a signal that is called when the value of the attribute changes
     **/
    void nap::AttributeBase::connectToAttribute(nap::Slot<nap::AttributeBase&>& slot) 
	{ 
		valueChanged.connect(slot); 
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
        
        if (mAtomic)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            converter->convert(&stringAttr, this);
        }
        else {
            converter->convert(&stringAttr, this);
        }
    }


	/**
	@brief Links @source attribute to this attribute
	**/
    void AttributeBase::link(AttributeBase& source) 
	{
        assert(&source != this);
        assert(getValueType() == source.getValueType());
        getLink().setTarget(source);
    }


	/**
	@brief Links an external attribute by path to this attribute
	**/
    void AttributeBase::linkPath(const std::string& path) 
	{
        getLink().setTarget(path);
    }


	/**
	@brief Clears any existing link
	**/
    void AttributeBase::unLink() 
	{
        getLink().clear();
    }


	/**
	@brief Returns if the attribute is currently linked to a different attribute
	**/
    bool AttributeBase::isLinked() { return getLink().isLinked(); }
}
// RTTI Define
RTTI_DEFINE(nap::AttributeBase)
RTTI_DEFINE(nap::SignalAttribute)

