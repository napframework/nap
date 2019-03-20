#include "parametergui.h"
#include "parameter.h"
#include "imgui/imgui.h"

namespace nap
{
	template<class PARAMETERTYPE>
	static void showFloatParameter(PARAMETERTYPE& parameter)
	{
		float value = parameter.mValue;
		if (ImGui::SliderFloat(parameter.mID.c_str(), &value, parameter.mMinimum, parameter.mMaximum))
			parameter.setValue(value);
	}

	template<class PARAMETERTYPE>
	static void showIntParameter(PARAMETERTYPE& parameter)
	{
		int value = parameter.mValue;
		if (ImGui::SliderInt(parameter.mID.c_str(), &value, parameter.mMinimum, parameter.mMaximum))
			parameter.setValue(value);
	}

	//////////////////////////////////////////////////////////////////////////

	ParameterGUI::ParameterGUI(ParameterService& parameterService) :
		mParameterService(parameterService)
	{
		mPresets = mParameterService.getPresets();
	}

	void ParameterGUI::HandleLoadPopup()
	{
		if (ImGui::BeginPopupModal("Load", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Combo("Presets", &mSelectedPresetIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* preset_files = (ParameterService::PresetFileList*)data;
				*out_text = (*preset_files)[index].data();
				return true;
			}, &mPresets, mPresets.size());

			if (ImGui::Button("OK"))
			{
				utility::ErrorState errorState;
				if (mParameterService.loadPreset(mPresets[mSelectedPresetIndex], errorState))
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
				RestorePresets();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	bool ParameterGUI::HandleNewPopup(std::string& outNewFilename)
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

	void ParameterGUI::HandleSaveAsPopup()
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
			if (HandleNewPopup(newFilename))
			{
				// Insert before the '<new...>' item
				mPresets.insert(mPresets.end() - 1, newFilename);
			}

			ImGui::SameLine();

			if (ImGui::Button("OK"))
			{
				utility::ErrorState errorState;
				if (mParameterService.savePreset(mPresets[mSelectedPresetIndex], errorState))
				{
					ImGui::CloseCurrentPopup();
					std::string previous_selection = mPresets[mSelectedPresetIndex];

					mPresets = mParameterService.getPresets();

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
				RestorePresets();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ParameterGUI::SavePresets()
	{
		mPrevSelectedPresetIndex = mSelectedPresetIndex;
		mPrevPresets = mPresets;
	}

	void ParameterGUI::RestorePresets()
	{
		mSelectedPresetIndex = mPrevSelectedPresetIndex;
		mPresets = mPrevPresets;
	}

	void ParameterGUI::showPresets()
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
				if (!mParameterService.savePreset(mPresets[mSelectedPresetIndex], errorState))
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
				SavePresets();
				mPresets.push_back("<New...>");
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Save As"))
		{
			ImGui::OpenPopup("Save As");
			SavePresets();
			mPresets.push_back("<New...>");
		}

		ImGui::SameLine();

		if (ImGui::Button("Load"))
		{
			ImGui::OpenPopup("Load");
			SavePresets();
		}

		HandleLoadPopup();
		HandleSaveAsPopup();
	}

	void ParameterGUI::showParameters(ParameterContainer& parameterContainer, bool isRoot)
	{
		if (isRoot || ImGui::TreeNode(parameterContainer.mID.c_str()))
		{
			for (auto& parameter : parameterContainer.mParameters)
			{
				const rtti::TypeInfo& type = parameter->get_type();
				if (type == RTTI_OF(ParameterFloat))
				{
					ParameterFloat* float_parameter = rtti_cast<ParameterFloat>(parameter.get());
					showFloatParameter(*float_parameter);
				}
				else if (type == RTTI_OF(ParameterDouble))
				{
					ParameterDouble* double_parameter = rtti_cast<ParameterDouble>(parameter.get());
					showFloatParameter(*double_parameter);
				}
				else if (type == RTTI_OF(ParameterInt))
				{
					ParameterInt* int_parameter = rtti_cast<ParameterInt>(parameter.get());
					showIntParameter(*int_parameter);
				}
				else if (type == RTTI_OF(ParameterLong))
				{
					ParameterLong* int_parameter = rtti_cast<ParameterLong>(parameter.get());
					showIntParameter(*int_parameter);
				}
				else if (type == RTTI_OF(ParameterByte))
				{
					ParameterByte* int_parameter = rtti_cast<ParameterByte>(parameter.get());
					showIntParameter(*int_parameter);
				}
				else if (type == RTTI_OF(ParameterChar))
				{
					ParameterChar* int_parameter = rtti_cast<ParameterChar>(parameter.get());
					showIntParameter(*int_parameter);
				}
				else if (type == RTTI_OF(ParameterRGBColorFloat))
				{
					ParameterRGBColorFloat* color_parameter = rtti_cast<ParameterRGBColorFloat>(parameter.get());
					ImGui::ColorEdit3(color_parameter->mID.c_str(), color_parameter->mValue.getData());
				}
				else if (type == RTTI_OF(ParameterVec2))
				{
					ParameterVec2* vec2_parameter = rtti_cast<ParameterVec2>(parameter.get());
					ImGui::SliderFloat2(vec2_parameter->mID.c_str(), &(vec2_parameter->mValue[0]), vec2_parameter->mMinimum, vec2_parameter->mMaximum);
				}
				else if (type == RTTI_OF(ParameterVec3))
				{
					ParameterVec3* vec3_parameter = rtti_cast<ParameterVec3>(parameter.get());
					ImGui::SliderFloat3(vec3_parameter->mID.c_str(), &(vec3_parameter->mValue[0]), vec3_parameter->mMinimum, vec3_parameter->mMaximum);
				}
			}

			for (auto& child : parameterContainer.mChildren)
				showParameters(*child, false);
		}
	}


	void ParameterGUI::show()
	{
		ImGui::Begin("Parameters");

		showPresets();

		if (mParameterService.hasParameters())
		{
			ImGui::Separator();
			showParameters(mParameterService.getParameters(), true);
		}

		ImGui::End();
	}
}