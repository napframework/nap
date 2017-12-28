System {#system}
=======================

*	[Overview](@ref system_overview)
*	[Modules & Services](@ref modules_services)
*   [Apps](@ref apps)
*   [Core](@ref core)
*	[Resources](@ref resources)
*	[Resourcemanager](@ref resourcemanager)
*	[Exposing Resources](@ref exposing_resources)
*	[Exposing Properties](@ref exposing_properties)
* 	[Pointing to Resources](@ref pointing)
* 	[Real Time Editing](@ref editing)
* 	[Linking Media](@ref media)
* 	[Scene Setup](@ref scene_setup)
*	[Resources vs Instances](@ref resources_instances)
*	[Creating Components](@ref creating_components)
* 	[Embedding Objects](@ref embedding_objects)
* 	[Embedding Pointers](@ref embedding_pointers)

Overview {#system_overview}
=======================

NAP enables you to connect and exchange data between various types of external hardware in a generic fashion. The system is designed to make it easy to re-use specific parts or components for future projects and keep app specific code local to your project. The underlying system provides you with all the handles to get up and running in no time. But it's important to understand what parts contribute to the overall system architecture. Below you see a dumbed-down schematic of an application build with NAP. This schematic shows some of the key components of the NAP system architecture:

![](@ref content/nap_overview.png)

Let's start reading the graph left to right. Starting from the left we see an application runner that combines 3 objects, of which 2 are important: The Application and Core. Applications are the entry point for project specific code. This is where you define what parts of your application:
- Receive an update call 
- Are rendered
- Receive messages
- Etc.

Core is the heart of every NAP application and manages (among other things) modules. Core is also the gateway to the ResourceManager. Every NAP application requires a Core object. That's the reason you explicitly create one and give it to the object that runs your application. When creating Core you also create a ResourceManager. The resource manager does a lot of things but most importantly: it makes your life easy. It creates all the objects that are associated with your application, initializes them in the right order and keeps track of any content changes. When a change is detected, the resource manager automatically patches the system without having to re-compile your application. The initialzation call of your application is the perfect place to load the file and check for content errors.

Modules are libraries that expose building blocks. You can use these building blocks to construct your application. Most modules expose specific building blocks, for example. The OSC module exposes osc receiving and sending objects, a generic interface to create and extract osc events and a service that deals with the osc library and network. Core loads all available modules automatically and initializes them in the right order. After a module is loaded all the building blocks are registered and the module can be initialized. You, as a user, don't have to do anything.

The diagram has 4 resources from 3 different modules:
- 1 OSC Receiver from the OSC Module
- 2 Windows from the Render Module
- 1 Midi Sender from the Midi Module

After initializing core (and therefore all modules) the building blocks can be created by the resource manager. We add the building blocks as individual resources to our JSON file and tell the resourcemanager to load the file and voila: 
- You now have an OSC receiver that already opened it's port and is listening to messages
- Your 2 windows are visible on screen
- You are ready to send some midi notes over the just opened port

You might notice that working this way saves you from typing many lines of code. You don't have to declare objects in C++ or have to worry about the right order of initialization. You can directly access the resources and start building what you had in mind.

Modules & Services {#modules_services}
=======================

Following the modular design of NAP, functionality is split up in Modules. Each module contains blocks of functionality that can be reused as: 
- [Resources](@ref resources) 
- [Entities](@ref scene_setup)
- [Components](@ref creating_components)

These are discussed later in separate sections. A module gets compiled into dynamically linkable libraries that are loaded into the application at startup. Each module can contain a [Service](@ref nap::Service). A Service is a rather abstract concept and can be used in many different ways. To understand what it does, let’s start with an example: the [RenderService](@ref nap::RenderService). The render service provides the following things:
- It initializes the OpenGL subsystem, and terminates it on exit.
- Each frame, the service processes system events such as the window resize event.
- It provides a high-level rendering interface for all Resources, Components and Entities to use.

In a more abstract sense, a [Service](@ref nap::Service) can be used to perform any application wide operations like connecting with a server, or initializing devices. A service also receives an update() call on each frame, so you can perform any frame-based logic there. The Service receives an init() call when the module is loaded. It is possible that a Service wants to use functionality from other services. NAP takes care of the correct order of initialization if you tell the system what other services you are dependent on by deriving from Service and implementing the getDependentServices() function. In this function you need to return a list of the types of Services you are depending on. This will both make sure that init() is called in the proper order, as well as the update() order.

Apps {#apps}
=======================

The main entrypoint for applications is the [AppRunner](@ref nap::AppRunner) object. The AppRunner object requires two things: an [Application](@ref nap::App) to run and an [object](@ref nap::BaseAppEventHandler) that knows how to forward events that happen in the system, such as mouse and keyboard events. The application object should be derived from [BaseApp](@ref nap::BaseApp) and the event handler should be derived from [BaseAppEventHandler](@ref nap::BaseAppEventHandler).

A good default to start with is to derive your application from [App](@ref nap::App) and use the default class [AppEventHandler](@ref nap::AppEventHandler). This will make sure you have input handling set up in your application. This is how such a default setup would look like in main.cpp:

~~~~~~~~~~~~~~~{.cpp}
// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner
	nap::AppRunner<nap::MyApp, nap::AppEventHandler> app_runner(core);

	// Start running: This will initialize the engine, register all the modules and start the application loop
	nap::utility::ErrorState error;
	if (!app_runner.start(error))
	{
		nap::Logger::fatal("error: %s", error.toString().c_str());
		return -1;
	}

	// Return if the app ran successfully
	return app_runner.exitCode();
}
~~~~~~~~~~~~~~~

Core {#core}
=======================

Most of the functionality in NAP is split up into modules. There is one exception, which is functionality that is present in the [Core](@ref nap::Core) library. This library is a static library and therefore always loaded and can be used everywhere. For each application, a single nap::Core object instance is passed around throughout the system. It manages Services (and the order of initialization and updating of the Services as discussed earlier) and it manages the [ResourceManager](@ref resourcemanager) object. The ResourceManager is an important object that we will discuss separately. 

The Core object is the primary entry point into the system and it is generally used to get to the Services you need and to find objects that you need in the ResourceManager.

Resources {#resources}
=======================

Any content that is created in NAP is authored in a readable text format. NAP is using the ‘json’ format to store contents in a text file. Here is an example of how this content looks like:
```
{
	"Objects" : 
	[
		{
			"Type" : "nap::RenderWindow",
			"mID" : "Window",
			"Width" : 512,
			"Height" : 512,
			"Title" : "TestWindow"
		},
		{
			"Type" : "nap::Image",
			"mID" : "BackgroundImage",
			"ImagePath" : "data/test/background.jpg"
		},
		{
			"Type" : "nap::Shader",
			"mID": "UIShader",
			"mVertShader": "shaders/test/ui.vert",
			"mFragShader": "shaders/test/ui.frag"
		},	
		{
			"Type" : "nap::PlaneMesh",
			"mID": "PlaneMesh"
		}
	]
}
```

The json format can be used to author your content, but NAP also provides an editor that reads and writes the json format for you and provides an easy to use interface to add, remove and modify objects.

As you can see in the json example, the file contains a list of various objects. The objects have different types and different properties. These objects in json match with objects that are defined in C++. For instance, there are objects of type nap::RenderWindow, nap::Image, nap::Shader and nap::PlaneMesh in the NAP C++ codebase. When this json file is loaded, an object of that type is created and initialized with the properties that are in the json file. All of these objects are Resources.

Resourcemanager {#resourcemanager}
=======================

The ResourceManager is responsible for loading the file through the [loadFile()](@ref nap::ResourceManager::loadFile()) function. When loading this file, the objects are created and initialized. All of the objects in a json file are Resources. Any resource loaded is owned by the ResourceManager, meaning that the lifetime is fully managed by the ResourceManager (and not managed by the client).

The json example shows that each Resource has an ID. After loading, resources can be found through their ID by using ResourceManager::findObject().
Any resource must be derived from rtti::RTTIObject. This object contains the ID that is used to identify the objects and it contains a very important function: the init() function. Derived classes can implement this function to initialize the object and return whether this has failed or succeeded. A good example of a Resource is the nap::Image from the json example. The init() function will load the image from disk located at its “ImagePath” property (in this case "data/test/background.jpg") and store it internally. The init() function and how it should be implemented is discussed in more detail in a later section.

Exposing Resources {#exposing_resources}
=======================
To make sure that C++ classes and properties can be created and edited in json you need to expose them explicitly through something called RTTI (better known as [RunTime Type Information](https://en.wikipedia.org/wiki/Run-time_type_information)). NAP uses a number of macros to ease the way you can expose classes and properties. To create an object that can be authored in json you need to derive if from [RTTIObject](@ref nap::rtti::RTTIObject) and tell the system what the parent object is. To accomplish this you add, in the class declaration, the RTTI_ENABLE macro:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Shader : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
};
~~~~~~~~~~~~~~~

And in the cpp file, the following macros are added:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(nap::Shader)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

This is the basis for setting up an RTTI-enabled class. In the example above we defined a Shader. Without the RTTI_ENABLE macro, the system cannot detect that Shader is derived from RTTIObject. The NAPAPI macro is a convenience macro that is used to make sure we can use classes in modules. It is used to export symbols in to dynamic link libraries. It is recommended to use this macro for all NAP classes and structures in modules. 

Exposing Properties {#exposing_properties}
=======================
To extend a class with properties you add a Property field to your class. In this example we add two strings called 'mVertPath' and 'mFragPath'. We use these properties to point to shader files on disk. The two shader files are loaded when creating the Shader object.
~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Shader : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
	public:
		std::string	mVertPath;				// Property: Path to the vertex shader on disk
		std::string mFragPath;				// Property: Path to the fragment shader on disk
};
~~~~~~~~~~~~~~~

And extend the rtti macro in the cpp file with information about our properties:
~~~~~~~~~~~~~~~{.cpp}
// Adding serialzable properties
RTTI_BEGIN_CLASS(nap::Shader)
  RTTI_PROPERTY("VertPath", &nap::Shader::mFragPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
  RTTI_PROPERTY("FragPath", &nap::Shader::mVertPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

mFragPath and mVertPath are now exposed to the system. The [EPropertyMetaData](@ref nap::rtti::EPropertyMetaData) enum provides additional information about properties. In this examples the vertex and fragment shader paths are required and the paths are a link to a (shader) file on disk. NAP exposes the following types of properties:

- Default  : will use the class default if the property was not set
- Required : the load of the json file will fail if the property was not set
- FileLink : the property defines a relationship with an [external file](@ref media)(.bmp, .wav etc.)
- Embedded : the property is an [embedded pointer](@ref embedding_pointers)

After defining the Shader it be created and edited in json:
```
{
	"Type" : "nap::Shader",
	"mID" : "FogShader",
	"VertPath" : "/data/myproject/shaders/fogshader.vert"
	"FragPath" : "/data/myproject/shaders/fogshader.frag"
}
```

The ID can be chosen as the user wishes. It can be used to retrieve the object from code using [findObject()](@ref nap::ResourceManager::findObject()), but it can also be used to refer to this object by other objects in json, as we will see later. More things are possible with the RTTI system. For instance, it has support for constructors with one or more arguments and it can also expose C++ enums in a way that is still readable in json. See typeinfo.h or the reference documentation for more detailed information how to do this.

Pointing to Resources {#pointing}
=======================

It is often useful if a resource can access information from another resource. NAP allows you to create links between objects (in json) to accomplish just that. A resource can point to other resources in json by referring to the name (nap::RTTIObject::mID) of a resource. In C++, we use a specific type of pointer to accomplish this: [ObjectPtr](@ref nap::ObjectPtr). Let'''s assume there is a class called 'Material' that points to a 'Shader'. 'Material' wants to use the information stored in the shader and exposes that in the form of a link to a shader. The material can now access the shader without having to worry about order of initialization. When the Material is initialized the shader has been created and resolved. You only have to implement the logic you want to perform based on the information that is present in the shader. 

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Material : public rtti::RTTIObject
{
	...
public:
	ObjectPtr<nap::Shader> mShader;	// Property: Link to a 'Shader' resource
};
~~~~~~~~~~~~~~~

We can now construct and edit these objects in json:

```
{
	"Objects" : 
	[
		{
			"Type" : "nap::Shader",
			"mID" : "FogShader",
			"VertPath" : "/data/myproject/shaders/fogshader.vert"
			"FragPath" : "/data/myproject/shaders/fogshader.frag"
		}

		{
			"Type" : "nap::Material",
			"mID"  : "FogMaterial",
			"Shader"  : "FogShader"
		}
	}
}
```

Both 'Material' and 'Shader' need to [register](@ref exposing_resources) their type and associated properties with RTTI. Since we already defined the shader we only have to define the material:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(nap::Material)
  RTTI_PROPERTY("Shader", &nap::Material::mShader, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

The material is now registered and (exposes as as property) the link to a shader. After calling nap::ResourceManager::loadFile(), these two objects will be created and the pointers will be ‘resolved’, meaning that they will be set to the correct object. In this case, Material::mShader points to a 'FogShader'. ResourceManager also makes sure that any object you are pointing to will be initialized prior to its own initialization. So, it is safe to assume that any object you are pointing to already has its init() called successfully. One thing to be careful with is that cyclic dependencies are not supported. It is not possible to point to objects that eventually point back at the original object. The system cannot determine correct initialization orders in these situations.

Real Time Editing {#editing}
=======================

NAP contains a powerful ‘real-time editing’ engine. This means that it is possible to make modifications to the original data while the application is running. This results in extremely fast iteration times. After a file is loaded through ResourceManager::loadFile, the file that is loaded is internally watched by the system, including any external files that it references (like textures or audio files). If changes to these files are encountered, the changes that are made are instantly reloaded into the system.

All it takes from a users perspective is to press save in your content tool. For example, a json file can be updated in a text editor or a texture can be saved in your favorite image editing program and NAP will transparently reload the files immediately.

From a programmers perspective, there are some rules to adhere to, to make sure this always works correctly. Each Resource follows the same init() pattern. When a file is loaded, all objects are initialized in the correct order. If any init() returns false, it means that initialization of that object failed. In that case, the entire load is canceled and the system returns back to the original state before loadFile was called, with an error message describing what went wrong during (re)loading. It is guaranteed that all the objects remain in their original state until the initialization sequence is completed successfully.

These are the rules for writing a correct init() function:
- Return true on success, false on failure
- Only assert (or halt program execution in any other way) on programmer errors, never on user errors
- If the function fails, make sure that a clear error message is present in the errorState object that is passed to init().
- Make sure that the init() function does not have any side effects. This means that it should not update any global state. The system cannot revert such changes in case of an error.

nap::utility::ErrorState is a class that offers convenient ways of reporting errors to the user. The general pattern is the following:

~~~~~~~~~~~~~~~{.cpp}
if (!errorState.check(loadImage(), 
	"Failed to load image %s because the dimensions are unsupported (%d:%d)", mPath.c_str(), mDimensions.x, mDimensions.y))
  return false;
~~~~~~~~~~~~~~~

The pattern is somewhat similar to the way asserts work: the first parameter is evaluated and if it evaluated to false, the error message in the parameters that follow is stored in the errorState object. Notice that multiple messages can be stacked in the errorState object. This is convenient in many situations where a very low-level message is generated, but the context where the error occurred is missing. By nesting nap::utility::ErrorState::check() calls in various functions, the context can still be provided.

Linking Media {#media}
=======================

In the [Real Time Editing](@ref editing) section we briefly touched upon linking to external files. Some objects read information from other files. Examples are: texture objects that read .PNG, .TGA files, or audio files that read .WAV or .MP3 files. The real-time editing system will reload any of these external files when a modification to them is made. For these situations we need to explicitly define the relationship to the external file in RTTI. You do this by marking a property as a file link in the RTTI properties:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Texture)
  RTTI_PROPERTY("Path", &Texture::mPath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

~~~~~~~~~~~~~~~

Scene Setup {#scene_setup}
=======================
Modern applications can grow considerably in size when it comes to the amount of data they have to manage and the complex logic the app needs to support. NAP uses a powerful [Entity Component](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system) system to aid the development process and manage your application. What separates Entities from regular Resource objects are the following properties:

- At the root level, we have one or more [Scenes](@ref nap::Scene). 
- A scene contains a hierarchy of [Entities](@ref nap::Entity)
- Entities hold [Components](@ref nap::Component). 
- Components add functionality to an Entity, ie: define it's behaviour
- Entities and Components can link to each other
- It is possible to create multiple ['instances'](@ref nap::EntityInstance) of a single [Entity](@ref nap::Entity)
- Entity and Component instances are updated each frame

Here is an example of a Scene containing Entities and Components:

 ```
Scene 
	Entity “Bike”
		Component “BikeInput”
		Component “Transform”
		Entity “Frame”
				Component “Transform”
				Component “RenderableMesh”
			Entity “Wheel”
				Component “RenderableMesh”
				Component “Transform”
			Entity “Wheel”
				Component “RenderableMesh”
				Component “Transform”
 ```

In this example there is a single Scene with a 'Bike' in it. The bike consists of a 'Frame' and two 'Wheels'. The 'BikeInput' component takes care of moving the bike in the right direction. Notice that the same Entity is placed under the Bike Entity two times. This means it is instantiated two times. But we can change the transformation for each instantation, as discussed here.

In json, such a structure looks like this (the components are omitted for brevity):
```
{
	"Type" : "nap::Scene",
		"mID": "Scene",
				
		"Entities" : 
		[
			{
				"Entity" : "Bike"
			}
		]
	},
	{
		"Type" : "nap::Entity",
		"mID": "Bike",
		
		"Components" : 
		[
			...
		],
		"Children" :
		[
			"Wheel",
			"Wheel"
		]
	},
	{
		"Type" : "nap::Entity",
		"mID": "Wheel",
		
		"Components" : 
		[
			...
		]
	},
	{
		"Type" : "nap::Entity",
		"mID": "Frame",
		
		"Components" : 
		[
			...
		]
	}
}
```

Resources vs Instances {#resources_instances}
=======================

Resources are completely static objects. They are read-only data containers. An [Entity](@ref nap::Entity) is a Resource but has a [runtime counterpart](@ref nap::EntityInstance) that is updated by NAP every frame. In the 'Bike' example, the position of the bike changes as it moves through the world. The bike's initial position is declared in json but the runtime position changes each frame. When there are multiple bikes in the scene, each bike has its own position. As a programmer you want to change the position of each bike programmatically, ie: set it based on a set of conditions. When you do that you modify the run-time state of a bike, not the resource that was used to create 'an instance of' the bike.

To summarize:
- Resources contain static, shared, read-only data
- Instances contain runtime-varying data and can be updated each frame

Both the Scene, Entity and Component have a resource and instance counterpart. NAP omits the resource part of the class name for readability:

- nap::Scene becomes a nap::SceneInstance
- nap::Entity becomes a nap::EntityInstance
- nap::Component becomes a nap::ComponentInstance

The resources are defined in json. When a resources is created (instantiated) NAP creates an instance of the resource behind the scenes and adds that to the scene hierarchy. SceneInstances contain EntityInstances which in turn hold ComponentInstances. This structure mirrors the structure in json. Just remember that at run-time, in your application, you work with the instances of Scenes, Entities and Components.

Creating Components {#creating_components}
=======================
A scene is a container for entities and an entity is a container for components. Scenes and entities do not execute any behavior by themselves. They allow you to group and organize your objects. Components are used to add functionality to an Entity, ie: define it's behaviour. It’s the component that receives an [init()](@ref nap::ComponentInstance::init) and [update()](@ref nap::ComponentInstance::update) call. Any programmable behavior is therefore executed in the Component.

NAP offers a number of components off the shelve, like the [TransformComponent](@ref nap::TransformComponent) and the [RenderableMeshComponent](@ref nap::RenderableMeshComponent). These can be used to build hierarchies of visual objects. Many other components exist as well, for instance, input, osc, midi and audio components. However, it is very likely that you want to create your own components. When creating your own component, derive from nap::Component and add the properties that need to be edited in json:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI PerspCameraComponent : public Component
{
	RTTI_ENABLE(Component)
	DECLARE_COMPONENT(PerspCameraComponent, PerspCameraComponentInstance)

	virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override 
	{ 
		components.push_back(RTTI_OF(TransformComponent)); 
	}

public:
	float mFieldOfView = 50.0f;				// Property: Camera Field Of View
};
~~~~~~~~~~~~~~~

Here we create a perspective camera with a field of view property. Some concepts are familiar to creating resources, but some others are new:

- Instead of deriving from nap::RTTIObject, we derive from nap::Component (which is derived from nap::RTTIObject).
- The DECLARE_COMPONENT macro tells NAP that this component has a PerspCameraComponentInstance counterpart, see [Resources vs Instances](@ref resources_instances)
- [getDependentComponents()](@ref nap::Component::getDependentComponents) can be overridden to create a dependency towards another component. 

If your component needs another component, in this case a transform to position the camera in the world, you can hint at it. NAP will make sure that a transform is available and initialized before the camera is initialized. If the entity doesn't have a transform the component can't be created and initialization fails. In json we can extend the “Bike“ scene with an entity that holds both the new component and a nap::TransformComponent

```
{
    "Type" : "nap::Scene",
    "mID": "Scene",
            
    "Entities" : 
    [
        {
            "Entity" : "Bike",
            "Entity" : "CameraEntity"
        }
    ]
},
{
    "Type" : "nap::Entity",
    "mID": "CameraEntity",
    
    "Components" : 
    [
        {
            "Type" : "nap::PerspCameraComponent",
            "mID" : " PerspCameraComponent",
            "FieldOfView" : 90
        },
        {
            "Type" : "nap::TransformComponent",
            "Properties": {
                "Translate": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                },
                "Rotate": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                },
                "Scale": {
                    "x": 1.0,
                    "y": 1.0,
                    "z": 1.0
                },
                "UniformScale": 1.0                        
            }
        }
    ]
}
```

 When calling [loadFile()](@ref nap::ResourceManager::loadFile()) the PerspCameraComponent is created as part of the CameraEntity. On [init()](@ref nap::ComponentInstance::init()) the camera component is able to access the transform because the dependency to the transform was set-up correctly. NAP will now attempt to to create an instance of the PerspCameraComponent: a PerspCameraComponentInstance. But we haven't created that object yet:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI PerspCameraComponentInstance : public ComponentInstance
{
	RTTI_ENABLE(ComponentInstance)
public:
	PerspCameraComponentInstance(EntityInstance& entity, Component& resource);

	virtual bool init(utility::ErrorState& errorState) override;
};
~~~~~~~~~~~~~~~

The RTTI needs to be setup in a similar way (although registering properties is not required as this object is not read from json). The init pattern and the error handling is exactly the same as with regular resources. A small difference is the fact that this object does not contain a default constructor. The constructor receives the entity this component belongs to and the resource counterpart of this instance. To make sure the object can be created (without a default constructor) we have to tell RTTI what constructor to use. You explicitly tell RTTI that there is no default constructor and for each custom constructor, we have to add one seperately:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PerspCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

When loadFile() is called, the constructor mentioned above will be invoked and our scene is instantiated correctly. The [update()](@ref nap::ComponentInstance::update) function can be overridden to add per-frame functionality. In the case of this camera component, the roles of the various components could look something like this:

Entity
- TransformComponent: initial camera location
- PerspCameraComponent: initial field of view

EntityInstance
- TransformComponent: The transform is modified at runtime by some other component, for example a component that handles mouse input
- PerspCameraComponentInstance: Calculates and stores the camera transform for the renderer


Embedding Objects {#embedding_objects}
=======================

C++ objects are often embedded into each other. For example:

~~~~~~~~~~~~~~~{.cpp}
 // RGB Color
class NAPAPI Color
{
	float r;
	float g;
	float b;
};

// Palette that contains two colors
class NAPAPI Palette : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
public:
	Color mColorOne;		// First Color of the palette
	Color mColorTwo;		// Second Color of the palette
};
~~~~~~~~~~~~~~~

Here we see a Palette object, derived from nap::rtti::RTTIObject, intended to be authored in json. It contains an embedded object (a compound) of type Color. Both classes Palette and Color have their properties set up in the cpp file as required:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Color)
	RTTI_PROPERTY("r", &Color::r, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("g", &Color::g, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("b", &Color::b, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(Palette)
	RTTI_PROPERTY("ColorOne", &Palette::mColorOne, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ColorTwo", &Palette::mColorTwo, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

In json this can now be authored as follows:
 ```
{
	"Objects" : 
	[
		{
			"Type" : "Palette",
			"mID" : "MyPalette",
			"ColorOne" : 
			{
				"r" : 0.0,
				"g" : 0.5,
				"b" : 0.7
			},
			"ColorTwo" : 
			{
				"r" : 1.0,
				"g" : 0.25,
				"b" : 0.5
			}
		}
	}
}
 ```

Notice that Color is not derived from RTTIObject. It does not need to as it does not require an ID, nor is it an object that lives by itself. Therefore it also does not need the RTTI_ENABLE macro: the system does not need to know what it is derived from, it is embedded in another object. The system does need to know the properties that it is using. It does not do any harm either to derive from [RTTIObject](@ref nap::rtti::RTTIObject), so if you have existing objects that you want to embed, you can.

Embedding Pointers {#embedding_pointers}
=======================

Embedded objects have the benefit that their notation in json is rather compact. But often pointers are used to link to other objects. For example:

~~~~~~~~~~~~~~~{.cpp}
// A palette container links to a color palette
class NAPAPI PaletteContainer : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
public:
	ObjectPtr<Palette>	mColorPalette;			// Property: Link to a color palette
};
~~~~~~~~~~~~~~~

In this example, PaletteContainer points to a color palette. Both PaletteContainer and the ColorPalette object will become root objects in the json file, which is sometimes messy and harder to read. For this reason, NAP supports ‘embedded pointers’. If an object logically belongs to another object, you can mark the pointer ‘embedded’, and the syntax will become similar to the way that compound objects are written. The RTTI definition in the cpp needs to change slightly:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Foo)
	RTTI_PROPERTY("ColorPalette", &PaletteContainer::mColorPalette,  nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

Now, we can write this in json as follows:
```
{
	"Objects" : 
	[
		{
			"Type" : "PaletteContainer",
			"mID" : "MyPaletteContainer",
			"ColorPalette" : 
			{
				"ColorOne" : 
				{
					"r" : 0.0,
					"g" : 0.5,
					"b" : 0.7
				},
				"ColorTwo" : 
				{
					"r" : 1.0,
					"g" : 0.25,
					"b" : 0.5
				}
			}
		}
	}
}
```
Not that the embedded object is still a regular object that is living inside the [ResourceManager](@ref nap::ResourceManager). The ID can be omitted in json, in this case an ID will be generated for you. If it wasn’t omitted, we could find it by ID in the ResourceManager.