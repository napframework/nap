Resources {#resources}
=======================

*	[Overview](@ref overview)
*	[Creating Resources](@ref creating_resources)
*	[Exposing Properties](@ref exposing_properties)
*	[Pointing to Resources](@ref pointing)
* 	[Real Time Editing](@ref editing)
* 	[Linking Media](@ref media)
*	[Working With Arrays](@ref arrays)
*	[Structs, Classes and Resources](@ref structs_classes_resources)
*	[Devices](@ref devices)
* 	[Embedding Objects](@ref embedding_objects)
* 	[Embedding Pointers](@ref embedding_pointers)

Overview {#overview}
=======================

Resources are small stand-alone building blocks that can be added to your application. Resources are used to load an image from disk, define a three-dimensional shape, create a render window, etc. All resources are objects that can be authored in <a href="https://en.wikipedia.org/wiki/JSON" target="_blank">JSON</a>. In fact: all content in NAP is authored in a readable text format. NAP uses JSON because it's a well known format that is easy to edit and read. If you don't feel like typing JSON you can use Napkin (our editor) to add, remove and modify resources. Here is an example of a JSON file that exposes two resources:

```
{
	"Objects" : 
	[
		{
			"Type" : "nap::ImageFromFile",
			"mID" : "BackgroundImage",
			"ImagePath" : "background.jpg"
		},
		{
			"Type" : "nap::PlaneMesh",
			"mID": "PlaneMesh"
		}
	]
}
```

The resources mentioned above are both of a different kind and have their own set of properties. All resources in JSON have a C++ counterpart: nap::Image and nap::PlaneMesh both exist as classes in the NAP C++ codebase. Both objects are created and initialized when this file is loaded. After creation the C++ properties will match the properties defined in JSON.

Creating Resources {#creating_resources}
=======================

To make sure that C++ classes and properties can be created and edited in JSON you need to expose them explicitly through something called <a href="https://en.wikipedia.org/wiki/Run-time_type_information" target="_blank">RTTI</a> (better known as RunTime Type Information). NAP uses a number of macros to ease the way you can expose classes and properties. To create a resource that can be authored in JSON you need to derive if from [Resource](@ref nap::Resource) and tell the system what the parent object is. To accomplish this you add to the class declaration the `RTTI_ENABLE` macro:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Shader : public Resource
{
	RTTI_ENABLE(Resource)
};
~~~~~~~~~~~~~~~

And in the .cpp file, the following macros are added:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(nap::Shader)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

This is the basis for setting up an RTTI-enabled class. In the example above we defined a shader. The `RTTI_ENABLE` macro makes sure that the system knows this class is derived from a Resource. The input for the `RTTI_ENABLE` macro is always the parent object. The code in the .cpp file exposes this class as a resource to NAP. The shader is now available as an object that can be authored in JSON.

The `NAPAPI` macro makes sure that the class can be read and accessed (from the outside world) by the NAP system. The compiler (on Windows) won't expose the class to the outside world when you forget to put in the `NAPAPI` macro. Remember that on startup a NAP application (automatically) loads all modules. NAP tries to find all the exposed resources that are compatible with the NAP system when loading a module. When it encounters a resource in a JSON file that is not available to the system the resource manager will raise a warning and stop execution. It is therefore important to always expose a class to the outside world using the `NAPAPI` macro.

Exposing Properties {#exposing_properties} 
=======================

Properties belong to a resource. You can think of properties as attributes and add as many as you want. To extend a resource with properties you add a property field to your class. In the example below we add two properties: 'VertPath' and 'FragPath':

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Shader : public Resource
{
	RTTI_ENABLE(Resource)
	public:
		std::string	mVertPath;				// Property: Path to the vertex shader on disk
		std::string mFragPath;				// Property: Path to the fragment shader on disk
};
~~~~~~~~~~~~~~~

And extend the RTTI macro in the .cpp file with information about our properties:

~~~~~~~~~~~~~~~{.cpp}
// Adding serialzable properties
RTTI_BEGIN_CLASS(nap::Shader)
  RTTI_PROPERTY("VertPath", &nap::Shader::mFragPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
  RTTI_PROPERTY("FragPath", &nap::Shader::mVertPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

 These properties are important. Both properties point to a file on disk that contain GLSL code that is compiled when the shader is initialized. Both properties wrap a class member: 'mFragPath' and 'mVertPath'. These are exposed to the system using the `RTTI_PROPERTY` macro. The [EPropertyMetaData](@ref nap::rtti::EPropertyMetaData) enum provides additional information about properties. In this example the vertex and fragment shader paths are required. When they remain empty in JSON the resource manager won't be able to load the shader. NAP exposes the following types of properties:

- Default  : will use the class default if the property was not set
- Required : loading the JSON file will fail if the property was not set
- FileLink : the property defines a relationship with an [external file](@ref media) (.bmp, .wav etc.)
- Embedded : the property is an [embedded pointer](@ref embedding_pointers)

After defining the shader together with its properties, the shader can be authored in JSON:

```
{
	"Type" : "nap::Shader",
	"mID" : "FogShader",
	"VertPath" : "shaders/fogshader.vert"
	"FragPath" : "shaders/fogshader.frag"
}
```

The resource identifier (`mID`) can be anything you like. The identifier is used to retrieve a resource in your application and to refer to this object by other objects in JSON. More things are possible with the RTTI system. For instance: it has support for constructors with one or more arguments and it can also expose C++ enums in a way that is still readable in JSON. See typeinfo.h or the reference documentation for more detailed information on how to do this.

Pointing to Resources {#pointing}
=======================

It is often useful if a resource can access information from another resource. NAP allows you to create links between objects (in JSON) to accomplish just that. A resource can point to other resources in JSON by referring to the name (identifier) of a resource. In C++, we use a specific type of pointer to accomplish this: a [resource pointer](@ref nap::ResourcePtr). Let's assume there is a resource called 'Material' that points to a 'Shader'. The material wants to use the information stored in the shader and exposes that in the form of a link to a shader. The material can now access the shader without having to worry about order of initialization. When the material is initialized the shader has been created and validated. You only have to implement the logic you want to perform based on the information that is present in the shader:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Material : public Resource
{
	RTTI_ENABLE(Resource)
public:
	ResourcePtr<nap::Shader> mShader;	// Property: Link to a 'Shader' resource
};
~~~~~~~~~~~~~~~

In the header the material exposes (as a member) a link to a shader in the form of an object pointer:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(nap::Material)
	RTTI_PROPERTY("Shader",	&nap::Material::mShader, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

In the .cpp file we register the material as a resource and add the link to a shader as a property of the material. This is similar to how we just defined the shader and its properties. We can now author these two objects in JSON. Note that the material points to the shader by its name: 'FogShader'

```
{
	"Objects" : 
	[
		{
			"Type" : "nap::Shader",
			"mID" : "FogShader",
			"VertPath" : "shaders/fogshader.vert"
			"FragPath" : "shaders/fogshader.frag"
		}

		{
			"Type" : "nap::Material",
			"mID"  : "FogMaterial",
			"Shader"  : "FogShader"
		}
	}
}
```

The material is now registered and exposes (as as property) the link to a shader. After calling [loadfile()](@ref nap::ResourceManager::loadFile), these two objects will be created and the pointers will be ‘resolved’. This means that they will point to the right resource. In this case the material points to a 'FogShader'. The resource manager also makes sure that the shader is initialized before the material. Cyclic dependencies are unfortunately not (yet) supported. It is not possible to point to a resource that eventually points back at the original resource. The system cannot determine the correct order of initialization in these situations.

Real Time Editing {#editing}
=======================

NAP contains a powerful ‘real-time editing’ engine. This means that it is possible to make modifications to the JSON file while the application is running. This results in extremely fast iteration times. NAP watches the JSON file that is loaded, including any external files that are referenced (such as textures or audio files). When changes to these files are detected they are instantly hot-loaded into the system. All it takes from a user perspective is to press 'save'.

From a programmer perspective there are some rules to adhere to. Each resource follows the same initialization pattern. When a file is loaded, all objects are initialized in the correct order. When an init() call returns false it means that resource could not be initialized correctly. In that case, the entire load is cancelled and the system returns back to the original state before load was called. The returned error message describes what went wrong during (re)loading. It is guaranteed that all the objects remain in their original state until the initialization sequence is completed successfully. When there is no original state (ie, the file is loaded for the first time) and initialization fails the application will exit immediately.

Here are the rules for writing a correct [init](@ref nap::rtti::Object::init) function:
- Return true on success, false on failure
- Only assert (or halt program execution in any other way) on programmer errors, never on user errors
- If the function fails make sure that a clear error message is presented to the [ErrorState](@ref nap::utility::ErrorState) object that is passed to init().
- Make sure that the init() function does not have any side effects. This means that it should not update any global state. The system cannot revert such changes in case of an error.

[ErrorState](@ref nap::utility::ErrorState) is a class that offers a convenient way to report an error to a user. The general pattern is as follows:

~~~~~~~~~~~~~~~{.cpp}
if (!errorState.check(loadImage(), "Failed to load image %s, dimensions are unsupported (%d:%d)", mPath.c_str(), mDimensions.x, mDimensions.y))
  return false;
~~~~~~~~~~~~~~~

The pattern is somewhat similar to the way asserts work: the first parameter is evaluated. When it evaluates to false the error message is stored in the `ErrorState` object. Notice that multiple messages can be stacked in the error object. This is convenient in many situations where a very low-level message is generated, but the context where the error occurred is missing. By nesting [check()](@ref nap::utility::ErrorState::check) calls in various functions the context can still be provided.

Linking Media {#media}
=======================
In the [real time editing](@ref editing) section we briefly touched upon linking to external files. Some objects read information from other files. Examples include a texture resource that reads (among others) .png files or an audio player that reads .wav or .mp3 files. The real-time editing system will reload any of these external files when a modification to them is made. For this to work you need to tell the system if a property is a link to a file. You do this by marking a property of a resource as a file link:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Texture)
  RTTI_PROPERTY("Path", &Texture::mPath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

~~~~~~~~~~~~~~~

Working With Arrays {#arrays}
=======================

You can group items together into an array in JSON. This works for both links to objects as for simple compounds (structs) such as a [color](@ref nap::RGBColor). The video modulation demo uses two arrays: one to create a group of videos and another to create a group of meshes. In the application the user can select a video or mesh based on an index. 

Creating an array is easy. In C++ an array is a regular vector. This vector becomes a property of the resource, similar to how you normally expose a property. In the example below we add a member called 'mVideoFiles'. This is an array of video files that the user can choose from. Every video in the array is a link to an existing [video](@ref nap::Video) resource:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI VideoContainer : public rtti::RTTIObject
{
	RTTI_ENABLE(rtti::RTTIObject)
public:
	std::vector<ResourcePtr<Video>> mVideoFiles;		///< Property: "Videos" link to videos
}
~~~~~~~~~~~~~~~

and in the .cpp file we register the vector as a regular property:

~~~~~~~~~~~~~~~{.cpp}
// nap::VideoContainer 
RTTI_BEGIN_CLASS(nap::VideoContainer)
	RTTI_PROPERTY("Videos", &nap::VideoContainer::mVideoFiles, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

The system recognizes that the property is an array of video links. You can now author the array in JSON:

```
{
	"Type": "nap::Video",
    "mID": "SnowVideo",
    "Path": "snow.mp4"
},
{
	"Type": "nap::Video",
	"mID": "StreakVideo",
	"Path": "streak.mp4"
},
{
	"Type": "nap::VideoContainer",
	"mID": "VideoContainer",
	"Videos": 
	[
		"StreakVideo",
        "SnowVideo"
	]
}
```

The example above is a simplification of the classes used in the video modulation demo.

Structs, Classes and Resources {#structs_classes_resources}
=======================
Up to this point we only worked with resources. Resources are classes. But sometimes you want to use a simple struct to define (for example) a color or date. NAP supports structs in JSON but they have to be declared in a different way. Let's create a simple RGB color in C++:

~~~~~~~~~~~~~~~{.cpp}
 // RGB Color
class NAPAPI Color
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
};
~~~~~~~~~~~~~~~

As you can see the color isn't a native C++ struct but a class. This is because the RTTI system does not support C++ structs. What we do instead is label it as a struct in the .cpp file:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_STRUCT(Color)
	RTTI_PROPERTY("r", &Color::r, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("g", &Color::g, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("b", &Color::b, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT
~~~~~~~~~~~~~~~

So what are the advantages of using a struct? A struct is a light weight object that can be copied by value. When the system encounters a struct it is created using copy construction instead of being new'd. This is faster when the object is small and allows the editor to add and remove simple structures from an array. So in short, when you have a simple object that is: 

- Used to group some data under a common name
- Easy to copy
- Not a resource
- Necessary to edit in JSON ..

.. define it as a struct. 

In all other cases: define it as a class. But what is the difference between a class and resource? A resource is a class that can live by itself in JSON. As you know every resource carries an identifier and is initialized by NAP after construction. A resource is always derived from [Resource](@ref nap::Resource). On the other hand, structs and classes that are not derived from Resource can't be declared in JSON as a resource because they don't have a name and can't be initialized. You can only use those objects in JSON as an [embedded object](@ref embedding_objects). Only when a class is derived from a Resource is it considered as a resource to the system.

This might sound confusing but try to follow these rules: Do I need to author (edit) my class in JSON?
- Yes
	-  Is it a resource?
		- Yes
			- Derive your class from [Resource](@ref nap::Resource) (.h)
			- Or any object that is derived from [Resource](@ref nap::Resource) (.h)
			- Always implement the `RTTI_ENABLE` macro (.h)
			- Define it as a class (.cpp)
		- No
			- Is it a simple structure?
				- Yes
					- Don't implement the `RTTI_ENABLE` macro (.h)
					- Define it as a struct (.cpp)
				- No
					- Implement the `RTTI_ENABLE` macro (.h)
					- Leave the `RTTI_ENABLE` macro empty if there is no parent class (.h)
					- Define it as a class (.cpp)
- No
	- Don't expose it to the system
	- Declare it as a regular C++ class

This diagram doesn't take [components](@ref nap::Component) into account. You can read more about components in a later [section](@ref creating_components).

Devices {#devices}
=======================

A [device](@ref nap::Device) is a special type of resource. You can think of a device as a class that represents and manages the connection to an external piece of hardware (such as a DAC) or a computer. Every device has a [start()](@ref nap::Device::start) and [stop()](@ref nap::Device::stop) method that you can override. Both methods are called by the resource manager at the appropiate time, ie: when the device is created, has changed or is removed from the resource tree. The resource manager 'stops' a device that is running before it is destroyed. 

NAP ships with a couple of devices such as the [OSCReceiver](@ref nap::OSCReceiver), [OSCSender](@ref nap::OSCSender), [ArtnetController](@ref nap::ArtNetController), [EtherDreamDac](@ref nap::EtherDreamDac) etc.

Embedding Objects {#embedding_objects}
=======================

C++ objects are often embedded into each other. For example:

~~~~~~~~~~~~~~~{.cpp}
 // RGB Color
class NAPAPI Color
{
	...
};

// Palette that contains two colors
class NAPAPI Palette : public Resource
{
	RTTI_ENABLE(Resource)
public:
	Color mColorOne;		// First Color of the palette
	Color mColorTwo;		// Second Color of the palette
};
~~~~~~~~~~~~~~~

Here we see a 'Palette' that contains two colors. Wouldn't it be nice to assign both colors to the palette directly in JSON? As we saw [before](@ref structs_classes_resources), a 'Color' is a struct, not a resource. We can't create a single color in JSON. But NAP can create and assign registered structs (and classes) on the fly when it encounters them in the file. We call these objects 'embedded objects', or 'compounds'. For this to work it's important that both objects have their properties registered in the .cpp file:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_STRUCT(Color)
	RTTI_PROPERTY("r", &Color::r, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("g", &Color::g, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("b", &Color::b, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(Palette)
	RTTI_PROPERTY("ColorOne", &Palette::mColorOne, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ColorTwo", &Palette::mColorTwo, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT
~~~~~~~~~~~~~~~

In JSON this selection can now be authored as follows:

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


When creating the palette the system finds the two colors and assigns them on the spot. Notice that Color isn't derived from `RTTIObject`. It doesn't need to because it's not intended to be a resource. The system does however need to know the properties of a color: in this case the three color channels. You can however embed any object, including objects derived from `RTTIObject`. It's perfectly valid to embed an image into another resource directly.

Embedding Pointers {#embedding_pointers}
=======================

Embedded objects have the benefit that their notation in JSON is rather compact. But often pointers are used to link to other objects. For example:

~~~~~~~~~~~~~~~{.cpp}
// A palette container links to a color palette
class NAPAPI PaletteContainer : public Resource
{
	RTTI_ENABLE(Resource)
public:
	ResourcePtr<Palette>	mColorPalette;			// Property: Link to a color palette
};
~~~~~~~~~~~~~~~

In this example, PaletteContainer points to a color palette. Both the container and the color palette are root objects in the JSON file, which is sometimes messy and harder to read. For this reason, NAP supports ‘embedded pointers’. If an object logically belongs to another object, you can mark the pointer ‘embedded’, and the syntax will become similar to the way that compound objects are written. The RTTI definition in the .cpp needs to change slightly:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Foo)
	RTTI_PROPERTY("ColorPalette", &PaletteContainer::mColorPalette, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

We can write this in JSON as follows:

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

Note that the embedded object is still a regular object that lives inside the [resource manager](@ref nap::ResourceManager). The identifier can be omitted in JSON, in that case an identifier will be generated for you. If you declare an identifier you can still find it by that name in the resource manager.