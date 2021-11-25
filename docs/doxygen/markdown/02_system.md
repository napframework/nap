System {#system}
=======================

*	[Overview](@ref system_overview)
*	[Modules & Services](@ref modules_services)
*   [Apps](@ref apps)
*   [Core](@ref core)
*	[Resourcemanager](@ref resourcemanager)
*	[Events](@ref events)
*	[Configuration](@ref service_config)

Overview {#system_overview}
=======================

NAP enables you to connect and exchange data between various types of external hardware in a generic fashion. The system is designed to make it easy to re-use specific parts or components for future projects and keep app specific code local to your project. The underlying system provides you with all the handles to get up and running in no time. But it's important to understand what parts contribute to the overall system architecture. Below you see a dumbed-down schematic of an application built with NAP. This schematic shows some of the key components of the NAP system architecture:

![](@ref content/nap_overview.png)

Let's start reading the graph left to right. Starting from the left we see an application runner that combines three objects, of which two are important: The Application and Core. Applications are the entry point for project specific code. This is where you define what parts of your application..
- Receive an update call 
- Are rendered
- Receive messages
- Etc.

[Core](@ref nap::Core) is the heart of every NAP application and manages (among other things) modules. Core is also the gateway to the [ResourceManager](@ref nap::ResourceManager). Every NAP application requires a Core object. That's the reason you explicitly create one and give it to the [AppRunner](@ref nap::AppRunner) that runs your application. When creating Core you also create a ResourceManager. The resource manager does a lot of things but most importantly: it makes your life easy. It creates all the objects that are associated with your application, initializes them in the right order and keeps track of any content changes. When a change is detected, the resource manager automatically patches the system without having to re-compile your application. The initialization call of your application is the perfect place to load the file and check for content errors.

Modules are libraries that expose building blocks. You can use these building blocks to construct your application. Most modules expose specific building blocks, for example. The OSC module exposes OSC receiving and sending objects, a generic interface to create and extract OSC events and a service that deals with the OSC library and network. Core loads all available modules automatically and initializes them in the right order. After a module is loaded all the building blocks are registered and the module can be initialized. You, as a user, don't have to do anything.

The diagram has four resources from three different modules:
- One [OSCReceiver](@ref nap::OSCReceiver) from the OSC Module
- Two [Windows](@ref nap::RenderWindow) from the Render Module
- One [MidiSender](@ref nap::MidiOutputPort) from the Midi Module

After initializing core (and therefore all modules) the building blocks can be created by the ResourceManager. We add the building blocks as individual resources to our JSON file and tell the ResourceManager to load the file and voila: 
- You now have an OSC receiver that already opened it's port and is listening to messages
- Your two windows are visible on screen
- You are ready to send some midi notes over the just opened port

You might notice that working this way saves you from typing many lines of code. You don't have to declare objects in C++ or have to worry about the right order of initialization. You can directly access the resources and start building what you had in mind.

Modules & Services {#modules_services}
=======================

Following the modular design of NAP: all functionality is split into [modules](@ref nap::ModuleDescriptor). Each module contains specific blocks of functionality that can be used as a: 
- [Resource](@ref resources) 
- [Entity](@ref scene)
- [Component](@ref scene)

The specifics of these objects are discussed in separate sections. Every module gets compiled into a dynamically linkable library (DLL). NAP loads all available modules automatically when your application starts. Each module has the option to expose a [service](@ref nap::Service). A service is a rather abstract concept and can be used in many different ways. Let's look at an example to understand what a service does. The render service manages, among other things, the following:
- It initializes the render system and terminates it on exit.
- It processes system events such as resizing a window.
- It provides a high-level render interface for all compatible resources, components and entities.

In a more abstract sense: a [service](@ref nap::Service) can be used to perform system-wide operations such as initializing a library or keeping track of specific resources. A service receives [update()](@ref nap::Service::update), [init()](@ref nap::Service::init) and [shutdown()](@ref nap::Service::shutdown) calls. Update is called every frame, init is called before the application is initialized and shutdown is called directly after stopping the app. 

It is possible that a service wants to use functionality from other services. NAP takes care of the correct order of initialization if you tell the system what other services your module depends on by implementing the [getDependentServices()](@ref nap::Service::getDependentServices) call. This function returns a list of services your module depends on with as a result that init, update and shutdown are called in the right order.

Apps {#apps}
=======================

The main entrypoint for running an application is the [AppRunner](@ref nap::AppRunner). This objects requires two objects: an [application](@ref nap::App) to run and an [event handler](@ref nap::AppEventHandler). The event handler forwards system events to the application. These events include mouse and keyboard input. Every application needs to be derived from [BaseApp](@ref nap::BaseApp) and every event handler needs to be derived from [AppEventHandler](@ref nap::AppEventHandler).

The easiest way to set up a new project is to:
- Derive a new applicaton from [App](@ref nap::App) 
- Use the default [GUIAppEventHandler](@ref nap::GUIAppEventHandler) to pass input events to your application
- Give both of them to the [AppRunner](@ref nap::AppRunner) and start the loop

~~~~~~~~~~~~~~~{.cpp}
// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner
	nap::AppRunner<nap::MyApp, nap::GuiAppEventHandler> app_runner(core);

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

Most of the building blocks of NAP are grouped into modules with one exception: the functionality that is present in the core library. Core is a shared library, which is always loaded and can be used everywhere. Every application must have exactly one [Core](@ref nap::Core) instance. The Core object manages all services and is the main gateway to all the available resources. These resources are kept and maintained by the resource manager. 

This example from the helloworld demo shows how to:
- Retrieve initialized services
- Extract loaded content, including: the render window, camera, mesh etc.

~~~~~~~~~~~~~~~{.cpp}
bool HelloWorldApp::init(utility::ErrorState& error)
{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mWorldEntity = scene->findEntity("World");
		mCameraEntity = scene->findEntity("Camera");

		// Find the mesh
		mWorldMesh = mResourceManager->findObject("WorldMesh");
		return true;
	}
~~~~~~~~~~~~~~~

The Resource Manager {#resourcemanager}
=======================

This [object](@ref nap::ResourceManager) is responsible for [loading](@ref nap::ResourceManager::loadFile) the JSON file that contains all the resources that are necessary for your application to run. On startup, the [nap::AppRunner](@ref nap::AppRunner) calls [nap::ResourceManager::loadFile](@ref nap::ResourceManager::loadFile) for you, together with the data file linked to by the [project configuration](@ref nap::ProjectInfo). All the objects declared inside that file are created and initialized by the resource manager. We call these objects 'resources'. Every loaded resource is owned by the resource manager. This means that the lifetime of a resource is fully managed by the resource manager and not by the client (ie: you).

Every resource has an identifier. In the example above we use various identifiers to [find](@ref nap::ResourceManager::findObject) specific resources in the application after load.

Every resource is derived from [Resource](@ref nap::Resource). Every resource carries an identifier that the resource manager uses to identify an object. The most important task of a resource is to tell the resource manager if initialization succeeded using the [init](@ref nap::Resource::init) function. A good example of a resource is the [Image](@ref nap::Image). On initialization the image will try to load a picture from disk and store the result internally. Initialization will fail if the picture doesn't exist or isn't supported. If that's the case the resource manager will halt execution, return an error message and as a result stop further execution of your program. This is the point where NAP tries to validate data for you.

The resource manager and init structure are further explained in the [resource](@ref resources) section.

Events {#events}
=======================

NAP uses [events](@ref nap::Event) to signal the occurrence of an action to the running application. Events often originate from an external environment and are handled by their respective services. When the event is generated asynchronously the service makes sure it is consumed before making it available to potential listeners (often components) on the main thread. This is the general design pattern behind event handling in NAP. [Input](@ref nap::InputEvent), [OSC](@ref nap::OSCEvent) and [Midi](@ref nap::MidiEvent) events are handled this way. This also ensures that new messages don't stall the running application.

Configuration {#service_config}
=======================
Some services are configurable, including the [audio](@ref nap::AudioService), [render](@ref nap::RenderService) and [gui](@ref nap::IMGuiService) service. Every service that is configurable is initialized using a class derived from [ServiceConfiguration](@ref nap::ServiceConfiguration). The render service is initialized using a [render service configuration](@ref nap::RenderServiceConfiguration) and the audio service is initialized using an [audio service configuration](@ref nap::audio::AudioServiceConfiguration).

All service configurable settings are stored in a `config.json` file, which should be placed next to the executable. This file is not placed in the `data` folder because service configurable settings are system specific, not application specific. You might want to select a different audio output port, change the gui font size or disable high dpi rendering. If no `config.json` file is provided the system defaults are used.

Use [Napkin](@ref napkin) to generate and edit service config files.

`config.json` example:

```
{
    "Objects": 
    [
        {
            "Type": "nap::IMGuiServiceConfiguration",
            "mID": "nap::IMGuiServiceConfiguration",
            "FontSize": 17.0
        },
        {
            "Type": "nap::RenderServiceConfiguration",
            "mID": "nap::RenderServiceConfiguration",
            "PreferredGPU": "Discrete",
            "Layers": 
            [
                "VK_LAYER_KHRONOS_validation"
            ],
            "Extensions": [],
            "VulkanMajor": 1,
            "VulkanMinor": 0,
            "EnableHighDPI": true,
            "ShowLayers": false,
            "ShowExtensions": true,
            "AnisotropicSamples": 8
        },
        {
            "Type": "nap::audio::AudioServiceConfiguration",
            "mID": "nap::audio::AudioServiceConfiguration",
            "InputChannelCount": 1,
            "OutputChannelCount": 2,
            "AllowChannelCountFailure": true,
            "SampleRate": 44100.0,
            "BufferSize": 1024,
            "InternalBufferSize": 1024
        }
    ]
}
```
