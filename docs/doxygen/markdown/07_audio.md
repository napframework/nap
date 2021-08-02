Audio {#audio}
=======================

* [Audio Playback](@ref audio_playback)
	* [Files](@ref audio_files)
	* [Playback](@ref audio_playback_comp)
	* [Output](@ref audio_output_comp)
* [Audio Input](@ref audio_input_comp)
	* [Analysis](@ref audio_analysis)
* [Writing Custom Audio Components](@ref audio_custom)
	* [Nodes](@ref audio_nodes)
	* [Components](@ref audio_comps)
	* [Thread Safety](@ref audio_thread_safety)

NAP is equipped with a very flexible and modular audio engine that can be used to send or receive audio from a hardware audio interface, open and read audio files and to perform all kinds of DSP processing on audio signals. On top of this engine a few components are offered to guide you with some common audio tasks, such as playing back audio from a file, receiving audio input from an audio device and reading the output level from an audio signal. A more advanced audio module that is tailored to allow you to design and control custom DSP networks dynamically is currently in development.

Audio Playback {#audio_playback}
=======================
A very common audio task is playing back samples from an audio file in order to equip your app with sound. For an example how to do this have a look at the demo "audioplayback" in the demos directory.

Files {#audio_files}
-----------------------

First of all we need a resource that loads an audio file from disk and keeps it in memory. This is the [AudioFileResource](@ref nap::audio::AudioFileResource). Note that all classes and functions in the audio engine are defined within the audio namespace.

```
{
    "Type": "nap::audio::AudioFileResource",
    "mID": "audioFile",
    "AudioFilePath": "hang.wav"
}
```
(Note: altough displayed here, please don’t edit JSON files by hand and use [Napkin](@ref napkin)!)

It is pretty straightforward, the audio file named in AudioFilePath will be loaded into memory on initialization.

Playback {#audio_playback_comp}
-----------------------

Secondly, we need an entity with a [PlaybackComponent](@ref nap::audio::PlaybackComponent):

```
{
    "Type": "nap::audio::PlaybackComponent",
    "mID": "playbackComponent",
    "ChannelRouting": [ 0, 1 ],
    "Buffer": "audioFile",
    "AutoPlay": false,
    "StartPosition": 0,
    "Duration": 0,
    "FadeInTime": 50,
    "FadeOutTime": 50,
    "Pitch": 1.0,
    "Gain": 1.0,
    "StereoPanning": 0.5
}
```
(again, please don’t type this by hand and use [Napkin](@ref napkin))

The PlaybackComponent offers the functionality to play audio from a buffer resource. (like the [AudioFileResource](@ref nap::audio::AudioFileResource)) As you can see there are a number of parameters available to control the playback. The Buffer points to the resource containing the audio data to be played back. The AutoPlay parameter tells the component to start playback immediately after initialization. The other parameters are pretty self explanatory and can also be modulated on the instance of the component at runtime. Note that all parameters that contain a time value are expressed in milliseconds, because in audio-land we often have to deal with smaller timescales.

Output {#audio_output_comp}
-----------------------

This PlaybackComponent on its own does not produce any sound coming out of the speakers. The reason for this is we do not only need to produce an audio signal, we also have to rout it to the audio device. This is done using an [OutputComponent](@ref nap::audio::OutputComponent).

```
{
    "Type": "nap::audio::OutputComponent",
    "mID": "output",
    "Routing": [ 0, 1 ],
    "Input": "playbackComponent"
}
```
This component simply tells the audio engine to send the audio signals produced by the component specified as Input to the hardware outputs. The int array property Routing tells for each hardware output channel which channel of the input audio it will be routed to it. A value of -1 means no output will be sent to the corresponding channel.

If we add these three objects to an app and set the AutoPlay property to true we should hear a sound once we fire up the app.  Furthermore we can control the [PlaybackComponent](@ref nap::audio::PlaybackComponent) from our app’s [update()](@ref nap::App::update) method to make the sound responsive to app logic, for example through a GUI button:

~~~cpp
ImGui::Begin("Audio Playback");
if (!playbackComponent->isPlaying())
{
    if (ImGui::Button("Play"))
        playbackComponent->start(mStartPosition, mDuration);
}
else {
    if (ImGui::Button("Stop"))
        playbackComponent->stop();
}
~~~

Audio Input {#audio_input_comp}
==========================

In many cases we might need to use an audio signal directly from the hardware input of your audio device, such as a microphone input or line in signal. For this case we use [InputComponent](@ref nap::audio::AudioInputComponent):

~~~
{
    "Type": "nap::audio::AudioInputComponent",
    "mID": "input",
    "Channels": [ 0 ]
}
~~~

This component provides us with an audio signal containing the input from hardware channel 0 of our audio device. Note that in NAP (like everywhere else in C++, but possibly unlike most high level audio software) we start counting channel numbers from zero. This component can be used as an input source for the [OutputComponent](@ref nap::audio::OutputComponent) for example, pretty much like the [PlaybackComponent](@ref nap::audio::PlaybackComponent). The reason for this is that they are both derived from [AudioComponentBase](@ref nap::audio::AudioComponentBase). More about this later in [Writing a custom audio component](@ref audio_custom).

Analysis {#audio_analysis}
-----------------------

In case you intend to use NAP to render visuals that respond to an audio signal you can use [LevelMeterComponent](@ref nap::audio::LevelMeterComponent). This component, pretty much like the [OutputComponent](@ref nap::audio::OutputComponent), takes its input from a component derived from [AudioComponentBase](@ref nap::audio::AudioComponentBase). Instead of routing the signal to a hardware output though the component makes an analysis of the mean amplitude of the signal. It can be tuned to analyze a certain frequency band and also it's responsiveness can be finetuned. The output level can be requested at any given moment and used as a parameter to render visuals. For an example how this works, have a look at the "audioanalysis" demo:

```
{
    "Type": "nap::audio::LevelMeterComponent",
    "mID": "levelMeter",
    "Input": "playbackComponent",
    "AnalysisWindowSize": 10.0,
    "MeterType": "RMS",
    "FilterInput": true,
    "CenterFrequency": 400.0,
    "BandWidth": 100.0,
    "Channel": 0
}
```
Be aware that the component always analyzes one single frequency band on one single input channel. If you need to analyze multiple channels or multiple different frequency bands, you can use multiple LevelMeterComponents together to achieve this.

Writing Custom Audio Components {#audio_custom}
=======================

Nodes {#audio_nodes}
-----------------------

Behind the audio components described in this chapter runs a very modular open system that is designed to implement all sorts of custom DSP systems for audio purposes. This is the backbone of the NAP audio module. The heart of this system is the [Nodemanager](@ref nap::audio::NodeManager), that lives within the [AudioService](@ref nap::audio::AudioService). The NodeManager processes a collection of [Nodes](@ref nap::audio::Node) that are connected together to form a DSP network. The baseclass of all nodes is [Node](@ref nap::audio::Node). Each node can have a number of [input](@ref nap::audio::InputPin) and [output](@ref nap::audio::OutputPin) pins that can be used to connect nodes. The pins and their connections represent the flow of audio signals within the DSP system. Each [Node](@ref nap::audio::Node) has a process() method that defines how the node calculates it's output signals from it's input signals.

There are two special case nodes: the [InputNode](@ref nap::audio::InputNode) and the [OutputNode](@ref nap::audio::OutputNode). The InputNode has one output pin that contains the audio signal from a hardware input channel on the audio device. The OutputNode has one input from which the audio signal will be routed to a hardware output channel.

Audio Components {#audio_comps}
-----------------------

Users can design their custom audio components by deriving from [AudioComponentBase](@ref nap::audio::AudioComponentBase) and [AudioComponentBaseInstance](@ref nap::audio::AudioComponentBaseInstance). Custom audio components can be used as input for [OutputComponent](@ref nap::audio::OutputComponent) and [LevelMeterComponent](@ref nap::audio::LevelMeterComponent). An audio component instance always contains a number of nodes that are interconnected to form a DSP system. The methods [getChannelCount](@ref nap::audio::AudioComponentBaseInstance::getChannelCount) and [getOutputForChannel](@ref nap::audio::AudioComponentBaseInstance::getOutputForChannel) have to be overwritten to respectively return the number of audio signals the component outputs and an output pin from one of it's nodes for a given channel.

Thread Safety {#audio_thread_safety}
-----------------------

Because Nodes are processed on a separate audio thread we need to declare them enclosed within a [SafeOwner](@ref nap::audio::SafeOwner) object. Pointers to the node have to be of the type [SafePtr](@ref nap::audio::SafePtr) and are aqcuired using SafeOwner's [get](@ref nap::audio::SafeOwner::get) method. This all is to make sure that when the node is destroyed or goes out of scope it will not crash the audio thread that might currently be processing the node. The [AudioService](@ref nap::audio::AudioService) takes care of this using a garbage collection system.

