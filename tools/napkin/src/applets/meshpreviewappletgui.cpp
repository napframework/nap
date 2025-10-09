/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewappletgui.h"
#include "framemeshcomponent.h"
#include "meshpreviewapplet.h"

// External includes
#include <vulkan/vk_enum_string_helper.h>
#include <imgui/imgui.h>
#include <imguiutils.h>

namespace napkin
{
	void MeshPreviewAppletGUI::update(double elapsedTime)
	{
		// Fetch loaded texture
		auto& renderer = mApplet.mRenderEntity->getComponent<FrameMeshComponentInstance>();
		auto* loaded_mesh = renderer.getMesh();

		// Setup GUI for window
		ImGui::BeginMainMenuBar();

		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.7f;
		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mApplet.mClearColor.getData());
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Mesh", loaded_mesh != nullptr))
		{
			// Mesh attributes
			const auto& mesh_instance = loaded_mesh->getMeshInstance();
			ImGui::PushID(loaded_mesh);
			texDetail("Identifier", loaded_mesh->mID);
			texDetail("Vertices", utility::stringFormat("%d", mesh_instance.getNumVertices()));
			texDetail("Shapes", utility::stringFormat("%d", mesh_instance.getNumShapes()));
			texDetail("Cull Mode", RTTI_OF(ECullMode), mesh_instance.getCullMode());
			texDetail("Draw Mode", RTTI_OF(EDrawMode), mesh_instance.getDrawMode());
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
			const auto& bounds = renderer.getBounds();
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

		if (ImGui::BeginMenu("Applet"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", mApplet.getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", elapsedTime * 1000.0).c_str());
			ImGui::EndMenu();
		}

		// Add frame icon
		if (loaded_mesh != nullptr &&
			ImGui::ImageButton(mApplet.mGuiService->getIcon(nap::icon::frame), { ico_height, ico_height }, "Frame Selection"))
		{
			// Frame object & reset rotation
			renderer.frame();
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
