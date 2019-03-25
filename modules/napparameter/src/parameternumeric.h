#pragma once

// External Includes
#include "parameter.h"
#include "mathutils.h"
#include "nap/signalslot.h"

namespace nap
{
	/**
	* 
	*/
	template<typename T>
	class ParameterNumeric : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		virtual void setValue(const Parameter& value) override
		{
			const ParameterNumeric<T>* derived_type = rtti_cast<const ParameterNumeric<T>>(&value);
			assert(derived_type != nullptr);

			mMinimum = derived_type->mMinimum;
			mMaximum = derived_type->mMaximum;

			setValue(derived_type->mValue);
		}

		void setValue(T value)
		{
			T oldValue = mValue;
			mValue = math::clamp(value, mMinimum, mMaximum);
			if (oldValue != mValue)
			{
				valueChanged(mValue);
			}
		}

	public:
		T	mValue;											///< managed value
		T	mMinimum = std::numeric_limits<T>::min();
		T	mMaximum = std::numeric_limits<T>::max();

		Signal<T> valueChanged;
	};

	template<typename T>
	class ParameterNumericVec : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		virtual void setValue(const Parameter& value) override
		{
			const ParameterNumericVec<T>* derived_type = rtti_cast<const ParameterNumericVec<T>>(&value);
			assert(derived_type != nullptr);

			mMinimum = derived_type->mMinimum;
			mMaximum = derived_type->mMaximum;

			setValue(derived_type->mValue);
		}

		void setValue(T value)
		{
			T oldValue = mValue;
			for (int i = 0; i != mValue.length(); ++i)
				mValue[i] = math::clamp(value[i], mMinimum, mMaximum);

			if (oldValue != mValue)
			{
				valueChanged(mValue);
			}
		}

	public:
		T	mValue;											///< managed value
		typename T::value_type	mMinimum = std::numeric_limits<typename T::value_type>::min();
		typename T::value_type	mMaximum = std::numeric_limits<typename T::value_type>::max();

		Signal<T> valueChanged;
	};


	//////////////////////////////////////////////////////////////////////////
	// API Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterFloat = ParameterNumeric<float>;
	using ParameterInt = ParameterNumeric<int>;
	using ParameterChar = ParameterNumeric<char>;
	using ParameterByte = ParameterNumeric<uint8_t>;
	using ParameterDouble = ParameterNumeric<double>;
	using ParameterLong = ParameterNumeric<int64_t>;

	using ParameterVec2 = ParameterNumericVec<glm::vec2>;
	using ParameterIVec2 = ParameterNumericVec<glm::ivec2>;
	using ParameterVec3 = ParameterNumericVec<glm::vec3>;
}
