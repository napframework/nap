/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <parametergui.h>
#include <parameter.h>
#include <parameterservice.h>
#include <imgui/imgui.h>
#include <rtti/rttiutilities.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parameterenum.h>
#include <parametercolor.h>
#include <parameterquat.h>

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


	//////////////////////////////////////////////////////////////////////////

	ParameterGUI::ParameterGUI(ParameterService& parameterService) :
		mParameterService(parameterService)
	{
		mParameterGroups = mParameterService.getParameterGroups();
		
		registerDefaultParameterEditors();
	}


	void ParameterGUI::registerDefaultParameterEditors()
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

		registerParameterEditor(RTTI_OF(ParameterChar), [](Parameter& parameter)
		{
			ParameterChar* char_parameter = rtti_cast<ParameterChar>(&parameter);
			showIntParameter(*char_parameter);
		});

		registerParameterEditor(RTTI_OF(ParameterBool), [](Parameter& parameter)
		{
			ParameterBool* bool_parameter = rtti_cast<ParameterBool>(&parameter);

			bool value = bool_parameter->mValue;
			if (ImGui::Checkbox(bool_parameter->getDisplayName().c_str(), &value))
				bool_parameter->setValue(value);
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


	void ParameterGUI::handleLoadPresetPopup()
	{
		if (ImGui::BeginPopupModal("Load", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			mPresets = mParameterService.getPresets(*mParameterGroups[mSelectedGroupIndex].mGroup);
			if (!mPresets.empty() && mSelectedPresetIndex == -1)
				mSelectedPresetIndex = 0;

			ImGui::Combo("Presets", &mSelectedPresetIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* preset_files = (ParameterService::PresetFileList*)data;
				*out_text = (*preset_files)[index].data();
				return true;
			}, &mPresets, mPresets.size());

			if (ImGui::Button("OK"))
			{
				utility::ErrorState errorState;
				if (mParameterService.loadPreset(*mParameterGroups[mSelectedGroupIndex].mGroup, mPresets[mSelectedPresetIndex], errorState))
					ImGui::CloseCurrentPopup();
				else
					ImGui::OpenPopup("Failed to load preset");

				if (ImGui::BeginPopupModal("Failed to load preset"))
				{
					ImGui::Text(errorState.toString().c_str());
					if (ImGui::Button("OK"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				restorePresetState();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}


	bool ParameterGUI::handleNewPresetPopup(std::string& outNewFilename)
	{
		bool result = false;

		if (ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char name[256] = { 0 };
			ImGui::InputText("Name", name, 256);

			if (ImGui::Button("OK") && strlen(name) != 0)
			{
				outNewFilename = std::string(name, strlen(name));
				outNewFilename += ".json";
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		return result;
	}


	void ParameterGUI::handleSaveAsPresetPopup()
	{
		if (ImGui::BeginPopupModal("Save As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::Combo("Presets", &mSelectedPresetIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* preset_files = (ParameterService::PresetFileList*)data;
				*out_text = (*preset_files)[index].data();
				return true;
			}, &mPresets, mPresets.size()))
			{
				if (mSelectedPresetIndex == mPresets.size() - 1)
				{
					ImGui::OpenPopup("New");
				}
			}

			std::string newFilename;
			if (handleNewPresetPopup(newFilename))
			{
				// Insert before the '<new...>' item
				mPresets.insert(mPresets.end() - 1, newFilename);
			}

			ImGui::SameLine();

			if (ImGui::Button("OK"))
			{
				utility::ErrorState errorState;
				if (mParameterService.savePreset(*mParameterGroups[mSelectedGroupIndex].mGroup, mPresets[mSelectedPresetIndex], errorState))
				{
					ImGui::CloseCurrentPopup();
					std::string previous_selection = mPresets[mSelectedPresetIndex];

					mPresets = mParameterService.getPresets(*mParameterGroups[mSelectedGroupIndex].mGroup);

					// After we have retrieved the filenames from the service, the list may be in a different order,
					// so we search for the item in the list to find the selected index.
					for (int index = 0; index < mPresets.size(); ++index)
					{
						if (mPresets[index] == previous_selection)
						{
							mSelectedPresetIndex = index;
							break;
						}
					}

				}
				else
				{
					ImGui::OpenPopup("Failed to save preset");
				}

				if (ImGui::BeginPopupModal("Failed to save preset"))
				{
					ImGui::Text(errorState.toString().c_str());
					if (ImGui::Button("OK"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				restorePresetState();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}


	void ParameterGUI::savePresetState()
	{
		mPrevSelectedPresetIndex = mSelectedPresetIndex;
		mPrevPresets = mPresets;
	}


	void ParameterGUI::restorePresetState()
	{
		mSelectedPresetIndex = mPrevSelectedPresetIndex;
		mPresets = mPrevPresets;
	}


	void ParameterGUI::showPresets(const ParameterGroup* parameterGroup)
	{
		if (parameterGroup != nullptr)
		{
			mSelectedGroupIndex = -1;
			for (int i = 0; i != mParameterGroups.size(); ++i)
			{
				if (parameterGroup == mParameterGroups[i].mGroup.get())
				{
					mSelectedGroupIndex = i;
					break;
				}
			}

			assert(mSelectedGroupIndex != -1);
		}
		else
		{
			struct GroupState
			{
				ParameterService::ParameterGroupList& mParameterGroups;
				std::string mGroupName;
			};

			GroupState group_state{ mParameterGroups };

			std::string group_name;
			ImGui::Combo("Groups", &mSelectedGroupIndex, [](void* data, int index, const char** out_text)
			{
				GroupState* state = (GroupState*)data;

				const ParameterService::ParameterGroupInfo& group_info = state->mParameterGroups[index];

				state->mGroupName.clear();
				for (int i = 0; i < group_info.mDepth; ++i)
					state->mGroupName += "   ";

				state->mGroupName += group_info.mGroup->mID;

				*out_text = state->mGroupName.data();
				return true;
			}, &group_state, mParameterGroups.size());
		}

		if (hasSelectedGroup())
		{
			ImGui::Text("Current preset: ");
			ImGui::SameLine();

			bool hasPreset = mSelectedPresetIndex >= 0 && mSelectedPresetIndex < mPresets.size();

			if (hasPreset)
				ImGui::Text(mPresets[mSelectedPresetIndex].data());
			else
				ImGui::Text("<No preset>");

			if (ImGui::Button("Save"))
			{
				if (hasPreset)
				{
					utility::ErrorState errorState;
					if (!mParameterService.savePreset(*mParameterGroups[mSelectedGroupIndex].mGroup, mPresets[mSelectedPresetIndex], errorState))
						ImGui::OpenPopup("Failed to save preset");

					if (ImGui::BeginPopupModal("Failed to save preset"))
					{
						ImGui::Text(errorState.toString().c_str());
						if (ImGui::Button("OK"))
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

				}
				else
				{
					ImGui::OpenPopup("Save As");
					savePresetState();
					mPresets.push_back("<New...>");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Save As"))
			{
				ImGui::OpenPopup("Save As");
				savePresetState();
				mPresets.push_back("<New...>");
			}

			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				ImGui::OpenPopup("Load");
				savePresetState();
			}

			handleLoadPresetPopup();
			handleSaveAsPresetPopup();
		}
	}


	void ParameterGUI::showParameters(ParameterGroup& parameterGroup, bool isRoot)
	{
		if (isRoot || ImGui::CollapsingHeader(parameterGroup.mID.c_str()))
		{
			for (auto& parameter : parameterGroup.mParameters)
			{
				const rtti::TypeInfo& type = parameter->get_type();
				ParameterEditorMap::iterator pos = mParameterEditors.find(type);
				assert(pos != mParameterEditors.end());
				ImGui::PushID(&parameter);
				pos->second(*parameter);
				ImGui::PopID();
			}

			for (auto& child : parameterGroup.mChildren)
				showParameters(*child, false);
		}
	}


	void ParameterGUI::show(const ParameterGroup* parameterGroup, bool newWindow)
	{
		if(newWindow)
			ImGui::Begin("Parameters");
		showPresets(parameterGroup);
		if (hasSelectedGroup())
		{
			ImGui::Spacing();
			ImGui::Spacing();
			showParameters(*mParameterGroups[mSelectedGroupIndex].mGroup, true);
		}
		if(newWindow)
			ImGui::End();
	}


	void ParameterGUI::registerParameterEditor(const rtti::TypeInfo& type, const CreateParameterEditor& createParameterEditorFunc)
	{
		std::vector<rtti::TypeInfo> types;
		types.push_back(type);
		rtti::getDerivedTypesRecursive(type, types);
		
		for (const rtti::TypeInfo& type : types)
			mParameterEditors[type] = createParameterEditorFunc;
	}
}