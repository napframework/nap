#include <nap/attribute.h>
#include <nap/attributeobject.h>
#include <nap/entity.h>
#include <nap/coremodule.h>

// RTTI Define

namespace nap {

    /**
    @brief Attribute Constructor
    **/
	// TODO: MAKE OWNER REF?
	AttributeBase::AttributeBase(AttributeObject* parent, const std::string& name, bool atomic) : mAtomic(atomic)
	{
        mName = name;
        parent->addChild(*this);
	}


	/**
	 @brief set attribute value using fromString() method
	 **/
	void AttributeBase::setValue(const std::string& value)
	{
        if (mAtomic)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            fromString(value);
        }
        else
            fromString(value);
        
		valueChanged(*this);
        emitValueChanged();
	}


	AttributeObject* AttributeBase::getParent() const
	{
		return static_cast<AttributeObject*>(getParentObject());
	}


	/**
     @brief Returns a signal that is called when the value of the attribute changes
     **/
    void nap::AttributeBase::connectToAttribute(nap::Slot<nap::AttributeBase&>& slot) { valueChanged.connect(slot); }

    
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
    
    
    void AttributeBase::fromString(const std::string &stringValue)
    {
        const Entity* root = dynamic_cast<const Entity*>(getRootObject());
        if (!root)
            return;
        
        auto converter = root->getCore().getModuleManager().getTypeConverter(getValueType(), RTTI::TypeInfo::get<std::string>());
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


    void AttributeBase::link(AttributeBase& source) {
        assert(&source != this);
        assert(getValueType() == source.getValueType());
        getLink().setTarget(source);
    }

    void AttributeBase::linkPath(const std::string& path) {
        getLink().setTarget(path);
    }


    void AttributeBase::unLink() {
        getLink().clear();
    }


    bool AttributeBase::isLinked() { return getLink().isLinked(); }


}
// RTTI Define
RTTI_DEFINE(nap::AttributeBase)

