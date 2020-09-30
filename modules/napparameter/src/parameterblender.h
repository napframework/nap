/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/rtti.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametercolor.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Registration
	//////////////////////////////////////////////////////////////////////////

	// Forward Declares
	class BaseParameterBlender;

	/**
	 * @return a blender for the given parameter, nullptr if no blender is available.
	 */
	std::unique_ptr<BaseParameterBlender> NAPAPI getParameterBlender(nap::Parameter& param);

	/**
	 * Register a new blender for a specific type of parameter.
	 * Note that the parameter type needs to be derived from nap::Parameter and
	 * the blender type from nap::BaseParameterBlender.
	 * @param inParameterType the type of parameter that is blended.
	 * @param inBlenderType the blender type.
	 * @return if the blender for the given parameter registered.
	 */
	bool NAPAPI registerParameterBlender(rtti::TypeInfo inParameterType, rtti::TypeInfo inBlenderType);


	//////////////////////////////////////////////////////////////////////////
	// Base Parameter Blender
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all parameter blenders. 
	 * Blends a parameter (source) towards a target parameter based on a lerp value from 0 to 1.
	 * Override the virtual methods to create a blender for a specific parameter type.
	 */
	class NAPAPI BaseParameterBlender
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructs the blender based on the given parameter.
		 * This parameter is updated based on the target blend value.
		 * @param parameter the parameter to update.
		 */
		BaseParameterBlender(Parameter& parameter);
		
		// Default destructor
		virtual ~BaseParameterBlender() = default;

		/**
		 * Blends parameters based on the given value (0-1).
		 * Result is immediately applied to the parameter.
		 * @param value normalized blend value 
		 */
		void blend(float value);

		/**
		 * Sets the target parameter.
		 * Note that the target must be of the same type as the source parameter
		 * @param target parameter to blend towards
		 */
		void setTarget(const Parameter* target);

		/**
		 * Clears blending target
		 */
		void clearTarget();

		/**
		 * Ensures that subsequent calls to blend() are computed relative to the current parameter value.
		 */
		void sync();

		/**
		 * @return if the blender has a target
		 */
		bool hasTarget() const;

		/**
		 * @return target parameter
		 */
		const Parameter& getTarget() const;

		/**
		 * @return the parameter this blender updates
		 */
		const Parameter& getParameter() const;

		/**
		 * @return target parameter as parameter of type T, asserts if target is not of type T
		 */
		template<typename T>
		const T& getTarget() const;

		/**
		 * @return parameter as parameter of type T, asserts if the parameter is not of type T
		 */
		template<typename T>
		T& getParameter();

	protected:
		/**
		 * Called after calling blend().
		 * Override in derived class to correctly blend parameter from source to target.
		 * @param value the blend value (0-1)
		 */
		virtual void onBlend(float value) = 0;

		/**
		 * Called after calling sync().
		 * Override in derived class to sync the current parameter source value.
		 */
		virtual void onSync() = 0;

		/**
		 * Called when a new target is set, only occurs when target is derived from the source parameter.
		 */
		virtual void onTargetSet() = 0;

	private:
		Parameter* mParameter = nullptr;
		const Parameter* mTarget = nullptr;
	};


	//////////////////////////////////////////////////////////////////////////
	// Default Parameter Blender
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Default (float, vec2, double, etc.) parameter blender.
	 * Blends a parameter (source) towards a target parameter based on a lerp value from 0 to 1.
	 * This blender uses the default math::lerp function to interpolate between the values.
	 * The ParameterType class should have an 'mValue' member to bind to.
	 * Provide an ParameterBlender::onBlend() specialization if no lerp function is available.
	 */
	template<typename ParamType, typename ValueType>
	class ParameterBlender : public BaseParameterBlender
	{
		RTTI_ENABLE(BaseParameterBlender)
	public:
		/**
		* Constructs the default parameter blender
		* @param parameter the parameter that is updated based on the computed blend value.
		*/
		ParameterBlender(ParamType& parameter);

		/**
		 * Constructs the default parameter blender based on base type
		 * @param parameter the parameter that is updated based on the computed blend value.
		 */
		ParameterBlender(Parameter& parameter);

	protected:
		/**
		* Blends the numeric parameter based on current target value.
		* The default implementation uses the math::lerp<T> function.
		* Provide a specialization if required for your own parameter type.
		* @param value the blend value (0-1)
		*/
		virtual void onBlend(float value) override;

		/**
		 * Ensures that subsequent calls to blend() are based on the current parameter value.
		 */
		virtual void onSync() override;

		/**
		* Occurs when the target is updated, ensures the current blend value is cached.
		* @param target new parameter target to blend towards.
		*/
		virtual void onTargetSet() override;

	private:
		ValueType mSourceValue;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type declarations for all supported (out of the box) parameter blenders
	//////////////////////////////////////////////////////////////////////////

	using ParameterFloatBlender		= ParameterBlender<ParameterFloat, float>;
	using ParameterDoubleBlender	= ParameterBlender<ParameterDouble, double>;
	using ParameterVec2Blender		= ParameterBlender<ParameterVec2, glm::vec2>;
	using ParameterVec3Blender		= ParameterBlender<ParameterVec3, glm::vec3>;
	using ParameterRGBAFloatBlender = ParameterBlender<ParameterRGBAColorFloat, RGBAColorFloat>;
	using ParameterRGBFloatBlender	= ParameterBlender<ParameterRGBColorFloat, RGBColorFloat>;
	using ParameterRGBA8Blender		= ParameterBlender<ParameterRGBAColor8, RGBAColor8>;
	using ParameterRGB8Blender		= ParameterBlender<ParameterRGBColor8, RGBColor8>;



	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	const T& nap::BaseParameterBlender::getTarget() const
	{
		assert(mTarget != nullptr);
		assert(mTarget->get_type().is_derived_from(RTTI_OF(T)));
		return static_cast<const T&>(*mTarget);
	}

	template<typename T>
	T& nap::BaseParameterBlender::getParameter()
	{
		assert(mParameter != nullptr);
		assert(mParameter->get_type().is_derived_from(RTTI_OF(T)));
		return static_cast<T&>(*mParameter);
	}

	template<typename ParamType, typename ValueType>
	nap::ParameterBlender<ParamType, ValueType>::ParameterBlender(ParamType& parameter) : BaseParameterBlender(parameter)
	{
		this->mSourceValue = parameter.mValue;
	}

	template<typename ParamType, typename ValueType>
	nap::ParameterBlender<ParamType, ValueType>::ParameterBlender(Parameter& parameter) : BaseParameterBlender(parameter)
	{
		assert(parameter.get_type().is_derived_from(RTTI_OF(ParamType)));
		this->mSourceValue = this->getParameter<ParamType>().mValue;
	}

	template<typename ParamType, typename ValueType>
	void nap::ParameterBlender<ParamType, ValueType>::onBlend(float value)
	{
		ValueType lval = math::lerp<ValueType>(mSourceValue, getTarget<ParamType>().mValue, value);
		getParameter<ParamType>().setValue(lval);
	}

	template<typename ParamType, typename ValueType>
	void nap::ParameterBlender<ParamType, ValueType>::onTargetSet()
	{
		onSync();
	}

	template<typename ParamType, typename ValueType>
	void nap::ParameterBlender<ParamType, ValueType>::onSync()
	{
		const ParamType& source = getParameter<ParamType>();
		this->mSourceValue = source.mValue;
	}

	template<>
	void nap::ParameterBlender<ParameterRGBAColorFloat, RGBAColorFloat>::onBlend(float value);

	template<>
	void nap::ParameterBlender<ParameterRGBColorFloat, RGBColorFloat>::onBlend(float value);

	template<>
	void nap::ParameterBlender<ParameterRGBAColor8, RGBAColor8>::onBlend(float value);

	template<>
	void nap::ParameterBlender<ParameterRGBColor8, RGBColor8>::onBlend(float value);
}
