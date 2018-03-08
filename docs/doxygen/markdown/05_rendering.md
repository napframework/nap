Rendering {#rendering}
=======================

*	[Introduction](@ref render_intro)
*	[Key Features](@ref key_features)
*	[Example](@ref render_example)
*	[Meshes](@ref meshes)
	*	[Creating Meshes](@ref creating_meshes)
	*	[Mesh Format](@ref mesh_format)
*	[Materials and Shaders](@ref materials)
	*	[Mapping Attributes](@ref mapping_attrs)
	*	[Default Attributes](@ref default_attrs)
	*	[Uniforms](@ref uniforms)
	*	[Color Blending](@ref blending)
	*	[Depth](@ref depth)
	*	[Rendering Meshes](@ref renderwithmaterials)
*	[Textures](@ref textures)
	*	[Creating Textures](@ref creating_textures)
		*	[GPU Textures](@ref gpu_textures)
		*	[Images](@ref images)
		*	[Image From File](@ref image_from_file)
	* 	[Reading Textures From The GPU](@ref reading_textures)
	*	[Parameters](@ref texture_parameters)
*	[Offscreen Rendering](@ref offscreen_rendering)
*	[Cameras](@ref cameras)

Introduction {#render_intro}
=======================

The NAP renderer is designed to be open and flexible. All render related functionality is exposed as a set of building blocks instead of a fixed function pipeline. You can use these blocks to set up your own rendering pipeline. Meshes, textures, materials, shaders, rendertargets, cameras and windows form the basis of these tools. They can be authored using json and are exported using your favourite content creation program (Photoshop, Maya etc.)

NAP uses the OpenGL 3.3+ <a href="https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview" target="_blank">programmable rendering pipeline</a> to draw objects. This means NAP uses shaders to manipulate and render items. Using shaders improves overall performance and offers a more flexible method of working compared to the old fixed function render pipeline. The result is a more data driven approach to rendering, similar to what you see in popular game engines such as Unreal4 or Unity, except: NAP doesn't lock down the rendering process. An object that can be rendered isn't rendered by default: you explicetly have to tell the renderer:

- That you want to render an object
- How you want to render an object
- Where to render it to, ie: it's destination

The destination is always a render target and NAP currently offers two: directly to screen or an off-screen target. This process sounds difficult but offers a lot of flexibility. Often you want to render a set of objects to a texture before rendering the textures to screen. Or you might want to render only a sub-set of objects to screen 1 and another set of objects to screen 2. 

Traditionally this is a problem, most renderers tie a drawable object to 1 render context. This context is always associated with 1 screen. This makes sharing objects between multiple screens pretty much impossible. NAP solves this issues by allowing any object to be drawn to any screen, even sharing the same object between two or multiple screens. This is one of NAPs biggest strengths and offers a lot of flexibility in deciding how you want to draw things. You, as a user, don't have to worry about the possibility of rendering one or multiple objects to a particular screen, you know you can.

Key Features {#key_features}
=======================
- Build your own render pipeline using easy to understand building blocks.

- Meshes are not limited to a specific set of vertex attributes such as 'color', 'uv' etc. Any attribute, such as ‘wind’ or ‘heat’, can be added to drive procedural effects. 

- Nap supports both static and dynamic meshes. It's easy to bind your own attributes to materials using a safe and easy to understand binding system.

- Rendering the same content to multiple windows is supported natively.

- All render functionality is fully compatible with the real-time editing system. Textures, meshes or shaders can be modified and reloaded instantly without having to restart the application

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
	"ImagePath" : "data/helloworld/world_texture.png"
},

{
	"Type" : "nap::Shader",
	"mID": "WorldShader",
	"mVertShader": "shaders/helloworld/world.vert",
	"mFragShader": "shaders/helloworld/world.frag"
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
	if (!mResourceManager->loadFile("data/helloworld/helloworld.json", error))
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
	nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
	components_to_render.emplace_back(&renderable_world);
	
	// Find the camera
	nap::PerspCameraComponentInstance& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();

	// Render the world with the right camera directly to screen
	mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, components_to_render);
	
	// Swap screen buffers
	mRenderWindow->swap();
}
~~~~~~~~~~~~~~~

That's it, you should see a rotating textured sphere in the center of your viewport. This example tried to cover the basics of rendering with NAP but as you might have suspected: modern day rendering is a vast and complex subject. NAPs philosophy is to be open, it doesn't render for you. What NAP does best is getting you set up with the building blocks to render complex scenes. This also applies to many other facets of the framework. But in return you get a very open engine that allows you to render most things without having to write thousands of lines of code. 

To get a better understanding of rendering with NAP continue reading or play around with a render demo that ships swith NAP. This example is included as 'HelloWorld'.

Meshes {#meshes}
=======================

Creating Meshes {#creating_meshes}
-----------------------

At the heart of it all is the [IMesh](@ref nap::IMesh). This is the resource the [RenderableMeshComponent](@ref nap::RenderableMeshComponent) (and other components) link to. The IMesh is responsible for only one thing: to supply a [MeshInstance](@ref nap::MeshInstance). The meshinstance contains the actual data that is rendered or manipulated. A mesh instance can be created in various ways:

- The [MeshFromFile](@ref nap::MeshFromFile) loads a mesh from an external file. NAP only supports the fbx file format and automatically converts any .fbx file in to a .mesh file using the fbx converter tool. The result is a heavily compressed binary file. Here is an example:

```
{
	"Type" : "nap::MeshFromFile",
	"mID": "CarMesh",
	"Path": "car.mesh"
}
```

- A few simple shapes (such as a [plane](@ref nap::PlaneMesh) or [sphere](@ref nap::SphereMesh)) can be created directly using configurable parameters:

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
	"Rows" : 256,
	"Columns" : 256
}
```

- The [Mesh](@ref nap::Mesh) resource can be used to explicitly define the contents of a mesh in a json file. Below you see an example of a mesh in the shape of a plane. This mesh contains 4 vertices. The plane has a position and uv attribute. The triangles are formed as a TriangleStrip:

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

- Instead of using predefined resources you can also create meshes at runtime. To do so, derive from IMesh and create your own custom mesh in code. This is extremely useful when building procedural meshes. In the following example we derive from IMesh and create our own mesh instance. For the mesh to behave and render correctly we add a set of attributes. In this case 'Position', 'uv', 'id' and color. The mesh contains no actual (initial) vertex data. The mesh is constructed (over time) by the application.

~~~~~~~~~~~~~~~{.cpp}
class ParticleMesh : public IMesh
{
public:
	bool init(utility::ErrorState& errorState)
	{
		// Because the mesh is populated dynamically we set the initial amount of vertices to be 0
		mMesh.setNumVertices(0);

		// Reserve 1000 vertices
		mMesh.reserveVertices(1000);

		// Add position attribute
		... position_attribute = mMesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		
		// Add uv attribute
		... uv_attribute = mMesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));

		// Add color attribute
		... color_attribute = mMesh.getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));

		// Add unique identifier attribute
		... id_attribute = mMesh.getOrCreateAttribute<float>("pid");
			
		// Create the shape
		MeshShape& shape = mMesh.createShape();

		// Reserve CPU memory for all the particle geometry.
		// We want to draw the mesh as a set of triangles, 2 triangles per particle
		shape.setDrawMode(opengl::EDrawMode::TRIANGLES);
		shape.reserveIndices(1000);

		// Initialize our instance
		return mMesh.init(errorState);
	}

	/**
	 * @return MeshInstance as created during init().
	 */
	virtual MeshInstance& getMeshInstance()	override 					{ return mMesh; }

	/**
	 * @return MeshInstance as created during init().
	 */
	virtual const MeshInstance& getMeshInstance() const	override 		{ return mMesh; }

private:
	MeshInstance mMesh;
};
~~~~~~~~~~~~~~~

The Mesh Format {#mesh_format}
-----------------------

The mesh instance format is best explained by an example. Consider a mesh that represents the letter ‘P’:

![](@ref content/mesh_shapes.png)

This letter contains a list of 24 points. However, the mesh is split up into two separate pieces: the closed line that forms the outer shape and the closed line that forms the inner shape. The points of both shapes are represented by the blue and orange colors. 

Every mesh instance contains a list of points (vertices). Each vertex can have multipe attributes such as a normal, a uv coordinate and a color. In this example the mesh holds a list of exactly 24 vertices. To add each individual line to the mesh we create a [shape](@ref nap::MeshShape). The shape tells the system two things:

- How the vertices are connected using a list of indices. In this case vertices 0-12 define the outer shape, vertices 13-23 define the inner shape.
- How the system interprets the indices, ie: how are the points connected? For this example we use LINE_LOOP. A single list of connected lines that loops back to the beginning of the shape.

Within a single mesh you can define multiple shapes that share the same set of vertices (points). It is allowed to share vertices between shapes and it is allowed to define a single mesh with different types of shapes. Take a sphere for example. The vertices can be used to define both the sphere as a triangle mesh and the normals of that sphere as a set of individual lines. The normals are rendered as lines (instead of triangles) but share (part of) the underlying vertex structure. This sphere therefore contains 2 shapes, 1 triangle shape (to draw the sphere) and 1 line shape (to draw the normals).

All common opengl shapes are supported: POINTS, LINES, LINE_STRIP, LINE_LOOP, TRIANGLES, TRIANGLE_STRIP and TRIANGLE_FAN.

As a user you can work on individual vertices or on the vertices associated with a specific shape. Often its necessary to walk over all the shapes that constitute a mesh. On a higher level NAP provides utility functions (such as computeNormals and reverseWindingOrder) to operate on a mesh as a whole. But for custom work NAP provides a very convenient and efficient [iterator](@ref nap::TriangleIterator) that is capable of looping over all the triangles within multiple shapes. This ensures that as a user you don’t need to know about the internal connectivity of the various shapes. Consider this example:

~~~~~~~~~~~~~~~{.cpp}
// fetch uv attribute
Vec3VertexAttribute* uv_nattr = mesh.findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));

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

Materials and Shaders {#materials}
=======================

A shader is a piece of code that is executed on the GPU. You can use shaders to perform many tasks including rendering a mesh to screen or in to a different buffer. The material tells the shader how to execute that piece of code. A material therefore:

- defines the mapping between the mesh vertex attributes and the shader vertex attributes
- stores and updates the uniform shader values
- stores and updates global render settings such as the blend and depth mode

 This schematic shows how to bind a shader to a mesh and render it to screen using the [RenderableMeshComponent](@ref nap::RenderableMeshComponent)

![](@ref content/shader_material_binding.png)

Multiple materials can reference the same shader. You can change the properties of a material on a global (resource) and instance level. To change the properties of a material on an instance you use a [MaterialInstance](@ref nap::MaterialInstance) object. A material instance is used to override  uniform values and change the render state of a material. This makes it possible to create a complex material with default attribute mappings and uniform values but override specific settings for a specific object. Imagine you have twenty buttons on your screen that all look the same, but when you move your mouse over a button you want it to light up. You can do this by making a single material that is configured to show a normal button and change the unifom 'color' for the button you are hovering over. Changing the color uniform is done by altering the material instance attribute 'color'.

Mapping Attributes {#mapping_attrs}
-----------------------
Meshes can contain any number of vertex attributes. How those attributes correspond to vertex attributes in the shader is defined in the material. It is simply a mapping from a mesh attribute ID ('position') to a shader attribute ID ('in_Position'). 
Consider this simple vertex shader:

~~~~~~~~~~~~~~~{.c}
uniform mat4 projectionMatrix;	//< camera projection matrix
uniform mat4 viewMatrix;		//< camera view matrix (world space location)
uniform mat4 modelMatrix;		//< vertex model to world matrix

in vec3	in_Position;			//< in vertex position object space
in vec4	in_Color;				//< in vertex color
in vec3	in_UV;					//< in vertex uv channel 0

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

This (vertex) shader doesn't do a lot. It transforms the vertex position and passes the vertex color and uv coordinates to the fragment shader. The vertex attributes are called 'in_Position', 'in_Color' and 'in_UV'. To match these settings we create a mesh in json that contains the following vertex attributes: 'Position', 'UV0' and "Color0": 
```
{
	"Type" : "nap::Mesh",
	"mID" : "MyMesh",
	"Properties" : 
	{
		"NumVertices" : 4,
		"Shapes" : 
		[
			{
				"DrawMode" : "TriangleStrip"
			}
		],
		"Attributes" : 
		[
			{
				"Type" : "nap::Vec3VertexAttribute",
				"AttributeID" : "Position",
				"Data" : [
					// data here
				]
			},
			{
				"Type" : "nap::Vec3VertexAttribute",
				"AttributeID" : "UV0",
				"Data" : [
					// data here
				]
			},
			{
				"Type" : "nap::Vec4VertexAttribute",
				"AttributeID" : "Color0",
				"Data" : [
					// data here
				]
			}

		],
		// indices here..
	}
}
```

The last thing we do is bind the mesh vertex attributes to the shader using a material. To do that we provide the material with a table that binds the two together:

```
{
	"Type" : "nap::Material",
	"mID": "MyMaterial",
	"Shader": "MyShader",
	"VertexAttributeBindings" : 
	[
		{
			"MeshAttributeID": "Position",				//< Mesh position vertex attribute
			"ShaderAttributeID": "in_Position"			//< Binds to the shader 'in_Position' input
		},
		{
			"MeshAttributeID": "UV0",					//< Mesh uv vertex attribute
			"ShaderAttributeID": "in_UV"				//< Binds the the shader 'in_UV' input
		},
		{
			"MeshAttributeID": "Color0",				//< Mesh color vertex attribute
			"ShaderAttributeID": "in_Color"				//< Binds to the shader 'in_Color' input 
		}
	]		
}
```

The shader is always leading when it comes to mapping vertex attributes. This means that all the exposed shader vertex attributes need to be present in the material and on the mesh. It is also required that they are of the same internal type. To make things a bit more manageable and convenient: a mesh can contain more attributes than exposed by a shader. The mapping (as demonstrated above) can also contain more entries than exposed by a shader. This makes it easier to create common mappings and iterate on your shader. It would be inconvenient if the application yields an error when you comment out attributes in your shader. Even worse, if certain code in the shader is optimized out while working on it, certain inputs might not exist anymore. In these cases you don't want the initialization of your material to fail.

Default Attributes {#default_attrs}
-----------------------

Meshes that are loaded using the mesh from file resource have a fixed set of (partly optional) vertex attributes:
-	Position (required)
-	Normal (optional)
-	Tangent (auto generated when not available)
- 	Bitangents (auto generated when not available)
-	Multiple UV channels (optional)
-	Multiple Color channels (optional)

The names of these default attributes can be retreived using a set of global commands. The color and uv functions require an index:

~~~~~~~~~~~~~~~{.cpp}
VertexAttributeIDs::getPositionName()
VertexAttributeIDs::getNormalName()
VertexAttributeIDs::getColorName(0)
//etc...
~~~~~~~~~~~~~~~

Every material creates a default mapping using the above mentioned attributes when no mapping is provided. The UV and Color attributes are included up to four channels. The default naming on the shader side can be found using a similar construct:

~~~~~~~~~~~~~~~{.cpp}
opengl::Shader::VertexAttributeIDs 
~~~~~~~~~~~~~~~

The following table shows the default mesh to shader attribute bindings:

Mesh 			| Shader 			|
:-------------: | :-------------:	|
Position 		| in_Position		|
Normal 			| in_Normal			|
Tangent 		| in_Bitangent 		|
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

Uniforms are shader input parameters that can be set through the material interface. Every material stores a value for each uniform in the shader. It is allowed to have more uniforms in the material than the shader. This is similar to vertex attributes with one major exception: not every uniform in the shader needs to be present in the material. If there is no matching uniform, a default uniform will be created internally. That uniform can also be accessed by client code and changed at runtime. Consider the following example:

```
{
	"Type" : "nap::Material",
	"mID": "ParticleMaterial",
	"Shader": "ParticleShader",
	"BlendMode": "AlphaBlend",
	"Uniforms" : 
	[
		{
			"Type" : "nap::UniformTexture2D",
			"Name" : "particleTexture",
			"Texture" : "Particle"
		},
		{
			"Type" : "nap::UniformVec4",
			"Name" : "particleColor",
			"Value" : 
			{
				“x” : 1.0,
				“y” : 0.0,
				“z” : 0.0,
				“w” : 1.0
			}
		}
	]
}
```
The material mentioned above binds two uniforms: a texture to “particleTexture” and a color to “particleColor”. Initialization of the material will fail when you try to bind a resource or object to the wrong type of parameter. Every uniform that is created (or present) in the instance of a material takes presedence over the value in the material. It's easy to create (and access) a uniform parameter at runtime:

~~~~~~~~~~~~~~~{.cpp}
nap::UniformVec3&  color = mMaterial->getOrCreateUniform<nap::UniformVec3>("color");
color.setValue({1.0,0.0,0.0});
~~~~~~~~~~~~~~~

The snippet above creates a new uniform 'color' (if it didn't exist already) and changes the value of that parameter to red. This immediately overrides the material's default 'color' value.

Color Blending {#blending}
-----------------------

Materials also update the global GPU state, specifically the [blend](@ref nap::EBlendMode) and [depth](@ref nap::EDepthMode) state of a material before rendering an object to a target. The blend state specifies how a color that is rendered using a shader is combined into the target buffer. Thee modes are available:

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

There are a lot of similarities between meshes and [textures](@ref nap::Texture2D). Both can be loaded from file and created (or updated) using the CPU. There are (however) some operations that only apply to textures:

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

You can also use a more low level interface to upload data directly into your texture. This interface works with pointers and can be used to stream in large quantities of external data.

###GPU Textures {#gpu_textures}###

The [RenderTexture](@ref nap::RenderTexture) can be used to declare a texture on the GPU in json. Every GPU texture can be attached to a [render target](@ref nap::RenderTarget). The render target is used by the render service to draw a set of objects directly into the attached texture. This type of texture exposes a set of attributes that can be changed / authored in json. The following example creates a depth and color texture on the GPU and attaches them both to a render target:

```
{
	"Type" : "nap::RenderTexture2D",
	"mID" : "ColorTexture",
	"Width" : 640,
	"Height" : 480,
	"Format" : "RGBA8"
},
{
	"Type": "nap::RenderTexture2D",
    "mID": "DepthTexture",
    "Width": 640,
    "Height": 480,
    "Format": "Depth"
},
{
	"Type": "nap::RenderTarget",
    "mID": "RenderTarget",
    "mColorTexture": "ColorTexture",
    "mDepthTexture": "DepthTexture",
    "mClearColor": 
    {
    	"x": 1.0,
    	"y": 0.0,
        "z": 0.0,
        "w": 1.0
	}
},
```

###Images {#images}###

An [Image](@ref nap::Image) is a 2 dimensional texture that manages the data associated with a texture on the CPU and GPU. The CPU data is stored internally as a [bitmap](@ref nap::Bitmap). This makes it easy to: 
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

[ImageFromFile](@ref nap::ImageFromFile) allows you to load an image from disk. This object offers the exact same functionality as a native image. You can update your content or read data from the GPU using the same interface. To create an image in json:

```
{
	"Type" : "nap::Image",
	"mID" : "background",
	"ImagePath" : "background.jpg"
}
```

Reading Textures From The GPU {#reading_textures}
-----------------------

Textures contain the output of a GPU rendering step when they are assigned to a render target. You can read back the result from a texture on the GPU to the CPU using the 2D texture or image interface. The following functions allow you to transfer the rendered texture back from the GPU to the CPU:

- nap::Texture2D::getData(Bitmap& bitmap);
- nap::Texture2D::startGetData() / nap::Texture2D::endGetData(Bitmap& bitmap);
- nap::Image::getData();
- nap::Image::startGetData() / nap::Image::endGetData();

You can see that the 2D texture interface requires you to pass in external storage in the form of a bitmap. The image interface will transfer the image back into its internal bitmap. The getData functions will block until the GPU is fully done rendering the texture. Note that this is very costly: the GPU needs to complete everything in its queue before it can start the copy operation. The copy itself (from the GPU to the CPU) is another costly operation. It is therefore recommended to use the startGetData and endGetData function calls for high performance scenarios. The startGetData call will queue the copy operation on the GPU. Once the copy can be executed by the GPU it will first transfer the data to an internal buffer. When endGetData is called, it will block and copy the contents from the internal buffer to the destination bitmap. If the GPU isn't ready yet it will stall. It is therefore recommended to issue the startGetData call as quickly as possible and wait for endGetData until the very last moment: preferably the next frame.

Another thing that will affect performance is the [ETextureUsage](@ref opengl::ETextureUsage) flag. This flag allows you to specify how the texture is going to be used:

- Static: The texture never, or rarely changes and is never read back to CPU memory.
- DynamicRead: The texture is frequently read back from GPU to CPU memory.
- DynamicWrite: The texture is frequently updated from CPU to CPU memory.

This flag allows the video card driver to choose the most optimal memory on the GPU for your use case scenario. For instance: when DynamicWrite is set the texture will (most likely) be placed into WriteCombined memory.


Texture Parameters {#texture_parameters}
-----------------------

Texture [parameters](@ref nap::TextureParameters) are used to specify how a texture is sampled and how mip-mapping levels can be controlled. These are the parameters that can be specified:
- Filter mode: Controls how blending between texels is managed when the texture is either minified or magnified. You can specify filter modes for minification and magnification.
- Wrap mode: Controls how the UV mapping is interpreted. It can repeat, mirror or clamp the UV mapping. Wrap mode can be specified both for the horizontal and vertical direction.
- Max LOD level: Controls the max mipmap level.

The default filter mode when a texture is scaled down (minified) is: LinearMipmapLinear
The default filter mode when a texture is scaled up (magnified) is: Linear
The default vertical wrap mode is: ClampToEdge
The default horizontal wrap mode is: ClampToEdge
The default max lod level is: 20

A lod level of 0 prevents the texture from mipmapping, ie: the renderer only chooses the highest (native) texture resolution. This setting has no influence when mip mapping is turned off. You can change the parameters of every 2D texture (image, render texture etc.) in json:

```
{
 	"Type": "nap::RenderTexture2D",
    "mID": "VideoColorTexture",
    "Parameters": 
    {
    	"MinFilter": "Linear",
    	"MaxFilter": "Linear",
    	"WrapVertical": "ClampToEdge",
    	"WrapHorizontal": "ClampToEdge",
        "MaxLodLevel": 10
    },
    "Usage": "DynamicRead",
    "Width": 1920,
    "Height": 1080,
	"Format": "RGB8"
}
```

Offscreen Rendering {#offscreen_rendering}
=======================

Often you want to render a selection of objects to a texture instead of a screen. But you can't render to a texture directly, you need a [render target](@ref nap::RenderTarget) to do that for you. To see how this works take a look at the video modulation demo. In this demo a video is applied to a plane and rendered to a texture. This texture is used as an input for two materials. 

Every render target is a resource that links to two textures: a color and depth texture. The result of the render step is stored in both. The color texture contains the color information. The depth texture holds information about the distance of an object to the camera based on the clipping planes of the camera. You can declare a render target in json just like any other resource:

```
{
    "Type": "nap::RenderTexture2D",
    "mID": "VideoColorTexture",
    "Usage": "DynamicRead",
    "Width": 1920,
    "Height": 1080,
	"Format": "RGB8"
},
{
	"Type": "nap::RenderTexture2D",
	"mID": "VideoDepthTexture",
    "Usage": "Static",
    "Width": 1920,
    "Height": 1080,
    "Format": "Depth"
},
{
	"Type": "nap::RenderTarget",
    "mID": "VideoRenderTarget",
    "mColorTexture": "VideoColorTexture",
    "mDepthTexture": "VideoDepthTexture",
    "mClearColor": 
    {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
    	"w": 1.0
    }
}
```

In this example we create two textures and a render target. The render target links to both textures. The only thing left to do is locate the target in your application and give it to the render service together with a selection of components to render. Notice that rendering to a render target or screen works exactly the same:

~~~~~~~~~~~~~~~{.cpp}
// Clear buffers of video render target
mRenderService->clearRenderTarget(mVideoRenderTarget->getTarget());
			
// Get objects to render
std::vector<RenderableComponentInstance*> render_objects;
render_objects.emplace_back(&mVideoEntity->getComponent<RenderableMeshComponentInstance>());

// Render
mRenderService->renderObjects(mVideoRenderTarget->getTarget(), ortho_cam, render_objects);
~~~~~~~~~~~~~~~

Cameras {#cameras}
=======================