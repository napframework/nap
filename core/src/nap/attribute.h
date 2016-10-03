#pragma once

#include <assert.h>
#include <cstring>
#include <nap/object.h>
#include <nap/signalslot.h>
#include <nap/link.h>
#include <rtti/rtti.h>
#include <sstream>
#include <string>


namespace nap
{
	// Forward Declares
	class AttributeObject;
	class Object;

	template <typename T>
	class ArrayAttribute;

	/**
	@brief AttributeObject

	Attribute is a system wide available parameter that holds a name and contains a value.
	This value can be queried by other objects, set using script etc.
	An Attribute often defines a property of a object containing the attribute, such as Width, Transform etc.
	**/
	class AttributeBase : public Object
	{
		friend class AttributeObject;
		RTTI_ENABLE_DERIVED_FROM(Object)
	public:
		AttributeBase() {}
		AttributeBase(AttributeObject* parent, const std::string& name, bool atomic = false);

        // Virtual destructor because of virtual methods!
        virtual ~AttributeBase() = default;
        
		virtual void		getValue(AttributeBase& attribute) const = 0;
		virtual void		setValue(const AttributeBase& attribute) = 0;

		AttributeObject*	getParent() const;

        void				link(AttributeBase& source);
        void				linkPath(const std::string& path);
        void				unLink();
        bool				isLinked();

        const ObjectPath&	getLinkSource() const { return getLink().getPath(); }

		// Conversion
        void				fromString(const std::string& stringValue);
        void				toString(std::string& outStringValue) const;

		// Uses fromString() method to set the value
		void				setValue(const std::string& value);

		virtual const RTTI::TypeInfo getValueType() const = 0;

		// Copy (Declared but needs thinking, moving not supported for now)
		AttributeBase(AttributeBase&) = default;
		AttributeBase& operator=(const AttributeBase&) = default;

		void				connectToAttribute(Slot<AttributeBase&>& slot);

		// sets the attribute to internally lock a mutex when accessing it's value
		void				setAtomic(bool atomic) { mAtomic = atomic; }
		bool				isAtomic() const { return mAtomic; }

		Signal<AttributeBase&> valueChanged;


	protected:
        virtual Link&		getLink() const = 0;

        // Emitted in derived (templated) classes (?)
		virtual void		emitValueChanged() = 0;

		// Indicates wether the attribute should internally lock o mutex for thread safety when accessing it's value
		bool				mAtomic = false;

		// The mutex that will be locked when accessing the attribute's value to make it threadsafe
		std::mutex			mMutex;

	};


	/**
	@brief Attribute

	Concrete implementation of attribute, invisible to UI and scripting
	**/
	template <typename T>
	class Attribute : public AttributeBase
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeBase)
	public:
		// Default constructor
		Attribute() : AttributeBase() {  }

		// Constructor with defult value
		Attribute(AttributeObject* parent, const std::string& name, const T& inValue, bool atomic = false)
			: AttributeBase(parent, name, atomic), mValue(inValue)
		{
			setValueSlot.setFunction({[this](const T& value) { this->setValue(value); }});
		}

		// Constructor without default value
		Attribute(AttributeObject* parent, const std::string& name) : AttributeBase(parent, name, false)
		{
			setValueSlot.setFunction({[this](const T& value) { this->setValue(value); }});
		}

		// Constructor to declare an attribute with a member function pointer for the @valueChangedSignal as last argument.
		template <typename U, typename F>
		Attribute(U* parent, const std::string& name, const T& inValue, F function, bool atomic = false)
			: AttributeBase(parent, name, atomic), mValue(inValue)
		{
			setValueSlot.setFunction({[this](const T& value) { this->setValue(value); }});
			valueChangedSignal.connect(parent, function);
		}

		virtual const RTTI::TypeInfo getValueType() const override;

		// Getters
		virtual void		getValue(AttributeBase& inAttribute) const override;
		const T&			getValue() const;
		T&					getValueRef();

		// Setters
		virtual void		setValue(const AttributeBase &inAttribute) override;
		virtual void		setValue(const T& inValue);

		// Connect to
		virtual void		connectToValue(Slot<const T&>& inSlot);
		virtual void		disconnectFromValue(Slot<const T&>& inSlot);

		// Inherited from BaseAttribute, so that BaseAttribute can trigger the valueChanged() signal to be emitted
		void				emitValueChanged() override final { valueChangedSignal(mValue); }

		// Signal emited when the value changes
		Signal<const T&>	valueChangedSignal;
		// This slot will be invoked when the value is set
		Slot<const T&>		setValueSlot;

		// Operator overloads
		operator const T&() const { return getValue(); }

	protected:
		// Members
		T				mValue;
		Link&			getLink() const override { return mLink; }

	private:
        // Keep constructor hidden, use factory methods to instantiate
		Attribute(const T& inValue) : mValue(inValue) {}

        mutable TypedLink<Attribute<T>> mLink = { *this };
	};


	/**
	@brief Specialization of the default Attribute


	**/
	template<typename T>
	class NumericAttribute : public Attribute<T>
	{
		RTTI_ENABLE_DERIVED_FROM(Attribute<T>)
	public:
		// Default constructor
		NumericAttribute() : Attribute()	{ }

		// Constructor with defult value and min max
		NumericAttribute(AttributeObject* parent, const std::string& name, const T& value, const T& minValue, const T& maxValue, bool atomic = false, bool clamped = true)
			: Attribute(parent, name, value , atomic) 
		{
			setRange(minValue, maxValue);
			setClamped(clamped);
		}


		// Constructor with default value and no min / max
		NumericAttribute(AttributeObject* parent, const std::string& name, const T& value, bool atomic = false)
			: Attribute(parent, name, value, atomic)
		{
			setRange(value, value);
		}


		// Constructor to declare an attribute with a member function pointer for the @valueChangedSignal as last argument.
		template <typename U, typename F>
		NumericAttribute(U* parent, const std::string& name, const T& inValue, const T& minValue, const T& maxValue, F function, bool atomic = false, bool clamped = true)
			: Attribute(parent, name, inValue, function, atomic)	
		{
			setRange(minValue, maxValue);
			setClamped(clamped);
		}

		// Setters
		virtual void	setValue(const T& value) override;
		void			setRange(const T& min, const T& max);
		void			setClamped(bool value);

		// Getters
		void			isClamped()	const	{ return mClamped; }

		// Clamp function
		virtual T		clampValue(const T& value, const T& min, const T& max);

	private:
		// Range
		T mMinValue;
		T mMaxValue;

		bool mClamped = false;
	};


	//////////////////////////////////////////////////////////////////////////
	// Attribute Template Definitions
	//////////////////////////////////////////////////////////////////////////


	/**
	@brief Returns the containing type of Attribute<T>
	**/
	template <typename T>
	const RTTI::TypeInfo Attribute<T>::getValueType() const
	{
		return RTTI::TypeInfo::get<T>();
	}


	/**
	@brief Sets the value of @inAttribute to this attribute's value
	**/
	template <typename T>
	void Attribute<T>::getValue(AttributeBase& inAttribute) const
	{
		static_cast<Attribute<T>&>(inAttribute).setValue(getValue());
	}


	/**
	@brief Attribute<T>::getValue()

	Returns the attribute value, as a link or member 
	Links are automatically resolved, otherwise return default vlaue
	**/
	template <typename T>
	const T& Attribute<T>::getValue() const
	{
		assert(getTypeInfo().isKindOf(mLink.getTargetType()));

        // No link, just return the value
        if (!mLink.isLinked())
            return mValue;

        // Target might not be valid, attempt to resovle
        const Attribute<T>* targetAttr = mLink.getTypedTarget();

        if (!targetAttr)
            return mValue;// Failed to resolve

        // Return linked value
        return targetAttr->getValue();
	}


	/**
	@brief Returns the this attribute's value as a reference
	**/
	template <typename T>
	T& Attribute<T>::getValueRef()
	{
		// When an attribute is atomic calling getValueRef() potentially breaks thread safety
		assert(!mAtomic);

		return mValue;
	}


	/**
	@brief Sets this attributes value to @inValue
	**/
	template <typename T>
	void Attribute<T>::setValue(const T& inValue)
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
	void Attribute<T>::setValue(const AttributeBase &inAttribute)
	{
		const Attribute<T>& in_attr = static_cast<const Attribute<T>&>(inAttribute);
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


	/**
	@brief Connect an attribute's value changed signal to a slot
	**/
	template <typename T>
	void Attribute<T>::connectToValue(Slot<const T&>& inSlot)
	{
		valueChangedSignal.connect(inSlot);
	}


	/**
	@brief Disconnect an attribute's value changed signal from a slot
	**/
	template <typename T>
	void Attribute<T>::disconnectFromValue(Slot<const T&>& inSlot)
	{
		valueChangedSignal.disconnect(inSlot);
	}

	//////////////////////////////////////////////////////////////////////////
	// Numeric Attribute Template Definitions
	//////////////////////////////////////////////////////////////////////////

	/**
	@brief Maps @value between @min and @max

	Override to specify specific clamp behavior
	**/
	template <typename T>
	T NumericAttribute<T>::clampValue(const T& value, const T& min, const T& max)
	{
		return std::max(min, std::min(value, max));
	}


	/**
	@brief Maps the incoming value before setting it
	**/
	template <typename T>
	void NumericAttribute<T>::setValue(const T& value)
	{
		T new_v = mClamped ? clampValue(value, mMinValue, mMaxValue) : value;
		Attribute<T>::setValue(new_v);
	}


	/**
	@brief set the min / max range of the attribute
	**/
	template <typename T>
	void NumericAttribute<T>::setRange(const T& min, const T& max)
	{
		mMinValue = min;
		mMaxValue = max;
	}


	/**
	@brief Sets if the range is clamped or not
	**/
	template <typename T>
	void NumericAttribute<T>::setClamped(bool value)
	{
		// Skip if the same
		if (value == mClamped)
			return;

		// If we're clamping, make sure to update the value
		mClamped = value;
		if (mClamped)
			setValue(mValue);
	}
}


//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////


RTTI_DECLARE_BASE(nap::AttributeBase)

// Create and bind attribute slot with @NAME to @FUNCTION
#define ATTR_SLOT(NAME, FUNCTION) SLOT(NAME, nap::Attribute&, FUNCTION)

// Create attribute slow with @NAME without binding it to a function
#define CREATE_ATTR_SLOT(NAME) CREATE_SLOT(NAME, nap::Attribute&)

// Bind an already created attribute slot with @NAME to @FUNCTION
#define BIND_ATTR_SLOT(NAME, FUNCTION) BIND_SLOT(NAME, nap::Attribute&, FUNCTION)
