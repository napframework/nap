Rendering {#rendering}
=======================

*	[Introduction](@ref render_intro)
*	[Key Features](@ref key_features)
*	[Configuration](@ref render_config)
*	[Example](@ref render_example)
*	[Meshes](@ref meshes)
	*	[Creating Meshes](@ref creating_meshes)
		* [Mesh From File](@ref mesh_from_file)
		* [Predefined Shapes](@ref predefined_shapes)
		* [Mesh Resource](@ref mesh_resource)
		* [Custom Mesh C++](@ref custom_mesh)
	*	[Mesh Format](@ref mesh_format)
	*	[Mesh Usage](@ref mesh_usage)
*	[Text](@ref text)
*	[Materials and Shaders](@ref materials)
	*	[Vertex Attributes](@ref vertex_attrs)
	*	[Default Vertex Attributes](@ref default_attrs)
	*	[Uniforms](@ref uniforms)
	*	[Samplers](@ref samplers)
	*	[Color Blending](@ref blending)
	*	[Depth](@ref depth)
	*	[Rendering Meshes](@ref renderwithmaterials)
*	[Textures](@ref textures)
	*	[Creating Textures](@ref creating_textures)
		*	[GPU Textures](@ref gpu_textures)
		*	[Images](@ref images)
		*	[Image From File](@ref image_from_file)
	* 	[Reading Textures From The GPU](@ref reading_textures)
	*	[Texture Sampling](@ref texture_sampling)
*	[Windows](@ref multi_screen)
*	[Offscreen Rendering](@ref offscreen_rendering)
*	[Cameras](@ref cameras)

Introduction {#render_intro}
=======================

The NAP renderer is designed to be open and flexible. All render related functionality is exposed as a set of building blocks instead of a fixed function pipeline. You can use these blocks to set up your own rendering pipeline. Meshes, textures, materials, shaders, rendertargets, cameras and windows form the basis of these tools. They can be authored using JSON and are exported using your favourite content creation program (Photoshop, Maya etc.)

NAP uses <a href="https://www.khronos.org/vulkan/" target="_blank">Vulkan</a> to render objects but, contrary to popular game engines such as Unity or Unreal, doesn't lock down the rendering process. An object that can be rendered isn't rendered by default: you explicitly have to tell the renderer:

- That you want to render an object
- How you want to render an object
- Where to render it to, ie: it's destination

The destination is always a render target and NAP currently offers two: directly to a [window](@ref nap::RenderWindow) or an [off-screen target](@ref nap::RenderTarget). Often you want to render a set of objects to a texture before rendering the textures to screen. Or you might want to render only a sub-set of objects to screen one and another set of objects to screen two. This is one of NAPs biggest strengths and offers a lot of flexibility in deciding how you want to draw things.

Key Features {#key_features}
=======================
- Build your own render pipeline using easy to understand building blocks.

- Meshes are not limited to a specific set of vertex attributes such as 'color', 'uv' etc. Any attribute, such as ‘wind’ or ‘heat’, can be added to drive procedural effects. 

- NAP supports both static and dynamic meshes. It's easy to bind your own attributes to materials using a safe and easy to understand binding system.

- Rendering the same content to multiple windows is supported natively.

- All render functionality is fully compatible with the real-time editing system. Textures, meshes or shaders can be modified and reloaded instantly without having to restart the application.

Configuration {#render_config}
=======================
All global render settings are configurable using using the [nap::RenderServiceConfiguration](@ref nap::RenderServiceConfiguration). The render engine creates a Vulkan 1.0 instance by default, but applications may use Vulkan 1.1 and 1.2 functionality if required. Make sure to set the required major and minor vulkan version accordingly. The application will not start if the device does not support the selected (and therefore required) version of Vulkan. 

Example {#render_example}
=======================

To follow this example it's good to read the high level [system](@ref system) documentation first. This example renders a rotating sphere with a world texture to the first screen. This example is part of the 'HelloWorld' demo. To render a rotating sphere we need a:

- [Window](@ref nap::RenderWindow)
- [Sphere](@ref nap::SphereMesh)
- [Shader](@ref nap::Shader)
- [Material](@ref nap::Material)
- [Image](@ref nap::Image)
- [Camera Component](@ref nap::PerspCameraComponent)
- [Render Component](@ref nap::RenderableMeshComponent)
- [Transform Component](@ref nap::TransformComponent)
- [Rotate Component](@ref nap::RotateComponent)

Some parts might look familiar, others are new. The most important new parts are the material, render and rotate component. The rotate component rotates the sphere along the y axis, the render component ties a renderable [mesh](@ref nap::IMesh) to a material. Every material points to a shader. The material is applied to the mesh before being rendered to (in this case) the screen. You will notice that the actual application will contain almost no code, most of the functionality is defined by the various objects and components. The only thing we have to do is tell the renderer to render the sphere using a particular camera.

But let's begin by defining some of the resources in JSON. Make sure to use [Napkin](@ref napkin) (our JSON editor) to do this for you:

```
{
	"Type": "nap::RenderWindow",
    "mID": "Window0",
    "Borderless": false,
    "Resizable": true,
    "Visible": true,
    "Title": "Window 1",
    "Width": 1280,
    "Height": 720,
   	"Mode": "Immediate",
   	"Samples": "Four"
},
{
	"Type": "nap::ImageFromFile",
	"mID": "WorldTexture",
	"Usage": "Static",
	"ImagePath": "world_texture.png",
	"GenerateLods": true
},
{
	"Type" : "nap::ShaderFromFile",
	"mID": "WorldShader",
	"VertShader": "shaders/world.vert",
	"FragShader": "shaders/world.frag"
},
{
	"Type": "nap::SphereMesh",
    "mID": "WorldMesh",
    "Radius": 1.0,
    "Rings": 50.0,
    "Sectors": 50.0
},
```

These resources are rather straight-forward. We tell NAP we want a [render window](@ref nap::RenderWindow), [image](@ref nap::ImageFromFile), [shader](@ref nap::Shader) and [mesh](@ref nap::SphereMesh). The image points to the world texture. This is the texture we want to apply to the sphere. Behind the scenes NAP loads the image from disk and uploads the pixel data to the GPU. This image is now a texture that can be used by shaders as a sampler texture input. In this particular case we want the world shader to use that texture. The world shader exposes a sampler with the name 'inWorldTexture'. But how do we bind the world texture to the shader? As you can see in the example above: the world texture is not referenced anywhere. For that purpose we use a material. Let's add one:

```
{
    "Type": "nap::Material",
    "mID": "WorldMaterial",
    "Uniforms": [],
    "Samplers": 
    [
        {
            "Type": "nap::Sampler2D",
            "mID": "world_input_tex_uniform",
            "Name": "inWorldTexture",
            "Texture": "WorldTexture"
        }
    ],
    "Shader": "WorldShader",
    "BlendMode": "Opaque",
    "DepthMode": "InheritFromBlendMode"
},
```

A [material](@ref nap::Material) serves a very important function. It allows you to apply the same shader to different objects. Often you only need a limited set of shaders to represent a large set of objects. The same shader can be used to render objects that share the same physical properties, such as a window and a wine glass. Creating separate shaders for each individual object is inefficient. In the example above we create a world material that links to a world shader. NAP creates both the shader and material for you. The only thing left to do is link the 'WorldTexture' to the right shader input exposed by the material. 

Every material carries a set of 'Uniforms' and 'Samplers'. Material 'Uniforms' allow you to bind values to specific inputs of a shader. Samplers allow you to bind textures to specific inputs of a shader. This material binds 'WorldTexture' to 'inWorldTexture'. When you apply this material to a mesh (in this case the sphere) the renderer binds the texture to the right input of the shader with, as a result, a sphere that looks like planet earth. NAP validates that the shader exposes a texture input called 'inWorldTexture' and the 'WorldTexture' is loaded and valid. Initialization fails when the world texture can't be loaded or the shader doesn't have an input called 'inWorldTexture'. In both cases an error message is generated.

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
			"mID": "Renderable Mesh Component",
			"Mesh" : "WorldMesh",
			"MaterialInstance" : 
			{
				"mID": "WorldMaterialInstance",
				"Material": "WorldMaterial"
			}
		},
		{
			"Type" : "nap::TransformComponent",
			"mID": "Transform Component"
		},
		{
			"Type" : "nap::RotateComponent",
			"mID": "Rotate Component",
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

Before we add the camera it's good to take a closer look at the [renderable mesh component](@ref nap::RenderableMeshComponent). As mentioned before, this component binds both a mesh and material together. When doing so it performs a very important task: it makes sure that the mesh can be rendered to a render target. This component also pushes all the material properties to the GPU before rendering the mesh. The [transform component](@ref nap::TransformComponent) positions the 'World' at the origin of the scene and the [rotate component](@ref nap::RotateComponent) rotates the 'World' around the y axis once every 10 seconds. Last thing to do is add a [camera](@ref nap::PerspCameraComponent), we place it five units back from the origin of the scene to ensure the world is visible:

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
		
	// Get resource manager
	mResourceManager = getCore().getResourceManager();
        
	// Extract loaded resources
	mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

	// Find the world and camera entities
	ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

	mWorldEntity  = scene->findEntity("World");
	mCameraEntity = scene->findEntity("Camera");
}
~~~~~~~~~~~~~~~

And in the render call of your application you explicitly tell the renderer to render your 'World' at the origin (defined by it's transform) with the right material:

~~~~~~~~~~~~~~~{.cpp}
// Called when the window is going to render
void ExampleApp::render()
{
	// Signal the beginning of a new frame, allowing it to be recorded.
	mRenderService->beginFrame();

	// Begin recording the render commands for the main render window
	if (mRenderService->beginRecording(*mRenderWindow))
	{
		// Begin the render pass
		mRenderWindow->beginRendering();

		// Find the world and add as an object to render
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
		components_to_render.emplace_back(&renderable_world);

		// Find the perspective camera
		nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<nap::PerspCameraComponentInstance>();

		// Render the world with the right camera directly to screen
		mRenderService->renderObjects(*mRenderWindow, persp_camera, components_to_render);
		
		// End the render pass
		mRenderWindow->endRendering();

		// End recording
		mRenderService->endRecording();
	}

	// Signal the end of the frame
	mRenderService->endFrame();
}
~~~~~~~~~~~~~~~

That's it, you should see a rotating textured sphere in the center of your viewport. This example tried to cover the basics of rendering with NAP but as you might have suspected: modern day rendering is a vast and complex subject. NAP's philosophy is to be open; it doesn't render for you. What NAP does best is getting you set up with the building blocks to render complex scenes. This also applies to many other facets of the framework. But in return you get a very open engine that allows you to render most things without having to write thousands of lines of code. 

To get a better understanding of rendering with NAP continue reading or play around with a render demo that ships with NAP. This example is part of the 'HelloWorld' demo:

![](@ref content/helloworld.png)

Meshes {#meshes}
=======================

Creating Meshes {#creating_meshes}
-----------------------

The underlying resource of all mesh types is the [IMesh](@ref nap::IMesh). The `IMesh` does only one thing: provide the renderer with a [mesh instance](@ref nap::MeshInstance). The mesh instance contains the actual data that is drawn. The following resources create a mesh instance:

###Mesh From File {#mesh_from_file}###

The [MeshFromFile](@ref nap::MeshFromFile) loads a mesh from an external file. NAP only supports the FBX file format and automatically converts any .fbx file in to a .mesh file using the FBX converter tool. The result is a heavily compressed binary file. The FBX converter runs automatically after compilation and only converts .fbx files when new. Alternatively you can run the tool from the command line. Type --help for instructions. If an .fbx file contains multiple meshes each mesh is stored into an individual .mesh file.

Here is an example:

```
{
	"Type" : "nap::MeshFromFile",
	"mID": "CarMesh",
	"Path": "car.mesh"
}
```

###Predefined Shapes {#predefined_shapes}###

A few simple shapes (such as a [plane](@ref nap::PlaneMesh), [sphere](@ref nap::SphereMesh) or [box](@ref nap::BoxMesh)) can be created directly using configurable parameters:

```
{
	"Type" : "nap::SphereMesh",
	"mID": "SphereMesh",
	"Radius" : 5,
	"Rings" : 50,
	"Sectors" : 50
},
{
	"Type" : "nap::PlaneMesh",
	"mID": "PlaneMesh",
	"Rows" : 128,
	"Columns" : 128
},
{
	"Type" : "nap::BoxMesh".
	"mID" : "BoxMesh",
    "Size": 
    {
        "x": 1.0,
        "y": 1.0,
        "z": 1.0
    }
}
```

###Mesh Resource {#mesh_resource}###

- The [Mesh](@ref nap::Mesh) resource can be used to explicitly define the contents of a mesh in a JSON file. Below you see an example of a mesh in the shape of a plane. This mesh contains four vertices. The plane has a position and UV attribute. The triangles are formed as a TriangleStrip:

```
{
	"Type" : "nap::Mesh",
	"mID" : "CustomPlaneMesh",
	"Properties" : {
		"NumVertices" : 4,
		"Shapes" : [
			{
				"DrawMode" : "TriangleStrip"
			}
		],
		"Attributes" : [
			{
				"Type" : "nap::Vec3VertexAttribute",
				"AttributeID" : "Position",
				"Data" : [
					{ "x": -0.5, 	"y": -0.5, "z": 0.0 },
					{ "x":  0.5, 	"y": -0.5,	"z": 0.0 },
					{ "x": -0.5, 	"y":  0.5,	"z": 0.0 },
					{ "x":  0.5, 	"y":  0.5,	"z": 0.0 }
				]
			},
			{
				"Type" : "nap::Vec3VertexAttribute",
				"AttributeID" : "UV0",
				"Data" : [
					{ "x": 0.0, 	"y": 0.0, 	"z": 0.0 },
					{ "x": 1.0, 	"y": 0.0,	"z": 0.0 },
					{ "x": 0.0, 	"y": 1.0,	"z": 0.0 },
					{ "x": 1.0, 	"y": 1.0,	"z": 0.0 }
				]
			}
		],
		"Indices" : [
			0,
			1,
			3,
			0,
			3,
			2
		]
	}
}

```

###Custom Mesh C++ {#custom_mesh}###

You can create your own custom or procedural mesh in code. Both the 'dynamicgeo' and 'heightmap' demo show you how to do this. In the following example we define a new mesh. On initialization the instance is created. For the mesh to behave and render correctly we add a set of attributes. In this case 'Position', 'uv', 'id' and 'color'. The mesh contains no actual (initial) vertex data. The vertex data grows / shrinks over time based on the number of active particles in the scene. For a more complete example refer to the 'dynamicgeo' demo.

~~~~~~~~~~~~~~~{.cpp}
class ParticleMesh : public IMesh
{
public:

	/**
	 * Construct the mesh using the render service
	 */
	ParticleMesh(Core& core) : mRenderService(core.getService<RenderService>()) { }


	/**
	 * Create and initialize the mesh
	 */
	bool init(utility::ErrorState& errorState)
	{
		// Create the mesh instance
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

		// Because the mesh is populated dynamically we set the initial amount of vertices to be 0
		mMeshInstance->setNumVertices(0);
		mMeshInstance->setUsage(EMeshDataUsage::DynamicWrite);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setCullMode(ECullMode::None);

		// Allocate room for 1000 vertices
		mMeshInstance->reserveVertices(1000);

		// Add mesh attributes
		mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));
		mMeshInstance->getOrCreateAttribute<float>("pid");
			
		// Create the shape that connects the vertices
		MeshShape& shape = mMeshInstance->createShape();

		// Reserve CPU memory for the particle geometry indices.
		shape.reserveIndices(1000);

		// Initialize the instance
		return mMeshInstance->init(errorState);
	}

	/**
	 * @return MeshInstance as created during init().
	 */
	virtual MeshInstance& getMeshInstance()	override 					{ return *mMeshInstance; }

	/**
	 * @return MeshInstance as created during init().
	 */
	virtual const MeshInstance& getMeshInstance() const	override 		{ return *mMeshInstance; }

private:
	std::unique_ptr<MeshInstance> mMeshInstance = nullptr;
};
~~~~~~~~~~~~~~~

The Mesh Format {#mesh_format}
-----------------------

The mesh instance format is best explained by an example. Consider a mesh that represents the letter ‘P’:

![](@ref content/mesh_shapes.png)

This letter contains a list of 24 points. However, the mesh is split up into two separate pieces: the closed line that forms the outer shape and the closed line that forms the inner shape. The points of both shapes are represented by the blue and orange colors. 

Every mesh instance contains a list of points (vertices). Each vertex can have multipe attributes such as a normal, a UV coordinate and a color. In this example the mesh holds a list of exactly 24 vertices. To add each individual line to the mesh we create a [shape](@ref nap::MeshShape). The shape tells the system two things:

- How the vertices are connected using a list of indices. In this case vertices 0-12 define the outer shape, vertices 13-23 define the inner shape.
- How the GPU interprets the indices, ie: how are the points connected? For this example we tell the GPU to connect the vertices as a single line using `LineStrip`

Within a single mesh you can define multiple shapes that share the same set of vertices (points). It is allowed to share vertices between shapes and it is allowed to define a single mesh with different types of shapes. Take a sphere for example. The vertices can be used to define both the sphere as a triangle mesh and the normals of that sphere as a set of individual lines. The normals are rendered as lines (instead of triangles) but share (part of) the underlying vertex structure. This sphere therefore contains two shapes, one triangle shape (to draw the sphere) and one line shape (to draw the normals).

All common shapes are supported: `Points`, `Lines`, `LineStrip`, `Triangles`, `TriangleStrip` and `TriangleFan`.

As a user you can work on individual vertices or on the vertices associated with a specific shape. Often its necessary to walk over all the shapes that constitute a mesh. On a higher level NAP provides utility functions (such as computeNormals and reverseWindingOrder) to operate on a mesh as a whole. But for custom work NAP provides a very convenient and efficient [iterator](@ref nap::TriangleIterator) that is capable of looping over all the triangles within multiple shapes. This ensures that as a user you don’t need to know about the internal connectivity of the various shapes. Consider this example:

~~~~~~~~~~~~~~~{.cpp}
// fetch uv attribute
Vec3VertexAttribute* uv_nattr = mesh.findAttribute<glm::vec3>(vertexid::getUVName(0));

// fetch uv center attribute 
Vec3VertexAttribute* uv_cattr = mesh.findAttribute<glm::vec3>("uvcenter");

// Create triangle iterator
TriangleShapeIterator shape_iterator(*mMeshInstance);

// Iterate over all the triangles and calculate average uv value for every triangle
TriangleIterator tri_iterator(*mMeshInstance);
while (!tri_iterator.isDone())
{
	// Get triangle
	Triangle triangle = tri_iterator.next();

	// Get uv values associated with triangle
	TriangleData<glm::vec3> uvTriangleData = triangle.getVertexData(*uv_nattr);

	// Calculate uv average
	glm::vec3 uv_avg = { 0.0, 0.0, 0.0 };
	uv_avg += uvTriangleData.first();
	uv_avg += uvTriangleData.second();
	uv_avg += uvTriangleData.third();
	uv_avg /= 3.0f;
	
	// Set average to all vertices associated with triangle
	triangle.setVertexData(*uv_cattr, uv_avg);
}
~~~~~~~~~~~~~~~

Mesh Usage {#mesh_usage}
-----------------------

The [mesh data usage flag](@ref nap::EMeshDataUsage) determines how the mesh data is used at runtime. A `Static` mesh is uploaded from the CPU to the GPU exactly once. This allows the system to remove unused buffers after the upload is complete. If there is the need to update a mesh more frequently, even once after upload, it is required the usage is set to `DynamicWrite`. Note that static meshes are often placed in a different cache on the GPU, not accessible by the CPU, which allows for faster drawing times. `DynamicWrite` meshes are uploaded into shared CPU / GPU memory and are therefore slower to draw. Keep this in mind when selecting the appropriate data use.

Text {#text}
=======================

Rendering text is similar to rendering meshes, but instead of a mesh every [component](@ref nap::Renderable2DTextComponent) that can draw text links to a [font](@ref nap::Font). You can change text at runtime by calling setText() or declare a line of text in json.

The [font resource](@ref nap::Font) loads a font file from disk. All well known font formats are supported, including ttf en otf. Fonts can scale up to any size and are always rendered in their native resolution when using the [Renderable2DTextComponent](@ref nap::Renderable2DTextComponent). This ensures a perfect text representation at every size.

There are currently two components that can draw text to screen: [Renderable2DTextComponent](@ref nap::Renderable2DTextComponent) and [Renderable3DTextComponent](@ref nap::Renderable3DTextComponent). When rendering text in screen space use the 2D version, when placing text somewhere in the world use the 3D version.

The [Renderable2DTextComponent](@ref nap::Renderable2DTextComponent) has a draw call that can be used to draw text directly at a specific location. The provided coordinates are in screen space (pixels), where 0,0 is the bottom left corner of your screen or back-buffer. Alternatively you can use the [render service](@ref nap::RenderService) to render your 2D text. This is similar to rendering meshes. 3D text is always rendered using the render-service. The component that renders text uses it's own hard coded [shader](@ref nap::FontShader) so you don't have to link in a custom material.

The HelloWorld demo shows you how to set this up.

Materials and Shaders {#materials}
=======================

A [shader](@ref nap::Shader) is a piece of code that is executed on the GPU. You can use shaders to perform many tasks including rendering a mesh to screen or in to a different buffer. The material tells the shader how to execute that piece of code. A material therefore:

- Defines the mapping between the mesh vertex attributes and the shader vertex attributes
- Stores and updates the uniform shader inputs
- Stores and updates the sampler shader inputs
- Controls render settings such as the blend and depth mode

Multiple materials can reference the same shader. You can change the properties of a material on a global (resource) and instance level. To change the properties of a material on an instance you use a [MaterialInstance](@ref nap::MaterialInstance) object. A material instance is used to override uniform and sampler inputs and change the render state of a material. This makes it possible to create a complex material with default attribute mappings and uniform inputs but override specific settings for a specific object. 

Imagine you have twenty buttons on your screen that all look the same, but when you move your mouse over a button you want it to light up. You can do this by making a single material that is configured to show a normal button and change the unifom 'color' for the button you are hovering over. Changing the color uniform is done by altering the material instance attribute 'color'.

Vertex Attributes {#vertex_attrs}
-----------------------
Meshes can contain any number of vertex attributes. How those attributes correspond to vertex attributes in the shader is defined in the material. It is simply a mapping from a mesh attribute ID ('Position') to a shader attribute ID ('in_Position'). Consider this simple .vert shader:

~~~~~~~~~~~~~~~{.c}
uniform mat4 projectionMatrix;	//< camera projection matrix
uniform mat4 viewMatrix;		//< camera view matrix (world space location)
uniform mat4 modelMatrix;		//< vertex model to world matrix

in vec3	in_Position;			//< in vertex position object space
in vec4	in_Color0;				//< in vertex color
in vec3	in_UV0;					//< in vertex uv channel 0

out vec4 pass_Color;			//< pass color to fragment shader
out vec3 pass_Uvs;				//< pass uv to fragment shader

void main(void)
{
	// Calculate vertex position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Color = in_Color;
	pass_Uvs = in_UV;
}
~~~~~~~~~~~~~~~

This (vertex) shader doesn't do a lot. It transforms the vertex position and passes the vertex color and UV coordinates to the fragment shader. The vertex attributes are called 'in_Position', 'in_Color0' and 'in_UV0'. Next we bind the mesh vertex attributes to the shader vertex inputs using a material. To do that we provide the material with a table that binds the two together:

```
{
	"Type" : "nap::Material",
	"mID": "MyMaterial",
	"Shader": "MyShader",
	"VertexAttributeBindings" : 
	[
		{
			"MeshAttributeID": "Position",				//< Mesh position vertex attribute
			"ShaderAttributeID": "in_Position"			//< Shader position input
		},
		{
			"MeshAttributeID": "UV0",					//< Mesh uv vertex attribute
			"ShaderAttributeID": "in_UV0"				//< Shader uv input
		},
		{
			"MeshAttributeID": "Color0",				//< Mesh color vertex attribute
			"ShaderAttributeID": "in_Color0"			//< Shader color input
		}
	]		
}
```

The shader is always leading when it comes to mapping vertex attributes. This means that all the exposed shader vertex attributes need to be present in the material and on the mesh. It is also required that they are of the same internal type. To make things a bit more manageable and convenient: a mesh can contain more attributes than exposed by a shader. The mapping (as demonstrated above) can also contain more entries than exposed by a shader. This makes it easier to create common mappings and iterate on your shader. It would be inconvenient if the application yields an error when you comment out attributes in your shader. Even worse, if certain code in the shader is optimized out while working on it, certain inputs might not exist anymore. In these cases you don't want the initialization of your material to fail.

Default Vertex Attributes {#default_attrs}
-----------------------

Meshes that are loaded from file contain a fixed set of vertex attributes:
-	Position (required)
-	Normal (optional)
-	Tangent (auto generated when not available)
- 	Bitangents (auto generated when not available)
-	Multiple UV channels (optional)
-	Multiple Color channels (optional)

The names of the default vertex attributes can be retreived using a set of [global variables](@ref renderglobals.h).

~~~~~~~~~~~~~~~{.cpp}
nap::vertexid::position
nap::vertexid::normal
nap::vertexid::color
nap::vertexid::uv
//etc...
~~~~~~~~~~~~~~~

Every material creates a default mapping if no mapping is provided. The UV and Color attributes are included up to four channels. Default shader input names can be retrieved using a set of [global variables](@ref renderglobals.h), similar to vertex attributes:

~~~~~~~~~~~~~~~{.cpp}
nap::vertexid::shader::position
nap::vertexid::shader::normal
nap::vertexid::shader::color
nap::vertexid::shader::uv
//etc...
~~~~~~~~~~~~~~~

The following table shows the default mesh to shader vertex bindings:

Mesh 			| Shader 			|
:-------------: | :-------------:	|
Position 		| in_Position		|
Normal 			| in_Normal			|
Tangent 		| in_Tangent 		|
Bitangent 		| in_Bitangent		|
UV0 			| in_UV0			|
UV1 			| in_UV1			|
UV2 			| in_UV2			|
UV3 			| in_UV3			|
Color0 			| in_Color0			|
Color1 			| in_Color1			|
Color2 			| in_Color2			|
Color2 			| in_Color2			|

Uniforms {#uniforms}
-----------------------

Uniforms are shader input 'values' that can be set using the material interface. Every material stores a value for each uniform in the shader. It is allowed to have more uniforms in the material than the shader. This is similar to vertex attributes with one major exception: not every uniform in the shader needs to be present in the material. If there is no matching uniform, a default uniform will be created internally. Every uniform can be accessed by client code and changed at runtime through the material instance interface. Consider the following `font.frag` shader example:

```
#version 450 core

in vec3 passUVs;
uniform sampler2D glyph;

uniform UBO
{
	uniform vec3 textColor;				//< Text color input
} ubo;

// output
out vec4 out_Color;

void main() 
{
	// Get alpha from glyph 
	float alpha = texture(glyph, passUVs.xy).r;

	// Use alpha together with text color as fragment output
    out_Color = vec4(ubo.textColor, alpha);
}
```

And corresponding JSON:

```
{
    "Type": "nap::Material",
    "mID": "FontMaterial",
    "Uniforms": [
        {
            "Type": "nap::UniformStruct",
            "mID": "nap::UniformStruct",
            "Name": "UBO",
            "Uniforms": [
                {
                    "Type": "nap::UniformVec3",
                    "mID": "UniformVec3",
                    "Name": "textColor",
                    "Value": {
                        "x": 1.0,
                        "y": 1.0,
                        "z": 1.0
                    }
                }
            ]
        }
    ],
    "Samplers": [],
    "Shader": "FontShader",
    "VertexAttributeBindings": [],
    "BlendMode": "AlphaBlend",
    "DepthMode": "InheritFromBlendMode"
}
```

This material binds the color 'white' to the `textColor` uniform input of the shader. This means that all the text rendered with this material will be 'white' unless overridden. The `textColor` uniform value is part of the `UBO` uniform struct. Every uniform value must be a member of a uniform struct and can't be declared independent from a struct inside a shader. Uniform values can be directly overridden in JSON (using a [nap::MaterialInstanceResource](@ref nap::MaterialInstanceResource)) or overridden at run-time using code:

~~~~~~~~~~~~~~~{.cpp}
// Get 'UBO' struct that holds 'textColor'
nap::UniformStructInstance* ubo = text_comp.getMaterialInstance().getOrCreateUniform("UBO");

// Get text color, creates override if it doesn't exist
nap::UniformVec3Instance* text_color = ubo->getOrCreateUniform<UniformVec3Instance>("textColor");

// Override text color
text_color->setValue({1.0f, 0.0f, 0.0f});
~~~~~~~~~~~~~~~

The snippet above overrides the default text color from white to red at run-time. 

Note that uniform value (and sampler) names must be unique accross all shader stages. This means that for this example the `UBO.textColor` uniform can't be declared in both the '.frag' and '.vert' part of the shader. Doing this will lead to unexpected results. Initialization of the material will fail when you try to bind a value to the wrong type of input.

Samplers {#samplers}
-----------------------

A sampler binds a texture to a shader input. They are declared independent from uniforms in the shader and don't have to be part of a uniform struct. Consider the following .frag example:

```
#version 450 core

// vertex shader input  
in vec4 passColor;						//< frag color

// unfiorm sampler inputs 
uniform sampler2D inTexture;			//< Input Texture

// output
out vec4 out_Color;

void main() 
{
	// Extract output color from texture
	vec3 out_color =  texture(inTexture, passUVs.xy).rgb * passColor.rgb;

	// Set fragment color
	out_Color =  vec4(out_color, 1.0);
}
```

And the following JSON:

```
{
	"Type": "nap::Material",
	"mID": "WorldMaterial",
	"Uniforms": [],
	"Samplers": [
	    {
	        "Type": "nap::Sampler2D",
	        "mID": "world_input_tex_uniform",
	        "Name": "inWorldTexture"
	    }
	],
	"Shader": "WorldShader",
	"VertexAttributeBindings": [],
	"BlendMode": "Opaque",
	"DepthMode": "InheritFromBlendMode"
}
```

This material binds the `WorldTexture` resource to the `inTexture` sampler of the shader. All objects rendered with this material will use this texture as input unless overridden. Samplers can be directly overridden in JSON (using a [nap::MaterialInstanceResource](@ref nap::MaterialInstanceResource)) or overridden at run-time using code:

~~~~~~~~~~~~~~~{.cpp}
	// Get or create sampler override
	nap::MaterialInstance& material = render_comp.getMaterialInstance();
	Sampler2DInstance* sampler = material.getOrCreateSampler<Sampler2DInstance>("inTexture");

	// Update texture
	sampler->setTexture(newTexture);
~~~~~~~~~~~~~~~

Color Blending {#blending}
-----------------------

Materials also control the [blend](@ref nap::EBlendMode) and [depth](@ref nap::EDepthMode) state of a material before rendering an object to a target. The blend state specifies how a color that is rendered using a shader is combined into the target buffer. Three modes are available:

- Opaque: The shader overwrites the target value
- AlphaBlend: The alpha value is used to blend between the current and target value
- Additive: The shader output is added to the target value

Depth {#depth}
-----------------------

The [depth](@ref nap::EDepthMode) state controls how the z-buffer is treated. These modes are available:

- ReadWrite: The z output value is tested against the z-buffer. If the test fails, the pixel is not written. If the test succeeds, the new z-value is written back into the z-buffer.
- ReadOnly: The z output value is tested against the z-buffer. If the test fails, the pixel is not written. The current z-value is never written back to the z-buffer.
- WriteOnly: The z buffer always overwrites the current z value with the new z value.
- NoReadWrite: The z buffer is never tested and therefore not updated. 
- InheritFromBlendMode: This is a special mode that determines how the z-buffer is treated based on the blend mode. For Opaque blend modes ReadWrite is used. For the other (transparent) modes ReadOnly is used. Transparent objects generally want to use the z-buffer but not use it.

You can specify the GPU state for material resources and material instances.

Rendering Meshes {#renderwithmaterials}
-----------------------

The [RenderableMeshComponent](@ref nap::RenderableMeshComponent) is responsible for rendering a mesh with a material.

To render an object you need to combine a mesh with a material instance. This combination is called a [RenderableMesh](@ref nap::RenderableMesh) and is created by the renderable mesh component. Every mesh / material combination is validated by the system. An error is generated when the mesh does not contain the attributes that are required by the shader. In most cases the renderable mesh is created by the system for you. This happens when you link to a mesh and material from a renderable mesh component. The renderable mesh is automatically created when the component is initialized. When initialization succeeds the component is able to render all the shapes in the mesh instance. The [example](@ref render_example) at the top of this page shows you how to set this up.

You can switch between materials and meshes by providing the renderable mesh component with a different renderable mesh. When you want to switch only the material you can create new renderable mesh by calling [createRenderableMesh()](@ref nap::RenderableMeshComponentInstance::createRenderableMesh) using the existing mesh and a different material. Using this construct you can change a material, mesh or both. The mesh / material combination will be validated when creating a new renderable mesh. It is strongly recommended to create all selectable mesh / material combinations on initialization. This ensures that you can safely swap them at run time. The video modulation demo shows you how to create and switch between a selection of meshes at run-time.

Textures {#textures}
=======================

There are a lot of similarities between meshes and [textures](@ref nap::Texture2D). Both can be loaded from file and created (or updated) using the CPU. There are however some operations that only apply to textures:

- Textures can be read back from the GPU into CPU memory. Synchronous and asynchronously.
- Textures don't require a CPU data representation. For example: the [render texture](@ref nap::RenderTexture2D) only exists on the GPU.
- Some textures are continuously updated. This occurs when working with video or image sequences.

Creating Textures {#creating_textures}
-----------------------

NAP offers a small set of classes to work with textures.

![](@ref content/nap_textures.png)

The base class for all textures in NAP is [Texture2D](@ref nap::Texture2D). This object only holds the GPU data. External CPU storage is required when:
- Pixel data needs to be uploaded to the GPU.
- Pixel data needs to be read from the GPU to a CPU buffer.

CPU storage is provided in the form of a [Bitmap](@ref nap::Bitmap). The bitmap offers a high level CPU interface to work with pixel data. It allows you to:
- Retrieve individual pixels from the underlying data buffer.
- Set individual pixels in the buffer.
- Perform pixel color conversion operations.
- Retrieve information such as the amount of color channels, ordering of the pixel data etc.

You can also use a more low-level interface to upload data directly into your texture. This interface works with pointers and can be used to stream in large quantities of external data.

###GPU Textures {#gpu_textures}###

The [RenderTexture](@ref nap::RenderTexture2D) can be used to declare a texture on the GPU in JSON. Every GPU texture can be attached to a [render target](@ref nap::RenderTarget). The render target is used by the render service to draw a set of objects directly into the attached texture. This type of texture exposes a set of attributes that can be changed / authored in JSON. The following example creates a color texture on the GPU and attaches it to a render target:

```
{
	"Type" : "nap::RenderTexture2D",
	"mID" : "ColorTexture",
	"Usage": "Static",
	"Width" : 1920,
	"Height" : 1080,
	"Format" : "RGBA8"
},
{
	"Type": "nap::RenderTarget",
    "mID": "RenderTarget",
    "mColorTexture": "ColorTexture",
    "mClearColor": 
    {
    	"x": 1.0,
    	"y": 0.0,
        "z": 0.0,
        "w": 1.0
	}
},
```

Note that the `Usage` of the texture is set to `Static`. This is important because we never read or write from or to the texture using the CPU. Only the GPU uses the texture as a target for the render operation. 

###Images {#images}###

An [Image](@ref nap::Image) is a two-dimensional texture that manages the data associated with a texture on the CPU and GPU. The CPU data is stored internally as a [bitmap](@ref nap::Bitmap). This makes it easy to: 
- Quicky upload pixel data from the CPU to the GPU
- Transfer pixel data from the GPU to the CPU

It is easy to change the contents of an image at runtime:

~~~~~~~~~~~~~~~{.cpp}
void App::update()
{	
	// Get the CPU image data as a bitmap
	Bitmap& bitmap = mImage.getBitmap();

	// Adjust the pixel data here....

	// Upload changes to the GPU
	mImage.update();
}
~~~~~~~~~~~~~~~

###Image From File {#image_from_file}###

[ImageFromFile](@ref nap::ImageFromFile) allows you to load an image from disk. This object offers the exact same functionality as a native image. You can update your content or read data from the GPU using the same interface. To create an image in JSON:

```
{
	"Type" : "nap::ImageFromFile",
	"mID" : "background",
	"ImagePath" : "background.jpg",
	"Usage": "Static",
	"GenerateLods": true
}
```

Reading Textures From The GPU {#reading_textures}
-----------------------

Textures contain the output of a GPU rendering step when they are assigned to a render target. You can read back the result from a texture on the GPU to the CPU using the 2D texture or image interface. The following functions allow you to transfer the rendered texture back from the GPU to the CPU:

- nap::Texture2D::asyncGetData(Bitmap& bitmap);
- nap::Image::asyncGetData();

You can see that the 2D texture interface requires you to pass in external storage in the form of a bitmap. The image interface will transfer the image back into its internal bitmap. The asyncGetData() function will not stall the CPU and queues the copy operation on the GPU. After the copy is executed by the GPU the data is automatically transferred.

Texture Usage {#texture_usage}
-----------------------

The texture `Usage` flag allows you to specify how the texture is going to be used.

- Static: The texture does not change after initial upload.
- DynamicRead: Texture is frequently read from GPU to CPU.
- DynamicWrite: Texture is frequently updated from CPU to GPU.

It's important to choose the right setting based on your needs. It is for example not allowed to update `Static` or `DynamicRead` textures after the initial upload from the CPU because the staging buffer is deleted after upload. Doing so will result in render artifacts (depending on the driver) or potentially a system crash. On the other hand: `DynamicWrite` allocates additional resources on the GPU and should therefore only be used if you are going to write to the texture more than once from the CPU. Note that it is perfectly safe to set the usage to `Static` when frequently writing to it on the GPU only, for example when using it as a render target.

Texture Sampling {#texture_sampling}
-----------------------

A sampler [parameters](@ref nap::Sampler) controls how a texture is sampled. These are the parameters that can be specified:
- `MinFilter`: Controls how the texels are blended when the texture is minified.
- `MaxFilter`: Controls how the texels are blended when the texture is magnified.
- `MipMapMode`: Controls how texels are blended between mip-maps.
- `AddressModeVertical`: How the UV mapping is interpreted vertically.
- `AddressModeHorizontal`: How the UV mapping is interpreted horizontally.
- `AnisotropicSamples`: Max number of anisotropic filter samples.
- `MaxLodLevel`: Max number of lods to use.

A LOD level of 0 prevents the texture from mipmapping, ie: the renderer only chooses the highest (native) texture resolution. This setting has no influence when mip mapping is turned off. You can change the parameters of every 2D texture (image, render texture etc.) in JSON:

```
{
    "Samplers": 
    [
        {
            "Type": "nap::Sampler2D",
            "mID": "tex_input_uniform",
            "Name": "inTexture",
            "MinFilter": "Linear",
            "MaxFilter": "Linear",
            "MipMapMode": "Linear",
            "AddressModeVertical": "ClampToEdge",
            "AddressModeHorizontal": "ClampToEdge",
            "AnisotropicSamples": "Default",
            "MaxLodLevel": 1000,
            "Texture": "WorldTexture"
        }
    ]
}
```

Windows {#multi_screen}
=======================

You can add as many windows to your application as you want. Take a look at the multi window demo for a working example. That demo spawns three windows and renders the same set of objects (in different configurations) to every one of them. In your application you have to activate the window you want to render to before issuing any draw commands. This is demonstrated in the example below:

~~~~~~~~~~~~~~~{.cpp}
void MultiWindowApp::render()
{
	// Signal the beginning of a new frame, allowing it to be recorded.
	mRenderService->beginFrame();

	// Render to window one
	if(mRenderService->beginRecording(*mRenderWindowOne))
	{
		// Begin the render pass
		mRenderWindowOne->beginRendering();

		...

		// Draw gui to window one
		mGuiService->draw();

		// End rendering and recording
		mRenderWindowOne->endRendering();
		mRenderService->endRecording();
	}

	// Render to window two
	if(mRenderService->beginRecording(*mRenderWindowTwo))
	{
		// Begin render pass
		mRenderWindowTwo->beginRendering();

		...

		// Draw gui to window two
		mGuiService->draw();

		// End rendering and recording
		mRenderWindowTwo->endRendering();
		mRenderService->endRecording();
	}

	// End frame
	mRenderService->endFrame();
}
~~~~~~~~~~~~~~~

Offscreen Rendering {#offscreen_rendering}
=======================

Often you want to render a selection of objects to a texture instead of a screen. But you can't render to a texture directly, you need a [render target](@ref nap::RenderTarget) to do that for you. Every render target requires a link to a color texture. The result of the render step is stored in the texture. You can declare a render target in JSON just like any other resource:

```
{
    "Type": "nap::RenderTexture2D",
    "mID": "OutputColorTexture",
    "Usage": "Static",
    "Width": 1920,
    "Height": 1080,
	"Format": "RGBA8"
},
{
	"Type": "nap::RenderTarget",
    "mID": "VideoRenderTarget",
    "SampleShading": true,
    "Samples": "Four",
    "ClearColor": 
    {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
    	"w": 1.0
    },
   	"ColorTexture": "OutputColorTexture"
}
```

In this example we create a color texture and a render target. The render target links to the color texture. The only thing left to do is locate the target in your application and give it to the render service together with a selection of components to render. All headless (non window) render operations need to be executed within a beginHeadlessRecording() and endHeadlessRecording() block:

~~~~~~~~~~~~~~~{.cpp}

void VideoModulationApp::render()
{
	// Signal the beginning of a new frame, allowing it to be recorded.
	mRenderService->beginFrame();

	// Start recording into the headless recording buffer.
	if (mRenderService->beginHeadlessRecording())
	{
		// Render into the render target
		mVideoRenderTarget->beginRendering();
		...
		mVideoRenderTarget->endRendering();

		// Tell the render service we are done rendering into render-targets.
		mRenderService->endHeadlessRecording(); 
	}

	// Done rendering this frame
	mRenderService->endFrame();
}
~~~~~~~~~~~~~~~

Alternatively you can use the [RenderToTextureComponent](@ref nap::RenderToTextureComponent). This component allows you to render to a texture directly in screen space, without the need to define a render target or mesh, and can be used to apply a 'post process' render step. The video modulation demo uses this component to convert the output of a video player into a greyscale texture.

Cameras {#cameras}
=======================

NAP supports two camera types:

- [Orthographic](@ref nap::OrthoCameraComponent)
- [PerSpective](@ref nap::PerspCameraComponent)

With an orthographic camera the scene is rendered using a flat projection matrix. With an orthographic camera the scene is rendered using a perspective matrix. The render service expects a reference to one of the cameras when rendering a set of objects to a target. You can create as many cameras as you want by adding one as a component to an entity in JSON:

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
			"Type" : "nap::TransformComponent"
		}
	]
}
```

Two things define a camera: its location in the world and type. The world space location of a camera is used to compose the view matrix. The camera projection method is used to compose the projection matrix. Both are extracted by the renderer and forwarded to the shader. Every camera therefore needs access to a transform component that is a sibling of the parent entity. For a working example take a look at the multi window demo. This demo renders a set of objects to different windows using a mix of cameras.