Getting Started {#getting_started}
=======================
* [Create a New App](@ref create_blank_app)
* [Specifying Modules](@ref specifying_modules)
* [Adding Resources](@ref defining_resources)
* [App Logic](@ref app_logic)

Create a New App {#create_blank_app}
=======================
In this tutorial we're going to show you how to create an app which plays an audio track and displays a simple UI to start and stop playback.

First, use the NAP build system to create a new application from scratch:

- In a terminal window, navigate to the directory containing your NAP installation on disk.
- Run `tools\create_project NewProject` (Windows) or  `./tools/create_project NewProject` (macOS and Linux) to generate a new project for your OS.

After creation your new project is located in the `projects` folder.

Specifying Modules {#specifying_modules}
=======================

Within your newly created project you will find the project definition file `project.json`, which looks like this:

```
{
    "title": "NewProject",
    "version": "1.0",
    "modules": [
        "mod_napapp",
        "mod_napaudio"
    ]
}
```

All projects created via `create_project` will automatically be setup to use modules `mod_napapp` and `mod_napaudio`.  The simple project that we create in the steps below makes use of the module `mod_napaudio` for audio playback.  When you find a need to use some of the other NAP modules in your project these should be added to the `modules` element in your `project.json`.  After changing `project.json` you need to regenerate the project (via CMake) by executing the `regenerate` shortcut in the project folder.

You can build and run the project using Visual Studio on Windows (in directory `msvc64`), Xcode on macOS (in `xcode`) or make on Linux (in `build`).

To learn more about setting up projects, modules and third-party dependencies read the [Project Management](@ref project_management) documentation.

Adding Resources {#defining_resources}
================

The `data` folder within your project folder contains an `app_structure.json` file. This file describes the general structure of your app and any additional resources that are required. Objects in this file can be split up into three different categories:
- [Resources](@ref resources): static, often read-only, data such as an image, window, mesh etc.
- [Entities](@ref scene): objects that structure functionality by combining a set of components
- [Components](@ref scene): used to add functionality to an entity and receive an update call

Refer to the [Resource](@ref resources) and [Scene](@ref scene) documentation for more information. 

Every blank app contains a window and a scene:

```
{
    "Objects" : 
    [
        {
            "Type": "nap::RenderWindow",
            "mID": "Window",
            "Title": "NewProject",
            "Width": 1280,
            "Height": 720
        },
        {
            "Type" : "nap::Scene",
            "mID": "Scene",   
            "Entities" : []
        }      
    ]
} 
```

Let's add a new resource: an audio file that is loaded from disk. Make sure to add it to the `Objects` array. Note that instead of editing JSON files by hand it is highly recommended that you use [Napkin](@ref napkin), our JSON editor, instead.

```
{
    "Type": "nap::audio::AudioFileResource",
    "mID": "audioFile",
    "AudioFilePath": "myaudiofile.wav"
}
```

Note that the file specified in `AudioFilePath` should be located within the `data` directory of your project. 

Continue by adding an entity that holds two components: an [AudioPlaybackComponent](@ref nap::audio::PlaybackComponent) to be able to play back the audio file and an [OutputComponent](@ref nap::audio::OutputComponent) to route the output of the playback component to the audio device. Again, make sure to add it to the `Objects` array:

```
{
    "Type": "nap::Entity",
    "mID": "audioEntity",
    "Components":
    [
        {
            "Type": "nap::audio::PlaybackComponent",
            "mID": "playbackComponent",
            "ChannelRouting": [ 0, 1 ],
            "Buffer": "audioFile",
            "AutoPlay": "True"
        },
        {
            "Type": "nap::audio::OutputComponent",
            "mID": "output",
            "Routing": [ 0, 1 ],
            "Input": "playbackComponent"
        }
    ]
}
```

As you see can see the `Buffer` property of the PlaybackComponent points to the 'audioFile' and the `Input` property of the OutputComponent points to the 'playbackComponent'. 

Now update the `Entities` list in the existing scene entry to add the newly created audio entity. This makes sure the entity is created by the application on startup:

```
{
    "Type": "nap::Scene",
    "mID": "Scene",   
    "Entities": [
        {
          "Entity": "audioEntity"
        }       
    ]
} 
```

Your final `app_structure.json` should look like this:

```
{
    "Objects" : 
    [
        {
            "Type": "nap::RenderWindow",
            "mID": "Window",
            "Title": "NewProject",
            "Width": 1280,
            "Height": 720
        },
        {
            "Type": "nap::Scene",
            "mID": "Scene",   
            "Entities": [
                {
                  "Entity": "audioEntity"
                }       
            ]
        },  
        {
            "Type": "nap::audio::AudioFileResource",
            "mID": "audioFile",
            "AudioFilePath": "myaudiofile.wav"
        },
        {
            "Type": "nap::Entity",
            "mID": "audioEntity",
            "Components":
            [
                {
                    "Type": "nap::audio::PlaybackComponent",
                    "mID": "playbackComponent",
                    "ChannelRouting": [ 0, 1 ],
                    "Buffer": "audioFile",
                    "AutoPlay": "True"
                },
                {
                    "Type": "nap::audio::OutputComponent",
                    "mID": "output",
                    "Routing": [ 0, 1 ],
                    "Input": "playbackComponent"
                }
            ]
        }
    ]
}
```

Now save the JSON file and fire up your app in your IDE of choice.

When you run the project you should see a blank window and hear the audio file being played on the default sound device.

App Logic {#app_logic}
==========================

In your new project there is a file called `newprojectapp.h`. This file contains the class NewProjectApp that contains all logic specific to the app we are building. NewProjectApp is derived from the App base class.

## Init

The `init` method is used to initialize important parts of your application and store references to resources. For this example we need access to the audio entity. To do so, declare the following variable at the bottom of the NewProjectApp class header.

~~~{cpp}
ObjectPtr<EntityInstance> mAudioEntity = nullptr;
~~~

And add the following line to the init() method of your application:

~~~{cpp}
mAudioEntity = mScene->findEntity("audioEntity");
~~~

We just initialized a pointer (link) to the audio entity. We can use this pointer to manipulate the entity and it's components while the app is running.

## Update

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

When we compile and run the app you should see a button to start and stop playback of the audio file.

## Render

`render` is called after `update`. Use this call to render objects and UI elements to screen or a different target. By default nothing is rendered. You have to tell the renderer what you want to render and where to render it to. To learn more about rendering with NAP take a look at our [render documentation](@ref rendering). 

The example below (which is part of the default template) shows you how to render the GUI above the other contents in the primary window:

~~~{cpp}
void NewProjectApp::render()
{
    // Signal the beginning of a new frame, allowing it to be recorded.
    mRenderService->beginFrame();

    // Begin recording the render commands for the main render window
    if (mRenderService->beginRecording(*mRenderWindow))
    {
        // Begin render pass
        mRenderWindow->beginRendering();

        // Render GUI elements
        mGuiService->draw();

        // Stop render pass
        mRenderWindow->endRendering();

        // End recording
        mRenderService->endRecording();
    }

    // Proceed to next frame
    mRenderService->endFrame();
}
~~~