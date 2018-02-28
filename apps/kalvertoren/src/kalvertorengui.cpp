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

#include <imguiservice.h>
#include <nap/core.h>
#include <imgui/imgui.h>

namespace nap
{
	KalvertorenGui::KalvertorenGui(KalvertorenApp& app) : mApp(app)
	{	}


	void KalvertorenGui::update()
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
		ImGui::Begin("Kalvertoren");

		ImGui::Spacing();
		utility::getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		ImGui::Text("Week %02d", mDateTime.getWeek());
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("DisplaySettings"))
		{
			// Changes the mesh paint mode
			if (ImGui::Combo("Mode", &mPaintMode, "Channel Walker\0Bounding Box\0Composition\0\0"))
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

			ColorPaletteComponentInstance& color_palette_comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
			bool lockWeek = color_palette_comp.getLockWeek();
			if (ImGui::Checkbox("Lock week to current week", &lockWeek))
				color_palette_comp.setLockWeek(lockWeek);

			mSelectedWeek = color_palette_comp.getSelectedWeek() + 1;
			if (ImGui::InputInt("Week Number", &mSelectedWeek, 1))
			{
				if (!lockWeek)
					selectPaletteWeek();
			}

			// Changes the color palette
			if (ImGui::InputInt("Variation", &mPaletteSelection, 1))
			{
				palette_selector.selectVariation(mPaletteSelection);
			}

			// Changes the mesh paint mode
			if (ImGui::Combo("Variation Cycle Mode", &mColorPaletteCycleMode, "Off\0Random\0List\0\0"))
			{
				selectPaletteCycleMode();
			}

			// Changes the time at which a new color palette is selected
			if (ImGui::SliderFloat("Cycle Time (minutes)", &mColorCycleTime, 0.0f, 60.0f, "%.3f", 3.0f))
			{
				setColorPaletteCycleSpeed(mColorCycleTime);
			}
		}

		if (ImGui::CollapsingHeader("Brightness"))
		{
			LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();
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

		if (ImGui::CollapsingHeader("Information"))
		{
			RGBColorFloat float_clr = mTextColor.convert<RGBColorFloat>();
			ImVec4 float_clr_gui = { float_clr.getRed(), float_clr.getGreen(), float_clr.getBlue(), 1.0f };
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

			// Fps
			ImGui::TextColored(float_clr_gui, "Framerate");
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			// Light
			LightIntensityComponentInstance& light_comp = mApp.compositionEntity->getComponent<LightIntensityComponentInstance>();
			ImGui::TextColored(float_clr_gui, "Lux Sensor Average");
			ImGui::Text(utility::stringFormat("%f", light_comp.getLuxAverage()).c_str());
			ImGui::TextColored(float_clr_gui, "Output Brightness");
			ImGui::Text(utility::stringFormat("%f", light_comp.getBrightness()).c_str());
		}
		ImGui::End();
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


	void KalvertorenGui::selectPaletteWeek()
	{
		ColorPaletteComponentInstance& comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
		comp.selectWeek(mSelectedWeek - 1);
	}


	void KalvertorenGui::selectPaletteCycleMode()
	{
		ColorPaletteComponentInstance& comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
		comp.setCycleMode(static_cast<nap::ColorPaletteCycleMode>(mColorPaletteCycleMode));
	}


	void KalvertorenGui::setColorPaletteCycleSpeed(float minutes)
	{
		ColorPaletteComponentInstance& comp = mApp.compositionEntity->getComponent<ColorPaletteComponentInstance>();
		comp.setCycleSpeed(minutes * 60.0f);
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
		selectPaletteCycleMode();
		setColorPaletteCycleSpeed(mColorCycleTime);
	}

}


