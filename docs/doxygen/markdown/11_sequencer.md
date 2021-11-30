Sequencer {#sequencer}
=======================
* [Introduction](@ref introduction)
* [Player, Editor & EditorGUI](@ref player_editor_gui) 
* [Thread independent playback](@ref thread_independent_playback)
  * [Standard clock](@ref standard_clock)
  * [Threaded clock](@ref threaded_clock)
  * [Audio clock](@ref audio_clock)
* [Outputs & Adapters](@ref outputs_and_adapters)
  * [Examples](@ref outputs_and_adapters_examples)
    * [Event Track](@ref outputs_and_adapters_examples_event)
    * [Curve Track](@ref outputs_and_adapters_examples_curve)
    * [Audio Track](@ref outputs_and_adapters_examples_audio)
* [Example](@ref examples)
  * [Using an event track](@ref using_event_track)
  * [Using a curved track](@ref using_curve_track)
  * [Using an audio track](@ref using_audio_track)

# Introduction {#introduction}

The Sequencer of NAP 0.5 can be used to control and manipulate parameters and trigger events from within a NAP application and/or synchronise this together with audio playback or an external clock. You can animate an arbitrary amount of parameters while at the same time maintaining flexibility and control over synchronization.

The sequencer is set up to run independently of the application’s main render thread. Because of this, the sequencer introduces some concepts that can be hard to understand at first glance. This document serves as a starting point for those who want to work with the sequencer and/or developers that want to add their own features or custom track types to the sequencer.

The following three concepts should be understood when working with the sequencer.
* Relationship between the [Player, Editor & EditorGUI](@ref player_editor_gui)
* [Thread independent playback](@ref thread_independent_playback)
* [Outputs & Adapters](@ref outputs_and_adapters)

# Player, Editor & EditorGUI {#player_editor_gui}

The [SequencePlayer](@ref nap::SequencePlayer) logic and sequence data structure is completely independent of the [SequenceEditor](@ref nap::SequenceEditor) and [SequenceEditorGUI](@ref nap::SequenceEditorGUI). This means that it is possible to create an application without a window or without the render engine initialized but still use the [SequencePlayer](@ref nap::SequencePlayer) (for example when working in real-time or near real-time environments) without the [SequenceEditor](@ref nap::SequenceEditor) or [SequenceEditorGUI](@ref nap::SequenceEditorGUI). 

The [SequenceEditor](@ref nap::SequenceEditor) can be added without a [SequenceEditorGUI](@ref nap::SequenceEditorGUI), meaning you can perform edit calls on the loaded [Sequence](@ref nap::Sequence) of the SequencePlayer without a GUI attached.

The [SequenceEditorGUI](@ref nap::SequenceEditorGUI) is defined in a separate module called mod_sequencegui which depends on the module mod_napsequence but not vice versa. The [SequenceEditorGUI](@ref nap::SequenceEditorGUI) needs a [SequenceEditor](@ref nap::SequenceEditor) resource configured.

To ensure all (live)edits to a running sequence are carried out in a thread-safe manner, all edit calls are routed through the SequenceEditor. The SequenceEditor creates controllers for each available TrackType, SequenceTracks can be edited by these created controllers.

Controllers for each available track type are created by the application at run-time during the initialization of the [SequenceEditor](@ref nap::SequenceEditor). All created SequenceControllers are owned by the [SequenceEditor](@ref nap::SequenceEditor).

When performing an edit to the running sequence always make sure the edit calls are performed on the same thread that is currently reading from the [SequencePlayer](@ref nap::SequencePlayer)!

For example, the [SequenceEditorGUI](@ref nap::SequenceEditorGUI) is drawn on the main (render) thread of the application, so any edit calls to the [SequenceController](@ref nap::SequenceController) must be executed from the main thread as well. 

The following JSON creates a SequencePlayer that uses a [SequencePlayerAudioClock](@ref nap::SequencePlayerAudioClock) for playback, a SequenceEditor for this SequencePlayer and a SequenceEditorGUI for the SequenceEditor.
```
{
    "Type": "nap::SequenceEditor",
    "mID": "SequenceEditor",
    "Sequence Player": "SequencePlayer"
},
{
    "Type": "nap::SequenceEditorGUI",
    "mID": "SequenceEditorGUI",
    "Sequence Editor": "SequenceEditor",
    "Render Window": "SequencerWindow",
    "Draw Full Window": true
},
{
    "Type": "nap::SequencePlayer",
    "mID": "SequencePlayer",
    "Default Show": "Default Show.json",
    "Outputs": [],
    "Clock": {
        "Type": "nap::SequencePlayerAudioClock",
        "mID": "SequencePlayerAudioClock"
    }
}
```

# Thread independent playback {#thread_independent_playback}

You have complete control over synchronisation. The way the sequencer updates is by using a clock. You can decide what type of clock is used. It is possible to write your own clock when you need synchronisation with another already running process. For now, three types of clocks are already implemented in NAP 0.5.

## Standard Clock {#standard_clock}
The [standard clock](@ref nap::SequencePlayerStandardClock) is updated on the main thread from the [SequenceService](@ref nap::SequenceService). This is sufficient for most use cases.

The following example creates a [SequencePlayer](@ref nap::SequencePlayer) that uses a [SequencePlayerStandardClock](@ref nap::SequencePlayerStandardClock) for playback.

```
{
    "Type": "nap::SequencePlayer",
    "mID": "SequencePlayer",
    "Default Show": "Default Show.json",
    "Outputs": [],
    "Clock": {
        "Type": "nap::SequencePlayerStandardClock",
        "mID": "SequencePlayerStandardClock"
    }
}
```

## Threaded Clock {#threaded_clock}
The threaded clock launches its own thread. The thread's update cycle is dependent on the given frequency. Frequency is set in update calls per second (Hz).

The following example creates a SequencePlayer that uses a SequencePlayerThreadedClock for playback. The SequencePlayerThreadedClock will update the SequencePlayer 1000 times per second (1000 Hz).

```
{
    "Type": "nap::SequencePlayer",
    "mID": "SequencePlayer",
    "Default Show": "Default Show.json",
    "Outputs": [],
    "Clock": {
        "Type": "nap::SequencePlayerThreadedClock",
        "mID": "SequencePlayerThreadedClock",
        "Frequency" : 1000
    }
}
```

## Audio Clock {#audio_clock}
The update cycle is called on the audio thread of NAP’s audio engine. Amount of update calls is dependent on audio buffer size and sample rate. For example, a buffer size of 1024 and sample rate of 44100 samples means 44100 / 1024 =  +-43 calls per second. To increase the number of update calls per second make the buffer size smaller. The [SequencePlayerAudioClock](@ref nap::SequencePlayerAudioClock) must be used when using an [SequenceTrackAudio](@ref nap::SequenceTrackAudio) type in the sequencer.

To change the buffer size and/or samplerate used by the AudioService, see [AudioServiceConfiguration](@ref nap::audio::AudioServiceConfiguration)

The following example creates a SequencePlayer that uses a SequencePlayerAudioClock for playback.

```
{
    "Type": "nap::SequencePlayer",
    "mID": "SequencePlayer",
    "Default Show": "Default Show.json",
    "Outputs": [],
    "Clock": {
        "Type": "nap::SequencePlayerAudioClock",
        "mID": "SequencePlayerAudioClock"
    }
}
```

# Outputs & Adapters {#outputs_and_adapters}

When the [SequencePlayer](@ref nap::SequencePlayer) starts playback it will create an adapter for each track in the running sequence. An adapter is responsible for evaluating the track for which it is created during an update call ([tick](@ref nap::SequencePlayerAdapter::tick)) coming from the [SequencePlayer](@ref nap::SequencePlayer) and translates this to an action on the track’s assigned output. A [SequencePlayerOutput](@ref nap::SequencePlayerOutput) is a [Resource](@ref nap::Resource) embedded within the [SequencePlayer](@ref nap::SequencePlayer). When a track has an output id assigned the [SequencePlayer](@ref nap::SequencePlayer) will try to find the output with the corresponding id and correct type within its embedded outputs and create the appropriate adapter when playback is started linking the output to the [SequencePlayer](@ref nap::SequencePlayer). Outputs are Resources managed by the [ResourceManager](@ref nap::ResourceManager) and Adapters are created and owned/managed by the [SequencePlayer](@ref nap::SequencePlayer). 

For example, a [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) can be added to the [SequencePlayer](@ref nap::SequencePlayer). An [SequenceEventTrack](@ref nap::SequenceEventTrack) Output ID string property can be assigned to that [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput)’s ID. During playback the [SequencePlayer](@ref nap::SequencePlayer) will create an adapter that will tell the [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) to dispatch an event on the main thread whenever an event is passed by the [SequencePlayer](@ref nap::SequencePlayer)’s running time. The [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) then dispatches this event from the application’s main thread. A rule of thumb here is that SequencePlayerOutput’s should always be safe to link to from other parts of the application and/or from the applications main thread, while adapters are only created, used and owned by the [SequencePlayer](@ref nap::SequencePlayer).

The following example creates a [SequencePlayer](@ref nap::SequencePlayer) with two SequencePlayerOutput's. A [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) and a [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput).

- The [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) dispatches when an event occurs on an [Event track](@ref nap::SequenceEventTrack). Events are always dispatched from the main thread!
- The [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput) can be linked to a [Parameter](@ref nap::Parameter) that will follow the output of a [SequenceTrackCurve](@ref nap::SequenceTrackCurve). The [Use Main Thread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) property will determine if [Parameter](@ref nap::Parameter) is updated on main thread or on the thread that is currently used by the Clock updating the [SequencePlayer](@ref nap::SequencePlayer).

```
{
    "Type": "nap::SequencePlayer",
    "mID": "SequencePlayer",
    "Default Show": "Default Show.json",
    "Outputs": [
        {
            "Type": "nap::SequencePlayerEventOutput",
            "mID": "SequencePlayerEventOutput"
        },
        {
            "Type": "nap::SequencePlayerCurveOutput",
            "mID": "Float Output",
            "Parameter": "Float1",
            "Use Main Thread": true
        }]
    "Clock": {
        "Type": "nap::SequencePlayerAudioClock",
        "mID": "SequencePlayerAudioClock"
    }
}
```
## Flow Examples {#outputs_and_adapters_examples}

The images belows explain the relation between the [SequencePlayer](@ref nap::SequencePlayer), [Adapter](@ref nap::SequencePlayerAdapter) & [Output](@ref nap::SequencePlayerOutput). When the [SequencePlayer](@ref nap::SequencePlayer) receives a tick from the clock it asks all created Adapters to evaluate their assigned SequenceTracks.

### Event Track {#outputs_and_adapters_examples_event}

If an event needs to be dispatched the [EventAdapter](@ref nap::SequencePlayerEventAdapter) moves the [Event](@ref nap::SequenceEvent) to it's assigned [EventOutput](@ref nap::SequencePlayerEventOutput). The [EventOutput](@ref nap::SequencePlayerEventOutput) update method gets called on the main thread from the [SequenceService](@ref nap::SequenceService). On update all queued events will be consumed and dispatched using a [Signal](@ref nap::Signal) so any listeners will get notified.

<img src="content/sequencer-event.png" alt="Sequencer Event" style="width:100%;"/>

### Curve Track {#outputs_and_adapters_examples_curve}

When evaluating a Curve Track the [CurveAdapter](@ref nap::SequencePlayerCurveAdapter) evaluates the curve 
[track](@ref nap::SequenceCurveTrack) and extracts the value. It then passes the value to it's assigned [output](@ref nap::SequencePlayerCurveOutput). The [output](@ref nap::SequencePlayerCurveOutput) then updates the linked [parameter](@ref nap::SequencePlayerCurveOutput::mParameter) either directly (when [Use Main Thread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) property is set to false) or on the main thread (when [Use Main Thread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) is set to true). If [Use Main Thread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) is set to true, the [parameter](@ref nap::SequencePlayerCurveOutput::mParameter) will be updated on the next update coming from the [SequenceService](@ref nap::SequenceService).

<img src="content/sequencer-curve.png" alt="Sequencer Curve" style="width:100%;"/>

### Audio Track {#outputs_and_adapters_examples_audio}

When evaluating an [AudioTrack](@ref nap::SequenceTrackAudio) the [AudioAdapter](@ref nap::SequencePlayerAudioAdapter) determines the player's position in samples of the buffer that is linked to by the [AudioSegment](@ref nap::SequenceTrackSegmentAudio) on the [AudioTrack](@ref nap::SequenceTrackAudio) and it's playback speed (pitch). The speed & position is then set on the [AudioOutput](@ref nap::SequencePlayerAudioOutput) which in turns plays or stops up the correct [MultiSampleBufferPlayerNode](@ref nap::audio::MultiSampleBufferPlayerNode) created for each found [AudioBufferResource](@ref nap::audio::AudioBufferResource).

The [SequencePlayerAudioOutputComponent](@ref nap::SequencePlayerAudioOutputComponent) can be used to route the audio to the playback device selected by the [AudioService](@ref nap::audio::AudioService). You can also let the [AudioOutput](@ref nap::SequencePlayerAudioOutput) create its own output nodes by setting [Create Output Nodes](@ref nap::SequencePlayerAudioOutput::mCreateOutputNodes) to true.

<img src="content/sequencer-audio.png" alt="Sequencer Audio" style="width:100%;"/>

# Example {#example}

In the following example we will create a sequence using the SequencePlayer, SequenceEditor & SequenceEditorGUI. In this example we will create a sequence containing one EventTrack with a couple of events, one CurveTrack that animates a ParameterFloat and an AudioTrack that plays back an AudioSegment.

To make sure your project can use all features of the sequencer make sure you include the following module to your projects module. See [project management](08_project_management.md) for more info.



## Using an event track {#using_event_track}

## Using a curved track {#using_curve_track}

## Using an audio track {#using_audio_track}

