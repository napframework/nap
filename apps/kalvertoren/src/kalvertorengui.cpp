// Local Includes
#include "kalvertorengui.h"
#include "kalvertorenapp.h"
#include "selectledmeshcomponent.h"
#include "selectcolormethodcomponent.h"
#include "applytracercolorcomponent.h"
#include "applybbcolorcomponent.h"
#include "applycompositioncomponent.h"
#include "rendercompositioncomponent.h"
#include "lightintensitycomponent.h"
#include "applyvideocomponent.h"

#include <imguiservice.h>
#include <nap/core.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <composition.h>

namespace nap
{
	/**
	 *	Imgui statics
	 */
	static bool showControls	= false;
	static bool showInfo		= false;

	KalvertorenGui::KalvertorenGui(KalvertorenApp& app) : mApp(app)
	{
		mLuxValues.fill(0.0f);
		mBrightnessValues.fill(0.0f);
		mLedOn  = app.getCore().getResourceManager()->findObject<nap::ImageFromFile>("LedOnImage");
		mLedOff = app.getCore().getResourceManager()->findObject<nap::ImageFromFile>("LedOffImage");

		CompositionComponentInstance& comp = mApp.compositionEntity->getComponent<CompositionComponentInstance>();
		mCompositionCycleMode = static_cast<int>(comp.getCycleMode());
	}


	void KalvertorenGui::update(double deltaTime)
	{
		// Update buffers
		updateLuxHistogram(deltaTime);

		// Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Controls", NULL, &showControls);
				ImGui::MenuItem("Information", NULL, &showInfo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Show control menu
		if (showControls)
			showControlWindow();

		if (showInfo)
			showInfoWindow();
	}


	void KalvertorenGui::draw()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void KalvertorenGui::selectCompositionCycleMode()
	{
		CompositionComponentInstance& comp = mApp.compositionEntity->getComponent<CompositionComponentInstance>();
		comp.setCycleMode(static_cast<nap::CompositionCycleMode>(mCompositionCycleMode));
	}


	void KalvertorenGui::selectPaintMethod()
	{
		// Get all the color selection components
		std::vector<SelectColorMethodComponentInstance*> color_methods;

		for (auto& entity : mApp.compositionEntity->getChildren())
		{
			SelectColorMethodComponentInstance* color_method = &(entity->getComponent<SelectColorMethodComponentInstance>());
			color_methods.emplace_back(color_method);
		}

		for (auto& color_method : color_methods)
		{
			switch (mPaintMode)
			{
			case 0:
				color_method->select(RTTI_OF(nap::ApplyTracerColorComponentInstance));
				break;
			case 1:
				color_method->select(RTTI_OF(nap::ApplyBBColorComponentInstance));
				break;
			case 2:
				color_method->select(RTTI_OF(nap::ApplyCompositionComponentInstance));
				break;
			case 3:
				color_method->select(RTTI_OF(nap::ApplyVideoComponentInstance));
				break;
			default:
				assert(false);
				break;
			}
		}
	}


	void KalvertorenGui::init()
	{
		SelectLedMeshComponentInstance& selector = mApp.displayEntity->getComponent<SelectLedMeshComponentInstance>();
		mMeshSelection = selector.getIndex();

		// Force paint method
		selectPaintMethod();

		// Force cycle modes
		selectCompositionCycleMode();
	}


	void KalvertorenGui::showControlWindow()
	{

		// Get all the color selection components
		std::vector<SelectColorMethodComponentInstance*> color_methods;
		std::vector<ApplyTracerColorComponentInstance*>  tracer_painters;
		std::vector<ApplyCompositionComponentInstance*> composition_painters;

		for (auto& entity : mApp.compositionEntity->getChildren())
		{
			SelectColorMethodComponentInstance* color_method = &(entity->getComponent<SelectColorMethodComponentInstance>());
			color_methods.emplace_back(color_method);

			ApplyTracerColorComponentInstance* tracer_painter = &(entity->getComponent<ApplyTracerColorComponentInstance>());
			tracer_painters.emplace_back(tracer_painter);

			ApplyCompositionComponentInstance* comp_painter = &(entity->getComponent<ApplyCompositionComponentInstance>());
			composition_painters.emplace_back(comp_painter);
		}

		// Some extra comps
		SelectLedMeshComponentInstance& mesh_selector = mApp.displayEntity->getComponent<SelectLedMeshComponentInstance>();
		ColorPaletteComponentInstance& palette_selector = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
		CompositionComponentInstance& composition_selector = mApp.compositionEntity->getComponent<CompositionComponentInstance>();

		// Resets all the tracers
		ImGui::Begin("Controls");

		if (ImGui::CollapsingHeader("DisplaySettings"))
		{
			// Changes the mesh paint mode
			if (ImGui::Combo("Mode", &mPaintMode, "Channel Walker\0Bounding Box\0Composition\0Video\0\0"))
			{
				selectPaintMethod();
			}

			// Changes the display mesh
			if (ImGui::Combo("Display Mesh", &mMeshSelection, "Heiligeweg\0Kalverstraat\0Singel\0\0"))
			{
				mesh_selector.select(mMeshSelection);
			}
		}

		// Composition settings
		if (ImGui::CollapsingHeader("Composition Settings"))
		{
			// Changes the mesh paint mode
			if (ImGui::Combo("Day", &mDay, "Auto\0Sunday\0Monday\0Tuesday\0Wednesday\0Thursday\0Friday\0Saturday\0\0"))
			{
				if (mDay == 0)
				{
					composition_selector.switchMode(CompositionComponentInstance::EMode::Automatic);
				}
				else
				{
					nap::utility::EDay new_day = static_cast<nap::utility::EDay>(mDay - 1);
					composition_selector.selectDay(new_day);
				}
			}

			// Changes the mesh paint mode
			if (ImGui::Combo("Cycle Mode", &mCompositionCycleMode, "Off\0Random\0List\0\0"))
			{
				selectCompositionCycleMode();
			}

			// Changes the color palette
			if (ImGui::SliderInt("Select", &mCompositionSelection, 0, composition_selector.getCount() - 1))
			{
				composition_selector.select(mCompositionSelection);
			}

			// Changes the duration
			if (ImGui::SliderFloat("Cycle Speed", &mDurationScale, 0.0f, 10.0f))
			{
				composition_selector.setDurationScale(mDurationScale);
			}
		}

		if (ImGui::CollapsingHeader("Color Settings"))
		{
			// If index colors are drawn
			if (ImGui::Checkbox("Show Index Colors", &mShowIndexColors))
			{
				for (auto& comp_painter : composition_painters)
				{
					comp_painter->showIndexColors(mShowIndexColors);
				}
			}

			// Turn lock to week on / off
			ColorPaletteComponentInstance& color_palette_comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
			ImGui::Checkbox("Lock week to current week", &(color_palette_comp.mLockWeek));

			// Link 
			ImGui::Checkbox("Link To Composition", &(color_palette_comp.mLinked));

			mSelectedWeek = color_palette_comp.getSelectedWeek() + 1;
			if (ImGui::InputInt("Week Number", &mSelectedWeek, 1))
			{
				if (!color_palette_comp.isLocked())
				{
					color_palette_comp.selectWeek(mSelectedWeek - 1);
				}
			}

			// Changes the color palette
			if (ImGui::InputInt("Variation", &mPaletteSelection, 1))
			{
				palette_selector.selectVariation(mPaletteSelection);
			}
		}

		if (ImGui::CollapsingHeader("Brightness"))
		{
			LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();
			ImGui::Checkbox("Use Opening Hours", &(light_comp.mUseOpeningHours));
			if (ImGui::InputFloat2("Lux Range", &(mLuxRange.x)))
				light_comp.setLuxRange(mLuxRange);

			if (ImGui::InputFloat2("Light Range", &(mLightRange.x)))
				light_comp.setLightRange(mLightRange);

			if (ImGui::SliderFloat("Sensor Influence", &mSensorInfluence, 0.0f, 1.0f))
				light_comp.setSensorInfluence(mSensorInfluence);

			if (ImGui::SliderFloat("Lux Curve", &mLuxCurve, 0.0f, 2.0f, "%.3f", 2.0f))
				light_comp.setLuxPower(mLuxCurve);

			if (ImGui::SliderFloat("Smooth Time", &mLightSmoothTime, 0.0f, 20.0f, "%.3f", 2.0f))
				light_comp.setSmoothTime(mLightSmoothTime);

			if (ImGui::SliderFloat("Master", &mIntensity, 0.0f, 1.0f))
				light_comp.setMasterBrightness(mIntensity);
		}

		if (ImGui::CollapsingHeader("Walker Settings"))
		{
			if (ImGui::Button("Reset Walker"))
			{
				for (auto& tracer : tracer_painters)
				{
					tracer->reset();
				}
			}

			if (ImGui::SliderFloat("Walk Speed", &(mChannelSpeed), 0.0f, 20.0f))
			{
				for (auto& tracer : tracer_painters)
				{
					tracer->setSpeed(mChannelSpeed);
				}
			}

			// Allows the user to select a channel to display on the tracer
			if (ImGui::InputInt("Select Channel", &mSelectChannel, 1))
			{
				for (auto& tracer : tracer_painters)
				{
					tracer->selectChannel(mSelectChannel);
				}
			}
		}
		ImGui::End();
	}


	void KalvertorenGui::showInfoWindow()
	{

		// GUI Colors
		RGBColorFloat float_clr = mTextColor.convert<RGBColorFloat>();
		ImVec4 float_clr_gui = { float_clr.getRed(), float_clr.getGreen(), float_clr.getBlue(), 1.0f };

		ImGui::Begin("Information");
		ImGui::Spacing();
		utility::getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		ImGui::Text("Week %02d", mDateTime.getWeek());
		ImGui::TextColored(float_clr_gui, "%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Sensor Status"))
		{
			LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();

			// Sensors (online / offline)
			for (const auto& sensor : light_comp.getSensors())
			{
				ImGui::Image(sensor->isOnline() ? *mLedOn : *mLedOff, { 32, 32 });
				ImGui::SameLine();
				ImGui::Text(utility::stringFormat("%s: %s", sensor->mID.c_str(), sensor->isOnline() ? "online" : "offline").c_str());
			}
		}

		// Lux
		if (ImGui::CollapsingHeader("Lux"))
		{
			LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();

			// plot lux history
			ImGui::TextColored(float_clr_gui, "Lux Sensor Average");
			ImGui::Text(utility::stringFormat("%f", light_comp.getLuxAverage()).c_str());
			ImGui::SliderFloat("Sample Interval (sec)", &mLuxSampleTime, 0.0f, 100.0f, "%.3f", 2.0f);
			glm::vec2 lux_range = light_comp.getLuxRange();
			ImGui::InputFloat2("Display Bounds", &(mLuxDisplayBounds.x));
			ImGui::PlotHistogram("Sensor History", mLuxValues.data(), mLuxValues.size(), mLuxIdx, NULL, mLuxDisplayBounds.x, mLuxDisplayBounds.y, ImVec2(0, 80));

			// plot brightness history
			ImGui::TextColored(float_clr_gui, "Output Brightness");
			ImGui::Text(utility::stringFormat("%f", light_comp.getBrightness()).c_str());
			ImGui::PlotHistogram("Brightness History", mBrightnessValues.data(), mBrightnessValues.size(), mBrightnessIdx, NULL, 0.0f, 1.0f, ImVec2(0, 80));
		}

		// Opening Hours
		if (ImGui::CollapsingHeader("Opening Hours"))
		{
			LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();
			OpeningTime opening_time, closing_time;
			light_comp.getOpeningTimes(utility::getCurrentDateTime(), opening_time, closing_time);
			ImGui::TextColored(float_clr_gui, "Opening Hours:");
			ImGui::SameLine();
			ImGui::Text(utility::stringFormat("%02d:%02d", opening_time.mHour, opening_time.mMinute).c_str());
			ImGui::TextColored(float_clr_gui, "Closing Hours:");
			ImGui::SameLine();
			ImGui::Text(utility::stringFormat("%02d:%02d", closing_time.mHour, closing_time.mMinute).c_str());
			ImGui::TextColored(float_clr_gui, "Stores: ");
			ImGui::SameLine();
			ImGui::Text(light_comp.isOpen() ? "Open" : "Closed");
		}

		if (ImGui::CollapsingHeader("Composition"))
		{
			CompositionComponentInstance& composition_comp = mApp.compositionEntity->getComponent<CompositionComponentInstance>();

			ImGui::TextColored(float_clr_gui, "Current: ");
			ImGui::SameLine();
			ImGui::Text(composition_comp.getSelection().getName().c_str());
			ImGui::TextColored(float_clr_gui, "Type: ");
			ImGui::SameLine();
			ImGui::Text(composition_comp.getSelection().getMode() == CompositionPlayMode::Length ? "Length" : "Sequence");
			ImGui::TextColored(float_clr_gui, "Status: ");
			ImGui::SameLine();
			switch (composition_comp.getSelection().getStatus())
			{
			case CompositionInstance::EStatus::Active:
				ImGui::Text("Active");
				break;
			case CompositionInstance::EStatus::Completed:
				ImGui::Text("Completed");
				break;
			case CompositionInstance::EStatus::WaitingForSequence:
				ImGui::Text("Waiting for sequence to finish");
				break;
			}
			ImGui::ProgressBar(composition_comp.getSelection().getProgress());
		}

		if (ImGui::CollapsingHeader("Colors"))
		{
			ColorPaletteComponentInstance& palette_comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
			ImGui::TextColored(float_clr_gui, "Selected Week: ");
			ImGui::SameLine();
			ImGui::Text(utility::stringFormat("%d", palette_comp.getSelectedWeek()+1).c_str());
			ImGui::TextColored(float_clr_gui, "Palette Index: ");
			ImGui::SameLine();
			ImGui::Text(utility::stringFormat("%d", palette_comp.getVariation()).c_str());
		}

		if (ImGui::CollapsingHeader("Index Map"))
		{
			RenderCompositionComponentInstance& render_comp = mApp.renderCompositionEntity->getComponent<RenderCompositionComponentInstance>();
			float col_width = ImGui::GetContentRegionAvailWidth() * mDisplaySize;
			ImGui::Image(render_comp.getTexture(), { col_width, col_width });

			ColorPaletteComponentInstance& palette_comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
			float ratio = 1.0f / 12.0f;
			ImGui::Image(palette_comp.getIndexMap(), { col_width, col_width * ratio });
			ImGui::Image(palette_comp.getDebugPaletteImage(), { col_width, col_width * ratio });
			
			// Draw slider regarding display size
			ImGui::SliderFloat("Display Size", &mDisplaySize, 0.0f, 1.0f);
		}

		// Artnet information
		if (ImGui::CollapsingHeader("Artnet"))
		{
			SelectLedMeshComponentInstance& mesh_selector = mApp.displayEntity->getComponent<SelectLedMeshComponentInstance>();
			for (int i = 0; i < mesh_selector.getLedMeshes().size(); i++)
			{
				std::vector<std::string> parts;
				utility::splitString(mesh_selector.getLedMeshes()[i]->mTriangleMesh->mPath, '/', parts);

				ImGui::TextColored(float_clr_gui, parts.back().c_str());
				ImGui::Text(utility::stringFormat("Channel: %d", i).c_str());
				const std::unordered_set<ArtNetController::Address>& addresses = mesh_selector.getLedMeshes()[i]->mTriangleMesh->getAddresses();

				std::string universes = "Addresses:";
				for (auto& address : addresses)
				{
					uint8 sub(0), uni(0);
					ArtNetController::convertAddress(address, sub, uni);
					universes += utility::stringFormat(" %d:%d", sub, uni);
				}

				ImGui::Text(universes.c_str());
			}
		}

		ImGui::End();
	}


	void KalvertorenGui::updateLuxHistogram(double deltaTime)
	{
		// Update time, when 1 second has passed update lux value buffer
		mLuxTime += deltaTime;
		if (mLuxTime < mLuxSampleTime)
			return;
		mLuxTime = 0.0f;

		// Fill lux buffer
		LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();
		mLuxValues[mLuxIdx] = light_comp.getLuxAverage();

		// Reset index if necessary
		if (++mLuxIdx == mLuxValues.size())
			mLuxIdx = 0;

		// Fill brightness buffer
		mBrightnessValues[mBrightnessIdx] = light_comp.getBrightness();

		// Reset index
		if (++mBrightnessIdx == mBrightnessValues.size())
			mBrightnessIdx = 0;
	}
}