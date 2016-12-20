// attribute.h template definitions

namespace nap
{
	template <typename T>
	const RTTI::TypeInfo Attribute<T>::getValueType() const
	{
		return RTTI::TypeInfo::get<T>();
	}


	template <typename T>
	void Attribute<T>::getValue(AttributeBase& inAttribute) const
	{
		static_cast<Attribute<T>&>(inAttribute).setValue(getValue());
	}

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

	template <typename T>
	T& Attribute<T>::getValueRef()
	{
		// When an attribute is atomic calling getValueRef() potentially breaks thread safety
		assert(!mAtomic);

		return mValue;
	}

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

	template <typename T>
	void Attribute<T>::connectToValue(Slot<const T&>& inSlot)
	{
		valueChangedSignal.connect(inSlot);
	}


	template <typename T>
	void Attribute<T>::disconnectFromValue(Slot<const T&>& inSlot)
	{
		valueChangedSignal.disconnect(inSlot);
	}

	//////////////////////////////////////////////////////////////////////////
	// Numeric Attribute Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	void NumericAttribute<T>::setValue(const T& value)
	{
		T new_v = mClamped ? clampValue(value, mMinValue, mMaxValue) : value;
		Attribute<T>::setValue(new_v);
	}


	template <typename T>
	void NumericAttribute<T>::setRange(const T& min, const T& max)
	{
		mMinValue = min;
		mMaxValue = max;
		rangeChanged(*this);
	}


	template <typename T>
	void NumericAttribute<T>::setClamped(bool value)
	{
		// Skip if the same
		if (value == mClamped)
			return;

		// If we're clamping, make sure to update the value
		mClamped = value;
		if (mClamped)
			setValue(Attribute<T>::mValue);
	}

	template <typename T>
	void NumericAttribute<T>::getRange(T& outMin, T& outMax) const
	{
		outMin = mMinValue;
		outMax = mMaxValue;
	}

	template <typename T>
	NumericAttribute<T>::NumericAttribute(AttributeObject* parent, const std::string& name, const T& value, const T& minValue, const T& maxValue, bool atomic, bool clamped)
		: Attribute<T>(parent, name, value, atomic)
	{
		setRange(minValue, maxValue);
		setClamped(clamped);
	}


	// Constructor with default value and no min / max
	template <typename T>
	NumericAttribute<T>::NumericAttribute(AttributeObject* parent, const std::string& name, const T& value, bool atomic)
		: Attribute<T>(parent, name, value, atomic)
	{
		setRange(value, value);
	}

	//////////////////////////////////////////////////////////////////////////
	// Object attribute link definitions
	//////////////////////////////////////////////////////////////////////////
	template <typename T>
	T* ObjectLinkAttribute::getTarget()
	{
		Object* target = getTarget();
		if (target == nullptr)
			return nullptr;
		return static_cast<T*>(target);
	}
}
