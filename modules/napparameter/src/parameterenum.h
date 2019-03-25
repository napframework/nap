#pragma once

// External Includes
#include "parameter.h"
#include "nap/signalslot.h"

namespace nap
{
	class ParameterEnumBase : public Parameter
	{
		RTTI_ENABLE(Parameter)

	public:
		ParameterEnumBase(rtti::TypeInfo enumType) :
			mEnumType(enumType)
		{
		}

		virtual int getValue() const = 0;
		virtual void setValue(int value) = 0;
		const rtti::TypeInfo& getEnumType() const { return mEnumType; }

	private:
		rtti::TypeInfo mEnumType;
	};

	template<class T>
	class ParameterEnum : public ParameterEnumBase
	{
		RTTI_ENABLE(ParameterEnumBase)
	
	public:
		ParameterEnum() :
			ParameterEnumBase(RTTI_OF(T))
		{
		}

		virtual int getValue() const override { return (int)mValue; }

		virtual void setValue(int value) override { setValue((T)value); }

		virtual void setValue(const Parameter& value) override
		{
			const ParameterEnum<T>* derived_type = rtti_cast<const ParameterEnum<T>>(&value);
			assert(derived_type != nullptr);

			setValue(derived_type->mValue);
		}

		void setValue(T value)
		{
			T oldValue = mValue;
			mValue = value;
			if (oldValue != mValue)
			{
				valueChanged(mValue);
			}
		}

	public:
		T mValue;
		Signal<T> valueChanged;
	};
}
