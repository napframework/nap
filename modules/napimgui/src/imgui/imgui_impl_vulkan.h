// dear imgui: Renderer for Vulkan
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bit indices.
//  [X] Renderer: User texture binding. See https://github.com/ocornut/imgui/pull/914

// Modified (by naivisoftware) to support custom texture binding, based on: https://github.com/ocornut/imgui/pull/914
// Also added support for multiple ImGUI contexts, where every context has it's own set of buffers.
// Buffers are created lazily on draw, but can be destroyed explicitly by calling: ImGui_ImplVulkan_RemoveContext().
// Instead of 1 fixed pipeline, pipelines are created based on the number of MSAA samples.
// These changes allow for rendering a context to a different viewport, with different MSAA settings,
// Together with the display of custom textures in ImGUI

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// The aim of imgui_impl_vulkan.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/imgui/

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering back-end in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the back-end itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#include <vulkan/vulkan.h>
#include <renderwindow.h>

// Initialization data, for ImGui_ImplVulkan_Init()
// [Please zero-clear before use!]
struct ImGui_ImplVulkan_InitInfo
{
    VkInstance          Instance;
    VkPhysicalDevice    PhysicalDevice;
    VkDevice            Device;
    uint32_t            QueueFamily;
    VkQueue             Queue;
    VkPipelineCache     PipelineCache;
    VkDescriptorPool    DescriptorPool;
    uint32_t            MinImageCount;          // >= 2
    uint32_t            ImageCount;             // >= MinImageCount
    VkSampleCountFlagBits        MSAASamples;   // >= VK_SAMPLE_COUNT_1_BIT
    const VkAllocationCallbacks* Allocator;
    void                (*CheckVkResultFn)(VkResult err);
};

// Called by user code
IMGUI_IMPL_API bool			ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info, VkRenderPass render_pass);
IMGUI_IMPL_API void			ImGui_ImplVulkan_Shutdown();
IMGUI_IMPL_API void			ImGui_ImplVulkan_NewFrame();
IMGUI_IMPL_API void			ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, ImGuiContext* context, VkCommandBuffer command_buffer, VkRenderPass render_pass, VkSampleCountFlagBits samples);
IMGUI_IMPL_API bool			ImGui_ImplVulkan_CreateFontsTexture(VkCommandBuffer command_buffer);
IMGUI_IMPL_API void			ImGui_ImplVulkan_DestroyFontUploadObjects();
IMGUI_IMPL_API void			ImGui_ImplVulkan_RemoveContext(ImGuiContext* context);
IMGUI_IMPL_API void			ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)