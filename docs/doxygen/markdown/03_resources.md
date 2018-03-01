Resources {#resources}
=======================

*	[Overview](@ref overview)
*	[Exposing Resources](@ref exposing_resources)
*	[Exposing Properties](@ref exposing_properties)
*	[Pointing to Resources](@ref pointing)
* 	[Real Time Editing](@ref editing)
* 	[Linking Media](@ref media)
* 	[Embedding](@ref embedding_objects)
	* 	[Embedding Objects](@ref embedding_objects)
	* 	[Embedding Pointers](@ref embedding_pointers)

Overview {#overview}
=======================

All content in NAP is authored in a readable text format. NAP uses ‘json’ to store all the content of an application in a text file. Here is an example:
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

Exposing Resources {#exposing_resources}
=======================

To make sure that C++ classes and properties can be created and edited in json you need to expose them explicitly through something called RTTI (better known as [RunTime Type Information](https://en.wikipedia.org/wiki/Run-time_type_information)). NAP uses a number of macros to ease the way you can expose classes and properties. To create an getObject that can be authored in json you need to derive if from [RTTIObject](@ref nap::rtti::RTTIObject) and tell the system what the parent object is. To accomplish this you add, in the class declaration, the RTTI_ENABLE macro:

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

The ID can be chosen as the user wishes. It can be used to retrieve the getObject from code using [findObject()](@ref nap::ResourceManager::findObject()), but it can also be used to refer to this object by other objects in json, as we will see later. More things are possible with the RTTI system. For instance, it has support for constructors with one or more arguments and it can also expose C++ enums in a way that is still readable in json. See typeinfo.h or the reference documentation for more detailed information how to do this.

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

The material is now registered and (exposes as as property) the link to a shader. After calling nap::ResourceManager::loadFile(), these two objects will be created and the pointers will be ‘resolved’, meaning that they will be set to the correct object. In this case, Material::mShader points to a 'FogShader'. ResourceManager also makes sure that any getObject you are pointing to will be initialized prior to its own initialization. So, it is safe to assume that any getObject you are pointing to already has its init() called successfully. One thing to be careful with is that cyclic dependencies are not supported. It is not possible to point to objects that eventually point back at the original getObject. The system cannot determine correct initialization orders in these situations.

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

The pattern is somewhat similar to the way asserts work: the first parameter is evaluated and if it evaluated to false, the error message in the parameters that follow is stored in the errorState object. Notice that multiple messages can be stacked in the errorState getObject. This is convenient in many situations where a very low-level message is generated, but the context where the error occurred is missing. By nesting nap::utility::ErrorState::check() calls in various functions, the context can still be provided.

Linking Media {#media}
=======================

In the [Real Time Editing](@ref editing) section we briefly touched upon linking to external files. Some objects read information from other files. Examples are: texture objects that read .PNG, .TGA files, or audio files that read .WAV or .MP3 files. The real-time editing system will reload any of these external files when a modification to them is made. For these situations we need to explicitly define the relationship to the external file in RTTI. You do this by marking a property as a file link in the RTTI properties:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(Texture)
  RTTI_PROPERTY("Path", &Texture::mPath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

~~~~~~~~~~~~~~~

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

Here we see a Palette object, derived from nap::rtti::RTTIObject, intended to be authored in json. It contains an embedded getObject (a compound) of type Color. Both classes Palette and Color have their properties set up in the cpp file as required:

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

Notice that Color is not derived from RTTIObject. It does not need to as it does not require an ID, nor is it an getObject that lives by itself. Therefore it also does not need the RTTI_ENABLE macro: the system does not need to know what it is derived from, it is embedded in another object. The system does need to know the properties that it is using. It does not do any harm either to derive from [RTTIObject](@ref nap::rtti::RTTIObject), so if you have existing objects that you want to embed, you can.

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

In this example, PaletteContainer points to a color palette. Both PaletteContainer and the ColorPalette getObject will become root objects in the json file, which is sometimes messy and harder to read. For this reason, NAP supports ‘embedded pointers’. If an object logically belongs to another object, you can mark the pointer ‘embedded’, and the syntax will become similar to the way that compound objects are written. The RTTI definition in the cpp needs to change slightly:

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