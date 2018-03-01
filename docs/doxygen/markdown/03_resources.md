Resources {#resources}
=======================

*	[Overview](@ref overview)
*	[Creating Resources](@ref creating_resources)
*	[Exposing Properties](@ref exposing_properties)
*	[Pointing to Resources](@ref pointing)
* 	[Real Time Editing](@ref editing)
* 	[Linking Media](@ref media)
*	[Working With Arrays](@ref arrays)
* 	[Embedding Objects](@ref embedding_objects)
* 	[Embedding Pointers](@ref embedding_pointers)

Overview {#overview}
=======================

Resources are small stand-alone building blocks that can be added to your application. Resources are used to load an image from disk, define a 3 dimensional shape, create a render window etc. All resources are objects that can be authored in json. In fact: all content in NAP is authored in a readable text format. NAP uses ‘json’ becuase its a well known format that is easy to edit and read. If you don't feel like typing json you can use Napkin (our editor) to add, remove and modify resources. Here is an example of a json file that exposes 2 resources:

```
{
	"Objects" : 
	[
		{
			"Type" : "nap::Image",
			"mID" : "BackgroundImage",
			"ImagePath" : "data/test/background.jpg"
		},
		{
			"Type" : "nap::PlaneMesh",
			"mID": "PlaneMesh"
		}
	]
}
```

The resources mentioned above are both of a different kind and have their own set of properties. All resources in json have a C++ counterpart: nap::Image and nap::PlaneMesh both exist as classes in the NAP C++ codebase. Both objects are created and initialized when this file is loaded. After creation the C++ properties will match the properties defined in json.

Creating Resources {#creating_resources}
=======================

To make sure that C++ classes and properties can be created and edited in json you need to expose them explicitly through something called [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information) (better known as RunTime Type Information). NAP uses a number of macros to ease the way you can expose classes and properties. To create a resource that can be authored in json you need to derive if from [RTTIObject](@ref nap::rtti::RTTIObject) and tell the system what the parent object is. To accomplish this you add to the class declaration the RTTI_ENABLE macro:

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

This is the basis for setting up an rtti enabled class. In the example above we defined a shader. The RTTI_ENABLE macro makes sure that the system knows this class is derived from RTTIObject. The code in the cpp file exposes this class as a resource to NAP. The shader is now available as an object that can be authored in json.

The NAPAPI macro makes sure that the class can be read and accessed (from the outside world) by the NAP system. The compiler (on windows) won't expose the class to the outside world when you forget to put in the NAPAPI macro. Remember that on startup a NAP application (automatically) loads all modules. NAP tries to find all the exposed resources that are compatible with the NAP system when loading a module. When it encounters a resource in a json file that is not available to the system the resource manager will raise a warning and stop execution. It is therefore important to always expose a class to the outside world using the NAPAPI macro.

Exposing Properties {#exposing_properties} 
=======================

Properties belong to a resource. You can think of properties as attributes and add as many as you want. To extend a resource with properties you add a property field to your class. In the example below we add two properties: 'VertPath' and 'FragPath':

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

 These properties are important. Both properties point to a file on disk that contain glsl code that is compiled when the shader is initialized. Both properties wrap a class member: 'mFragPath' and 'mVertPath'. These are exposed to the system using the RTTI_PROPERTY macro. The [EPropertyMetaData](@ref nap::rtti::EPropertyMetaData) enum provides additional information about properties. In this example the vertex and fragment shader paths are required. When they remain empty in json the resource manager won't be able to load the shader. NAP exposes the following types of properties:

- Default  : will use the class default if the property was not set
- Required : the load of the json file will fail if the property was not set
- FileLink : the property defines a relationship with an [external file](@ref media)(.bmp, .wav etc.)
- Embedded : the property is an [embedded pointer](@ref embedding_pointers)

After defining the shader together with its properties, the shader can be authored in json:

```
{
	"Type" : "nap::Shader",
	"mID" : "FogShader",
	"VertPath" : "/data/myproject/shaders/fogshader.vert"
	"FragPath" : "/data/myproject/shaders/fogshader.frag"
}
```

The resource identifier ('mID') can be anything you like. The identifier is used to retrieve a resource in your application and to refer to this object by other objects in json. More things are possible with the RTTI system. For instance: it has support for constructors with one or more arguments and it can also expose C++ enums in a way that is still readable in json. See typeinfo.h or the reference documentation for more detailed information on how to do this.

Pointing to Resources {#pointing}
=======================

It is often useful if a resource can access information from another resource. NAP allows you to create links between objects (in json) to accomplish just that. A resource can point to other resources in json by referring to the name (identifier) of a resource. In C++, we use a specific type of pointer to accomplish this: [object pointer](@ref nap::ObjectPtr). Let's assume there is a resource called 'Material' that points to a 'Shader'. The material wants to use the information stored in the shader and exposes that in the form of a link to a shader. The material can now access the shader without having to worry about order of initialization. When the material is initialized the shader has been created and validated. You only have to implement the logic you want to perform based on the information that is present in the shader:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI Material : public rtti::RTTIObject
{
	...
public:
	ObjectPtr<nap::Shader> mShader;	// Property: Link to a 'Shader' resource
};
~~~~~~~~~~~~~~~

In the header the material exposes (as a member) a link to a shader in the form of an object pointer

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(nap::Material)
	RTTI_PROPERTY("Shader",	&nap::Material::mShader, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

In the cpp file we register the material as a resource and add the link to a shader as a property of the material. This is similar to how we just defined the shader and its properties. We can now author these two objects in json. Note that the material points to the shader by its name: 'FogShader'

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

The material is now registered and exposes (as as property) the link to a shader. After calling [loadfile()](@ref nap::ResourceManager::loadFile), these two objects will be created and the pointers will be ‘resolved’. This means that they will point to the right resource. In this case the material points to a 'FogShader'. The resource manager also makes sure that the shader is initialized before the material. Cyclic dependencies are unfortunately (not yet) supported. It is not possible to point to a resource that eventually points back at the original resource. The system cannot determine the correct order of initialization in these situations.

Real Time Editing {#editing}
=======================

NAP contains a powerful ‘real-time editing’ engine. This means that it is possible to make modifications to the json file while the application is running. This results in extremely fast iteration times. NAP watches the json file that is loaded, including any external files that are referenced (such as textures or audio files). When changes to these files are detected they are instantly hot-loaded into the system. All it takes from a user perspective is to press 'save'.

From a programmer perspective there are some rules to adhere to. Each resource follows the same initialization pattern. When a file is loaded, all objects are initialized in the correct order. When an init() call returns false it means that resource could not be initialized correctly. In that case, the entire load is cancelled and the system returns back to the original state before load was called. The returned error message describes what went wrong during (re)loading. It is guaranteed that all the objects remain in their original state until the initialization sequence is completed successfully. When there is no original state (ie, the file is loaded for the first time) and initialization fails the application will exit immediately.

Here are the rules for writing a correct [init](@ref nap::RTTIObject::init) function:
- Return true on success, false on failure
- Only assert (or halt program execution in any other way) on programmer errors, never on user errors
- If the function fails make sure that a clear error message is presented to [error](@ref nap::utility::ErrorState) object that is passed to init().
- Make sure that the init() function does not have any side effects. This means that it should not update any global state. The system cannot revert such changes in case of an error.

nap::utility::ErrorState is a class that offers a convenient way to report an error to a user. The general pattern is as follows:

~~~~~~~~~~~~~~~{.cpp}
if (!errorState.check(loadImage(), "Failed to load image %s, dimensions are unsupported (%d:%d)", mPath.c_str(), mDimensions.x, mDimensions.y))
  return false;
~~~~~~~~~~~~~~~

The pattern is somewhat similar to the way asserts work: the first parameter is evaluated. When it evaluates to false the error message is stored in the error state object. Notice that multiple messages can be stacked in the error object. This is convenient in many situations where a very low-level message is generated, but the context where the error occurred is missing. By nesting [check()](@ref nap::utility::ErrorState::check) calls in various functions the context can still be provided.

Linking Media {#media}
=======================
In the [real time editing](@ref editing) section we briefly touched upon linking to external files. Some objects read information from other files. Examples include a texture resource that reads (among others) .png files or an audio player that reads .wav or .mp3 files. The real-time editing system will reload any of these external files when a modification to them is made. For these situations we need to explicitly define the relationship to the external file in RTTI. You do this by marking a property as a file link in the RTTI properties:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Texture)
  RTTI_PROPERTY("Path", &Texture::mPath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

~~~~~~~~~~~~~~~

Working With Arrays {#arrays}
=======================

TODO

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

Here we see a Palette object derived from RTTIObject this is intended to be authored in json. It contains an embedded object (a compound) of type Color. Both Palette and Color have their properties setup (in the cpp file) as required:

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

In json this selection can now be authored as follows:

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

Notice that Color isn't derived from RTTIObject. It doesn't need to be because it doesn't have an identifier! It's also not a resource that can (or needs to) live by itself. That strippes away the need for the RTTI_ENABLE macro. The system doesn't need to know what it is derived from because it's always embedded into another object as a property. The system does (however) need to know the properties it uses, in this case the 3 channels of the color. You can (however) embed any object, including objects derived from RTTIObject. It's perfectly valid to embed an image into another resource directly.

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

In this example, PaletteContainer points to a color palette. Both the container and the color palette are root objects in the json file, which is sometimes messy and harder to read. For this reason, NAP supports ‘embedded pointers’. If an object logically belongs to another object, you can mark the pointer ‘embedded’, and the syntax will become similar to the way that compound objects are written. The RTTI definition in the cpp needs to change slightly:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Foo)
	RTTI_PROPERTY("ColorPalette", &PaletteContainer::mColorPalette,  nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

We can write this in json as follows:

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

Note that the embedded object is still a regular object that lives inside the [resource manager](@ref nap::ResourceManager). The identifier can be omitted in json, in that case an identifier will be generated for you. If you declare an identifier you can still find it by that name in the resource manager.