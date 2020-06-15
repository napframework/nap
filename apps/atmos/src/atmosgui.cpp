// Local Includes
#include "atmosgui.h"
#include "atmosapp.h"
#include "selectmeshcomponent.h"
#include "selectimagecomponent.h"
#include "updatematerialcomponent.h"
#include "rotatecomponent.h"

// External Includes
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <nap/core.h>
#include <selectvideocomponent.h>
#include <utility/fileutils.h>
#include <parametergui.h>

namespace nap
{
	/**
	 * Imgui statics
	 */
	static bool showInfo = false;
	static bool showPresetWindow = false;

	AtmosGui::AtmosGui(AtmosApp& app) : 
		mApp(app),
		mParameterService(*app.getCore().getService<ParameterService>())
	{
	}


	AtmosGui::~AtmosGui()
	{
	}


	void AtmosGui::init()
	{
		mParameterGUI = std::make_unique<ParameterGUI>(mParameterService);
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();
		mCameraControlMethod	= resourceManager->findObject<ParameterControlMethod>("Camera Mode");
		mRotateSpeed			= resourceManager->findObject<ParameterFloat>("Rotation Speed");
		mLinkFogToBackground	= resourceManager->findObject<ParameterBool>("Link Fog To Background");
		mFogColor				= resourceManager->findObject<ParameterRGBColorFloat>("Fog Color");
		mBackgroundColor		= resourceManager->findObject<ParameterRGBColorFloat>("Background Color");
		mUseTransparency		= resourceManager->findObject<ParameterBool>("Use Transparency");
		mRenderPath				= resourceManager->findObject<ParameterBool>("RenderPath");
		mRenderMode				= resourceManager->findObject<ParameterPolygonMode>("Render Mode");

		mLinkFogToBackground->valueChanged.connect([this](bool)				{ UpdateFogColor(); });
		mFogColor->valueChanged.connect([this](const RGBColorFloat&)		{ UpdateFogColor(); });
		mBackgroundColor->valueChanged.connect([this](const RGBColorFloat&)	{ UpdateFogColor(); });


		mLedOn = mApp.mResourceManager->findObject<nap::ImageFromFile>("LedOnImage");
		assert(mLedOn != nullptr);
		mLedOff = mApp.mResourceManager->findObject<nap::ImageFromFile>("LedOffImage");
		assert(mLedOff != nullptr);

		mRotateSpeed->valueChanged.connect([this](float newValue)
		{
			RotateComponentInstance& rot_comp = mApp.mWorldEntity->getComponent<RotateComponentInstance>();
			rot_comp.mProperties.mSpeed = newValue;
		});

		mUseTransparency->valueChanged.connect([this](bool newValue)
		{ 
			nap::RenderableMeshComponentInstance* normal_render_comp = mApp.mScanEntity->findComponentByID<RenderableMeshComponentInstance>("ScanNormalRenderableMesh");
			if (newValue)
			{
				normal_render_comp->getMaterialInstance().setBlendMode(EBlendMode::AlphaBlend);
				normal_render_comp->getMaterialInstance().setDepthMode(EDepthMode::ReadOnly);
			}
			else
			{
				normal_render_comp->getMaterialInstance().setBlendMode(EBlendMode::Opaque);
				normal_render_comp->getMaterialInstance().setDepthMode(EDepthMode::ReadWrite);
			}
		});

		const glm::vec4& clear_color = mApp.mRenderWindow->getBackbuffer().getClearColor();
		mBackgroundColor->setValue(RGBColorFloat(clear_color.r, clear_color.g, clear_color.b));
		UpdateFogColor();
	}


	void AtmosGui::UpdateFogColor()
	{
		nap::UpdateMaterialComponentInstance& up_mat_comp = mApp.mScanEntity->getComponent<UpdateMaterialComponentInstance>();
		if (mLinkFogToBackground->mValue)
			up_mat_comp.mFogColor = mBackgroundColor->mValue;
		else
			up_mat_comp.mFogColor = mFogColor->mValue;
	}


	void AtmosGui::update()
	{
		if (mHide)
			return;

		// Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Parameters", NULL, &showPresetWindow);
				ImGui::MenuItem("Information", NULL, &showInfo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showPresetWindow)
			mParameterGUI->show(mParameterService.hasRootGroup() ? &mParameterService.getRootGroup() : nullptr);

		if (showInfo)
			showInfoWindow();
	}
	

	void AtmosGui::render()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void AtmosGui::toggleVisibility()
	{
		mHide = !mHide;
	}

	void AtmosGui::showInfoWindow()
	{
		// Color used for highlights
		mApp.getCore().getFramerate();

		ImGui::Begin("Information");
		ImGui::Spacing();
		getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImGui::TextColored(text_color, "%.3f ms/frame (%.1f FPS)", 1000.0f / mApp.getCore().getFramerate(), mApp.getCore().getFramerate());
		if (ImGui::CollapsingHeader("Apply Sensor Component"))
		{
			ImGui::Separator();
			ImGui::Text("Sensors :");
			ImGui::Indent(10.0f);
			for (auto sensor : mApp.mApplySensorComponent->mSensors)
			{
				if (ImGui::CollapsingHeader(sensor->mSensorName.c_str()))
				{
					ImGui::PushID(sensor->mSensorName.c_str());
					ImGui::Image(sensor->isOnline() ? *mLedOn : *mLedOff, { 16, 16 });
					ImGui::SameLine();
					ImGui::Text(utility::stringFormat("%s: %s", sensor->isOnline() ? "online" : "offline", sensor->mID.c_str()).c_str());
					ImGui::Text(utility::stringFormat("value: %.2f", sensor->getValue()).c_str());
					ImGui::PopID();
				}
			}
			ImGui::Unindent(10.0f);
		}
		
		if (ImGui::CollapsingHeader("Texture Preview"))
		{
			float col_width = ImGui::GetContentRegionAvailWidth() * mTexPreviewDisplaySize;
			nap::SelectImageComponentInstance* img_selector_tileable = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentTileable");
			float ratio_tiled = static_cast<float>(img_selector_tileable->getImage().getWidth()) / static_cast<float>(img_selector_tileable->getImage().getHeight());
			ImGui::TextColored(text_color, "Selected: ");
			ImGui::SameLine();
			ImGui::Text(img_selector_tileable->getImage().mImagePath.c_str());
			ImGui::Image(img_selector_tileable->getImage(), { col_width, col_width / ratio_tiled });
			ImGui::SliderFloat("Tiled Preview Size", &mTexPreviewDisplaySize, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Wrapped Texture Preview"))
		{
			float col_width = ImGui::GetContentRegionAvailWidth() * mWraPreviewDisplaySize;
			nap::SelectImageComponentInstance* img_selector_single = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentSingle");
			float ratio_single = static_cast<float>(img_selector_single->getImage().getWidth()) / static_cast<float>(img_selector_single->getImage().getHeight());
			ImGui::TextColored(text_color, "Selected: ");
			ImGui::SameLine();
			ImGui::Text(img_selector_single->getImage().mImagePath.c_str());
			ImGui::Image(img_selector_single->getImage(), { col_width, col_width / ratio_single });
			ImGui::SliderFloat("Wrapped Preview Size", &mWraPreviewDisplaySize, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Video Preview"))
		{
			SelectVideoComponentInstance& video_comp = mApp.mVideoEntity->getComponent<SelectVideoComponentInstance>();
			float col_width = ImGui::GetContentRegionAvailWidth() * mVidPreviewDisplaySize;
			nap::Texture2D& video_tex = mApp.mVideoTarget->getColorTexture();
			float ratio_video = static_cast<float>(video_tex.getWidth()) / static_cast<float>(video_tex.getHeight());
			ImGui::TextColored(text_color, "Selected: ");
			ImGui::SameLine();
			ImGui::Text(video_comp.getCurrentVideo()->mPath.c_str());
			ImGui::Image(video_tex, { col_width, col_width / ratio_video });
			ImGui::SliderFloat("Video Preview Size", &mVidPreviewDisplaySize, 0.0f, 1.0f);
		}
		ImGui::End();
	}

}