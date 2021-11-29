Sequencer {#sequencer}
=======================
* [Introduction](@ref introduction)
  * [Player, Editor & EditorGUI](@ref player_editor_gui) 
  * [Thread independent playback](@ref thread_independent_playback)
    * [Standard clock](@ref standard_clock)
    * [Threaded clock](@ref threaded_clock)
    * [Audio clock](@ref audio_clock)
  * [Outputs & Adapters](@ref outputs_and_adapters)
* [Example](@ref example)

# Introduction {#introduction}

The sequencer of NAP 0.5 can be used to control and manipulate parameters and trigger events from within a NAP application and/or synchronise this together with audio playback or an external clock. You can animate an arbitrary amount of parameters while at the same time maintaining flexibility and control over synchronization.

The sequencer is set up to run independently of the application’s main render thread. Because of this, the sequencer introduces some concepts that can be hard to understand at first glance. This document serves as a starting point for those who want to work with the sequencer and/or developers that want to add their own features or custom track types to the sequencer.

The following three concepts should be understood when working with the sequencer.
* Relationship between the [Player, Editor & EditorGUI](@ref player_editor_gui)
* [Thread independent playback](@ref thread_independent_playback)
* [Outputs & Adapters](@ref outputs_and_adapters)

## Player, Editor & EditorGUI {#player_editor_gui}

The SequencePlayer logic and sequence data structure is completely independent of the SequenceEditor and SequenceEditorGUI. This means that it is possible to create an application without a window or without the render engine initialized but still use the SequencePlayer (for example when working in real-time or near real-time environments) without the SequenceEditor or SequenceEditorGUI. 

The SequenceEditor can be added without a SequenceEditorGUI, meaning you can perform edit calls on the running Sequence of the SequencePlayer without a GUI attached.
The SequenceEditorGUI is defined in a separate module called mod_sequencegui which depends on the module mod_napsequence but not vice versa. The SequenceEditorGUI needs a SequenceEditor configured.

To ensure all (live)edits to a running sequence are carried out in a thread-safe manner, all edit calls are routed through the SequenceEditor. The SequenceEditor creates controllers for each available TrackType, SequenceTracks can be edited by these created controllers.

Controllers for each available track type are created by the application at run-time during the initialization of the SequenceService. All created SequenceControllers are owned by the SequenceEditor.

**_When performing an edit to the running sequence always make sure the edit calls are performed on the same thread that is currently reading from the SequencePlayer!_**

For example, the SequenceEditorGUI is drawn on the main (render) thread of the application, so any edit calls to the SequenceController must be executed from the main thread as well. 

The following JSON creates a SequencePlayer that uses a SequencePlayerAudioClock for playback, a SequenceEditor for this SequencePlayer and a SequenceEditorGUI for the SequenceEditor.
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

## Thread independent playback {#thread_independent_playback}

You have complete control over synchronisation. The way the sequencer updates is by using a clock. You can decide what type of clock is used. It is possible to write your own clock when you need synchronisation with another already running process. For now, three types of clocks are already implemented in NAP 0.5.

### Standard Clock
The standard clock is updated on the main thread. This is sufficient for most use cases.

The following example creates a SequencePlayer that uses a SequencePlayerStandardClock for playback.

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

### Threaded Clock
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

### Audio Clock
The update cycle is called on the audio thread of NAP’s audio engine. Amount of update calls is dependent on audio buffer size and sample rate. For example, a buffer size of 1024 and sample rate of 44100 samples means 44100 / 1024 =  +-43 calls per second. To increase the number of update calls per second make the buffer size smaller. The SequencePlayerAudioClock must be used when using an SequenceTrackAudio type with the sequencer.

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

## Outputs & Adapters

When the SequencePlayer starts playback it will create an adapter for each track in the running sequence. An adapter is responsible for evaluating the track for which it is created during an update call (tick) coming from the SequencePlayer and translates this to an action on the track’s assigned output. A SequencePlayerOutput is a resource embedded within the SequencePlayer. When a track has an output id assigned the SequencePlayer will try to find the output with the corresponding id and correct type within its embedded outputs and create the appropriate adapter when playback is started. Outputs are Resources managed by the ResourceManager and Adapters are created and owned/managed by the SequencePlayer. 

For example, a SequencePlayerEventOutput can be added to the SequencePlayer. An SequenceEventTrack Output ID string property can be assigned to that SequencePlayerEventOutput’s ID. During playback the SequencePlayer will create an adapter that will tell the SequencePlayerEventOutput to dispatch an event on the main thread whenever an event is passed by the SequencePlayer’s running time. The SequencePlayerEventOutput then dispatches this event from the application’s main thread. A rule of thumb here is that SequencePlayerOutput’s should alwaysbe safe to link to from other parts of the application and/or from the applications main thread, while adapters are only created, used and owned by the SequencePlayer.

The following example creates a SequencePlayer with two SequencePlayerOutput's. A SequencePlayerEventOutput and a SequencePlayerCurveOutput.

- The SequencePlayerEventOutput dispatches when an event occurs on an Event track.  **_Events are always dispatched from the main thread!_** 
- The SequencePlayerCurveOutput can be linked to a Parameter that will follow the output of a SequenceTrackCurve. The "Use Main Thread" property will determine if Parameter is updated on main thread or on the thread that is currently used by the Clock updating the SequencePlayer.
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