/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewappletgui.h"
#include "meshpreviewloadcomponent.h"
#include "meshpreviewapplet.h"

// External includes
#include <vulkan/vk_enum_string_helper.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <directionallightcomponent.h>

namespace napkin
{
	static const std::array<const char*, 2>& getBlendOptions()
	{
		static const std::array<const char*, 2> options =
		{
			RTTI_OF(EBlendMode).get_enumeration().value_to_name(EBlendMode::Additive).data(),
			RTTI_OF(EBlendMode).get_enumeration().value_to_name(EBlendMode::Opaque).data()
		};
		return options;
	}


	static int getBlendIndex(EBlendMode mode)
	{
		// Current blend mode name
		auto blend_mode = RTTI_OF(EBlendMode).get_enumeration().value_to_name(mode);
		assert(!blend_mode.empty());

		const auto& options = getBlendOptions();
		for (int i = 0; i < options.size(); i++)
		{
			if (strcmp(options[i], blend_mode.data()) == 0)
				return i;
		}
		return -1;
	}


	MeshPreviewAppletGUI::MeshPreviewAppletGUI(MeshPreviewApplet& applet) :
		mApplet(applet)
	{ }


	void MeshPreviewAppletGUI::init()
	{
		// Push additive blend mode
		auto& controller = mApplet.mLoaderEntity->getComponent<MeshPreviewLoadComponentInstance>();
		controller.setBlendMode(EBlendMode::Additive);
	}


	void MeshPreviewAppletGUI::update(double elapsedTime)
	{
		// Fetch loaded texture
		auto& controller = mApplet.mLoaderEntity->getComponent<MeshPreviewLoadComponentInstance>();
		auto* loaded_mesh = controller.getMesh();

		// Setup GUI for window
		ImGui::BeginMainMenuBar();

		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.7f;
		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mApplet.mClearColor.getData());
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Details", loaded_mesh != nullptr))
		{
			// Mesh attributes
			const auto& mesh_instance = loaded_mesh->getMeshInstance();
			ImGui::PushID(loaded_mesh);
			texDetail("Identifier", loaded_mesh->mID);
			texDetail("Vertices", utility::stringFormat("%d", mesh_instance.getNumVertices()));
			if(controller.isTriangleMesh())
				texDetail("Triangles", utility::stringFormat("%d", controller.getTriangleCount()));
			texDetail("Shapes", utility::stringFormat("%d", mesh_instance.getNumShapes()));
			texDetail("Cull Mode", RTTI_OF(ECullMode), mesh_instance.getCullMode());
			texDetail("Topology", RTTI_OF(EDrawMode), controller.getTopology());
			texDetail("Polygon Mode ", RTTI_OF(EPolygonMode), mesh_instance.getPolygonMode());
			texDetail("Usage", RTTI_OF(EMemoryUsage), mesh_instance.getUsage());

			// Vertex attributes
			texDetail("Attributes", utility::stringFormat("%d", mesh_instance.getAttributes().size()));
			for (auto i = 0; i < mesh_instance.getAttributes().size(); i++)
			{
				auto& attr = *mesh_instance.getAttributes()[i];
				ImGui::PushID(&attr);
				texDetail(utility::stringFormat("\t%d", i), attr.mAttributeID,
					nap::utility::stripNamespace(attr.getElementType().get_name().data()));
				ImGui::PopID();
			}

			// Mesh bound dimensions
			const auto& bounds = controller.getObjectBounds();
			texDetail("Bounds", "");
			texDetail("\tWidth", utility::stringFormat("%.1f", bounds.getWidth()), "unit(s)");
			texDetail("\tHeight", utility::stringFormat("%.1f", bounds.getHeight()), "unit(s)");
			texDetail("\tDepth", utility::stringFormat("%.1f", bounds.getDepth()), "unit(s)");

			// Mesh bound coordinates
			static constexpr const float cutoff = 0.01;
			static constexpr const glm::vec3 zero = { 0.0f, 0.0f, 0.0f };
			auto min = glm::length(bounds.getMin()) < cutoff ? zero : bounds.getMin();
			auto max = glm::length(bounds.getMax()) < cutoff ? zero : bounds.getMax();
			auto cen = glm::length(bounds.getCenter()) < cutoff ? zero : bounds.getCenter();
			texDetail("\tMin", utility::stringFormat("%.1f, %.1f, %.1f", min.x, min.y, min.z));
			texDetail("\tMax", utility::stringFormat("%.1f, %.1f, %.1f", max.x, max.y, max.z));
			texDetail("\tCenter", utility::stringFormat("%.1f, %.1f, %.1f", cen.x, cen.y, cen.z));

			ImGui::PopID();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Mesh", loaded_mesh != nullptr))
		{
			// Handle blending -> controller is leading
			ImGui::PushID("mesh");
			int blend_index = getBlendIndex(controller.getBlendMode());
			if (blend_index >= 0)
			{
				const auto& blend_options = getBlendOptions();
				if (ImGui::Combo("Blend Mode", &blend_index, blend_options.data(), blend_options.size()))
				{
					auto blend_value = RTTI_OF(EBlendMode).get_enumeration().name_to_value(blend_options[blend_index]);
					assert(blend_value.is_valid());
					controller.setBlendMode(blend_value.get_value<EBlendMode>());
				}
			}

			// Mesh Color
			auto color = controller.getMeshColor();
			if (ImGui::ColorEdit4("Color", color.getData()))
				controller.setMeshColor(color);

			// Mesh rotate
			glm::vec3 rotate = controller.getRotate();
			if (ImGui::InputFloat3("Rotate", &rotate.x, 2))
				controller.setRotate(rotate);

			// Mesh scale
			glm::vec3 scale = controller.getScale();
			if (ImGui::InputFloat3("Scale", &scale.x, 2))
				controller.setScale(scale);

			// Mesh rotation speed
			float rotate_speed = controller.getRotateSpeed();
			if (ImGui::SliderFloat("Rotation Speed", &rotate_speed, 0.0f, 1.0f, "%.3f", 2.0f))
				controller.setRotateSpeed(rotate_speed);

			// Bounds
			if (ImGui::CollapsingHeader("Bounds"))
			{
				// If bounding box is visible
				ImGui::PushID("bounds");
				bool draw_bounds = controller.getDrawBounds();
				if (ImGui::Checkbox("Show", &draw_bounds))
					controller.setDrawBounds(draw_bounds);

				// Bounding box color
				auto bounds_color = controller.getBoundsColor();
				if (ImGui::ColorEdit3("Color", bounds_color.getData()))
					controller.setBoundsColor(bounds_color);
				ImGui::PopID();
			}

			// Wireframe
			if (controller.hasWireframe())
			{
				if (ImGui::CollapsingHeader("Wireframe"))
				{
					ImGui::PushID("wireframe");
					bool draw_wire = controller.getDrawWireframe();
					if (ImGui::Checkbox("Show", &draw_wire))
						controller.setDrawWireFrame(draw_wire);

					color = controller.getWireColor();
					if (ImGui::ColorEdit4("Color", color.getData()))
						controller.setWireColor(color);

					// Wireframe line width
					auto width = controller.getWireWidth();
					if (ImGui::SliderFloat("Width", &width, 1.0f, 5.0f))
						controller.setWireWidth(width);
					ImGui::PopID();
				}
			}

			ImGui::PopID();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Lights", loaded_mesh != nullptr))
		{
			auto& children = mApplet.mLightsEntity->getChildren(); 
			int i = 1;
			for (auto& child : children)
			{
				ImGui::PushID(child);

				// Light intensity
				auto& light = child->getComponent<DirectionalLightComponentInstance>();
				float inten = light.getIntensity();
				if (ImGui::SliderFloat(utility::stringFormat("Intensity %d", i).c_str(), &inten, 0.0f, 2.0f))
					light.setIntensity(inten);

				// Light color
				RGBColorFloat color = light.getColor();
				if (ImGui::ColorEdit3(utility::stringFormat("Color %d", i).c_str(), color.getData()))
					light.setColor(color);

				ImGui::PopID(); i++;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Applet"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", mApplet.getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", elapsedTime * 1000.0).c_str());
			ImGui::EndMenu();
		}

		// Add frame icon
		if (loaded_mesh != nullptr &&
			ImGui::ImageButton(mApplet.mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame Mesh"))
		{
			// Frame object & reset rotation
			controller.frame();
		}
		ImGui::EndMainMenuBar();
	}


	void MeshPreviewAppletGUI::draw()
	{
		mApplet.mGuiService->draw();
	}


	void MeshPreviewAppletGUI::texDetail(std::string&& label, const std::string& value, std::string&& appendix)
	{
		float xoff = gui::dpi * mApplet.mGuiService->getScale() * 1.25f;
		float yoff = xoff * 2.0f;
		ImGui::TextColored(mApplet.mGuiService->getPalette().mFront3Color, label.c_str());
		ImGui::SameLine(xoff);
		ImGui::Text(value.c_str());
		if (!appendix.empty())
		{
			ImGui::SameLine(yoff);
			ImGui::TextColored(mApplet.mGuiService->getPalette().mFront1Color, appendix.c_str());
		}
	}


	void MeshPreviewAppletGUI::texDetail(std::string&& label, rtti::TypeInfo enumerator, rtti::Variant argument)
	{
		assert(enumerator.is_enumeration());
		texDetail(std::move(label), enumerator.get_enumeration().value_to_name(argument).data());
	}
}

