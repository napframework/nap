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

Introduction {#render_intro}
=======================

The NAP renderer is designed to be open and flexible. All render related functionality is exposed as a set of building blocks instead of a fixed function pipeline. You can use these blocks to set up your own rendering pipeline. Meshes, textures, materials, shaders, rendertargets, cameras and windows form the basis of these tools. They can be authored using json and are exported using your favourite content creation program (Photoshop, Maya etc.)

NAP uses the OpenGL 3.3+ <a href="https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview" target="_blank">programmable rendering pipeline</a> to draw objects. This means NAP uses shaders to manipulate and render items. Using shaders improves overall performance and offers a more flexible method of working compared to the old fixed function render pipeline. The result is a more data driven approach to rendering, similar to what you see in popular game engines such as Unreal4 or Unity, except: NAP doesn't lock down the rendering process. An object that can be rendered isn't rendered by default: you explicetly have to tell the renderer:

- That you want to render an object
- How you want to render an object
- Where to render it to, ie: it's destination

The destination is always a render target and NAP currently offers two: Directly to screen or an off-screen target. This process sounds difficult but offers a lot of flexibility. Often you want to render a set of objects to a texture before rendering the textures to screen. Or you might want to render only a sub-set of objects to screen 1 and another set of objects to screen 2. 

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

 This schematic shows how to bind a shader to a mesh and render it to screen using the RenderableMeshComponent(@ref nap::RenderableMeshComponent)

![](@ref content/shader_material_binding.png)

Multiple materials can reference the same shader. You can change the properties of a material on a global (resource) and instance level. To change the properties of a material on an instance you use a MaterialInstance(@ref nap::MaterialInstance) object. A material instance is used to override  uniform values and change the render state of a material. This makes it possible to create a complex material with default attribute mappings and uniform values but override specific settings for a specific object. Imagine you have twenty buttons on your screen that all look the same, but when you move your mouse over a button you want it to light up. You can do this by making a single material that is configured to show a normal button and change the unifom 'color' for the button you are hovering over. Changing the color uniform is done by altering the material instance attribute 'color'.

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

Every material creates a default mapping using the above mentioned attributes when no mapping is provided. The UV and Color attributes are including up to four channels. The default naming on the shader side can be found using a similar construct:

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

