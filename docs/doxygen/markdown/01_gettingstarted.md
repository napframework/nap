Getting Started {#getting_started}
=======================
* [Create a New App](@ref create_blank_app)
* [Adding Resources](@ref defining_resources)
* [App Logic](@ref app_logic)

Create a New App {#create_blank_app}
=======================
Use the NAP build system to create a new application from scratch:

- In a terminal window, navigate to the directory containing your NAP installation on disk.
- Run `tools\create_project NewProject` to generate a new project for your OS.

After creation your new project is located under the 'projects' folder. You can build and run it using Visual Studio on Windows, Xcode on macOS or make on Linux. To learn more about setting up projects, modules and 3rd party dependencies read the [Project Management](@ref project_management) documentation.

Adding resources {#defining_resources}
================

The data folder within your project folder contains an `appstructure.json` file. This file describes the general structure of your app and any additional resources that are required. Objects in this file can be split up into three different categories:
- [Resources](@ref resources): static often read only data such as an image, window, mesh etc.
- [Entities](@ref scene): objects that structure functionality by combining a set of components
- [Components](@ref scene): add functionality to an entity and receive an update call

Refer to the [Resource](@ref resources) and [Scene](@ref scene) documentation for more information.

Every blank app contains a window and a scene:

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
    	},
		{
      		"Type" : "nap::Scene",
      		"mID": "Scene",   
      		"Entities" : []
    	}	
	]
} 
```

Lets add a new resource: an audio file that is loaded from disk. Make sure to add it to the `Objects` array:

```
{
    "Type": "nap::audio::AudioFileResource",
    "mID": "audioFile",
    "AudioFilePath": "myaudiofile.wav"
}
```

Note that the file specified in `AudioFilePath` should be located within the `data` directory of your project. Continue by adding an entity that holds two components: an [AudioPlaybackComponent](@ref nap::audio::PlaybackComponent) to be able to play back the audio file and an [OutputComponent](@ref nap::audio::OutputComponent) to route the output of the playback component to the audio device. Also this time, make sure to add it to the `Objects` array:

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

As you see can see the `Buffer` property of the PlaybackComponent points to the 'audioFile' and the `Input` property of the OutputComponent points to the 'playbackComponent'. Now add the newly created audio entity to the scene. This makes sure the entity is created by the application on startup:

```
{
	"Objects" : 
	[
		{
      		"Type" : "nap::Scene",
      		"mID": "Scene",   
      		"Entities" : 
      		[
      			audioEntity
      		]
    	}	
	]
} 
```

Now save the JSON file and fire up your app in your IDE of choice. You should see a blank window and hear the audio file being played on the default sound device.

App logic {#app_logic}
==========================

In your new project there is a file called `newprojectapp.h`. This file contains the class NewProjectApp that contains all logic specific to the app we are building. NewProjectApp is derived from the App base class.

## Init

The init method is used to initialize important parts of your application and store references to resources. For this example we need access to the audio entity. To do so, declare the following variable at the bottom of the NewProjectApp class header.

~~~{cpp}
ObjectPtr<Entity> mEntity = nullptr;
~~~

And add the following line to the init() method of your application:

~~~{cpp}
mEntity = mScene->findEntity("audioEntity");
~~~

We just initialized a pointer (link) to the audio entity. We can use this pointer to manipulate the entity and it's components while the app is running.

## Update

The update method is called every frame. The parameter 'deltaTime' indicates how many seconds have passed since the last update call. You should perform any app specific logic in here that does not concern rendering.

Because we set the property `AutoPlay` of the PlaybackComponent in the app structure file to 'True', the file starts playing automatically on startup. Suppose we want to add a button to start and stop the playback at runtime. Set `AutoPlay` to False and add the following lines to the update method:

~~~{cpp}

void NewProjectApp::update(double deltaTime)
{
	auto playbackComponent = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();

	// Draw some gui elements to control audio playback
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
}
~~~

Note: to make this work add the following includes to `newprojectapp.cpp`:

~~~{cpp}
#include <audio/component/playbackcomponent.h>
#include <imgui/imgui.h>
~~~

When we compile and run the app you should see a button to start and stop playback of the audio file.

## Render

Render is called after update. Use this call to render objects and UI elements to screen or a different target. By default nothing is rendered. You have to tell the renderer what you want to render and where to render it to. To learn more about rendering with NAP take a look at our [render documentation](@ref rendering). The example below shows you how to render the gui mentioned above to the primary window:

~~~{cpp}
void NewProjectApp::render()
{
	// Clear opengl context related resources that are not necessary any more
	mRenderService->destroyGLContextResources({ mRenderWindow });

    // Activate current window for drawing
    mRenderWindow->makeActive();

    // Clear back-buffer
    mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

    // Draw our UI last!
    mGuiService->draw();

    // Swap screen buffers
    mRenderWindow->swap();
}
~~~







