/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "parameterguiservice.h"

// External includes
#include <rtti/rttiutilities.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parameterstring.h>
#include <parameterenum.h>
#include <parametercolor.h>
#include <parameterquat.h>
#include <imgui/imgui.h>
#include <parameterservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterGUIService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Helper function to display the UI for a float parameter (either float or double)
	 */
	template<class PARAMETERTYPE>
	static void showFloatParameter(PARAMETERTYPE& parameter)
	{
		float value = parameter.mValue;
		if (ImGui::SliderFloat(parameter.getDisplayName().c_str(), &value, parameter.mMinimum, parameter.mMaximum))
			parameter.setValue(value);
	}


	/**
	 * Helper function to display the UI for an integer parameter (i.e. all non-floating point types)
	 */
	template<class PARAMETERTYPE>
	static void showIntParameter(PARAMETERTYPE& parameter)
	{
		int value = parameter.mValue;
		if (ImGui::SliderInt(parameter.getDisplayName().c_str(), &value, parameter.mMinimum, parameter.mMaximum))
			parameter.setValue(value);
	}


	void ParameterGUIService::registerDefaultParameterEditors()
	{
		registerParameterEditor(RTTI_OF(ParameterFloat), [](Parameter& parameter)
		{
			ParameterFloat* float_parameter = rtti_cast<ParameterFloat>(&parameter);
			showFloatParameter(*float_parameter);
		});

		registerParameterEditor(RTTI_OF(ParameterDouble), [](Parameter& parameter)
		{
			ParameterDouble* double_parameter = rtti_cast<ParameterDouble>(&parameter);
			showFloatParameter(*double_parameter);
		});

		registerParameterEditor(RTTI_OF(ParameterInt), [](Parameter& parameter)
		{
			ParameterInt* int_parameter = rtti_cast<ParameterInt>(&parameter);
			showIntParameter(*int_parameter);
		});

		registerParameterEditor(RTTI_OF(ParameterLong), [](Parameter& parameter)
		{
			ParameterLong* long_parameter = rtti_cast<ParameterLong>(&parameter);
			showIntParameter(*long_parameter);
		});

		registerParameterEditor(RTTI_OF(ParameterByte), [](Parameter& parameter)
		{
			ParameterByte* byte_parameter = rtti_cast<ParameterByte>(&parameter);
			showIntParameter(*byte_parameter);
		});

		registerParameterEditor(RTTI_OF(ParameterBool), [](Parameter& parameter)
		{
			ParameterBool* bool_parameter = rtti_cast<ParameterBool>(&parameter);

			bool value = bool_parameter->mValue;
			if (ImGui::Checkbox(bool_parameter->getDisplayName().c_str(), &value))
				bool_parameter->setValue(value);
		});

		registerParameterEditor(RTTI_OF(ParameterString), [](Parameter& parameter)
		{
			ParameterString* string_parameter = rtti_cast<ParameterString>(&parameter);

			std::string value = std::string(string_parameter->mValue.data(), string_parameter->mValue.capacity());
			if (ImGui::InputText(string_parameter->getDisplayName().c_str(), &value[0], string_parameter->mSize))
				string_parameter->setValue(value);
		});

		registerParameterEditor(RTTI_OF(ParameterRGBColorFloat), [](Parameter& parameter)
		{
			ParameterRGBColorFloat* color_parameter = rtti_cast<ParameterRGBColorFloat>(&parameter);

			RGBColorFloat value = color_parameter->mValue;
			if (ImGui::ColorEdit3(color_parameter->getDisplayName().c_str(), value.getData()))
				color_parameter->setValue(value);
		});

		registerParameterEditor(RTTI_OF(ParameterRGBAColorFloat), [](Parameter& parameter)
		{
			ParameterRGBAColorFloat* color_parameter = rtti_cast<ParameterRGBAColorFloat>(&parameter);

			RGBAColorFloat value = color_parameter->mValue;
			if (ImGui::ColorEdit4(color_parameter->getDisplayName().c_str(), value.getData()))
				color_parameter->setValue(value);
		});

		registerParameterEditor(RTTI_OF(ParameterRGBColor8), [](Parameter& parameter)
		{
			ParameterRGBColor8* color_parameter = rtti_cast<ParameterRGBColor8>(&parameter);

			RGBColorFloat value = color_parameter->mValue.convert<RGBColorFloat>();
			if (ImGui::ColorEdit3(color_parameter->getDisplayName().c_str(), value.getData()))
				color_parameter->setValue(value.convert<RGBColor8>());
		});

		registerParameterEditor(RTTI_OF(ParameterRGBAColor8), [](Parameter& parameter)
		{
			ParameterRGBAColor8* color_parameter = rtti_cast<ParameterRGBAColor8>(&parameter);

			RGBAColorFloat value = color_parameter->mValue.convert<RGBAColorFloat>();
			if (ImGui::ColorEdit4(color_parameter->getDisplayName().c_str(), value.getData()))
				color_parameter->setValue(value.convert<RGBAColor8>());
		});

		registerParameterEditor(RTTI_OF(ParameterVec2), [](Parameter& parameter)
		{
			ParameterVec2* vec2_parameter = rtti_cast<ParameterVec2>(&parameter);
			glm::vec2 value = vec2_parameter->mValue;
			if (vec2_parameter->mClamp)
			{
				if (ImGui::SliderFloat2(vec2_parameter->getDisplayName().c_str(), &(value[0]), vec2_parameter->mMinimum, vec2_parameter->mMaximum))
					vec2_parameter->setValue(value);
			}
			else
			{
				if (ImGui::InputFloat2(vec2_parameter->getDisplayName().c_str(), &(value[0])))
					vec2_parameter->setValue(value);
			}
		});

		registerParameterEditor(RTTI_OF(ParameterIVec2), [](Parameter& parameter)
		{
			ParameterIVec2* vec2_parameter = rtti_cast<ParameterIVec2>(&parameter);
			glm::ivec2 value = vec2_parameter->mValue;
			if (vec2_parameter->mClamp)
			{
				if (ImGui::SliderInt3(vec2_parameter->getDisplayName().c_str(), &value[0], vec2_parameter->mMinimum, vec2_parameter->mMaximum))
					vec2_parameter->setValue(value);
			}
			else
			{
				if (ImGui::InputInt2(vec2_parameter->getDisplayName().c_str(), &value[0]))
					vec2_parameter->setValue(value);
			}
		});

		registerParameterEditor(RTTI_OF(ParameterVec3), [](Parameter& parameter)
		{
			ParameterVec3* vec3_parameter = rtti_cast<ParameterVec3>(&parameter);
			glm::vec3 value = vec3_parameter->mValue;
			if (vec3_parameter->mClamp)
			{
				if (ImGui::SliderFloat3(vec3_parameter->getDisplayName().c_str(), &(value[0]), vec3_parameter->mMinimum, vec3_parameter->mMaximum))
					vec3_parameter->setValue(value);
			}
			else
			{
				if (ImGui::InputFloat3(vec3_parameter->getDisplayName().c_str(), &(value[0])))
					vec3_parameter->setValue(value);
			}
		});

		registerParameterEditor(RTTI_OF(ParameterQuat), [](Parameter& parameter)
		{
			ParameterQuat* quat_parameter = rtti_cast<ParameterQuat>(&parameter);

			glm::quat value = quat_parameter->mValue;
			if (ImGui::InputFloat4(quat_parameter->getDisplayName().c_str(), &(value[0])))
				quat_parameter->setValue(value);
		});

		registerParameterEditor(RTTI_OF(ParameterIVec3), [](Parameter& parameter)
		{
			ParameterIVec3* vec3_parameter = rtti_cast<ParameterIVec3>(&parameter);
			glm::ivec3 value = vec3_parameter->mValue;
			if (vec3_parameter->mClamp)
			{
				if (ImGui::SliderInt3(vec3_parameter->getDisplayName().c_str(), &value[0], vec3_parameter->mMinimum, vec3_parameter->mMaximum))
					vec3_parameter->setValue(value);
			}
			else
			{
				if (ImGui::InputInt3(vec3_parameter->getDisplayName().c_str(), &value[0]))
					vec3_parameter->setValue(value);
			}
		});

		registerParameterEditor(RTTI_OF(ParameterEnumBase), [](Parameter& parameter)
		{
			ParameterEnumBase* enum_parameter = rtti_cast<ParameterEnumBase>(&parameter);
			const rtti::TypeInfo& enum_type = enum_parameter->getEnumType();
			assert(enum_type.is_enumeration());

			rttr::enumeration enum_instance = enum_type.get_enumeration();
			std::vector<rttr::string_view> items(enum_instance.get_names().begin(), enum_instance.get_names().end());

			int value = enum_parameter->getValue();
			if (ImGui::Combo(parameter.getDisplayName().c_str(), &value, [](void* data, int index, const char** out_text)
			{
				std::vector<rttr::string_view>* items = (std::vector<rttr::string_view>*)data;
				*out_text = (*items)[index].data();
				return true;
			}, &items, items.size()))
			{
				enum_parameter->setValue(value);
			}
		});
	}


	ParameterGUIService::ParameterGUIService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	void ParameterGUIService::registerParameterEditor(const rtti::TypeInfo& type, const CreateParameterEditor& createParameterEditorFunc)
	{
		// Get all types associated with given type
		std::vector<rtti::TypeInfo> types;
		types.push_back(type);
		rtti::getDerivedTypesRecursive(type, types);

		// Add callback
		for (const rtti::TypeInfo& type : types)
			mParameterEditors[type] = createParameterEditorFunc;
	}


	const nap::ParameterGUIService::CreateParameterEditor* ParameterGUIService::findEditor(rtti::TypeInfo parameterType) const
	{
		auto it = mParameterEditors.find(parameterType);
		return it != mParameterEditors.end() ? &(it->second) : nullptr;
	}


	const nap::ParameterGUIService::CreateParameterEditor* ParameterGUIService::findEditor(const nap::Parameter& parameterType) const
	{
		return findEditor(parameterType.get_type());
	}


	bool ParameterGUIService::init(nap::utility::ErrorState& errorState)
	{
		registerDefaultParameterEditors();
		return true;
	}


	void ParameterGUIService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(ParameterService));
	}
}
