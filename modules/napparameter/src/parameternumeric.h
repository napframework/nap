#pragma once

// External Includes
#include "parameter.h"
#include "mathutils.h"
#include "nap/signalslot.h"

namespace nap
{
	/**
	 * A numeric parameter can hold any numeric type and provides a minimum & maximum value to ensure the value stays within range
	 */
	template<typename T>
	class ParameterNumeric : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:

		/**
		 * Set the value of this enum from another parameter
		 *
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * Set the value of this parameter. Will raise the valueChanged signal if the value actually changes.
		 *
		 * @param value The value to set
		 */
		void setValue(T value);

	public:
		T			mValue;							///< Property: 'Value' the value of this parameter
		T			mMinimum = math::min<T>();		///< Property: 'Minimum' the minimum value of this parameter
		T			mMaximum = math::max<T>();		///< Property: 'Maximum' the maximum value of this parameter

		Signal<T>	valueChanged;					///< Signal that's raised when the value of this parameter changes
	};

	/**
	 * A numeric vector parameter can hold any numeric vector type (i.e. glm::vec2, glm::vec3, etc)
	 */
	template<typename T>
	class ParameterNumericVec : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:

		/**
		 * Set the value of this enum from another parameter
		 *
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * Set the value of this parameter. Will raise the valueChanged signal if the value actually changes.
		 *
		 * @param value The value to set
		 */
		void setValue(T value);

	public:
		T						mValue;										        ///< Property: 'Value' the value of this parameter
		typename T::value_type	mMinimum = math::min<typename T::value_type>();		///< Property: 'Minimum' the minimum value of this parameter
		typename T::value_type	mMaximum = math::max<typename T::value_type>();		///< Property: 'Maximum' the maximum value of this parameter

		Signal<T>				valueChanged;								        ///< Signal that's raised when the value changes
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ParameterNumeric<T>::setValue(const Parameter& value)
	{
		const ParameterNumeric<T>* derived_type = rtti_cast<const ParameterNumeric<T>>(&value);
		assert(derived_type != nullptr);

		// Update min & max
		mMinimum = derived_type->mMinimum;
		mMaximum = derived_type->mMaximum;

		// Set value from the parameter
		setValue(derived_type->mValue);
	}

	template<typename T>
	void ParameterNumeric<T>::setValue(T value)
	{
		T oldValue = mValue;
		mValue = math::clamp(value, mMinimum, mMaximum);
		if (oldValue != mValue)
		{
			valueChanged(mValue);
		}
	}

	template<typename T>
	void ParameterNumericVec<T>::setValue(const Parameter& value)
	{
		const ParameterNumericVec<T>* derived_type = rtti_cast<const ParameterNumericVec<T>>(&value);
		assert(derived_type != nullptr);

		mMinimum = derived_type->mMinimum;
		mMaximum = derived_type->mMaximum;

		setValue(derived_type->mValue);
	}

	template<typename T>
	void ParameterNumericVec<T>::setValue(T value)
	{
		T oldValue = mValue;

		// Clamp& set the components for each element individually
		for (int i = 0; i != mValue.length(); ++i)
			mValue[i] = math::clamp(value[i], mMinimum, mMaximum);

		if (oldValue != mValue)
		{
			valueChanged(mValue);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterFloat	= ParameterNumeric<float>;
	using ParameterInt		= ParameterNumeric<int>;
	using ParameterChar		= ParameterNumeric<char>;
	using ParameterByte		= ParameterNumeric<uint8_t>;
	using ParameterDouble	= ParameterNumeric<double>;
	using ParameterLong		= ParameterNumeric<int64_t>;

	using ParameterVec2		= ParameterNumericVec<glm::vec2>;
	using ParameterIVec2	= ParameterNumericVec<glm::ivec2>;
	using ParameterVec3		= ParameterNumericVec<glm::vec3>;
	using ParameterIVec3	= ParameterNumericVec<glm::ivec3>;

	/**
	 * Helper macro that can be used to define the RTTI for a numeric (vector) parameter type
	 */
	#define DEFINE_NUMERIC_PARAMETER(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Value",		&Type::mValue,			nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("Minimum",	&Type::mMinimum,		nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("Maximum",	&Type::mMaximum,		nap::rtti::EPropertyMetaData::Default)				\
		RTTI_END_CLASS
}
