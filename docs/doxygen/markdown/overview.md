Overview {#overview}
=======================

*	[Install](@ref install)
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
* 	[Embedding Objects](@ref embedding_objects)
* 	[Embedding Pointers](@ref embedding_pointers)

Modules & Services {#modules_services}
=======================

Following the modular design of NAP, functionality is split up in Modules. Each module contains blocks of functionality that can be reused as: 
- [Resources](@ref resourcemanager_resources) 
- Entities
- Components 

These are discussed later in separate sections. A module gets compiled into dynamically linkable libraries that are loaded into the application at startup. Each module contains a single Service. A Service is a rather abstract concept and can be used in many different ways. To understand what it does, let’s start with an example: the [RenderService](@ref nap::RenderService). The render service provides the following things:
- It initializes the OpenGL subsystem, and terminates it on exit.
- Each frame, the service processes system events such as the window resize event.
- It provides a high-level rendering interface for all Resources, Components and Entities to use.

In a more abstract sense, a [Service](@ref nap::Service) can be used to perform any application wide operations like connecting with a server, or initializing devices. A service also receives an update() call on each frame, so you can perform any frame-based logic there. The Service receives an init() call when the module is loaded. It is possible that a Service wants to use functionality from other services. NAP takes care of the correct order of initialization if you tell the system what other services you are dependent on by deriving from Service and implementing the getDependentServices() function. In this function you need to return a list of the types of Services you are depending on. This will both make sure that init() is called in the proper order, as well as the update() order.

Apps {#apps}
=======================

The main entrypoint for applications is the [AppRunner](@ref nap::AppRunner) object. The AppRunner object requires two things: an application object to run and an object that knows how to handle events that happen in the system, like mouse and keyboard events. The application object should be derived from nap::BaseApp and the event handler should be derived from [BaseAppEventHandler](@ref nap::BaseAppEventHandler).

A good default to start with is to derive your application from [App](@ref nap::App) and use the default class [AppEventHandler](@ref nap::AppEventHandler). This will make sure you have input handling set up in your application. This is how you such a default setup would look like:
<TODO: needs more work>

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
To make sure that classes and properties that are defined in C++ can be authored in json, we need to expose them explicitly through something called RTTI, better known as [RunTime Type Information](https://en.wikipedia.org/wiki/Run-time_type_information). NAP uses a number of macros to ease the way you can expose classes and properties. When deriving from rtti::RTTIObject (or any other object), you should let the system know what type of object you are deriving from. In the class declaration, the RTTI_ENABLE macro is added:

```
class NAPAPI Foo : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
};
```

And in the cpp file, the following macros are added:

```
RTTI_BEGIN_CLASS(nap::Foo)
RTTI_END_CLASS
```

This is the basis for setting up an RTTI-enabled class. Without the RTTI_ENABLE macro, the system cannot detect that Foo is derived from RTTIObject. The NAPAPI macro is a convenience macro that is used to make sure we can use classes in modules. It is used to export symbols in dynamic link libraries. It is recommended to use this macro for all classes and structures in modules. 

Exposing Properties {#exposing_properties}
=======================
To extend a class with some properties, we just add a regular C++ field to our class. In this example that's an integer called 'mCount'
```
class NAPAPI Foo : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
	public:
		int	mCount = 0;
};
```

And extend the rtti macro in the cpp file with information about our property.
```
// Adding serialzable property 'mCount'
RTTI_BEGIN_CLASS(nap::Foo)
  RTTI_PROPERTY("Count", &nap::Foo::mCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
```

mCount is now exposed to the system. The EPropertyMetaData enum provides additional information about this property:
- Default  : will use the class default if the property was not set, in this case 0
- Required : the load of the json file will fail if the property was not set.
- FileLink : the property defines a relationship with an external file (.bmp, .wav etc.)
- Embedded : the property is an [embedded pointer](@ref embedding_pointers)

The object can be authored in json like this:
```
{
	"Type" : "Foo",
	"mID" : "SomeID",
	"Count" : 10
}
```

The ID can be chosen as the user wishes. It can be used to retrieve the object from code using ResourceManager::findObject(), but it can also be used to refer to this object by other objects in json, as we will see later.
More things are possible with the RTTI system. For instance, it has support for constructors with one or more arguments and it can also expose C++ enums in a way that is still readable in json. See typeinfo.h or the reference documentation for more detailed information how to do this.

Pointing to Resources {#pointing}
=======================

It is often useful if a resource can access information from another resource. NAP allows you to create links between objects (in json) to accomplish just that. A resource can point to other resources in json by referring to the name (nap::RTTIObject::mID ) of a resource. In C++, we use a specific type of pointer to accomplish this: the nap::ObjectPtr. Let's assume there is a class called 'Foo' that points to a class called 'Bar'. 'Foo' wants to use the information stored in Bar and exposes that in the form of an ObjectPtr of type 'Bar'. On initializtion 'Foo' performs some action on 'Bar's' member:

```
class NAPAPI Bar : public rtti::RTTIObject
{
	...
};

class NAPAPI Foo : public rtti::RTTIObject
{
	...
public:
	ObjectPtr<Bar> mBar;	///< Property: Link to a 'Bar' resource
};
```

We can then author these objects in json like this:

```
{
	"Objects" : 
	[
		{
			"Type" : "Foo",
			"mID"  : "MyFoo",
			"Bar"  : "MyBar"
		}

		{
			"Type" : "Bar",
			"mID" : "MyBar",
			"Value" : 10
		}
	}
}
```

Both Foo and Bar need to have their properties registered into RTTI in a similar way that we exposed properties earlier. After calling nap::ResourceManager::loadFile(), these two objects will be created and any pointers will be ‘resolved’, meaning that they will be set to the correct object. In this case, Foo::mBar will be pointing to a Bar object with ID “MyBar”. ResourceManager also makes sure that any object you are pointing to will be initialized prior to its own initialization. So, it is safe to assume that any object you are pointing to already has its init() called successfully. One thing to be careful with is that cyclic dependencies are not supported. It is not possible to point to objects that eventually point back at the original object. The system cannot determine correct initialization orders in these situations.

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

```
if (!errorState.check(loadImage(), 
	"Failed to load image %s because the dimensions are unsupported (%d:%d)", mPath.c_str(), mDimensions.x, mDimensions.y))
  return false;
 ```

The pattern is somewhat similar to the way asserts work: the first parameter is evaluated and if it evaluated to false, the error message in the parameters that follow is stored in the errorState object. Notice that multiple messages can be stacked in the errorState object. This is convenient in many situations where a very low-level message is generated, but the context where the error occurred is missing. By nesting nap::utility::ErrorState::check() calls in various functions, the context can still be provided.

Linking Media {#media}
=======================

In the [Real Time Editing](@ref editing) section we briefly touched upon linking to external files. Some objects read information from other files. Examples are: texture objects that read .PNG, .TGA files, or audio files that read .WAV or .MP3 files. The real-time editing system will reload any of these external files when a modification to them is made. For these situations we need to explicitly define the relationship to the external file in RTTI. You do this by marking a property as a file link in the RTTI properties:

```
RTTI_BEGIN_CLASS(Texture)
  RTTI_PROPERTY("Path", &Texture::mPath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

```

Scene Setup {#scene_setup}
=======================
Modern applications can grow considerably in size when it comes to the amount of data they have to manage and the complex logic the system needs to support. NAP uses a powerful [Entity Component](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system) system to aid the development process and manage your application. What separates Entities from regular Resource objects are the following properties:

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

Embedding Objects {#embedding_objects}
=======================

C++ objects are often embedded into each other. For example:

 ```
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
	Color mColorOne;		///< First Color of the palette
	Color mColorTwo;		///< Second Color of the palette
};
 ```

Here we see a Palette object, derived from nap::rtti::RTTIObject, intended to be authored in json. It contains an embedded object (a compound) of type Color. Both classes Palette and Color have their properties set up in the cpp file as required:

 ```
RTTI_BEGIN_CLASS(Color)
	RTTI_PROPERTY("r", &Color::r, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("g", &Color::g, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("b", &Color::b, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(Palette)
	RTTI_PROPERTY("ColorOne", &Palette::mColorOne, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ColorTwo", &Palette::mColorTwo, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
 ```

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

 ```
// A palette container links to a color palette
class NAPAPI PaletteContainer : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
public:
	ObjectPtr<Palette>	mColorPalette;			///< Property: Link to a color palette
};
```

In this example, PaletteContainer points to a color palette. Both PaletteContainer and the ColorPalette object will become root objects in the json file, which is sometimes messy and harder to read. For this reason, NAP supports ‘embedded pointers’. If an object logically belongs to another object, you can mark the pointer ‘embedded’, and the syntax will become similar to the way that compound objects are written. The RTTI definition in the cpp needs to change slightly:

 ```
RTTI_BEGIN_CLASS(Foo)
	RTTI_PROPERTY("ColorPalette", &PaletteContainer::mColorPalette,  nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS
```

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