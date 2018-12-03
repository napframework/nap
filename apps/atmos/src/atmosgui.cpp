// Local Includes
#include "atmosgui.h"
#include "atmosapp.h"
#include "selectmeshcomponent.h"
#include "selectimagecomponent.h"
#include "updatematerialcomponent.h"
#include "controlselectcomponent.h"
#include "rotatecomponent.h"

// External Includes
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <nap/core.h>

namespace nap
{
	/**
	 * Imgui statics
	 */
	static bool showControls = false;
	static bool showInfo = false;

	AtmosGui::AtmosGui(AtmosApp& app) : mApp(app)
	{

	}


	void AtmosGui::init()
	{
		// Store background color
		mBackgroundColor = mApp.mRenderWindow->getBackbuffer().getClearColor();

		// Store speed values
		FirstPersonControllerInstance& fps_comp = mApp.mCameraEntity->getComponent<FirstPersonControllerInstance>();
		mCamMaxMovSpeed = fps_comp.getMovementSpeed();
		mCamMaxRotSpeed = fps_comp.getRotationSpeed();

		if (mLinkFogToBackground) {
			nap::UpdateMaterialComponentInstance& up_mat_comp = mApp.mScanEntity->getComponent<UpdateMaterialComponentInstance>();
			up_mat_comp.mFogColor = RGBColorFloat(mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b);
		}
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

		// Set fog color if linked
		if (mLinkFogToBackground && mBackgroundColorDirty)
		{
			nap::UpdateMaterialComponentInstance& up_mat_comp = mApp.mScanEntity->getComponent<UpdateMaterialComponentInstance>();
			up_mat_comp.mFogColor = RGBColorFloat(mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b);
			mBackgroundColorDirty = false;
		}
	}
	

	void AtmosGui::render()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void AtmosGui::toggleVisibility()
	{
		mHide = !mHide;
	}


	void AtmosGui::showControlWindow()
	{
		// Resets all the tracers
		ImGui::Begin("Controls");

		// Select mesh slider
		nap::SelectMeshComponentInstance& mesh_selector = mApp.mScanEntity->getComponent<SelectMeshComponentInstance>();
		int ci = mesh_selector.getIndex();
		if (ImGui::SliderInt("Select Mesh", &ci, 0, mesh_selector.getCount() - 1))
		{
			mesh_selector.selectMesh(ci);
		}

		// Select image 1 (tileable) slider
		nap::SelectImageComponentInstance* img_selector_tileable = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentTileable");
		assert(img_selector_tileable != nullptr);
		ci = img_selector_tileable->getIndex();
		if (ImGui::SliderInt("Select Tileable Texture", &ci, 0, img_selector_tileable->getCount() - 1))
		{
			img_selector_tileable->selectImage(ci);
		}

		// Select image 2 (stretched / single) slider
		nap::SelectImageComponentInstance* img_selector_single = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentSingle");
		assert(img_selector_single != nullptr);
		ci = img_selector_single->getIndex();
		if (ImGui::SliderInt("Select Stretch Texture", &ci, 0, img_selector_single->getCount() - 1))
		{
			img_selector_single->selectImage(ci);
		}

		RotateComponentInstance& rot_comp = mApp.mWorldEntity->getComponent<RotateComponentInstance>();
		ImGui::SliderFloat("Rotate Speed", &(rot_comp.mProperties.mSpeed), -0.1f, 0.1f);

		// Mix Controls
		nap::UpdateMaterialComponentInstance& up_mat_comp = mApp.mScanEntity->getComponent<UpdateMaterialComponentInstance>();
		if (ImGui::CollapsingHeader("Color Mixing"))
		{
			ImGui::ColorEdit3("Diffuse Color", up_mat_comp.mDiffuseColor.getData());
			ImGui::SliderFloat("Premult Blend Value", &(up_mat_comp.mPremultValue), 0.0f, 1.0f);
			ImGui::SliderFloat("Texture Blend Value", &(up_mat_comp.mColorTexMix), 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse Blend Value", &(up_mat_comp.mDiffuseColorMix), 0.0f, 1.0f);
		}

		// Texture scale
		if (ImGui::CollapsingHeader("Texture Settings"))
		{
			ImGui::SliderFloat("Tileable Scale", &(up_mat_comp.mColorTexScaleOne), 0.01f, 200.0f, "%.3f", 3.0f);
			ImGui::SliderFloat("Stretch Scale", &(up_mat_comp.mColorTexScaleTwo), 0.01f, 200.0f, "%.3f", 3.0f);
			ImGui::SliderFloat2("Slide Speed", &(up_mat_comp.mTextureSpeed[0]), -1.0f, 1.0f, "%.3f", 1.5f);
		}

		// Light controls
		if (ImGui::CollapsingHeader("Lighting"))
		{
			ImGui::SliderFloat3("Light Position", &(up_mat_comp.mLightPos[0]), -300.0f, 300.0f);
			ImGui::SliderFloat("Light Intensity", &(up_mat_comp.mLightIntensity), 0.00001f, 2.0f);
			ImGui::SliderFloat("Ambient Intensity", &(up_mat_comp.mAmbientIntensity), 0.00001f, 1.0f);
			ImGui::SliderFloat("Diffuse Intensity", &(up_mat_comp.mDiffuseIntensity), 0.00001f, 1.0f);
		}

		if (ImGui::CollapsingHeader("Hair"))
		{
			ImGui::ColorEdit3("Hair Specular Color", up_mat_comp.mNormalSpecColor.getData());
			ImGui::SliderFloat("Hair Specular Intensity", &(up_mat_comp.mNormalSpecIntens), 0.00001f, 1.0f);
			ImGui::SliderFloat("Hair Shininess", &(up_mat_comp.mNormalSpecShine), 1.0, 100.0f, "%.3f", 2.0f);
			ImGui::SliderFloat("Hair Length", &(up_mat_comp.mNormalScale), 0.000001f, 10.0f, "%.3f", 2.0f);
			ImGui::SliderFloat("Hair Random Length", &(up_mat_comp.mNormalRandom), 0.0f, 1.0f);
			ImGui::SliderFloat("Hair Diffuse Influence", &(up_mat_comp.mDiffuseSpecInfl), 0.0f, 1.0f);
			ImGui::SliderFloat("Hair Direction", &(up_mat_comp.mNormalRotValue), 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("Mesh"))
		{
			ImGui::ColorEdit3("Mesh Specular Color", up_mat_comp.mScanSpecColor.getData());
			ImGui::SliderFloat("Mesh Specular Intensity", &(up_mat_comp.mScanSpecIntens), 0.00001f, 1.0f);
			ImGui::SliderFloat("Mesh Shininess", &(up_mat_comp.mScanSpecShine), 1.0, 100.0f, "%.3f", 2.0f);
			ImGui::SliderFloat("Mesh Light Direction", &(up_mat_comp.mScanRotValue), 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("Wind"))
		{
			ImGui::SliderFloat("Wind Speed", &(up_mat_comp.mWindSpeed), 0.0f, 2.5f, "%.3f", 2.0f);
			ImGui::SliderFloat("Wind Influence", &(up_mat_comp.mWindScale), 0.0f, 1.0f, "%.3f", 1.0f);
			ImGui::SliderFloat("Wind Frequency", &(up_mat_comp.mWindFreq), 0.0f, 50.0f, "%.3f", 3.0f);
			ImGui::SliderFloat("Wind Random", &(up_mat_comp.mWindRandom), 0.0f, 1.0f, "%.3f", 2.0f);
		}

		// Fog
		if (ImGui::CollapsingHeader("Fog"))
		{
			if (ImGui::Checkbox("Link To Background Color", &mLinkFogToBackground))
					mBackgroundColorDirty = true;

			if(!mLinkFogToBackground)
				ImGui::ColorEdit3("Fog Color", up_mat_comp.mFogColor.getData());
				
			ImGui::SliderFloat("Fog Min", &(up_mat_comp.mFogMin), 0.0f, 1.0f);
			ImGui::SliderFloat("Fog Max", &(up_mat_comp.mFogMax),0.0f, 1.0f);
			ImGui::SliderFloat("Fog Power", &(up_mat_comp.mFogPower), 0.01f, 4.0f);
			ImGui::SliderFloat("Fog Influence", &(up_mat_comp.mFogInfluence),0.0f,1.0f);
		}

		if (ImGui::CollapsingHeader("Camera"))
		{
			// Control mode
			nap::ControlSelectComponentInstance& control_comp = mApp.mCameraEntity->getComponent<ControlSelectComponentInstance>();
			int cmethod = static_cast<int>(control_comp.getCurrentControlMethod());
			if (ImGui::Combo("Fly Mode", &cmethod, "Orbit\0FirstPerson\0\0"))
			{
				control_comp.selectControlMethod(static_cast<EControlMethod>(cmethod));
			}

			nap::PerspCameraComponentInstance& cam_comp = mApp.mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();
			float cfov = cam_comp.getFieldOfView();
			if (ImGui::SliderFloat("Field Of View", &cfov, 25.0f, 150.0f))
			{
				cam_comp.setFieldOfView(cfov);
			}

			FirstPersonControllerInstance& fps_comp = mApp.mCameraEntity->getComponent<FirstPersonControllerInstance>();
			if (ImGui::SliderFloat("Camera Movement Speed", &mCameraMovSpeed, 0.0f, 1.0f))
			{
				fps_comp.setMovementSpeed(mCamMaxMovSpeed * mCameraMovSpeed);
			}

			if (ImGui::SliderFloat("Camera Rotation Speed", &mCameraRotSpeed, 0.0f, 1.0f))
			{
				fps_comp.setRotationSpeed(mCamMaxRotSpeed * mCameraRotSpeed);
			}
		}

		if (ImGui::CollapsingHeader("Rendering"))
		{
			nap::RenderableMeshComponentInstance* normal_render_comp = mApp.mScanEntity->findComponentByID<RenderableMeshComponentInstance>("ScanNormalRenderableMesh");
			if (ImGui::Checkbox("Use Transparency", &mTransparent))
			{
				if (mTransparent)
				{
					normal_render_comp->getMaterialInstance().setBlendMode(EBlendMode::AlphaBlend);
					normal_render_comp->getMaterialInstance().setDepthMode(EDepthMode::ReadOnly);
				}
				else
				{
					normal_render_comp->getMaterialInstance().setBlendMode(EBlendMode::Opaque);
					normal_render_comp->getMaterialInstance().setDepthMode(EDepthMode::ReadWrite);
				}
			}

			// Render (draw) mode
			int rmode = static_cast<int>(mRenderMode);
			if (ImGui::Combo("Render Mode", &rmode, "Point\0Wireframe\0Fill\0\0"))
			{
				mRenderMode = static_cast<opengl::EPolygonMode>(rmode);
			}

			// Background color
			if (ImGui::ColorEdit4("Background Color", &(mBackgroundColor[0])))
				mBackgroundColorDirty = true;
		}

		ImGui::End();
	}


	void AtmosGui::showInfoWindow()
	{
		// Color used for highlights
		mApp.getCore().getFramerate();

		ImGui::Begin("Information");
		ImGui::Spacing();
		utility::getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImGui::TextColored(text_color, "%.3f ms/frame (%.1f FPS)", 1000.0f / mApp.getCore().getFramerate(), mApp.getCore().getFramerate());
		if (ImGui::CollapsingHeader("Texture Preview"))
		{
			float col_width = ImGui::GetContentRegionAvailWidth() * mTexPreviewDisplaySize;
			nap::SelectImageComponentInstance* img_selector_tileable = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentTileable");
			float ratio_tiled = static_cast<float>(img_selector_tileable->getImage().getWidth()) / static_cast<float>(img_selector_tileable->getImage().getHeight());
			ImGui::Image(img_selector_tileable->getImage(), { col_width, col_width / ratio_tiled});

			nap::SelectImageComponentInstance* img_selector_single = mApp.mScanEntity->findComponentByID<SelectImageComponentInstance>("SelectImageComponentSingle");
			float ratio_single = static_cast<float>(img_selector_single->getImage().getWidth()) / static_cast<float>(img_selector_single->getImage().getHeight());
			ImGui::Image(img_selector_single->getImage(), { col_width, col_width / ratio_single });

			ImGui::SliderFloat("Preview Size", &mTexPreviewDisplaySize, 0.0f, 1.0f);
		}
		
		ImGui::End();
	}

}