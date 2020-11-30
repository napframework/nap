Getting Started {#getting_started}
=======================
* [Overview](@ref getting_started_overview)
* [Create a New Project](@ref create_blank_app)
* [Add a Module](@ref add_module)
* [Compile and Run](@ref compile_run)
* [Add Content](@ref add_content)
    * [Resource](@ref audio_resource)
    * [Entity](@ref audio_entity)
    * [Component](@ref audio_components)
    * [Scene](@ref content_scene)
* [Add Logic](@ref app_logic)
    * [Init](@ref app_init)
    * [Update](@ref app_update)
    * [Render](@ref app_render)
* [Package for Distribution](@ref app_package)

Overview {#getting_started_overview}
=======================

In this tutorial we're going to make a new application that plays an audio file from disk. You can start / stop playback using a button that is rendered to screen. This tutorial assumes you are working from a pre-compiled NAP package, not the NAP source-code.

Create a New Project {#create_blank_app}
=======================
To create a new project:

- Use a terminal to navigate to the `tools` directory, inside the NAP installation root.
- Run `create_project NewProject` (Windows) or  `./create_project NewProject` (macOS and Linux)

After creation your new project is located at `projects/newproject`. This directory contains the application source-code, scenes, assets and build instructions. The `project.json` files, in the root of the directory, defines how you application is called and which modules it requires:

```
{
    "Type": "nap::ProjectInfo",
    "mID": "ProjectInfo",
    "Title": "NewProject",
    "Version": "0.1.0",
    "RequiredModules": [
        "mod_napapp",
        "mod_napcameracontrol",
        "mod_napparametergui",
        "mod_newproject"
    ],
    "Data": "data/objects.json",
    "ServiceConfig": "",
    "PathMapping": "cache/path_mapping.json"
}
```

Add a Module {#add_module}
=======================

The `RequiredModules` field tells the build system which modules to include for your project. The most important module is `mod_newproject`. This is your custom application module, located inside the `module` directory of your project. By default the application module links to `mod_naprender`, `mod_napscene` and `mod_napparameter`. This means your application (and custom module) can make use of all the building blocks exposed by those modules. What's missing here is audio functionality, we can link in audio by adding `mod_napaudio` to the list of `RequiredModules`:

```
{
    "Type": "nap::ModuleInfo", 
    "mID": "ModuleInfo", 
    "RequiredModules": [
        "mod_naprender",
        "mod_napscene",
        "mod_napparameter",
        "mod_napaudio"
    ], 
    "WindowsDllSearchPaths": []
}
```

Compile and Run {#compile_run}
================

After changing the `module.json` file you need to (re)generate the project by executing the `regenerate` shortcut in the project folder. You can build and run the project using Visual Studio on Windows (in directory `msvc64`), Xcode on macOS (in `xcode`) or make on Linux (in `build`). 

Open the generated solution and select the `release` configuration. Compile and run your application. You should see an empty window pop up. To learn more about setting up projects, modules and third-party dependencies read the [Project Management](@ref project_management) documentation.

Add Content {#add_content}
================

The `data` folder within your project folder contains an `objects.json` file. This file describes the general structure of your application and all additional resources that are required. Objects in this file can be split up into three different categories:

- [Resources](@ref resources): static, often read-only, data such as an image, window, mesh etc.
- [Entities](@ref scene): objects that structure functionality by combining a set of components
- [Components](@ref scene): used to add functionality to an entity and receive an update call

Every new application contains a window and a scene:

```
{
    "Objects" : 
    [
        {
              "Type": "nap::RenderWindow",
              "mID": "Window",
              "Width": 1280,
              "Height": 720,
              "Title": "NewProject",
              "Mode": "Immediate",
              "SampleShading": true,
              "Samples": "Four"
        },
        {
              "Type" : "nap::Scene",
              "mID": "Scene",   
              "Entities" : []
        }    
    ]
} 
```

## Resource {#audio_resource}

Let's add an [audio file](@ref nap::audio::AudioFileResource) resource. Instead of editing JSON files by hand we're going to use [Napkin](@ref napkin):

- Go to the `tools/napkin` directory
- Launch the `napkin` executable
- Click on `File` -> `Open Project`
- Select the `project.json` file

If Napkin fails to load the project make sure to [build](@ref compile_run) the project (in `release` mode) at least once before loading it. This ensures that the custom application module `mod_newproject` is compiled for you. The editor can then load and inspect it. All other modules (render, audio etc.) are pre-compiled and should work out of the box.

To add an audio file:

- Right click on the `resources` item inside the resource panel
- Select `Create Resource`
- Select a nap::audio::AudioFileResource
- Double click on the new resource 
- Change the name to `AudioFile`

If we start the application right now it will fail to initialize because the resource is invalid, it doesn't point to a valid file on disk:

- Select the `AudioFile` resource
- Inside the inspector panel: click on the `folder` icon next to `AudioFilePath`
- Browse to the file you want to load

The audio file should be sourced from the `data` directory of your project. This allows the application to use relative paths, instead of absolute. Don't have a file on disk? Copy one from the `audioplaybackdemo`.

![](@ref content/napkin_audiofile.png)

## Entity {#audio_entity}

Continue by adding an entity that will hold the audio components.

- Right click on the `Entities` item in the resource panel.
- Select `Create Entity`.
- Double click on the new entity and change its name to `AudioEntity`.

## Components {#audio_components}

The `AudioEntity` requires 2 components: an [AudioPlaybackComponent](@ref nap::audio::PlaybackComponent) to play back the audio file and an [OutputComponent](@ref nap::audio::OutputComponent) to route the output of the playback component to the audio device.

- Right click on the `AudioEntity` 
- Select `Add Component` from the popup menu
- Select a nap::audio::PlaybackComponent 
- Rename it to `PlaybackComponent`

Do the same for the nap::audio::OutputComponent, rename it to `OutputComponent`. 
But we're not there yet: we need to tell the audio playback component which file to play and how many audio channels it outputs:

- Select the `PlaybackComponent`
- Click on the `arrow` icon next to `Buffer`
- Select the `AudioFile`
- Right click on `ChannelRouting` 
- Select `Add int`
- Set the new index to `1`

![](@ref content/napkin_audioplayback.png)

Next we instruct the output component how to route the stereo output of the playback component to the audio device. 

- Select the `OutputComponent` 
- Click on the `arrow` icon next to `Input`
- Select the `PlaybackComponent`
- Right click on `Routing`
- Select `Add int`. 
- Set the new index to `1`

![](@ref content/napkin_audiooutput.png)

## Scene {#content_scene}

To make sure the audio entity is created on startup we have to add it to the scene. 

- Right click on the `Scene` item inside the scene panel
- Select `Add Entity`.
- Pick the `AudioEntity` 
- Save the file (`File` -> `Save`) 
- Run the app from your IDE

You should see a blank window and hear the audio file being played on the default sound device.

![](@ref content/napkin_audioscene.png)

Application {#app_logic}
==========================

Open the `newprojectapp.h` file located inside the `src` directory. This document, together with the `.cpp` file, contains the application runtime code. It allows you to control the flow of data and render specific objects to screen using the resources we just created.

## Init {#app_init}

The `init` method is used to initialize important parts of your application and store references to resources. For this example we need access to the `AudioEntity`. To do so, declare the following variable at the bottom of the `NewProjectApp` class header.

~~~{cpp}
ObjectPtr<EntityInstance> mAudioEntity = nullptr;
~~~

And add the following line to the end of the init() method of your application:

~~~{cpp}
mAudioEntity = mScene->findEntity("AudioEntity");
~~~

We just initialized a pointer (link) to the audio entity. We can use this pointer to manipulate the entity and it's components when the app is running.

## Update {#app_update}

The `update` method is called every frame. The parameter `deltaTime` indicates how many seconds have passed since the last update call. You should perform any app specific logic in here that does not concern rendering.

Because we set the property `AutoPlay` of the PlaybackComponent in the app structure file to `True`, the file starts playing automatically on startup. Let's add the button to start and stop the playback at runtime. Set `AutoPlay` to `False` in your `app_structure.json`, add the following headers to the top of  your `newprojectapp.cpp`:

~~~{cpp}
#include <audio/component/playbackcomponent.h>
#include <imgui/imgui.h>
~~~

.. and add the following to the `update` method:

~~~{cpp}
auto playbackComponent = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();

// Draw some UI elements to control audio playback
ImGui::Begin("Audio Playback");
if (!playbackComponent->isPlaying())
{
  	if (ImGui::Button("Play"))
      	playbackComponent->start(0);
}
else 
{
  	if (ImGui::Button("Stop"))
      	playbackComponent->stop();
}
ImGui::End();
~~~

When we compile and run the app you should see a button. Click on it to start / stop the playback of the audio file. That concludes this little tutorial.

## Rendering {#app_render}

`render` is called after `update`. You use this call to render geometry and UI elements to a window or render target. You have to tell the renderer what you want to render and where to render it to. The button (recorded on `update`) is rendered when `mGuiService->draw()` is called. To learn more about rendering with NAP take a look at our [render documentation](@ref rendering). 

Package for Distribution {#app_package}
==========================

To create a distributable package of your application, including: 

- the executable
- dependencies (modules / third-party)
- napkin (editor)
- assets (json)

run: 

`./package` on macOS and Linux, `package.bat` on Windows. Note that on macOS you (might) have to [disable gatekeeper](https://disable-gatekeeper.github.io/) to run the application: NAP application are not signed by default.



