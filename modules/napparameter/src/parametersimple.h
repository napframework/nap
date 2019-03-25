#pragma once

// External Includes
#include "parameter.h"
#include "nap/signalslot.h"

namespace nap
{
	template<typename T>
	class ParameterSimple : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		virtual void setValue(const Parameter& value) override
		{
			const ParameterSimple<T>* derived_type = rtti_cast<const ParameterSimple<T>>(&value);
			assert(derived_type != nullptr);
			setValue(derived_type->mValue);
		}

		void setValue(const T& value)
		{
			if (value != mValue)
			{
				mValue = value;
				valueChanged(mValue);
			}
		}

	public:
		T mValue;
		Signal<T> valueChanged;
	};

	using ParameterBool = ParameterSimple<bool>;
}


#define DEFINE_SIMPLE_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,		nap::rtti::EPropertyMetaData::Default)					\
	RTTI_END_CLASS