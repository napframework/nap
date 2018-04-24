Getting Started {#getting_started}
=======================
* [Create a New App](@ref create_blank_app)
* [App Structure](@ref app_structure)
* [Adding Resources](@ref defining_resources)

Create a New App {#create_blank_app}
=======================
Use the NAP build system to create a new application from scratch:

- In a terminal window, navigate to the directory containing your NAP installation on disk.
- Run `tools\create_project NewProject` to generate a project file for a blank app template.

After creation your new project is located under the 'projects' folder. You can build and run it using Visual Studio on Windows, XCode on macOS or make on Linux. To learn more about setting up projects, modules and 3rd party dependencies read the [Project Management](@ref project_management) documentation.

App Structure {#app_structure}
==========================

There should be a file called 'NewProject.h'. This file contains the application that is run by NAP. Logic of the application if defined in 'NewProject.cpp'. Every NAP application is based on the 'BaseApp' template. For this section we continue by using the 'helloworldapp' as an example.

## Init

The init method is used to initialize your application. The first thing to do is find the resource manager and load a file. This file defines all the data (such as images, 3D meshes, audio files) that your application needs. After successfully loading the JSON file all objects are loaded, initialized and ready to be used. After load we ask the resource manager to find a couple of crucial objects: A camera, the world we want to render and the window that we want to render to:

~~~{cpp}
bool NewProject::init(utility::ErrorState& error)
{
    // Retrieve services
    mRenderService  = getCore().getService<nap::RenderService>();
    mSceneService   = getCore().getService<nap::SceneService>();
    mGuiService     = getCore().getService<nap::IMGuiService>();

    // Get resource manager and load all our data
    mResourceManager = getCore().getResourceManager();
    if (!mResourceManager->loadFile("helloworld.json", error))
        return false;

    // Find the window to render to
    mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

    // Find the scene and extract camera for rendering
    ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
    mCameraEntity = scene->findEntity("Camera");

    // Fetch the entity that is the world
    mWorldEntity = scene->findEntity("World");

    // All done!
    return true;
}
~~~

Take a look at the [system documentation](@ref system) to learn more about the resource manager and core.

## Update

The update method is called every frame. The parameter 'deltaTime' indicates how many seconds have passed since the last update call. You should perform any app specific logic in here that does not concern rendering. The update call is also used to forward input events and set-up any gui elements for drawing later on:

~~~{cpp}
void MyApp::update(double deltaTime)
{
    // Setup some gui elements to draw later on
    ImGui::Begin("Controls");
    ImGui::Text(utility::getCurrentDateTime().toString().c_str());
    ImGui::Text("left mouse button to rotate world, right mouse button to zoom");
    ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
    ImGui::End();
}
~~~

## Render

Render is called after update. Use this call to render objects and ui elements to screen or a different target. By default nothing is rendered. You have to tell the renderer what you want to render and where to render it to. To learn more about rendering with NAP take a look at our [render documentation](@ref rendering). The example below shows you how to render a sphere with a material to the primary window.

~~~{cpp}
void MyApp::render()
{
    // Activate current window for drawing
    mRenderWindow->makeActive();

    // Clear back-buffer
    mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

    // Find the camera
    nap::PerspCameraComponentInstance& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();

    // Find the world and add as an object to render
    std::vector<nap::RenderableComponentInstance*> render_comps;
    nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
    render_comps.emplace_back(&renderable_world);

    // Render the sphere to the main window using a perspective camera
    mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, render_comps);

    // Draw our gui last!
    mGuiService->draw();

    // Swap screen buffers
    mRenderWindow->swap();
}
~~~

Adding Resources {#defining_resources}
=======================

The data folder contains a .json file with the same name as the app. This file describes the general structure of your app and any additional resources that are required. Objects in this file can be split up into three different types: 
- Resources: static often read only data such as images, a window, meshes etc.
- Entities: hold components
- Components: add functionality to an entity and receive an update call

These objects tell our application which objects to create and how they relate to each other. Every object can carry properties. Properties are attributes that describe how an object behaves or is interpreted. Properties can be anyting such as a: float, string, array of strings, link etc. But two properties are extremely important:
- `Type`: The name of the class in code
- `mID`: The unique ID of an object

## Resources

Resources in NAP are objects that usually define input or output interfaces that the application sends or receives data to and from. Common examples are data files read from disk, such as image-, video- or audiofiles. In NAP an app window is also treated as a resource because it can be addressed as an output device to which all sorts of graphical content can be rendered. Have a look at the object of the window object with type `nap::RenderWindow` within the JSON file of the helloworld demo app as an example.
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

As you can see the window contains a `Type` and an `mID` and a set of specific properties: the window’s size, title and its background color.

## Entities and components

NAP uses an entity/component system to structure functionality within apps. This pattern has been proven its use in numerous game engines that are popular nowadays. For a detailed discription how entity/component systems work see [Scene](@ref scene). For now it is sufficient to know that entities are empty object shells with a name that can composite different types of behaviour and functionality. Also note that entities are nested objects: apart from components they can also contain  child entities. This is described in detail in [entities](@ref nap::Entity).
Functionality and behaviour can be assigned to an entity by adding components to the entity’s “Components” array. Components are objects that define one certain type of functionality or behaviour. Components, just like all objects, have a `Type` and `mID` property, but also have their own set of properties that is specific to the behavior or functionality they are adding to the entity. For example have a look at the “World” entity in the “helloworld” demo and look at the `nap::RotateComponent` component that it contains.
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

Entities with their components describe the types of objects that are available within the app. However we still have to tell what entity or which entities have to be instantiated when the app is started. This is done using one unique object within the JSON file: the scene object of type `nap::Scene`. In this object we define an array containing the mIDs of one or more of the entities that are defined within the JSON file. These are the entities that will be instantiated or “spawned” at the moment the app is started and initialized, along with all their components and child entities recursively. In other words the Scene object defines which are the “root” entities of the app. The scene of the helloworld demo looks like this:
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







