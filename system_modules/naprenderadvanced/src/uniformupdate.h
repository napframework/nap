/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <uniforminstance.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametercolor.h>

namespace nap
{
	/**
	 * Register Uniform update on Parameter::valueChanged signal
 	 * TypedUniformValueInstance x ParameterNumeric
	 */
	template<typename T>
	static void registerUniformUpdate(TypedUniformValueInstance<T>& uniformInstance, ParameterNumeric<T>& parameter)
	{
		uniformInstance.setValue(parameter.mValue);
		parameter.valueChanged.connect([uni = &uniformInstance](T value) { uni->setValue(value); });
	}


	/**
	 * Register Uniform update on Parameter::valueChanged signal
	 * TypedUniformValueInstance x ParameterVec
	 */
	template<typename T>
	static void registerUniformUpdate(TypedUniformValueInstance<T>& uniformInstance, ParameterVec<T>& parameter)
	{
		uniformInstance.setValue(parameter.mValue);
		parameter.valueChanged.connect([uni = &uniformInstance](T value) { uni->setValue(value); });
	}


	/**
	 * Register Uniform update on Parameter::valueChanged signal
	 * UniformVec3Instance x ParameterRGBColorFloat
	 */
	static void registerUniformUpdate(UniformVec3Instance& uniformInstance, ParameterRGBColorFloat& parameter)
	{
		uniformInstance.setValue(parameter.mValue.toVec3());
		parameter.valueChanged.connect([uni = &uniformInstance](RGBColorFloat value) { uni->setValue(value.toVec3()); });
	}


	/**
	 * Register Uniform update on Parameter::valueChanged signal
	 * UniformVec4Instance x ParameterRGBAColorFloat
	 */
	static void registerUniformUpdate(UniformVec4Instance& uniformInstance, ParameterRGBAColorFloat& parameter)
	{
		uniformInstance.setValue(parameter.mValue.toVec4());
		parameter.valueChanged.connect([uni = &uniformInstance](RGBAColorFloat value) { uni->setValue(value.toVec4()); });
	}


	/**
	 * Register Uniform update on Parameter::valueChanged signal
	 * UniformUIntInstance x ParameterBool
	 */
	static void registerUniformUpdate(UniformUIntInstance& uniformInstance, ParameterBool& parameter)
	{
		uniformInstance.setValue(parameter.mValue);
		parameter.valueChanged.connect([uni = &uniformInstance](bool value) { uni->setValue(value); });
	}
}
