// Local Includes
#include "imguiservice.h"
#include "imguifont.h"
#include "imgui/imgui.h"

// External Includes
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/core.h>
#include <color.h>
#include <SDL_clipboard.h>
#include <SDL_syswm.h>
#include <SDL_mouse.h> 
#include <SDL_keyboard.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMGuiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

#define IMGUI_VK_QUEUED_FRAMES 2

// Static data associated with IMGUI: TODO: Use own render classes and remove global state!
static bool         gMousePressed[3] = { false, false, false };
static float        gMouseWheel = 0.0f;
static int          gShaderHandle = 0, gVertHandle = 0, gFragHandle = 0;
static int          gAttribLocationTex = 0, gAttribLocationProjMtx = 0;
static int          gAttribLocationPosition = 0, gAttribLocationUV = 0, gAttribLocationColor = 0;
static unsigned int gVboHandle = 0, gVaoHandle = 0, gElementsHandle = 0;

// Vulkan Data
static VkAllocationCallbacks* g_Allocator = NULL;
static VkPhysicalDevice       g_Gpu = VK_NULL_HANDLE;
static VkDevice               g_Device = VK_NULL_HANDLE;
static VkPipelineCache        g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool       g_DescriptorPool = VK_NULL_HANDLE;
static void(*g_CheckVkResult)(VkResult err) = NULL;

static VkCommandBuffer        g_CommandBuffer = VK_NULL_HANDLE;
static size_t                 g_BufferMemoryAlignment = 256;
static VkPipelineCreateFlags  g_PipelineCreateFlags = 0;
static int                    g_FrameIndex = 0;

static VkDescriptorSetLayout  g_DescriptorSetLayout = VK_NULL_HANDLE;
static VkPipelineLayout       g_PipelineLayout = VK_NULL_HANDLE;
static VkDescriptorSet        g_DescriptorSet = VK_NULL_HANDLE;
static VkPipeline             g_Pipeline = VK_NULL_HANDLE;

static VkSampler              g_FontSampler = VK_NULL_HANDLE;
static VkDeviceMemory         g_FontMemory = VK_NULL_HANDLE;
static VkImage                g_FontImage = VK_NULL_HANDLE;
static VkImageView            g_FontView = VK_NULL_HANDLE;

static VkDeviceMemory         g_VertexBufferMemory[IMGUI_VK_QUEUED_FRAMES] = {};
static VkDeviceMemory         g_IndexBufferMemory[IMGUI_VK_QUEUED_FRAMES] = {};
static size_t                 g_VertexBufferSize[IMGUI_VK_QUEUED_FRAMES] = {};
static size_t                 g_IndexBufferSize[IMGUI_VK_QUEUED_FRAMES] = {};
static VkBuffer               g_VertexBuffer[IMGUI_VK_QUEUED_FRAMES] = {};
static VkBuffer               g_IndexBuffer[IMGUI_VK_QUEUED_FRAMES] = {};

static VkDeviceMemory         g_UploadBufferMemory = VK_NULL_HANDLE;
static VkBuffer               g_UploadBuffer = VK_NULL_HANDLE;

VkShaderModule				  g_VertModule = VK_NULL_HANDLE;
VkShaderModule				  g_FragModule = VK_NULL_HANDLE;

static uint32_t __glsl_shader_vert_spv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

static uint32_t __glsl_shader_frag_spv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
	0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
	0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
	0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
	0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
	0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
	0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
	0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
	0x00010038
};

namespace nap
{
	static uint32_t findMemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
	{
		VkPhysicalDeviceMemoryProperties prop;
		vkGetPhysicalDeviceMemoryProperties(g_Gpu, &prop);
		for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
			if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
				return i;
		return 0xffffffff; // Unable to find memoryType
	}

	static void checkVkResult(VkResult err)
	{
		if (g_CheckVkResult)
			g_CheckVkResult(err);
	}

	// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
	void renderDrawLists(ImDrawData* draw_data)
	{
		VkResult err;
		ImGuiIO& io = ImGui::GetIO();

		// Create the Vertex Buffer:
		size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		if (!g_VertexBuffer[g_FrameIndex] || g_VertexBufferSize[g_FrameIndex] < vertex_size)
		{
			if (g_VertexBuffer[g_FrameIndex])
				vkDestroyBuffer(g_Device, g_VertexBuffer[g_FrameIndex], g_Allocator);
			if (g_VertexBufferMemory[g_FrameIndex])
				vkFreeMemory(g_Device, g_VertexBufferMemory[g_FrameIndex], g_Allocator);
			size_t vertex_buffer_size = ((vertex_size - 1) / g_BufferMemoryAlignment + 1) * g_BufferMemoryAlignment;
			VkBufferCreateInfo buffer_info = {};
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.size = vertex_buffer_size;
			buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			err = vkCreateBuffer(g_Device, &buffer_info, g_Allocator, &g_VertexBuffer[g_FrameIndex]);
			checkVkResult(err);
			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(g_Device, g_VertexBuffer[g_FrameIndex], &req);
			g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.alignment) ? g_BufferMemoryAlignment : req.alignment;
			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = req.size;
			alloc_info.memoryTypeIndex = findMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
			err = vkAllocateMemory(g_Device, &alloc_info, g_Allocator, &g_VertexBufferMemory[g_FrameIndex]);
			checkVkResult(err);
			err = vkBindBufferMemory(g_Device, g_VertexBuffer[g_FrameIndex], g_VertexBufferMemory[g_FrameIndex], 0);
			checkVkResult(err);
			g_VertexBufferSize[g_FrameIndex] = vertex_buffer_size;
		}

		// Create the Index Buffer:
		size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		if (!g_IndexBuffer[g_FrameIndex] || g_IndexBufferSize[g_FrameIndex] < index_size)
		{
			if (g_IndexBuffer[g_FrameIndex])
				vkDestroyBuffer(g_Device, g_IndexBuffer[g_FrameIndex], g_Allocator);
			if (g_IndexBufferMemory[g_FrameIndex])
				vkFreeMemory(g_Device, g_IndexBufferMemory[g_FrameIndex], g_Allocator);
			size_t index_buffer_size = ((index_size - 1) / g_BufferMemoryAlignment + 1) * g_BufferMemoryAlignment;
			VkBufferCreateInfo buffer_info = {};
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.size = index_buffer_size;
			buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			err = vkCreateBuffer(g_Device, &buffer_info, g_Allocator, &g_IndexBuffer[g_FrameIndex]);
			checkVkResult(err);
			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(g_Device, g_IndexBuffer[g_FrameIndex], &req);
			g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.alignment) ? g_BufferMemoryAlignment : req.alignment;
			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = req.size;
			alloc_info.memoryTypeIndex = findMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
			err = vkAllocateMemory(g_Device, &alloc_info, g_Allocator, &g_IndexBufferMemory[g_FrameIndex]);
			checkVkResult(err);
			err = vkBindBufferMemory(g_Device, g_IndexBuffer[g_FrameIndex], g_IndexBufferMemory[g_FrameIndex], 0);
			checkVkResult(err);
			g_IndexBufferSize[g_FrameIndex] = index_buffer_size;
		}

		// Upload Vertex and index Data:
		{
			ImDrawVert* vtx_dst;
			ImDrawIdx* idx_dst;
			err = vkMapMemory(g_Device, g_VertexBufferMemory[g_FrameIndex], 0, vertex_size, 0, (void**)(&vtx_dst));
			checkVkResult(err);
			err = vkMapMemory(g_Device, g_IndexBufferMemory[g_FrameIndex], 0, index_size, 0, (void**)(&idx_dst));
			checkVkResult(err);
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				vtx_dst += cmd_list->VtxBuffer.Size;
				idx_dst += cmd_list->IdxBuffer.Size;
			}
			VkMappedMemoryRange range[2] = {};
			range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[0].memory = g_VertexBufferMemory[g_FrameIndex];
			range[0].size = VK_WHOLE_SIZE;
			range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[1].memory = g_IndexBufferMemory[g_FrameIndex];
			range[1].size = VK_WHOLE_SIZE;
			err = vkFlushMappedMemoryRanges(g_Device, 2, range);
			checkVkResult(err);
			vkUnmapMemory(g_Device, g_VertexBufferMemory[g_FrameIndex]);
			vkUnmapMemory(g_Device, g_IndexBufferMemory[g_FrameIndex]);
		}

		// Bind pipeline and descriptor sets:
		{
			vkCmdBindPipeline(g_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Pipeline);
			VkDescriptorSet desc_set[1] = { g_DescriptorSet };
			vkCmdBindDescriptorSets(g_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_PipelineLayout, 0, 1, desc_set, 0, NULL);
		}

		// Bind Vertex And Index Buffer:
		{
			VkBuffer vertex_buffers[1] = { g_VertexBuffer[g_FrameIndex] };
			VkDeviceSize vertex_offset[1] = { 0 };
			vkCmdBindVertexBuffers(g_CommandBuffer, 0, 1, vertex_buffers, vertex_offset);
			vkCmdBindIndexBuffer(g_CommandBuffer, g_IndexBuffer[g_FrameIndex], 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
		}

		// Setup viewport:
		{
			VkViewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = ImGui::GetIO().DisplaySize.x;
			viewport.height = ImGui::GetIO().DisplaySize.y;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(g_CommandBuffer, 0, 1, &viewport);
		}

		// Setup scale and translation:
		{
			float scale[2];
			scale[0] = 2.0f / io.DisplaySize.x;
			scale[1] = 2.0f / io.DisplaySize.y;
			float translate[2];
			translate[0] = -1.0f;
			translate[1] = -1.0f;
			vkCmdPushConstants(g_CommandBuffer, g_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
			vkCmdPushConstants(g_CommandBuffer, g_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
		}

		// Render the command lists:
		int vtx_offset = 0;
		int idx_offset = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					VkRect2D scissor;
					scissor.offset.x = (int32_t)(pcmd->ClipRect.x) > 0 ? (int32_t)(pcmd->ClipRect.x) : 0;
					scissor.offset.y = (int32_t)(pcmd->ClipRect.y) > 0 ? (int32_t)(pcmd->ClipRect.y) : 0;
					scissor.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
					scissor.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1); // FIXME: Why +1 here?
					vkCmdSetScissor(g_CommandBuffer, 0, 1, &scissor);
					vkCmdDrawIndexed(g_CommandBuffer, pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.Size;
		}
	}

	static const char* getClipboardText(void*)
	{
		return SDL_GetClipboardText();
	}


	static void setClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
	}

	VkCommandBuffer beginSingleTimeCommands(RenderService& renderService)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = renderService.getCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(renderService.getDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(RenderService& renderService, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(renderService.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(renderService.getGraphicsQueue());

		vkFreeCommandBuffers(renderService.getDevice(), renderService.getCommandPool(), 1, &commandBuffer);
	}


	/**
	* Deletes all GPU allocated resources
	* TODO: Implement using own render objects
	*/
	void invalidateFontUploadObjects()
	{
		if (g_UploadBuffer)
		{
			vkDestroyBuffer(g_Device, g_UploadBuffer, g_Allocator);
			g_UploadBuffer = VK_NULL_HANDLE;
		}
		if (g_UploadBufferMemory)
		{
			vkFreeMemory(g_Device, g_UploadBufferMemory, g_Allocator);
			g_UploadBufferMemory = VK_NULL_HANDLE;
		}
	}


	/**
	 * Creates font texture GPU resources
	 * TODO: Implement using own render objects
	 */
	bool createFontsTexture(RenderService& renderService)
	{
		VkCommandBuffer command_buffer = beginSingleTimeCommands(renderService);

		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		size_t upload_size = width*height * 4 * sizeof(char);

		VkResult err;

		// Create the Image:
		{
			VkImageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.extent.width = width;
			info.extent.height = height;
			info.extent.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			err = vkCreateImage(g_Device, &info, g_Allocator, &g_FontImage);
			checkVkResult(err);
			VkMemoryRequirements req;
			vkGetImageMemoryRequirements(g_Device, g_FontImage, &req);
			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = req.size;
			alloc_info.memoryTypeIndex = findMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
			err = vkAllocateMemory(g_Device, &alloc_info, g_Allocator, &g_FontMemory);
			checkVkResult(err);
			err = vkBindImageMemory(g_Device, g_FontImage, g_FontMemory, 0);
			checkVkResult(err);
		}

		// Create the Image View:
		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = g_FontImage;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = VK_FORMAT_R8G8B8A8_UNORM;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.layerCount = 1;
			err = vkCreateImageView(g_Device, &info, g_Allocator, &g_FontView);
			checkVkResult(err);
		}

		// Update the Descriptor Set:
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = g_FontSampler;
			desc_image[0].imageView = g_FontView;
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = g_DescriptorSet;
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(g_Device, 1, write_desc, 0, NULL);
		}

		// Create the Upload Buffer:
		{
			VkBufferCreateInfo buffer_info = {};
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.size = upload_size;
			buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			err = vkCreateBuffer(g_Device, &buffer_info, g_Allocator, &g_UploadBuffer);
			checkVkResult(err);
			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(g_Device, g_UploadBuffer, &req);
			g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.alignment) ? g_BufferMemoryAlignment : req.alignment;
			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = req.size;
			alloc_info.memoryTypeIndex = findMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
			err = vkAllocateMemory(g_Device, &alloc_info, g_Allocator, &g_UploadBufferMemory);
			checkVkResult(err);
			err = vkBindBufferMemory(g_Device, g_UploadBuffer, g_UploadBufferMemory, 0);
			checkVkResult(err);
		}

		// Upload to Buffer:
		{
			char* map = NULL;
			err = vkMapMemory(g_Device, g_UploadBufferMemory, 0, upload_size, 0, (void**)(&map));
			checkVkResult(err);
			memcpy(map, pixels, upload_size);
			VkMappedMemoryRange range[1] = {};
			range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[0].memory = g_UploadBufferMemory;
			range[0].size = upload_size;
			err = vkFlushMappedMemoryRanges(g_Device, 1, range);
			checkVkResult(err);
			vkUnmapMemory(g_Device, g_UploadBufferMemory);
		}
		// Copy to Image:
		{
			VkImageMemoryBarrier copy_barrier[1] = {};
			copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copy_barrier[0].image = g_FontImage;
			copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_barrier[0].subresourceRange.levelCount = 1;
			copy_barrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = width;
			region.imageExtent.height = height;
			region.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(command_buffer, g_UploadBuffer, g_FontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			VkImageMemoryBarrier use_barrier[1] = {};
			use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier[0].image = g_FontImage;
			use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			use_barrier[0].subresourceRange.levelCount = 1;
			use_barrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
		}

		// Store our identifier
		io.Fonts->TexID = (void *)(intptr_t)g_FontImage;

		endSingleTimeCommands(renderService, command_buffer);

		invalidateFontUploadObjects();

		return true;
	}


	void invalidateDeviceObjects()
	{
		invalidateFontUploadObjects();

		for (int i = 0; i < IMGUI_VK_QUEUED_FRAMES; i++)
		{
			if (g_VertexBuffer[i]) { vkDestroyBuffer(g_Device, g_VertexBuffer[i], g_Allocator); g_VertexBuffer[i] = VK_NULL_HANDLE; }
			if (g_VertexBufferMemory[i]) { vkFreeMemory(g_Device, g_VertexBufferMemory[i], g_Allocator); g_VertexBufferMemory[i] = VK_NULL_HANDLE; }
			if (g_IndexBuffer[i]) { vkDestroyBuffer(g_Device, g_IndexBuffer[i], g_Allocator); g_IndexBuffer[i] = VK_NULL_HANDLE; }
			if (g_IndexBufferMemory[i]) { vkFreeMemory(g_Device, g_IndexBufferMemory[i], g_Allocator); g_IndexBufferMemory[i] = VK_NULL_HANDLE; }
		}

		if (g_FontView) { vkDestroyImageView(g_Device, g_FontView, g_Allocator); g_FontView = VK_NULL_HANDLE; }
		if (g_FontImage) { vkDestroyImage(g_Device, g_FontImage, g_Allocator); g_FontImage = VK_NULL_HANDLE; }
		if (g_FontMemory) { vkFreeMemory(g_Device, g_FontMemory, g_Allocator); g_FontMemory = VK_NULL_HANDLE; }
		if (g_FontSampler) { vkDestroySampler(g_Device, g_FontSampler, g_Allocator); g_FontSampler = VK_NULL_HANDLE; }
		if (g_DescriptorSet) { vkFreeDescriptorSets(g_Device, g_DescriptorPool, 1, &g_DescriptorSet); g_DescriptorSet = VK_NULL_HANDLE; }
		if (g_DescriptorSetLayout) { vkDestroyDescriptorSetLayout(g_Device, g_DescriptorSetLayout, g_Allocator); g_DescriptorSetLayout = VK_NULL_HANDLE; }
		if (g_PipelineLayout) { vkDestroyPipelineLayout(g_Device, g_PipelineLayout, g_Allocator); g_PipelineLayout = VK_NULL_HANDLE; }
		if (g_Pipeline) { vkDestroyPipeline(g_Device, g_Pipeline, g_Allocator); g_Pipeline = VK_NULL_HANDLE; }

		if (g_VertModule) { vkDestroyShaderModule(g_Device, g_VertModule, g_Allocator); g_VertModule = VK_NULL_HANDLE; }
		if (g_FragModule) { vkDestroyShaderModule(g_Device, g_FragModule, g_Allocator); g_FragModule= VK_NULL_HANDLE; }

		if (g_DescriptorPool) { vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator); g_DescriptorPool = VK_NULL_HANDLE; }
	}


	/**
	 * Creates all GPU allocated resources
	 * TODO: Implement using own render objects
	 */
	bool createDeviceObjects()
	{
		VkResult err;

		// Create The Shader Modules:
		{
			VkShaderModuleCreateInfo vert_info = {};
			vert_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			vert_info.codeSize = sizeof(__glsl_shader_vert_spv);
			vert_info.pCode = (uint32_t*)__glsl_shader_vert_spv;
			err = vkCreateShaderModule(g_Device, &vert_info, g_Allocator, &g_VertModule);
			checkVkResult(err);
			VkShaderModuleCreateInfo frag_info = {};
			frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			frag_info.codeSize = sizeof(__glsl_shader_frag_spv);
			frag_info.pCode = (uint32_t*)__glsl_shader_frag_spv;
			err = vkCreateShaderModule(g_Device, &frag_info, g_Allocator, &g_FragModule);
			checkVkResult(err);
		}

		if (!g_FontSampler)
		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.minLod = -1000;
			info.maxLod = 1000;
			info.maxAnisotropy = 1.0f;
			err = vkCreateSampler(g_Device, &info, g_Allocator, &g_FontSampler);
			checkVkResult(err);
		}

		if (!g_DescriptorSetLayout)
		{
			VkSampler sampler[1] = { g_FontSampler };
			VkDescriptorSetLayoutBinding binding[1] = {};
			binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding[0].descriptorCount = 1;
			binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding[0].pImmutableSamplers = sampler;
			VkDescriptorSetLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = 1;
			info.pBindings = binding;
			err = vkCreateDescriptorSetLayout(g_Device, &info, g_Allocator, &g_DescriptorSetLayout);
			checkVkResult(err);
		}

		// Create Descriptor Set:
		{
			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = g_DescriptorPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &g_DescriptorSetLayout;
			err = vkAllocateDescriptorSets(g_Device, &alloc_info, &g_DescriptorSet);
			checkVkResult(err);
		}

		if (!g_PipelineLayout)
		{
			VkPushConstantRange push_constants[1] = {};
			push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			push_constants[0].offset = sizeof(float) * 0;
			push_constants[0].size = sizeof(float) * 4;
			VkDescriptorSetLayout set_layout[1] = { g_DescriptorSetLayout };
			VkPipelineLayoutCreateInfo layout_info = {};
			layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layout_info.setLayoutCount = 1;
			layout_info.pSetLayouts = set_layout;
			layout_info.pushConstantRangeCount = 1;
			layout_info.pPushConstantRanges = push_constants;
			err = vkCreatePipelineLayout(g_Device, &layout_info, g_Allocator, &g_PipelineLayout);
			checkVkResult(err);
		}

		return true;
	}

	static void createPipeline(VkRenderPass renderPass, VkSampleCountFlagBits sampleCount, bool enableSampleShading)
	{
		if (g_Pipeline != nullptr)
			vkDestroyPipeline(g_Device, g_Pipeline, g_Allocator);

		VkPipelineShaderStageCreateInfo stage[2] = {};
		stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage[0].module = g_VertModule;
		stage[0].pName = "main";
		stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage[1].module = g_FragModule;
		stage[1].pName = "main";

		VkVertexInputBindingDescription binding_desc[1] = {};
		binding_desc[0].stride = sizeof(ImDrawVert);
		binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attribute_desc[3] = {};
		attribute_desc[0].location = 0;
		attribute_desc[0].binding = binding_desc[0].binding;
		attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_desc[0].offset = (size_t)(&((ImDrawVert*)0)->pos);
		attribute_desc[1].location = 1;
		attribute_desc[1].binding = binding_desc[0].binding;
		attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_desc[1].offset = (size_t)(&((ImDrawVert*)0)->uv);
		attribute_desc[2].location = 2;
		attribute_desc[2].binding = binding_desc[0].binding;
		attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attribute_desc[2].offset = (size_t)(&((ImDrawVert*)0)->col);

		VkPipelineVertexInputStateCreateInfo vertex_info = {};
		vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_info.vertexBindingDescriptionCount = 1;
		vertex_info.pVertexBindingDescriptions = binding_desc;
		vertex_info.vertexAttributeDescriptionCount = 3;
		vertex_info.pVertexAttributeDescriptions = attribute_desc;

		VkPipelineInputAssemblyStateCreateInfo ia_info = {};
		ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo raster_info = {};
		raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_info.cullMode = VK_CULL_MODE_NONE;
		raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_info.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo ms_info = {};
		ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms_info.rasterizationSamples = sampleCount;
		ms_info.pNext = nullptr;
		ms_info.flags = 0;
		ms_info.minSampleShading = 1.0f;
		ms_info.sampleShadingEnable = static_cast<int>(enableSampleShading);

		VkPipelineColorBlendAttachmentState color_attachment[1] = {};
		color_attachment[0].blendEnable = VK_TRUE;
		color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
		color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
		color_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineDepthStencilStateCreateInfo depth_info = {};
		depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		VkPipelineColorBlendStateCreateInfo blend_info = {};
		blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend_info.attachmentCount = 1;
		blend_info.pAttachments = color_attachment;

		VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = 2;
		dynamic_state.pDynamicStates = dynamic_states;

		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.flags = g_PipelineCreateFlags;
		info.stageCount = 2;
		info.pStages = stage;
		info.pVertexInputState = &vertex_info;
		info.pInputAssemblyState = &ia_info;
		info.pViewportState = &viewport_info;
		info.pRasterizationState = &raster_info;
		info.pMultisampleState = &ms_info;
		info.pDepthStencilState = &depth_info;
		info.pColorBlendState = &blend_info;
		info.pDynamicState = &dynamic_state;
		info.layout = g_PipelineLayout;
		info.renderPass = renderPass;
		VkResult err = vkCreateGraphicsPipelines(g_Device, g_PipelineCache, 1, &info, g_Allocator, &g_Pipeline);
		checkVkResult(err);
	}


	static void setGuiWindow(SDL_Window* window)
	{
#ifdef _WIN32
		ImGuiIO& io = ImGui::GetIO();
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);
		io.ImeWindowHandle = wmInfo.info.win.window;
#else
		(void)window;
#endif
	}


	static void newFrame(GLWindow& window)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		SDL_GetWindowSize(window.getNativeWindow(), &w, &h);
		SDL_GL_GetDrawableSize(window.getNativeWindow(), &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

		// Setup inputs
		// (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
		int mx, my;
		Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
		if (SDL_GetWindowFlags(window.getNativeWindow()) & SDL_WINDOW_MOUSE_FOCUS)
			io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		else
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

		io.MouseDown[0] = gMousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = gMousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = gMousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		gMousePressed[0] = gMousePressed[1] = gMousePressed[2] = false;

		io.MouseWheel = gMouseWheel;
		gMouseWheel = 0.0f;

		// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
		ImGui::NewFrame();
	}


	static void applyStyle()
	{
		// Get colors
		static const RGBColorFloat NAPDARK = RGBColor8(0x11, 0x14, 0x26).convert<RGBColorFloat>();
		static const RGBColorFloat NAPBACK = RGBColor8(0x2D, 0x2E, 0x42).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO1 = RGBColor8(0x52, 0x54, 0x6A).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO2 = RGBColor8(0x5D, 0x5E, 0x73).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO3 = RGBColor8(0x8B, 0x8C, 0xA0).convert<RGBColorFloat>();
		static const RGBColorFloat NAPHIGH = RGBColor8(0xC8, 0x69, 0x69).convert<RGBColorFloat>();

		const ImVec4 IMGUI_NAPDARK(NAPDARK.getRed(), NAPDARK.getGreen(), NAPDARK.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPBACK(NAPBACK.getRed(), NAPBACK.getGreen(), NAPBACK.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPMODAL(NAPBACK.getRed(), NAPBACK.getGreen(), NAPBACK.getBlue(), 0.5f);
		const ImVec4 IMGUI_NAPFRO1(NAPFRO1.getRed(), NAPFRO1.getGreen(), NAPFRO1.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO2(NAPFRO2.getRed(), NAPFRO2.getGreen(), NAPFRO2.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO3(NAPFRO3.getRed(), NAPFRO3.getGreen(), NAPFRO3.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPHIGH(NAPHIGH.getRed(), NAPHIGH.getGreen(), NAPHIGH.getBlue(), 1.0f);

		// Apply style
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding = ImVec2(15, 15);
		style.WindowRounding = 3.0f;
		style.FramePadding = ImVec2(5, 5);
		style.FrameRounding = 2.0f;
		style.ItemSpacing = ImVec2(12, 6);
		style.ItemInnerSpacing = ImVec2(8, 6);
		style.IndentSpacing = 25.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 7.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 1.0f;

		style.Colors[ImGuiCol_Text]						= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_TextDisabled]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_WindowBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ChildBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_PopupBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_Border]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_BorderShadow]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBg]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgHovered]			= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgActive]			= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_TitleBg]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgCollapsed]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TitleBgActive]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_MenuBarBg]				= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ScrollbarBg]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_ScrollbarGrab]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ScrollbarGrabHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ScrollbarGrabActive]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CheckMark]				= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SliderGrab]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SliderGrabActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Button]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ButtonHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ButtonActive]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_Header]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_Separator]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SeparatorHovered]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SeparatorActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_HeaderHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_HeaderActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ResizeGrip]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ResizeGripHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ResizeGripActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Tab]						= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TabHovered]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TabActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TabUnfocused]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TabUnfocusedActive]		= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_PlotLines]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_PlotLinesHovered]			= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_PlotHistogram]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotHistogramHovered]		= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_TextSelectedBg]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDarkening]		= IMGUI_NAPMODAL;
		style.Colors[ImGuiCol_Separator]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SeparatorHovered]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SeparatorActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavHighlight]				= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavWindowingHighlight]	= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavWindowingDimBg]		= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDimBg]			= IMGUI_NAPFRO1;
	}


	IMGuiService::IMGuiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	void IMGuiService::draw(VkCommandBuffer commandBuffer)
	{
		g_CommandBuffer = commandBuffer;
		ImGui::Render();
		g_CommandBuffer = VK_NULL_HANDLE;
		g_FrameIndex = (g_FrameIndex + 1) % IMGUI_VK_QUEUED_FRAMES;
	}

	void IMGuiService::selectWindow(nap::ResourcePtr<RenderWindow> window)
	{
		assert(window != nullptr);
		mUserWindow = window;
		setGuiWindow(window->getWindow()->getNativeWindow());
		mWindowChanged = true;

		// Store number of samples
		createPipeline(window->getBackbuffer().getRenderPass(), window->getBackbuffer().getSampleCount(), window->getBackbuffer().getSampleShadingEnabled());
	}


	void IMGuiService::processInputEvent(InputEvent& event)
	{

		ImGuiIO& io = ImGui::GetIO();
		
		// Key event
		if (event.get_type().is_derived_from(RTTI_OF(nap::KeyEvent)))
		{
			bool pressed = event.get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent));
			KeyEvent& key_event = static_cast<KeyEvent&>(event);
			io.KeysDown[(int)(key_event.mKey)] = pressed;

			// TODO: Keep track of modifier states in NAP so we don't directly interface with SDL2 here
			// Goal is to remove SDL calls from imgui service
			io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
			io.KeyCtrl	= ((SDL_GetModState() & KMOD_CTRL)	!= 0);
			io.KeyAlt	= ((SDL_GetModState() & KMOD_ALT)	!= 0);
			io.KeySuper	= ((SDL_GetModState() & KMOD_GUI)	!= 0);
		}
		
		// Text input event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::TextInputEvent)))
		{
			nap::TextInputEvent& press_event = static_cast<nap::TextInputEvent&>(event);
			io.AddInputCharactersUTF8(press_event.mText.c_str());
		}
		
		// Mouse wheel event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::MouseWheelEvent)))
		{
			nap::MouseWheelEvent& wheel_event = static_cast<nap::MouseWheelEvent&>(event);
			gMouseWheel = wheel_event.mY > 0 ? 1 : -1;
		}

		// Pointer Event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			nap::PointerPressEvent& press_event = static_cast<nap::PointerPressEvent&>(event);
			switch (press_event.mButton)
			{
			case EMouseButton::LEFT:
				gMousePressed[0] = true;
				break;
			case EMouseButton::MIDDLE:
				gMousePressed[1] = true;
				break;
			case EMouseButton::RIGHT:
				gMousePressed[2] = true;
				break;
			default:
				break;
			}
		}
	}


	bool IMGuiService::isCapturingKeyboard()
	{
		// Get the interface
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureKeyboard;
	}


	bool IMGuiService::isCapturingMouse()
	{
		// Get the interface
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureMouse;
	}


	bool IMGuiService::init(utility::ErrorState& error)
	{
		// Get our renderer
		mRenderService = getCore().getService<nap::RenderService>();
		assert(mRenderService != nullptr);
	
		g_Gpu = mRenderService->getPhysicalDevice();
		g_Device = mRenderService->getDevice();

		VkDescriptorPoolSize pool_size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)(1) };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &pool_size;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VkResult result = vkCreateDescriptorPool(g_Device, &poolInfo, nullptr, &g_DescriptorPool);
		assert(result == VK_SUCCESS);

		ImGuiContext* context = ImGui::CreateContext();

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab]			= (int)EKeyCode::KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow]	= (int)EKeyCode::KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow]	= (int)EKeyCode::KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow]		= (int)EKeyCode::KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow]	= (int)EKeyCode::KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp]		= (int)EKeyCode::KEY_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown]	= (int)EKeyCode::KEY_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home]		= (int)EKeyCode::KEY_HOME;
		io.KeyMap[ImGuiKey_End]			= (int)EKeyCode::KEY_END;
		io.KeyMap[ImGuiKey_Delete]		= (int)EKeyCode::KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace]	= (int)EKeyCode::KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter]		= (int)EKeyCode::KEY_KP_ENTER;
		io.KeyMap[ImGuiKey_Escape]		= (int)EKeyCode::KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A]			= (int)EKeyCode::KEY_a;
		io.KeyMap[ImGuiKey_C]			= (int)EKeyCode::KEY_c;
		io.KeyMap[ImGuiKey_V]			= (int)EKeyCode::KEY_v;
		io.KeyMap[ImGuiKey_X]			= (int)EKeyCode::KEY_x;
		io.KeyMap[ImGuiKey_Y]			= (int)EKeyCode::KEY_y;
		io.KeyMap[ImGuiKey_Z]			= (int)EKeyCode::KEY_z;

		// initialize IMGUI, most important part here is to assign render callback
		io.RenderDrawListsFn  = renderDrawLists;
		io.SetClipboardTextFn = setClipboardText;
		io.GetClipboardTextFn = getClipboardText;
		io.ClipboardUserData = NULL;

		// Push default style
		applyStyle();

		// Add font
		//io.Fonts->AddFontFromFileTTF("c:/naivi/NunitoSans-Regular.ttf", 17.5f);

		ImFontConfig font_config;
		font_config.OversampleH = 3;
		font_config.OversampleV = 1;
		io.Fonts->AddFontFromMemoryCompressedTTF(nunitoSansSemiBoldData, nunitoSansSemiBoldSize, 17.5f, &font_config);

		createDeviceObjects();
		createFontsTexture(*mRenderService);

		// Set primary window to be default
//		setGuiWindow(mRenderer->getPrimaryWindow().getNativeWindow());

		return true;
	}


	void IMGuiService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	void IMGuiService::update(double deltaTime)
	{
		if (mUserWindow == nullptr)
		{
			nap::Logger::error("No GUI target window specified, make sure to call `selectWindow()` first");
			return;
		}
		newFrame(*mUserWindow->getWindow());
	};


	void IMGuiService::shutdown()
	{
		invalidateDeviceObjects();
		//ImGui::Shutdown();
	}
}
