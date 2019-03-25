#pragma once

// External Includes
#include "parameter.h"
#include "nap/signalslot.h"
#include "../../naprender/src/color.h"

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

	//////////////////////////////////////////////////////////////////////////
	// API Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterRGBColorFloat = ParameterSimple<RGBColorFloat>;
	using ParameterBool = ParameterSimple<bool>;
}
