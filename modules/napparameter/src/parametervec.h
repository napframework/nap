#pragma once

// Local Includes
#include "mathutils.h"

// External Includes
#include <parameter.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * A numeric vector parameter can hold any numeric vector type (i.e. glm::vec2, glm::vec3, etc)
	 * The values associated with the vector are clamped to the specified min and max bounds.
	 */
	template<typename T>
	class ParameterVec : public Parameter
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
		void setValue(const T& value);

		/**
		 * Sets the min/max range of this parameter to the specified values.
		 * If the current value is outside of the specified range, it will be clamped.
		 *
		 * @param minimum The minimum value for this parameter
		 * @param maximum The maximum value for this parameter
		 */
		void setRange(typename T::value_type minimum, typename T::value_type maximum);

        /**
         * Connects an action to the parameter's valueChanged signal and calls it straightaway, to make sure the connected system is in sync with the parameter's current value.
         * ActionType can be of any type that can be connected to the valueChanged signal.
         */
        template <typename ActionType>
        void connect(ActionType action)
        {
            valueChanged.connect(std::forward<ActionType>(action));
            action(mValue);
        }

    public:
		T						mValue;										        ///< Property: 'Value' the value of this parameter
		bool					mClamp = false;										///< Property: 'Clamp' if the vector is clamped to the min / max value
		typename T::value_type	mMinimum = static_cast<typename T::value_type>(0);	///< Property: 'Minimum' the minimum value of this parameter
		typename T::value_type	mMaximum = static_cast<typename T::value_type>(1);	///< Property: 'Maximum' the maximum value of this parameter

		Signal<const T&>				valueChanged;								///< Signal that's raised when the value changes
	};


	//////////////////////////////////////////////////////////////////////////
	// Vector Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterVec2		= ParameterVec<glm::vec2>;
	using ParameterVec3		= ParameterVec<glm::vec3>;
	using ParameterIVec2	= ParameterVec<glm::ivec2>;
	using ParameterIVec3	= ParameterVec<glm::ivec3>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ParameterVec<T>::setValue(const Parameter& value)
	{
		const ParameterVec<T>* derived_type = rtti_cast<const ParameterVec<T>>(&value);
		assert(derived_type != nullptr);
		setValue(derived_type->mValue);
	}

	template<typename T>
	void ParameterVec<T>::setValue(const T& value)
	{
		// Clamp& set the components for each element individually
		T oldValue = mValue;

		if (mClamp) 
		{
			for (int i = 0; i != mValue.length(); ++i)
				mValue[i] = math::clamp(value[i], mMinimum, mMaximum);
		}
		else
		{
			mValue = value;
		}

		if (oldValue != mValue)
			valueChanged(mValue);
	}


	template<typename T>
	void nap::ParameterVec<T>::setRange(typename T::value_type minimum, typename T::value_type maximum)
	{
		mMinimum = minimum;
		mMaximum = maximum;
		setValue(mValue);
	}

	/**
	 * Helper macro that can be used to define the RTTI for a vector parameter type
	 */
	 #ifdef NAP_ENABLE_PYTHON
	    #define DEFINE_VECTOR_PARAMETER(Type)																		\
		    RTTI_BEGIN_CLASS(Type)																					\
			    RTTI_PROPERTY("Value",		&Type::mValue,			nap::rtti::EPropertyMetaData::Default)			\
			    RTTI_PROPERTY("Clamp",		&Type::mClamp,			nap::rtti::EPropertyMetaData::Default)			\
			    RTTI_PROPERTY("Minimum",	&Type::mMinimum,		nap::rtti::EPropertyMetaData::Default)			\
			    RTTI_PROPERTY("Maximum",	&Type::mMaximum,		nap::rtti::EPropertyMetaData::Default)			\
    		    RTTI_FUNCTION("setValue", static_cast<void (Type::*)(const decltype(Type::mValue)&)>(&Type::setValue))\
    		    RTTI_FUNCTION("connect", &Type::connect<pybind11::function>) \
		    RTTI_END_CLASS
     #else
        #define DEFINE_VECTOR_PARAMETER(Type)																		\
		    RTTI_BEGIN_CLASS(Type)																					\
			    RTTI_PROPERTY("Value",		&Type::mValue,			nap::rtti::EPropertyMetaData::Default)			\
			    RTTI_PROPERTY("Clamp",		&Type::mClamp,			nap::rtti::EPropertyMetaData::Default)			\
			    RTTI_PROPERTY("Minimum",	&Type::mMinimum,		nap::rtti::EPropertyMetaData::Default)			\
			    RTTI_PROPERTY("Maximum",	&Type::mMaximum,		nap::rtti::EPropertyMetaData::Default)			\
    		    RTTI_FUNCTION("setValue", static_cast<void (Type::*)(const decltype(Type::mValue)&)>(&Type::setValue))\
		    RTTI_END_CLASS
     #endif
}
