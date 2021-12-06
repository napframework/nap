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
  * [Create and use a SequenceTrackEvent](@ref using_event_track)
  * [Create and use a SequenceTrackCurve](@ref using_curve_track)
  * [Create and use a SequenceTrackAudio](@ref using_audio_track)

<img src="content/sequencer-overview.png" alt="Sequencer Overview" style="width:100%;"/>

# Introduction {#introduction}

The Sequencer of NAP 0.5 can be used to control and manipulate parameters and trigger events from within a NAP application and/or synchronise this together with audio playback or an external clock. You can animate an arbitrary amount of parameters while at the same time maintaining flexibility and control over synchronization.

The sequencer is set up to run independently of the application’s main render thread. Because of this, the sequencer introduces some concepts that can be hard to understand at first glance. This document serves as a starting point for those who want to work with the sequencer and/or developers that want to add their own features or custom track types to the sequencer.

The following three concepts should be understood when working with the sequencer.
* Relationship between the [SequencePlayer, SequenceEditor & SequenceEditorGUI](@ref player_editor_gui)
* [Thread independent playback using the SequencePlayerClock](@ref thread_independent_playback)
* [Outputs & Adapters](@ref outputs_and_adapters)

# Player, Editor & EditorGUI {#player_editor_gui}

A fully featured NAP Sequencer is constructed from three different parts.
- The [SequencePlayer](@ref nap::SequencePlayer). Responsible for loading and playing a [Sequence](@ref nap::Sequence).
- The [SequenceEditor](@ref nap::SequenceEditor). Responsible for creating [SequenceControllers](@ref nap::SequenceController) for each [SequenceTrack](@ref nap::SequenceTrack) type and responsible for editing and saving the [Sequence](@ref nap::Sequence) loaded by the [SequencePlayer](@ref nap::SequencePlayer). Contains a [ResourcePtr](@ref nap::ResourcePtr) to the [SequencePlayer](@ref nap::SequencePlayer).
- The [SequenceEditorGUI](@ref nap::SequenceEditorGUI). Responsible for drawing a user interface for the player and editor using ImGUI. Contains a [ResourcePtr](@ref nap::ResourcePtr) to the [SequenceEditor](@ref nap::SequenceEditor)

The [SequencePlayer](@ref nap::SequencePlayer) logic and [Sequence](@ref nap::Sequence) data structure is completely independent of the [SequenceEditor](@ref nap::SequenceEditor) and [SequenceEditorGUI](@ref nap::SequenceEditorGUI). This means that it is possible to create an application without a window or without the render engine initialized but still use the [SequencePlayer](@ref nap::SequencePlayer) (for example when working in real-time or near real-time environments) without the [SequenceEditor](@ref nap::SequenceEditor) or [SequenceEditorGUI](@ref nap::SequenceEditorGUI). 

The [SequenceEditor](@ref nap::SequenceEditor) can be added without a [SequenceEditorGUI](@ref nap::SequenceEditorGUI), meaning you can perform edit calls on the loaded [Sequence](@ref nap::Sequence) of the [SequencePlayer](@ref nap::SequencePlayer) without a GUI attached.

The [SequenceEditorGUI](@ref nap::SequenceEditorGUI) is defined in a separate module called mod_sequencegui which depends on the module mod_napsequence but not vice versa. The [SequenceEditorGUI](@ref nap::SequenceEditorGUI) needs a [SequenceEditor](@ref nap::SequenceEditor) resource linked.

To ensure all (live)edits to a running [Sequence](@ref nap::Sequence) are carried out in a thread-safe manner, all edit calls are routed through the [SequenceEditor](@ref nap::SequenceEditor). The [SequenceEditor](@ref nap::SequenceEditor) creates [SequenceControllers](@ref nap::SequenceController) for each available TrackType, [SequenceTracks](@ref nap::SequenceTrack) can be edited by these created controllers.

Controllers for each available track type are created by the application at run-time during the initialization of the [SequenceEditor](@ref nap::SequenceEditor). All created [SequenceControllers](@ref nap::SequenceController) are owned by the [SequenceEditor](@ref nap::SequenceEditor).

When performing an edit to the running [sequence](@ref nap::Sequence) always make sure the edit calls are performed on the same thread that is currently reading from the [SequencePlayer](@ref nap::SequencePlayer)!

For example, the [SequenceEditorGUI](@ref nap::SequenceEditorGUI) is drawn on the main (render) thread of the application, so any edit calls to the [SequenceController](@ref nap::SequenceController) must be executed from the main thread as well. 

The following JSON creates a [SequencePlayer](@ref nap::SequencePlayer) that uses a [SequencePlayerAudioClock](@ref nap::SequencePlayerAudioClock) for playback, a [SequenceEditor](@ref nap::SequenceEditor) for this [SequencePlayer](@ref nap::SequencePlayer) and a [SequenceEditorGUI](@ref nap::SequenceEditorGUI) for the [SequenceEditor](@ref nap::SequenceEditor).
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

# Thread independent playback using the SequencePlayerClock {#thread_independent_playback}

You have complete control over synchronisation. The way the sequencer updates is by using a clock. You can decide what type of clock is used. It is possible to write your own clock when you need synchronisation with another already running process. For now, three types of clocks are already implemented in NAP 0.5.

## Standard Clock {#standard_clock}
The [SequencePlayerStandardClock](@ref nap::SequencePlayerStandardClock) is updated on the main thread from the [SequenceService](@ref nap::SequenceService). This is sufficient for most use cases.

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
The [SequencePlayerThreadedClock](@ref nap::SequencePlayerThreadedClock) launches its own thread. The thread's update cycle is dependent on the given frequency. Frequency is set in update calls per second (Hz).

The following example creates a [SequencePlayer](@ref nap::SequencePlayer) that uses a [SequencePlayerThreadedClock](@ref nap::SequencePlayerThreadedClock) for playback. The [SequencePlayerThreadedClock](@ref nap::SequencePlayerThreadedClock) will update the [SequencePlayer](@ref nap::SequencePlayer) 1000 times per second (1000 Hz).

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
The update cycle is called on the audio thread of NAP’s audio engine. Amount of update calls is dependent on audio buffer size and sample rate. For example, a buffer size of 1024 and sample rate of 44100 samples means 44100 / 1024 = 43.06640625 calls per second. To increase the number of update calls per second make the buffer size smaller. The [SequencePlayerAudioClock](@ref nap::SequencePlayerAudioClock) must be used when using an [SequenceTrackAudio](@ref nap::SequenceTrackAudio) type in the sequencer.

To change the buffer size and/or samplerate used by the [AudioService](@ref nap::audio::AudioService), see [AudioServiceConfiguration](@ref nap::audio::AudioServiceConfiguration)

The following example creates a [SequencePlayer](@ref nap::SequencePlayer) that uses a [SequencePlayerAudioClock](@ref nap::SequencePlayerAudioClock) for playback.

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

When the [SequencePlayer](@ref nap::SequencePlayer) starts playback it will create an adapter for each track in the running sequence. An adapter is responsible for evaluating the [SequenceTrack](@ref nap::SequenceTrack) for which it is created during an update call ([tick](@ref nap::SequencePlayerAdapter::tick)) coming from the [SequencePlayer](@ref nap::SequencePlayer) and translates this to an action on the track’s assigned [SequencePlayerOutput](@ref nap::SequencePlayerOutput). A [SequencePlayerOutput](@ref nap::SequencePlayerOutput) is a [Resource](@ref nap::Resource) embedded within the [SequencePlayer](@ref nap::SequencePlayer). When a track has an output id assigned the [SequencePlayer](@ref nap::SequencePlayer) will try to find the output with the corresponding id and correct type within its embedded outputs and create the appropriate adapter when playback is started linking the output to the [SequencePlayer](@ref nap::SequencePlayer). 

Outputs are resources managed by the [ResourceManager](@ref nap::ResourceManager) and adapters are created and owned/managed by the [SequencePlayer](@ref nap::SequencePlayer). 

For example, a [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) can be added to the [SequencePlayer](@ref nap::SequencePlayer). The [mOutputID](@ref nap::SequenceTrack::mAssignedOutputID) property of the [SequenceEventTrack](@ref nap::SequenceEventTrack) can be assigned to that [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput)’s ID. During playback the [SequencePlayer](@ref nap::SequencePlayer) will create an adapter that will tell the [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) to dispatch an event on the main thread whenever an event is passed by the [SequencePlayer](@ref nap::SequencePlayer)’s running time. The [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) then dispatches this event from the application’s main thread. A rule of thumb here is that [SequencePlayerOutput](@ref nap::SequencePlayerOutput)’s should always be safe to link to from other parts of the application and/or from the applications main thread, while adapters are only created, used and owned by the [SequencePlayer](@ref nap::SequencePlayer).

The following example creates a [SequencePlayer](@ref nap::SequencePlayer) with two [SequencePlayerOutput](@ref nap::SequencePlayerOutput)'s. A [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) and a [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput).

- The [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) dispatches when an event occurs on an [SequenceTrackEvent](@ref nap::SequenceTrackEvent). [SequenceTrackEvent](@ref nap::SequenceTrackEvent) are always dispatched from the main thread!
- The [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput) can be linked to a [Parameter](@ref nap::Parameter) that will follow the output of a [SequenceTrackCurve](@ref nap::SequenceTrackCurve). The [Use Main Thread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) property will determine if [Parameter](@ref nap::Parameter) is updated on main thread or on the thread that is currently used by the clock updating the [SequencePlayer](@ref nap::SequencePlayer).

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

The images belows explain the relation between the [SequencePlayer](@ref nap::SequencePlayer), [SequencePlayerAdapter](@ref nap::SequencePlayerAdapter) & [SequencePlayerOutput](@ref nap::SequencePlayerOutput). When the [SequencePlayer](@ref nap::SequencePlayer) receives a tick from the clock it asks all created adapters to evaluate their assigned SequenceTracks.

### Event Track {#outputs_and_adapters_examples_event}

If an event needs to be dispatched the [SequencePlayerEventAdapter](@ref nap::SequencePlayerEventAdapter) moves the [SequenceEvent](@ref nap::SequenceEvent) to it's assigned [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput). The [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) update method gets called on the main thread from the [SequenceService](@ref nap::SequenceService). On update all queued events will be consumed and dispatched using a [Signal](@ref nap::Signal) so any listeners will get notified.

<img src="content/sequencer-event.png" alt="Sequencer Event" style="width:100%;"/>

### Curve Track {#outputs_and_adapters_examples_curve}

When evaluating a [SequenceTrackCurve](@ref nap::SequenceTrackCurve) the [SequencePlayerCurveAdapter](@ref nap::SequencePlayerCurveAdapter) evaluates the curve 
track and extracts the value. It then passes the value to it's assigned [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput). The [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput) then updates the linked [Parameter](@ref nap::SequencePlayerCurveOutput::mParameter) either directly (when [mUseMainThread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) property is set to false) or on the main thread (when [mUseMainThread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) is set to true). If [mUseMainThread](@ref nap::SequencePlayerCurveOutput::mUseMainThread) is set to true, the [mParameter](@ref nap::SequencePlayerCurveOutput::mParameter) will be updated on the next update coming from the [SequenceService](@ref nap::SequenceService).

<img src="content/sequencer-curve.png" alt="Sequencer Curve" style="width:100%;"/>

### Audio Track {#outputs_and_adapters_examples_audio}

When evaluating an [SequenceTrackAudio](@ref nap::SequenceTrackAudio) the [SequencePlayerAudioAdapter](@ref nap::SequencePlayerAudioAdapter) determines the player's position in samples of the buffer that is linked to by the [SequenceTrackSegmentAudio](@ref nap::SequenceTrackSegmentAudio) on the [SequenceTrackAudio](@ref nap::SequenceTrackAudio) and it's playback speed (pitch). The speed & position is then set on the [SequencePlayerAudioOutput](@ref nap::SequencePlayerAudioOutput) which in turns plays or stops up the correct [MultiSampleBufferPlayerNode](@ref nap::audio::MultiSampleBufferPlayerNode) created for each found [AudioBufferResource](@ref nap::audio::AudioBufferResource).

The [SequencePlayerAudioOutputComponent](@ref nap::audio::SequencePlayerAudioOutputComponent) can be used to route the audio to the playback device selected by the [AudioService](@ref nap::audio::AudioService). You can also let the [SequencePlayerAudioOutput](@ref nap::SequencePlayerAudioOutput) create its own output nodes by setting [mCreateOutputNodes](@ref nap::SequencePlayerAudioOutput::mCreateOutputNodes) property to true.

<img src="content/sequencer-audio.png" alt="Sequencer Audio" style="width:100%;"/>

# Example {#example}

In the following example we will create a sequence using the [SequencePlayer](@ref nap::SequencePlayer), [SequenceEditor](@ref nap::SequenceEditor) & [SequenceEditorGUI](@ref nap::SequenceEditorGUI). In this example we will create a [Sequence](@ref nap::Sequence) containing one [SequenceTrackEvent](@ref nap::SequenceTrackEvent) with a couple of [SequenceEvent](@ref nap::SequenceEvent)s, one [SequenceTrackCuve](@ref nap::SequenceTrackCurve) that animates a [ParameterFloat](@ref nap::ParameterFloat) and an [SequenceTrackAudio](@ref nap::SequenceTrackAudio) that plays back a [SequenceTrackSegmentAudio](@ref nap::SequenceTrackSegmentAudio).

To make sure your project can use all features of the sequencer make sure you include the following modules to your projects module. 

- mod_napsequence
- mod_napsequencegui
- mod_napsequenceaudio
- mod_napsequenceaudiogui

To setup your [SequencePlayer](@ref nap::SequencePlayer) to use the following examples make sure to embed a [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput), [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput) and a [SequencePlayerAudioOutput](@ref nap::SequencePlayerAudioOutput) and make sure the [SequencePlayer](@ref nap::SequencePlayer) uses a [SequencePlayerAudioClock](@ref nap::SequencePlayerAudioClock). You can use the following JSON as an example

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
        },
        {
            "Type": "nap::SequencePlayerAudioOutput",
            "mID": "Audio Output",
            "Create Output Nodes": true,
            "Max Channels": 2
        }],
    "Clock": {
        "Type": "nap::SequencePlayerAudioClock",
        "mID": "SequencePlayerAudioClock"
    }
}
```

## Create and use a SequenceTrackEvent{#using_event_track}

Select the window that draws the [SequenceEditorGUI](@ref nap::SequenceEditorGUI). Select "Insert Track" and then click on "nap::SequenceTrackEvent".

<img src="content/sequencer-select-eventtrack-1.png" alt="SequenceEventTrack step one" style="width:50%;"/>

An empty [SequenceTrackEvent](@ref nap::SequenceTrackEvent) will appear. To make sure events on the track get routed to the embedded [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput) make sure to select the embedded output from the inspector.

<img src="content/sequencer-select-eventtrack-2.png" alt="SequenceEventTrack step two" style="width:50%;"/>

To insert an event right-click somewhere in the track. A popup will appear with different types of events. In this example we will insert an event of type [string](@ref nap::SequenceTrackSegmentEventString).

<img src="content/sequencer-select-eventtrack-3.png" alt="SequenceEventTrack step three" style="width:50%;"/>

An empty [string event](@ref nap::SequenceTrackSegmentEventString) is inserted on the track. You can move it by dragging the handler of the event or by right clicking it and change the timestamp in the appearing popup. You can also change the value of this event. Because the event holds a string we change the [string event](@ref nap::SequenceTrackSegmentEventString) to hold a string with the value "hello world".

<img src="content/sequencer-select-eventtrack-4.png" alt="SequenceEventTrack step four" style="width:50%;"/>

When playing back the sequence the event will be dispatched whenever the playback time of the sequencer passes the time at which the event is inserted. To receive the event or get notified attach a listener to the [Signal](@ref nap::SequencePlayerEventOutput::mSignal) of the [SequencePlayerEventOutput](@ref nap::SequencePlayerEventOutput).

## Create and use a SequenceTrackCurve{#using_curve_track}

Select the window that draws the [SequenceEditorGUI](@ref nap::SequenceEditorGUI). Select "Insert Track" and then click on "nap::SequenceTrackCurveFloat".

<img src="content/sequencer-select-curve-1.png" alt="SequenceEventTrack step one" style="width:50%;"/>

An empty [SequenceTrackCurve](@ref nap::SequenceTrackCurve) will appear. To make sure the drawn curve on the track gets routed to the embedded [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput) make sure to select the embedded output from the inspector.

<img src="content/sequencer-select-curve-2.png" alt="SequenceEventTrack step two" style="width:50%;"/>

To insert an event right-click somewhere in the track. A popup will appear asking you to insert a segment. Do so.

<img src="content/sequencer-select-curve-3.png" alt="SequenceEventTrack step three" style="width:50%;"/>

An [curve segment](@ref nap::SequenceTrackSegmentCurveFloat) is inserted into the track. You can extend it by dragging the handler of the segment or by right clicking it and change the timestamp in the appearing popup. You can also change the value of this event by dragging the segment value or right click on the segment value. You can also insert point on the curve. Move the mouse cursor somewhere on the curve, right click and select "Insert Point" in the appearing popup.

<img src="content/sequencer-select-curve-4.png" alt="SequenceEventTrack step four" style="width:50%;"/>

When playing back the sequence the [ParameterFloat](@ref nap::ParameterFloat) linked from the embedded [SequencePlayerCurveOutput](@ref nap::SequencePlayerCurveOutput) will get updated according to the value of the curved track at the current timestamp of the player.

## Create and use a SequenceTrackAudio{#using_audio_track}

