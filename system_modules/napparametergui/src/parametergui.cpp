/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <parametergui.h>
#include <rtti/rttiutilities.h>
#include <nap/core.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <imguiservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterGUI, "GUI window that displays all parameters inside a group")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Serializable",	&nap::ParameterGUI::mSerializable,		nap::rtti::EPropertyMetaData::Default,	"Allow the parameters to be saved / loaded")
	RTTI_PROPERTY("Group",			&nap::ParameterGUI::mParameterGroup,	nap::rtti::EPropertyMetaData::Required, "The group to display, including all child groups")
RTTI_END_CLASS

namespace nap
{
	ParameterGUI::ParameterGUI(nap::Core& core) : 
		mParameterService(*core.getService<ParameterService>()),
		mParameterGUIService(*core.getService<ParameterGUIService>()),
		mGUIService(*core.getService<IMGuiService>())
	{ }


	void ParameterGUI::handleLoadPresetPopup()
	{
		if (ImGui::BeginPopupModal("Load", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (!mPresets.empty() && mSelectedPresetIndex == -1)
				mSelectedPresetIndex = 0;

			ImGui::Combo("Presets", &mSelectedPresetIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* preset_files = (ParameterService::PresetFileList*)data;
				*out_text = (*preset_files)[index].data();
				return true;
			}, &mPresets, mPresets.size());

			if (!mPresets.empty())
			{
				if (ImGui::ImageButton(mGUIService.getIcon(icon::ok)))
				{
					utility::ErrorState errorState;
					if (mParameterService.loadPreset(*mParameterGroup, mPresets[mSelectedPresetIndex], errorState))
					{
						ImGui::CloseCurrentPopup();
					}
					else
					{
						nap::Logger::warn("Failed to load preset: %s", errorState.toString().c_str());
					}
				}
				ImGui::SameLine();
			}

			if (ImGui::ImageButton(mGUIService.getIcon(icon::cancel)))
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

			if (ImGui::ImageButton(mGUIService.getIcon(icon::ok)) && strlen(name) != 0)
			{
				outNewFilename = std::string(name, strlen(name));
				outNewFilename += ".json";
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SameLine();

			if (ImGui::ImageButton(mGUIService.getIcon(icon::cancel)))
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

			if (ImGui::ImageButton(mGUIService.getIcon(icon::ok)))
			{
				if (mSelectedPresetIndex != -1)
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
						if (ImGui::ImageButton(mGUIService.getIcon(icon::ok)))
							ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}
				}
			}

			ImGui::SameLine();
			if (ImGui::ImageButton(mGUIService.getIcon(icon::cancel)))
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
		for (auto& parameter : parameterGroup.mMembers)
		{
			const rtti::TypeInfo& type = parameter->get_type();
			auto editor = mParameterGUIService.findEditor(*parameter);
			assert(editor != nullptr);
			ImGui::PushID(&parameter);
			(*editor)(*parameter);
			ImGui::PopID();
		}

		// Add indentation if depth is higher than zero
		if (depth > 0)
		{
			ImGui::Indent(ImGui::GetStyle().FramePadding.x + 1.0f);
		}
		for (auto& child : parameterGroup.mChildren)
		{
			if (ImGui::CollapsingHeader(child->mID.c_str()))
			{
			    // Increment the current depth to keep track of the indentation level
				showParameters(*child, depth+1);
			}
		}
		// Undo previously added indentation
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

		if (ImGui::ImageButton(mGUIService.getIcon(icon::save)))
		{
			if (has_preset)
			{
				utility::ErrorState errorState;
				if (!mParameterService.savePreset(*mParameterGroup, mPresets[mSelectedPresetIndex], errorState))
					ImGui::OpenPopup("Failed to save preset");

				if (ImGui::BeginPopupModal("Failed to save preset"))
				{
					ImGui::Text(errorState.toString().c_str());
					if (ImGui::ImageButton(mGUIService.getIcon(icon::ok)))
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
		if (ImGui::ImageButton(mGUIService.getIcon(icon::saveAs)))
		{
			ImGui::OpenPopup("Save As");
			savePresetState();
			mPresets.push_back("<New...>");
		}

		ImGui::SameLine();
		if (ImGui::ImageButton(mGUIService.getIcon(icon::load)))
		{
			ImGui::OpenPopup("Load");
			savePresetState();
			mPresets = mParameterService.getPresets(*mParameterGroup);
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


	bool ParameterGUI::load(std::string preset, utility::ErrorState& errorState)
	{
		if (!errorState.check(mParameterGroup != nullptr, "No parameter group to reference"))
			return false;

		mPresets = mParameterService.getPresets(*mParameterGroup);
		if (!errorState.check(!mPresets.empty(), "No presets found"))
			return false;

		const auto it = std::find(mPresets.begin(), mPresets.end(), preset);
		if (!errorState.check(it != mPresets.end(), "Preset %s not found", preset.c_str()))
			return false;

		if (!mParameterService.loadPreset(*mParameterGroup, *it, errorState))
		{
			errorState.fail("Failed to load preset %s", (*it).c_str());
			return false;
		}

		mSelectedPresetIndex = it - mPresets.begin();
		return true;
	}


	int ParameterGUI::getSelectedPresetIndex() const
	{
		return mSelectedPresetIndex;
	}


	const std::string& ParameterGUI::getSelectedPreset() const
	{
		assert(mSelectedPresetIndex != -1);
		return mPresets[mSelectedPresetIndex];
	}


	bool ParameterGUI::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mParameterGroup != nullptr, "No parameter group to display"))
			return false;
		return true;
	}
}
