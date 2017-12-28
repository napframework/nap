Rendering {#rendering}
=======================

*	[Overview](@ref render_overview)
*	[Example](@ref render_example)

Overview {#render_overview}
=======================

NAP uses the OpenGL 3.3+ [programmable rendering pipeline](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview) to draw objects. This means NAP uses shaders to manipulate and render items. Using shaders improves overall performance and offers a more flexible method of working compared to the old fixed function render pipeline. The result is a more data driven approach to rendering, similar to what you see in popular game engines such as Unreal4 or Unity, except: NAP doesn't lock down the rendering process. An object that can be rendered isn't rendered by default: you explicetly have to tell the renderer:

- That you want to render an object
- How you want to render an object
- Where to render it to, ie: it's destination

The destination is always a render target and NAP currently offers two: Directly to screen or an off-screen target. This process sounds difficult but offers a lot of flexibility. Often you want to render a set of objects to a texture before rendering the textures to screen. Or you might want to render only a sub-set of objects to screen 1 and another set of objects to screen 2. 

Traditionally this is a problem, most renderers tie a drawable object to 1 render context. This context is always associated with 1 screen. This makes sharing objects between multiple screens pretty much impossible. NAP solves this issues by allowing any object to be drawn to any screen, even sharing the same object between two or multiple screens. This is one of NAPs biggest strengths and offers a lot of flexibility in deciding how you want to draw things. You, as a user, don't have to worry about the possibility of rendering one or multiple objects to a particular screen, you know you can.

Let's look at an example before we dive in some of the more technical aspects of rendering

Example {#render_example}
=======================

To follow this example it's good to read the high level [system](@ref system) documentation first. This example renders a rotating sphere with a world texture to the first screen. To render a rotating sphere we need a:

- [Window](@ref nap::RenderWindow)
- [Sphere](@ref nap::SphereMesh)
- [Shader](@ref nap::Shader)
- [Material](@ref nap::Material)
- [Image](@ref nap::Image)
- [Camera Component](@ref nap::PerspCameraComponent)
- [Render Component](@ref nap::RenderableMeshComponent)
- [Transform Component](@ref nap::TransformComponent)
- [Rotate Component](@ref nap::RotateComponent)

Some parts might look familiar, others are new. The most important new parts are the material, render and rotate component. The rotate component rotates the sphere along the y axis, the render component ties a renderable [mesh](@ref nap::IMesh) to a material. Every material points to a shader. The material is applied to the mesh before being rendered to (in this case) the screen. You will notice that the actual application will contain almost no code, most of the functionality is defined by the various objects and components. The only thing we have to do it tell the renderer to render the sphere using a particular camera.

But let's begin by defining some of the resources in JSON:

```
{		
	"Type" : "nap::RenderWindow",
	"mID" : "Window0",
	"Width" : 512,
	"Height" : 512,
	"Title" : "Window 1"
},

{
	"Type" : "nap::Image",	
	"mID" : "WorldTexture",
	"ImagePath" : "data/world_demo/world_texture.jpg"
},

{
	"Type" : "nap::Shader",
	"mID": "WorldShader",
	"mVertShader": "shaders/world_demo/world.vert",
	"mFragShader": "shaders/world_demo/world.frag"
},

{
	"Type" : "nap::SphereMesh",
	"mID": "WorldMesh"
},
```

These resources are rather straight-forward. We tell NAP we want a render window, image, shader and mesh. The image points to the world texture. This is the texture we want to apply to the sphere. Behind the scenes NAP loads the image from disk and uploads the pixel data to the GPU. This image is now a texture that can be used by shaders as a uniform texture input. In this particular case we want the world shader to use that texture. The world shader exposes a uniform with the name 'inWorldTexture'. But how do we bind the world texture to the shader? As you can see in the example above: the world texture is not referenced anywhere. For that purpose we use a material. Let's add one:

```
{
	"Type" : "nap::Material",
	"mID": "WorldMaterial",
	"Shader": "WorldShader",
	"VertexAttributeBindings" : 
	[
		{
			"MeshAttributeID": "Position",
			"ShaderAttributeID": "in_Position"
		},
		{
			"MeshAttributeID": "UV0",
			"ShaderAttributeID": "in_UV0"
		}
	],
	"Uniforms" : 
	[
		{
			"Type" : "nap::UniformTexture2D",
			"Name" : "inWorldTexture",
			"Texture" : "WorldTexture"
		}
	]			
},
```

A material serves a very important function. It allows you to apply the same shader to different objects. Often you only need a limited set of shaders to represent a large set of objects. The same shader can be used to render objects that share the same physical properties, such as a window and a wine glass. Creating separate shaders for each individual object is inefficient. In the example above we create a world material that links to a world shader. NAP creates both the shader and material for you. The only thing left to do is link the 'WorldTexture' to the right shader input exposed by the material. 

Every material carries a set of 'Uniforms'. Material 'Uniforms' allow you to bind values (in this case a texture) to specific inputs of a shader. This material binds 'WorldTexture' to 'inWorldTexture'. When you apply this material to a mesh (in this case the sphere) the renderer binds the texture to the right input of the shader with, as a result, a sphere that looks like planet earth. NAP validates that the shader exposes a texture input called 'inWorldTexture' and the 'WorldTexture' is loaded and valid. Initialization fails when the world texture can't be loaded or the shader doesn't have an input called 'inWorldTexture'. In both cases an error message is generated.

You now have a material that you can apply to your sphere. But we don't have a scene that represents the objects in space. Without that structure the renderer doesn't know where to place the sphere and what material to apply to the sphere. We therefore create a simple scene structure that holds both the camera and sphere to render:

```
{
	"Type" : "nap::Scene",
	"mID": "Scene",		
	"Entities" : 
	[
		{
			"Entity" : "World"
		},
		{
			"Entity" : "Camera"
		}
	]
},
```

And define our 'World':

```
{
	"Type" : "nap::Entity",
	"mID": "World",
	"Components" : 
	[
		{
			"Type" : "nap::RenderableMeshComponent",
			"Mesh" : "WorldMesh",
			"MaterialInstance" : 
			{
				"mID": "WorldMaterialInstance",
				"Material": "WorldMaterial"
			}
		},
		{
			"Type" : "nap::TransformComponent"
		},
		{
			"Type" : "nap::RotateComponent",
			"Properties": 
			{
				"Axis": 
				{
					"x": 0.0,
					"y": 1.0,
					"z": 0.0
				},
				"Speed": 0.1,
				"Offset": 2.0
			}
		}		
	]
},
```

Before we add the camera it's good to take a closer look at the nap::RenderableMeshComponent. As mentioned before, this component binds both a mesh and material together. When doing so it performs a very important task: it makes sure that the mesh can be rendered to any screen. This component also pushes all the material properties to the GPU before rendering the mesh. The transform component positions the 'World' at the origin of the scene and the rotate component rotates the 'World' around the y axis once every 10 seconds. Last thing to do is add a Camera, we place it 5 units back from the origin of the scene to ensure the world is visible:

```
{
	"Type" : "nap::Entity",
	"mID": "Camera",
	"Components" : 
	[
		{
			"Type" : "nap::PerspCameraComponent",
			"Properties": 
			{
				"FieldOfView": 45.0,
				"NearClippingPlane" : 1,
				"FarClippingPlane" : 1000.0
			}
		},
		{
			"Type" : "nap::TransformComponent",
			"Properties": 
			{
				"Translate": 
				{
					"x": 0.0,
					"y": 0.0,
					"z": 5.0
				}			
			}
		}
	]
}
```

You're now ready to load this file and fetch the created resources. You need those to render the world later on:

~~~~~~~~~~~~~~~{.cpp}
bool ExampleApp::init(utility::ErrorState& error)
{
	// Retrieve services
	mRenderService = getCore().getService<nap::RenderService>();
	mSceneService  = getCore().getService<nap::SceneService>();
		
	// Get resource manager and load
	mResourceManager = getCore().getResourceManager();
	if (!mResourceManager->loadFile("data/world_demo/demo.json", error))
	{
		return false;
	}
        
	// Extract loaded resources
	mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

	// Find the world and camera entities
	ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

	mWorldEntity  = scene->findEntity("World");
	mCameraEntity = scene->findEntity("Camera");
}
~~~~~~~~~~~~~~~

And in the render call of your application you explicetly tell the renderer to render your 'World' at the origin (defined by it's transform) with the right material:

~~~~~~~~~~~~~~~{.cpp}
// Called when the window is going to render
void ExampleApp::render()
{
	// Clear opengl context related resources that are not necessary any more
	mRenderService->destroyGLContextResources({ mRenderWindow });
	
	// Activate current window for drawing
	mRenderWindow->makeActive();
	
	// Clear back-buffer
	mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());
	
	// Find the world and add as an object to render
	std::vector<nap::RenderableComponentInstance*> components_to_render;
	nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>()
	components_to_render.emplace_back(&renderable_world);
	
	// Find the camera
	nap::PerspCameraComponentInstance& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>()

	// Render the world with the right camera directly to screen
	mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, components_to_render);
	
	// Swap screen buffers
	mRenderWindow->swap();
}
~~~~~~~~~~~~~~~

That's it, you should see a rotating textured sphere in the center of your viewport. This example tried to cover the basics of rendering with NAP but as you might have suspected: modern day rendering is a vast and complex subject. NAPs philosophy is to be open, it doesn't render for you. What NAP does best is getting you set up with the building blocks to render complex scenes. This also applies to many other facets of the framework. But in return you get a very open engine that allows you to render most things without having to write thousands of lines of code. 

To get a better understanding of rendering with NAP continue reading or play around with a render demo that ships swith NAP. This example is included as 'HelloWorld'.