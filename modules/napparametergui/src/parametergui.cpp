/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <parametergui.h>
#include <rtti/rttiutilities.h>
#include <nap/core.h>
#include <imgui/imgui.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterGUI)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Serializable",	&nap::ParameterGUI::mSerializable,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Group",			&nap::ParameterGUI::mParameterGroup,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	ParameterGUI::ParameterGUI(nap::Core& core) : 
		mParameterService(*core.getService<ParameterService>()),
		mParameterGUIService(*core.getService<ParameterGUIService>())
	{ }


	void ParameterGUI::handleLoadPresetPopup()
	{
		if (ImGui::BeginPopupModal("Load", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			mPresets = mParameterService.getPresets(*mParameterGroup);
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
				if (mParameterService.loadPreset(*mParameterGroup, mPresets[mSelectedPresetIndex], errorState))
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
					ImGui::OpenPopup("New");
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
				if (mParameterService.savePreset(*mParameterGroup, mPresets[mSelectedPresetIndex], errorState))
				{
					ImGui::CloseCurrentPopup();
					std::string previous_selection = mPresets[mSelectedPresetIndex];

					// After we have retrieved the filenames from the service, the list may be in a different order,
					// so we search for the item in the list to find the selected index.
					mPresets = mParameterService.getPresets(*mParameterGroup);
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
						ImGui::CloseCurrentPopup();
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


	void ParameterGUI::showParameters(ParameterGroup& parameterGroup, int depth)
	{
		for (auto& parameter : parameterGroup.mParameters)
		{
			const rtti::TypeInfo& type = parameter->get_type();
			auto editor = mParameterGUIService.findEditor(*parameter);
			assert(editor != nullptr);
			ImGui::PushID(&parameter);
			(*editor)(*parameter);
			ImGui::PopID();
		}

		if (depth > 0)
		{
			ImGui::Indent(ImGui::GetStyle().FramePadding.x + 1.0f);
		}
		for (auto& child : parameterGroup.mChildren)
		{
			if (ImGui::CollapsingHeader(child->mID.c_str()))
			{
				showParameters(*child, depth+1);
			}
		}
		if (depth > 0)
		{
			ImGui::Unindent(ImGui::GetStyle().FramePadding.x + 1.0f);
		}
	}


	void ParameterGUI::showPresets()
	{
		ImGui::Text("Current preset: ");
		ImGui::SameLine();

		bool has_preset = mSelectedPresetIndex >= 0 && mSelectedPresetIndex < mPresets.size();
		ImGui::Text(has_preset ? mPresets[mSelectedPresetIndex].data() : "<No preset>");

		if (ImGui::Button("Save"))
		{
			if (has_preset)
			{
				utility::ErrorState errorState;
				if (!mParameterService.savePreset(*mParameterGroup, mPresets[mSelectedPresetIndex], errorState))
					ImGui::OpenPopup("Failed to save preset");

				if (ImGui::BeginPopupModal("Failed to save preset"))
				{
					ImGui::Text(errorState.toString().c_str());
					if (ImGui::Button("OK"))
						ImGui::CloseCurrentPopup();
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


	void ParameterGUI::show(bool newWindow)
	{
		if (newWindow)
		{
			ImGui::Begin(utility::stringFormat("%s Group", mParameterGroup->mID.c_str()).c_str());
			if (mSerializable)
				showPresets();
			showParameters(*mParameterGroup);
			ImGui::End();
		}
		else
		{
			if (mSerializable)
				showPresets();
			showParameters(*mParameterGroup);
		}
	}


	bool ParameterGUI::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mParameterGroup != nullptr, "No parameter group to display"))
			return false;
		return true;
	}
}
