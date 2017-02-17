#pragma once

//
//  ArrayAttribute.hpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/3/16.
//
//

// External includes
#include <stdio.h>

// Local includes
#include <rtti/rtti.h>
#include "attribute.h"

namespace nap
{

    // Forward Declares
    class AttributeObject;
    
    /**
     ArrayAttributeBase
     
     */
    class ArrayAttributeBase : public AttributeBase
    {
        RTTI_ENABLE_DERIVED_FROM(nap::AttributeBase)
    public:
        // Constructor
        ArrayAttributeBase() = default;
        ArrayAttributeBase(AttributeObject* parent, const std::string& name, bool atomic = false) : AttributeBase(parent, name, atomic) {}
        
        // Enable default copy behavior
        ArrayAttributeBase(ArrayAttributeBase&) = default;
        ArrayAttributeBase& operator=(const ArrayAttributeBase&) = default;
        
        // Virtual destructor because of virtual methods!
        virtual ~ArrayAttributeBase() = default;
        
        virtual int getSize() = 0;
    };
    
    
    /**
     ArrayAttribute
     
     Concrete implementation of array attribute, invisible to UI and scripting
     **/
    template <typename T>
    class ArrayAttribute : public ArrayAttributeBase
    {
        RTTI_ENABLE_DERIVED_FROM(ArrayAttributeBase)
    public:
        // Default constructor
        ArrayAttribute() : ArrayAttributeBase() {  }
        
        // Constructor with defult value
        ArrayAttribute(AttributeObject* parent, const std::string& name, const std::vector<T>& inValue, bool atomic = false)
        : ArrayAttributeBase(parent, name, atomic), mValue(inValue)
        {
            setValueSlot.setFunction({ [this](const T& value) { this->setValue(value); } });
        }
        
        // Constructor without default value
        ArrayAttribute(AttributeObject* parent, const std::string& name) : ArrayAttributeBase(parent, name, false)
        {
            setValueSlot.setFunction({ [this](const std::vector<T>& value) { this->setValue(value); } });
        }
        
        // Constructor to declare an attribute with a member function pointer for the @valueChangedSignal as last argument.
        template <typename U, typename F>
        ArrayAttribute(U* parent, const std::string& name, const std::vector<T>& inValue, F function, bool atomic = false)
        : ArrayAttributeBase(parent, name, atomic), mValue(inValue)
        {
            setValueSlot.setFunction({ [this](const std::vector<T>& value) { this->setValue(value); } });
            valueChangedSignal.connect(parent, function);
        }
        
        virtual const RTTI::TypeInfo getValueType() const override;
        
        // Getters
        virtual void getValue(AttributeBase& inAttribute) const override;
        const std::vector<T>& getValue() const;
        std::vector<T>& getValueRef();
        const T& getValue(int index) const;
        
        // Setters
        virtual void setValue(const AttributeBase &inAttribute) override;
        virtual void setValue(const std::vector<T>& inValue);
        void setValue(int index, const T& inValue);
        
        // Connect to
        virtual void connectToValue(Slot<const std::vector<T>&>& inSlot);
        virtual void disconnectFromValue(Slot<const std::vector<T>&>& inSlot);
        
        // Inherited from BaseAttribute, so that BaseAttribute can trigger the valueChanged() signal to be emitted
        void emitValueChanged() override final { valueChangedSignal(mValue); }
        
        int getSize() override final { return mValue.size(); }
        
        // Adds a new @element to the end of the array
        void add(const T& element);
        // Insert a new element at @index
        void insert(int index, const T& element);
        // Remove element at @index
        void remove(int index);
        // Clear the array
        void clear();
        
        const typename std::vector<T>::iterator begin() { return mValue.begin(); }
        const typename std::vector<T>::iterator end() { return mValue.end(); }
        
        // Signal emited when the value changes
        Signal<const std::vector<T>&>	valueChangedSignal;
        // This slot will be invoked when the value is set
        Slot<const std::vector<T>&>		setValueSlot;
        
        // Operator overloads
        operator const std::vector<T>&() const { return getValue(); }

        const T& operator [] (int index) const { return mValue[index]; }
        
    protected:
        // Members
        std::vector<T> mValue;
        Link& getLink() const override { return mLink; }
        
    private:
        // Keep constructor hidden, use factory methods to instantiate
        ArrayAttribute(const std::vector<T>& inValue) : mValue(inValue) {}
        
        // Link of type T
        mutable TypedLink<ArrayAttribute<T>> mLink = { *this };
    };
    
    
    
    // TEMPLATE DEFINITIONS
    
    template <typename T>
    const RTTI::TypeInfo ArrayAttribute<T>::getValueType() const
    {
        return RTTI::TypeInfo::get<T>();
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::getValue(AttributeBase& inAttribute) const
    {
        assert(getTypeInfo().isKindOf(inAttribute.getTypeInfo()));
        
        static_cast<ArrayAttribute<T>&>(inAttribute).setValue(getValue());
    }
    
    
    template <typename T>
    const std::vector<T>& ArrayAttribute<T>::getValue() const
    {
        assert(getTypeInfo().isKindOf(mLink.getTargetType()));
        
        // No link, just return the value
        if (!mLink.isLinked())
            return mValue;
        
        // Target might not be valid, attempt to resovle
        const ArrayAttribute<T>* targetAttr = mLink.getTypedTarget();
        
        if (!targetAttr)
            return mValue;// Failed to resolve
        
						  // Return linked value
        return targetAttr->getValue();
    }
    
    
    template <typename T>
    std::vector<T>& ArrayAttribute<T>::getValueRef()
    {
        // When an attribute is atomic calling getValueRef() potentially breaks thread safety
        assert(!mAtomic);
        
        return mValue;
    }
    
    
    template <typename T>
    const T& ArrayAttribute<T>::getValue(int index) const
    {
        return mValue.at(index);
    }

    
    template <typename T>
    void ArrayAttribute<T>::setValue(const std::vector<T>& inValue)
    {
        // Don't change if it's the same
        if (inValue == mValue)
            return;
        
        // Otherwise lock if blocking
        if (mAtomic)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mValue = inValue;
        }
        else
        {
            mValue = inValue;
        }
        
        valueChanged(*this);
        valueChangedSignal(mValue);
    }
    
    
    // Sets mValue to @inAttribute.mValue
    template <typename T>
    void ArrayAttribute<T>::setValue(const AttributeBase &inAttribute)
    {
        const ArrayAttribute<T>& in_attr = static_cast<const ArrayAttribute<T>&>(inAttribute);
        if (in_attr.mValue == mValue)
            return;
        
        if (mAtomic)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mValue = in_attr.mValue;
        }
        else
        {
            mValue = in_attr.mValue;
        }
        
        valueChanged(*this);
        valueChangedSignal(mValue);
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::setValue(int index, const T& inValue)
    {
        mValue[index] = inValue;
        
        valueChanged(*this);
        valueChangedSignal(mValue);
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::connectToValue(Slot<const std::vector<T>&>& inSlot)
    {
        valueChangedSignal.connect(inSlot);
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::disconnectFromValue(Slot<const std::vector<T>&>& inSlot)
    {
        valueChangedSignal.disconnect(inSlot);
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::add(const T& element)
    {
        mValue.emplace_back(element);
        
        valueChanged(*this);
        valueChangedSignal(mValue);        
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::insert(int index, const T& element)
    {
        auto it = mValue.begin();
        std::advance(it, index);
        mValue.insert(it, element);
        
        valueChanged(*this);
        valueChangedSignal(mValue);
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::remove(int index)
    {
        auto it = mValue.begin();
        std::advance(it, index);
        mValue.erase(it);
        
        valueChanged(*this);
        valueChangedSignal(mValue);
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::clear()
    {
        mValue.clear();
        
        valueChanged(*this);
        valueChangedSignal(mValue);
    }
    
    
}

RTTI_DECLARE_BASE(nap::ArrayAttributeBase)

