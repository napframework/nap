Getting started {#getting_started}
=======================
* [Create a blank app](@ref create_blank_app)
* [Defining app logic](@ref app_logic)
* [Defining resources and setting up a scene](@ref defining_resources)

Create a blank app {#create_blank_app}
=======================
First let's use the NAP build system to create and run a blank app with one empty window from scratch.

1. In a terminal window, navigate to the directory containing your NAP installation on disk.
2. Run `tools\createProject NewProjectName` to generate a project file for a blank app template.

The freshly generated project file for your platform will be shown in the file explorer/finder for your OS. You can build and run it using Visual Studio on Windows, XCode on OSX or make on linux.

This is described in more detail in the section [Project management](@ref proj_management).

Defining app logic {#app_logic}
==========================

The starting point to write the client code defining our application is a specific subclass of the App baseclass. For an example have a look at “helloworldapp.h” and “helloworldapp.cpp” in the helloworld demo’s “src” folder. The HelloWorldApp class overrides certain virtual methods that implement app specific logic.

## The init method

The app’s init method is first of all used to load the JSON file and initialize all the objects within the scene. This is performed by an object called ResourceManager that lives inside the nap Core object. The Core object contains the complete NAP system. More information about the Core and the ResourcaManager can be found.
The init method is also used to intialize some of the app class’ members. Some of these members are pointers to Services. Services are objects tha cooperate with components to provide them access to certain system devices or hardware funcitonality. More about Services can be read in [Modules & Services](@ref modules_services). As can be seen in helloworldapp.cpp pointers to the resourcemanager and services can be retrieved from the core object.

## The update method

The app's update method is called periodically at the current frame rate. It's parameter deltaTime indicates how many seconds have passed since the last update call. Usually it is used to forward input events received from input devices like mouse and keyboard to entities that contain input event handling components. Examples of event handling components are KeyInputComponent for keyboard events and PointerInputComponent for mouse events. Have a look at how this is done in helloworldapp.cpp:

~~~{cpp}
// The default input router forwards messages to key and mouse input components
// attached to a set of entities.
nap::DefaultInputRouter input_router;

// Forward all input events associated with the first window to the listening components
std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
mInputService->processEvents(*mRenderWindow, input_router, entities);
~~~

## Immediate GUI

NAP uses the powerful ImGUI thirdparty library as a easy to use and effective toolkit to build graphical user interfaces with buttons, sliders, text, etc. ImGUI is an immediate GUI framework. Immediate GUI means that the whole GUI rendered to the screen at framerate, which basically means no extra code is needed to update the status of the GUI with the status of the app. Every GUI element can be displayed and it's state being read in a single line of code. Look at the following example:

~~~{cpp}
ImGui::Begin("This is a GUI");
ImGui::SliderFloat("Drag me!", &mSliderValue, 0, 1.0, "%.3f", 1);
if (ImGui::Button("Click me!"))
    doSomething();
ImGui::End();
~~~

IIf we create a member variable called mSliderValue and put these simple lines of code in our app's update() method we will see a slider with a range from 0 to 1 that controls the variable. If we create a method called doSomethin() this method will be called whenever the button is clicked.

Defining resources and setting up a scene {#defining_resources}
=======================

The data folder contains one JSON file with the same name as the app that is vital because it describes the general structure of your application. The JSON file defines an array of objects that can be split up in three different kinds: resources, entities and one scene object. These objects together tell our application what files to load in memory, which objects to create at runtime and how they relate to each other.
The elements nested in the different objects in the JSON file are used to specify values for their  properties. Properties can have all sorts of types like primitive data types, arrays, nested objects and also pointers to other objects that are defined within the JSON file. Every object of any type always has two properties:
- the property “Type” that indicates the type of the object we are dealing with and corresponds to the name of the class of the object in the source codes.
- the property “mID” that can be used to assign a unique ID to an object. The mID property among other things is used within the JSON file to assign pointer properties to point to the object in question.

## Resources

Resources in NAP are objects that usually define input or output interfaces that the application sends or receives data to and from. Common examples are data files read from disk, such as image-, video- or audiofiles. In NAP an app window is also treated as a resource because it can be addressed as an output device to which all sorts of graphical content can be rendered. Have a look at the object of the window object with type  “nap::RenderWindow” within the JSON file of the helloworld demo app as an example.
```
{
    "Type" : "nap::RenderWindow",
    "mID" : "Window0",
    "Width" : 1280,
    "Height" : 720,
    "Title" : "Window 1",
    "ClearColor": {
        "x": 0.0666,
        "y": 0.0784,
        "z": 0.1490,
        "w": 1.0
    }
}
```

As you can see the window contains a Type and an mID and a set of specific properties: the window’s size, title and it’s background color.

## Entities and components

NAP uses an entity/component system to structure functionality within apps. This pattern has been proven its use in numerous game engines that are popular nowadays. For a detailed discription how entity/component systems work see [Scene](@ref scene). For now it is sufficient to know that entities are empty object shells with a name that can composite different types of behaviour and functionality. Also note that entities are nested objects: apart from components they can also contain  child entities. This is described in detail in [entities](@ref nap::Entity).
Functionality and behaviour can be assigned to an entity by adding components to the entity’s “Components” array. Components are objects that define one certain type of functionality or behaviour. Components, just like all objects, have a “Type” and “mID” property, but also have their own set of properties that is specific to the behavior or functionality they are adding to the entity. For example have a look at the “World” entity in the “helloworld” demo and look at the “nap::RotateComponent” component that it contains.
```
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
        "Speed": 0.025,
        "Offset": 0.65
    }
}
```
The RotateComponent takes care of slowly rotating the world entity. It specifies the axis on which to rotate the component and the rotation speed and the offset of the first rotation cycle.


## The scene

Entities with their components describe the types of objects that are available within the app. However we still have to tell what entity or which entities have to be instantiated when the app is started. This is done using one unique object within the JSON file: the scene object of type “nap::Scene”. In this object we define an array containing the mIDs of one or more of the entities that are defined within the JSON file. These are the entities that will be instantiated or “spawned” at the moment the app is started and initialized, along with all their components and child entities recursively. In other words the Scene object defines which are the “root” entities of the app. The scene of the helloworld demo looks like this:
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
}
```
As we can see, the demo contains a rotating world entity and a camera that is pointed to this world and that is used to render it to the window resource.







