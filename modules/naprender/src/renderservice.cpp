// Local Includes
#include "renderservice.h"
#include "renderablemeshcomponent.h"
#include "rendercomponent.h"
#include "renderwindow.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"
#include "mesh.h"
#include "depthsorter.h"

// External Includes
#include <nap/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <rtti/factory.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>
#include <scene.h>
#include "boxmesh.h"
#include "meshfromfile.h"
#include "trianglemesh.h"
#include "shader.h"

RTTI_BEGIN_CLASS(nap::RenderServiceConfiguration)
	RTTI_PROPERTY("Settings",	&nap::RenderServiceConfiguration::mSettings,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	RenderService::RenderService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	// Register all object creation functions
	void RenderService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<RenderWindowResourceCreator>(*this));
		factory.addObjectCreator(std::make_unique<MeshCreator>(*this));
		factory.addObjectCreator(std::make_unique<BoxMeshCreator>(*this));
		factory.addObjectCreator(std::make_unique<TriangleMeshCreator>(*this));
		factory.addObjectCreator(std::make_unique<MeshFromFileCreator>(*this));
		factory.addObjectCreator(std::make_unique<ShaderCreator>(*this));
	}


	void RenderService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
	}


	std::shared_ptr<GLWindow> RenderService::addWindow(RenderWindow& window, utility::ErrorState& errorState)
	{
		assert(mRenderer != nullptr);

		// Get settings
		RenderWindowSettings window_settings;
		window_settings.width		= window.mWidth;
		window_settings.height		= window.mHeight;
		window_settings.borderless	= window.mBorderless;
		window_settings.resizable	= window.mResizable;
		window_settings.title		= window.mTitle;
		window_settings.sync		= window.mSync;

		std::shared_ptr<GLWindow> new_window = mRenderer->createRenderWindow(window_settings, window.mID, errorState);
		if (new_window == nullptr)
			return nullptr;

		mWindows.emplace_back(&window);

		// After window creation, make sure the primary window stays active, so that render resource creation always goes to that context
		//getPrimaryWindow().makeCurrent();

		return new_window;
	}


	void RenderService::removeWindow(RenderWindow& window)
	{
		WindowList::iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val) 
		{ 
			return val == &window; 
		});
		
		assert(pos != mWindows.end());
		mWindows.erase(pos);
	}
	

	RenderWindow* RenderService::findWindow(void* nativeWindow) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val) 
		{ 
			return val->getWindow()->getNativeWindow() == nativeWindow; 
		});

		if (pos != mWindows.end())
			return *pos;
		return nullptr;
	}


	RenderWindow* RenderService::findWindow(uint id) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val) 
		{ 
			return val->getNumber() == id; 
		});

		if (pos != mWindows.end())
			return *pos;
		return nullptr;
	}

	void RenderService::addEvent(WindowEventPtr windowEvent)
	{
        nap::Window* window = findWindow(windowEvent->mWindow);
		assert (window != nullptr);
		window->addEvent(std::move(windowEvent));
	}

	void createRenderPass(VkDevice device, VkFormat inColorFormat, VkRenderPass& renderPass) 
	{
		VkAttachmentDescription attachment = {};
		attachment.format = inColorFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}
	
	bool createGraphicsPipeline(VkDevice device, Material& material, const IMesh& mesh, VkRenderPass renderPass, VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline, utility::ErrorState& errorState)
	{
		Shader& shader = *material.getShader();

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		// Use the mapping in the material to bind mesh vertex attrs to shader vertex attrs
		uint32_t shader_attribute_binding = 0;
		for (auto& kvp : shader.getShader().getAttributes())
		{
			const opengl::ShaderVertexAttribute* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader()->mVertPath.c_str(), material.mID.c_str()))
				return false;

			const opengl::VertexAttributeBuffer* vertex_buffer = mesh.getMeshInstance().getGPUMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(vertex_buffer != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return false;

			if (!errorState.check(shader_vertex_attribute->mFormat == vertex_buffer->getFormat(), "Shader vertex attribute format does not match mesh attribute format for attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return false;

			bindingDescriptions.push_back({ shader_attribute_binding, (uint32_t)opengl::getVertexSize(shader_vertex_attribute->mFormat), VK_VERTEX_INPUT_RATE_VERTEX });
			attributeDescriptions.push_back({ (uint32_t)shader_vertex_attribute->mLocation, shader_attribute_binding, shader_vertex_attribute->mFormat, 0 });

			shader_attribute_binding++;
		}

		VkShaderModule vertShaderModule = shader.getShader().getVertexModule();
		VkShaderModule fragShaderModule = shader.getShader().getFragmentModule();

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = (int)bindingDescriptions.size();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = 256.0f;
		viewport.height = 256.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { 256, 256 };

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (!errorState.check(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout"))
			return false;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (!errorState.check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) == VK_SUCCESS, "Failed to create graphics pipeline"))
			return false;

		return true;
	}

	nap::RenderableMesh RenderService::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		VkRenderPass* render_pass = nullptr;
		switch (materialInstance.getMaterial()->getShader()->mOutputFormat)
		{
		case ERenderTargetFormat::RGBA8:
			{
				if (mRenderPassRGBA8 == nullptr)
					createRenderPass(mRenderer->getDevice(), VK_FORMAT_B8G8R8A8_SRGB, mRenderPassRGBA8);

				render_pass = &mRenderPassRGBA8;
			}
			break;
		case ERenderTargetFormat::RGB8:
			{
				if (mRenderPassRGB8 == nullptr)
					createRenderPass(mRenderer->getDevice(), VK_FORMAT_B8G8R8_SRGB, mRenderPassRGB8);

				render_pass = &mRenderPassRGB8;
			}
			break;
		case ERenderTargetFormat::R8:
			{
				if (mRenderPassR8 == nullptr)
					createRenderPass(mRenderer->getDevice(), VK_FORMAT_R8_SRGB, mRenderPassR8);

				render_pass = &mRenderPassR8;
			}
			break;
		case ERenderTargetFormat::Depth:
			{
				if (mRenderPassDepth == nullptr)
					createRenderPass(mRenderer->getDevice(), VK_FORMAT_D24_UNORM_S8_UINT, mRenderPassDepth);

				render_pass = &mRenderPassDepth;
			}
			break;
		}

		VkPipelineLayout layout;
		VkPipeline pipeline;
		if (!createGraphicsPipeline(mRenderer->getDevice(), *materialInstance.getMaterial(), mesh, *render_pass, layout, pipeline, errorState))
			return RenderableMesh();

		return RenderableMesh(mesh, materialInstance, layout, pipeline);
	}


	void RenderService::processEvents()
	{
		for (const auto& window : mWindows)
		{
			window->processEvents();
		}
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, CameraComponentInstance& camera)
	{
		renderObjects(renderTarget, commandBuffer, camera, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, CameraComponentInstance& camera, const SortFunction& sortFunction)
	{
		// Get all render-able components
		// Only gather renderable components that can be rendered using the given caera
		std::vector<nap::RenderableComponentInstance*> render_comps;
		std::vector<nap::RenderableComponentInstance*> entity_render_comps;
		for (Scene* scene : mSceneService->getScenes())
		{
			for (EntityInstance* entity : scene->getEntities())
			{
				entity_render_comps.clear();
				entity->getComponentsOfType<nap::RenderableComponentInstance>(entity_render_comps);
				for (const auto& comp : entity_render_comps) 
				{
					if (comp->isSupported(camera))
						render_comps.emplace_back(comp);
				}
			}
		}

		// Render these objects
		renderObjects(renderTarget, commandBuffer, camera, render_comps, sortFunction);
	}


	void RenderService::sortObjects(std::vector<RenderableComponentInstance*>& comps, const CameraComponentInstance& camera)
	{
		// Split into front to back and back to front meshes
		std::vector<nap::RenderableComponentInstance*> front_to_back;
		front_to_back.reserve(comps.size());
		std::vector<nap::RenderableComponentInstance*> back_to_front;
		back_to_front.reserve(comps.size());

		for (nap::RenderableComponentInstance* component : comps)
		{
			nap::RenderableMeshComponentInstance* renderable_mesh = rtti_cast<RenderableMeshComponentInstance>(component);
			if (renderable_mesh != nullptr)
			{
				nap::RenderableMeshComponentInstance* renderable_mesh = static_cast<RenderableMeshComponentInstance*>(component);
				EBlendMode blend_mode = renderable_mesh->getMaterialInstance().getBlendMode();	
				if (blend_mode == EBlendMode::AlphaBlend)
					back_to_front.emplace_back(component);
				else
					front_to_back.emplace_back(component);
			}
			else
			{
				front_to_back.emplace_back(component);
			}
		}

		// Sort front to back and render those first
		DepthSorter front_to_back_sorter(DepthSorter::EMode::FrontToBack, camera.getViewMatrix());
		std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);

		// Then sort back to front and render these
		DepthSorter back_to_front_sorter(DepthSorter::EMode::BackToFront, camera.getViewMatrix());
		std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);

		// concatinate both in to the output
		comps.clear();
		comps.insert(comps.end(), std::make_move_iterator(front_to_back.begin()), std::make_move_iterator(front_to_back.end()));
		comps.insert(comps.end(), std::make_move_iterator(back_to_front.begin()), std::make_move_iterator(back_to_front.end()));
	}


	// Updates the current context's render state by using the latest render state as set by the user.
	void RenderService::updateRenderState()
	{
		opengl::GLContext context = opengl::getCurrentContext();
		ContextSpecificStateMap::iterator context_state = mContextSpecificState.find(context);
		if (context_state == mContextSpecificState.end())
		{
			mContextSpecificState.emplace(std::make_pair(context, mRenderState));
			mContextSpecificState[context].force();
		}
		else
		{
			context_state->second.update(mRenderState);
		}
	}


	// Renders all available objects to a specific renderTarget.
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps)
	{
		renderObjects(renderTarget, commandBuffer, camera, comps, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps, const SortFunction& sortFunction)
	{
		// Sort objects to render
		std::vector<RenderableComponentInstance*> components_to_render = comps;
		sortFunction(components_to_render, camera);

		//renderTarget.bind();

		// Before we render, we always set aspect ratio. This avoids overly complex
		// responding to various changes in render target sizes.
		//camera.setRenderTargetSize(renderTarget.getSize());

		// Make sure we update our render state associated with the current context
		//updateRenderState();

		// Extract camera projection matrix
		const glm::mat4x4 projection_matrix = camera.getProjectionMatrix();

		// Extract view matrix
		glm::mat4x4 view_matrix = camera.getViewMatrix();

		// Draw components only when camera is supported
		for (auto& comp : components_to_render)
		{
			if (!comp->isSupported(camera))
			{
				nap::Logger::warn("unable to render component: %s, unsupported camera %s", 
					comp->mID.c_str(), camera.get_type().get_name().to_string().c_str());
				continue;
			}
			comp->draw(commandBuffer, view_matrix, projection_matrix);
		}

		//renderTarget.unbind();
	}


	// Clears the render target.
	void RenderService::clearRenderTarget(opengl::RenderTarget& renderTarget, opengl::EClearFlags flags)
	{
		renderTarget.bind();
		renderTarget.clear(flags);
		renderTarget.unbind();
	}


	void RenderService::clearRenderTarget(opengl::RenderTarget& renderTarget)
	{
		renderTarget.bind();
		renderTarget.clear(opengl::EClearFlags::Color | opengl::EClearFlags::Depth | opengl::EClearFlags::Stencil);
		renderTarget.unbind();
	}


	// Set the currently active renderer
	bool RenderService::init(nap::utility::ErrorState& errorState)
	{
		// Get handle to scene service
		mSceneService = getCore().getService<SceneService>();
		assert(mSceneService != nullptr);

		// Create the renderer and initialize
		std::unique_ptr<Renderer> renderer = std::make_unique<nap::Renderer>();
		if (!renderer->init(getConfiguration<RenderServiceConfiguration>()->mSettings, errorState))
			return false;
		mRenderer = std::move(renderer);
		return true;
	}
	

	void RenderService::preUpdate(double deltaTime)
	{
	//	getPrimaryWindow().makeCurrent();
	}


	void RenderService::update(double deltaTime)
	{
		processEvents();
	}


	void RenderService::resourcesLoaded()
	{
		opengl::flush();
	}


	void RenderService::setPolygonMode(opengl::EPolygonMode mode)
	{
		mRenderState.mPolygonMode = mode;
	}


	void RenderService::pushRenderState()
	{
		updateRenderState();
	}

	
	void RenderService::queueResourceForDestruction(std::unique_ptr<opengl::IGLContextResource> resource)
	{ 
		if (resource != nullptr)
			mGLContextResourcesToDestroy.emplace_back(std::move(resource)); 
	}


	void RenderService::destroyGLContextResources(const std::vector<RenderWindow*> renderWindows)
	{/*
		// If there is anything scheduled, destroy
		if (!mGLContextResourcesToDestroy.empty())
		{
			// Destroy resources for primary window
			getPrimaryWindow().makeCurrent();
			for (auto& resource : mGLContextResourcesToDestroy)
				resource->destroy(getPrimaryWindow().getContext());

			// We go over the windows to make the GL context active, and then destroy 
			// the resources for that context
			for (const rtti::ObjectPtr<RenderWindow>& render_window : renderWindows)
			{
				render_window->makeActive();
				for (auto& resource : mGLContextResourcesToDestroy)
					resource->destroy(render_window->getWindow()->getContext());
			}
			mGLContextResourcesToDestroy.clear();
		}
		*/
	}


	// Shut down renderer
	void RenderService::shutdown()
	{
        // If initializing the renderer failed, mRenderer will be null
        if (mRenderer != nullptr)
		    mRenderer->shutdown();
	}

	/**
	 * Two important things to notice in the internal structure for VAOs:
	 *	1) Internally, a cache of opengl::VertexArrayObjects is created for each mesh-material combination. When retrieving a VAO
	 *	   for an already known mesh-material combination, the VAO is retrieved from the cache.
	 *	2) Ownership of the VAO's does not lie in the RenderService: instead it is shared by all clients. To accomplish this, handles
	 *	   are returned to clients that perform refcounting into the internal RenderService cache. When there are no more references 
	 *	   to a VAO, it is queued for destruction. An important detail to notice is that the RenderService does not store handles 
	 *	   internally, it hands them out when pulling VAOs from the cache or when creating new VAOs.
	 */
	VAOHandle RenderService::acquireVertexArrayObject(const Material& material, const IMesh& mesh, utility::ErrorState& errorState)
	{
		/// Construct a key based on material-mesh, and see if we have a VAO for this combination
		VAOKey key(material, mesh.getMeshInstance());
		VAOMap::iterator kvp = mVAOMap.find(key);
		if (kvp != mVAOMap.end())
			return VAOHandle(*this, key, kvp->second.mObject.get());

		// VAO was not found for this material-mesh combination, create a new one
		RefCountedVAO ref_counted_vao;
		ref_counted_vao.mObject = std::make_unique<opengl::VertexArrayObject>();

		// Use the mapping in the material to bind mesh vertex attrs to shader vertex attrs
		for (auto& kvp : material.getShader()->getShader().getAttributes())
		{
			const opengl::ShaderVertexAttribute* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader()->mVertPath.c_str(), material.mID.c_str()))
				return VAOHandle();

			const opengl::VertexAttributeBuffer* vertex_buffer = mesh.getMeshInstance().getGPUMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(vertex_buffer != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return VAOHandle();

			ref_counted_vao.mObject->addVertexBuffer(shader_vertex_attribute->mLocation, *vertex_buffer);
		}

		auto inserted = mVAOMap.emplace(key, std::move(ref_counted_vao));

		return VAOHandle(*this, key, inserted.first->second.mObject.get());
	}


	void RenderService::incrementVAORefCount(const VAOKey& key)
	{
		VAOMap::iterator pos = mVAOMap.find(key);
		assert(pos != mVAOMap.end()); 

		++pos->second.mRefCount;
	}


	void RenderService::decrementVAORefCount(const VAOKey& key)
	{
		VAOMap::iterator pos = mVAOMap.find(key);
		assert(pos != mVAOMap.end());

		// If this is the last usage of this VAO, queue it for destruction (VAOs need to be destructed per active context,
		// so we defer destruction)
		if (--pos->second.mRefCount == 0)
		{
			queueResourceForDestruction(std::move(pos->second.mObject));
			mVAOMap.erase(pos);
		}
	}

} // Renderservice

