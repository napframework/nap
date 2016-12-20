#pragma once

// Local Includes
#include "object.h"
#include "signalslot.h"
#include "link.h"

// External Includes
#include <assert.h>
#include <cstring>
#include <rtti/rtti.h>
#include <sstream>
#include <string>
#include <algorithm>

namespace nap
{
	// Forward Declares
	class AttributeObject;
	class Object;

	/**
	AttributeObject

	Attribute is a system wide available parameter that holds a name and contains a value.
	This value can be queried by other objects, set using script etc.
	An Attribute often defines a property of a object containing the attribute, such as Width, Transform etc.
	*/
	class AttributeBase : public Object
	{
		friend class AttributeObject;
		RTTI_ENABLE_DERIVED_FROM(Object)
	public:
		// Constructor
		AttributeBase() = default;
		AttributeBase(AttributeObject* parent, const std::string& name, bool atomic = false);

		// Enable default copy behavior
		AttributeBase(AttributeBase&) = default;
		AttributeBase& operator=(const AttributeBase&) = default;

		// Virtual destructor because of virtual methods!
		virtual ~AttributeBase() = default;

		/**
		 * Copies the value of this attribute into @attribute
		 @param attribute the attribute to populate
		 */
		virtual void getValue(AttributeBase& attribute) const = 0;

		/**
		 * Copies the value of @attribute into this attribute
		 @param attribute container to copy the value from
		 */
		virtual void setValue(const AttributeBase& attribute) = 0;

		/**
		 * @return the parent associated with this attribute
		 */
		AttributeObject* getParent() const;

		/**
		 * TODO: REMOVE LINK FROM ATTRIBUTE
		 * DEPRECATED -> use ObjectLinkAttribute
		 */
		void link(AttributeBase& source);
		void linkPath(const std::string& path);
		void unLink();

		/**
		 * @return if the attribute is currently linked
		 */
		virtual bool isLinked() const;

		/**
		 * @return the path to the object this attribute links to
		 */
		const ObjectPath& getLinkSource() const { return getLink().getPath(); }

		/**
		 * Converts and sets the value from @strinValue
		 @param stringValue the value to convert
		 */
		void fromString(const std::string& stringValue);

		/**
		 * Converts the value associated with the attribute in to a string
		 @param outStringValue string that will hold the value after conversion
		 */
		void toString(std::string& outStringValue) const;

		/**
		 * Converts and sets the value from @value
		 */
		void setValue(const std::string& value);

		/**
		 * @return the type of the value this attribute holds
		 */
		virtual const RTTI::TypeInfo getValueType() const = 0;

		/**
		 * Sets this attribute value based on changes from an other attribute
		 @param slot the attribute to listen to
		 */
		void connectToAttribute(Slot<AttributeBase&>& slot);

		/**
		 * sets the attribute to internally lock a mutex when accessing it's value
		 * @param atomic if the attribute will be locked or not on access
		 */
		void setAtomic(bool atomic) { mAtomic = atomic; }

		/**
		 * @return if the attribute lock is enabled
		 */
		bool isAtomic() const { return mAtomic; }

		/**
		 * Emits when the value of this attribute changes
		 */
		Signal<AttributeBase&> valueChanged;

	protected:
		virtual Link&		getLink() const = 0;

		// Emitted in derived (templated) classes (?)
		// TODO: DEPRICATE THIS PLEASE, IT'S RATHER UNREADABLE
		// AND CREATES CONFUSION ON DERIVED CLASSES
		[[deprecated]]
		virtual void		emitValueChanged() = 0;

		// Indicates wether the attribute should internally lock o mutex for thread safety when accessing it's value
		bool				mAtomic = false;

		// The mutex that will be locked when accessing the attribute's value to make it threadsafe
		std::mutex			mMutex;


	};


	/**
	Attribute

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
			setValueSlot.setFunction({ [this](const T& value) { this->setValue(value); } });
		}

		// Constructor without default value
		Attribute(AttributeObject* parent, const std::string& name) : AttributeBase(parent, name, false)
		{
			setValueSlot.setFunction({ [this](const T& value) { this->setValue(value); } });
		}

		// Constructor to declare an attribute with a member function pointer for the @valueChangedSignal as last argument.
		template <typename U, typename F>
		Attribute(U* parent, const std::string& name, const T& inValue, F function, bool atomic = false)
			: AttributeBase(parent, name, atomic), mValue(inValue)
		{
			setValueSlot.setFunction({ [this](const T& value) { this->setValue(value); } });
			valueChangedSignal.connect(parent, function);
		}

		virtual const RTTI::TypeInfo getValueType() const override;

		// Getters
		virtual void getValue(AttributeBase& inAttribute) const override;
		const T& getValue() const;
		T& getValueRef();

		// Setters
		virtual void setValue(const AttributeBase &inAttribute) override;
		virtual void setValue(const T& inValue);

		// Connect to
		virtual void connectToValue(Slot<const T&>& inSlot);
		virtual void disconnectFromValue(Slot<const T&>& inSlot);

		// Inherited from BaseAttribute, so that BaseAttribute can trigger the valueChanged() signal to be emitted
		void emitValueChanged() override final { valueChangedSignal(mValue); }

		// Signal emited when the value changes
		Signal<const T&>	valueChangedSignal;
		// This slot will be invoked when the value is set
		Slot<const T&>		setValueSlot;

		// Operator overloads
		operator const T&() const { return getValue(); }

	protected:
		// Members
		T				mValue;
		Link& getLink() const override { return mLink; }

	private:
		// Keep constructor hidden, use factory methods to instantiate
		Attribute(const T& inValue) : mValue(inValue) {}

		// Link of type T
		mutable TypedLink<Attribute<T>> mLink = { *this };
	};


	/**
	Specialization of the default Attribute
	**/
	template<typename T>
	class NumericAttribute : public Attribute<T>
	{
		RTTI_ENABLE_DERIVED_FROM(Attribute<T>)
	public:
		// Default constructor
		NumericAttribute() : Attribute<T>() { }

		// Constructor with defult value and min max
		NumericAttribute(AttributeObject* parent, const std::string& name, const T& value, const T& minValue, const T& maxValue, bool atomic = false, bool clamped = true);

		// Constructor with default value and no min / max
		NumericAttribute(AttributeObject* parent, const std::string& name, const T& value, bool atomic = false);

		// Constructor to declare an attribute with a member function pointer for the @valueChangedSignal as last argument.
		// TODO: REMOVE THIS UGLY OVER TEMPLATED CONSTRUCTOR
		template <typename U, typename F>
		NumericAttribute(U* parent, const std::string& name, const T& inValue, const T& minValue, const T& maxValue, F function, bool atomic = false, bool clamped = true)
			: Attribute<T>(parent, name, inValue, function, atomic)
		{
			setRange(minValue, maxValue);
			setClamped(clamped);
		}

		// Setters
		void setValue(const T& value) override;
		void setRange(const T& min, const T& max);
		void setClamped(bool value);

		// Getters
		bool isClamped()	const { return mClamped; }
		T getMin() const { return mMinValue; }
		T getMax() const { return mMaxValue; }
		void getRange(T& outMin, T& outMax) const;

		// Signals
		Signal<const NumericAttribute<T>&> rangeChanged;

		// Clamp function
		T clampValue(const T& value, const T& min, const T& max);

	private:
		// Range
		T mMinValue;
		T mMaxValue;

		bool mClamped = false;
	};


	/**
	SignalAttribute

	Acts as a bang, connect to @signal signal to receive !bang!
	**/
	class SignalAttribute : public AttributeBase
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeBase)
	public:
		SignalAttribute() = default;
		SignalAttribute(AttributeObject* parent, const std::string& name) : AttributeBase(parent, name) { }

		/**
		 * signal emitted on trigger
		 */
		nap::Signal<const nap::SignalAttribute&> signal;

		/**
		 * triggers the signal to be emitted, convenience method
		 * similar to signal.trigger
		 */
		void trigger() { signal.trigger(*this); }

	protected:
		virtual Link& getLink() const override { return mlink; }
		virtual void emitValueChanged() override { signal.trigger(*this); }

	private:
		virtual void getValue(AttributeBase& attribute) const override {}
		virtual void setValue(const AttributeBase& attribute) override {}
		virtual const RTTI::TypeInfo getValueType() const override { return getTypeInfo(); }

		mutable nap::Link				mlink;
	};


	/**
	 * ObjectLinkAttribute
	 *
	 * Attribute that acts as a link to an other object
	 */
	class ObjectLinkAttribute : public AttributeBase
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeBase)
	public:
		// Default constructor
		ObjectLinkAttribute();
		ObjectLinkAttribute(AttributeObject* parent, const std::string& name, const RTTI::TypeInfo& type);

		// Conversion
		virtual void getValue(AttributeBase& attribute) const override;
		virtual void setValue(const AttributeBase& attribute) override;

		/**
		 * @return the link's target, nullptr if not linked
		 */
		Object* getTarget() { return mLink.getTarget(); }

		/**
		 * @return the link's target as type T, nullptr if not linked or type is invalid
		 */
		template <typename T>
		T* getTarget();

		/**
		 * @return the link's target object path, empty string if not valid
		 */
		const ObjectPath& getPath()	const { return mLink.getPath(); }

		/**
		 * sets the link's target, emits valueChanged when set
		 * @param target object that is the links new target
		 */
		void setTarget(Object& target);

		/**
		 * sets the link's target by resolving the target path
		 * @param targetPath path to target
		 */
		void setTarget(const std::string& targetPath);

		/**
		 * clears the link
		 */
		void clear() { mLink.clear(); }

		/**
		 * returns type of attribute
		 */
		virtual const RTTI::TypeInfo getValueType() const override;

	protected:
		/**
		* @return link associated with this attribute (TODO: DEPRECATE)
		*/
		virtual Link& getLink() const override { return mLink; }

		/**
		 * TODO: DEPRECATE
		 */
		virtual void emitValueChanged() override {  }

	private:
		// Link to object
		mutable Link mLink;

		void onLinkTargetChanged(const Link& link);
		Slot<const Link&> onLinkTargetChangedSlot = { this, &ObjectLinkAttribute::onLinkTargetChanged };
	};
}

// Include template specialization
#include "attribute.hpp"

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////


RTTI_DECLARE_BASE(nap::AttributeBase)
RTTI_DECLARE(nap::SignalAttribute)
RTTI_DECLARE(nap::ObjectLinkAttribute)

// Create and bind attribute slot with @NAME to @FUNCTION
#define ATTR_SLOT(NAME, FUNCTION) SLOT(NAME, nap::Attribute&, FUNCTION)

// Create attribute slow with @NAME without binding it to a function
#define CREATE_ATTR_SLOT(NAME) CREATE_SLOT(NAME, nap::Attribute&)

// Bind an already created attribute slot with @NAME to @FUNCTION
#define BIND_ATTR_SLOT(NAME, FUNCTION) BIND_SLOT(NAME, nap::Attribute&, FUNCTION)