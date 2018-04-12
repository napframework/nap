Napkin
======

# What is Napkin?
NAP's main storage format is JSON, which we consider to be quite a readable format. 
But as projects get larger, it sure helps having a more ergonomical view on the heaps of data you're managing.

Napkin is our modest data file editor that allows you to change your Json files in a more visual manner.
Fire up Napkin, launch your NAP application (from within or outside of the editor) 
and edit your data to see your changes reflected in the running application upon every save.
This way you can get near-realtime feedback from even the most invasive structural changes in your app.

Take note that the data files are multi platform enabled and may be shared between Windows, Mac OS and Linux.

### Terminology
The following slang will be used in this document:
- `File > New` A menu item called *New* in the menu *File* in the menu bar at the top of the window or your screen
- `LMB` Click left mouse button  
- `RMB` Click right mouse button
- `CTRL + X` Press the *X* key while holding *CTRL* (on Windows) or *CMD* (the curly symbol on Mac)

# General Structure
NAP Projects consist mainly of an executable, one or more Json files and likely a collection of assets like images, sound, 3d models, etc.
Napkin runs mostly on the same code as your application, this ensures a solid consistency between what is shown in the editor and your application. 

### Document Management
To make a new file, hit `File > New` or `CTRL + N` to create an empty document.<br>
Alternatively, you can use `File > Open` or `CTRL+O` to open an existing file, you know the drill.

Napkin can edit one file at a time, but you're free to open as many editors as you want.

### Resource Management
A document contains so-called resources and Entities that in themselves don't to much, but can be placed into a Scene later.
 
To create an Entity in the resources:
In the `Resources` panel, `RMB` on `Entities` and choose `Entity`.<br>
Double `LMB` on its name *Entity* and type to change the name to something that will describe its purpose.<br>
You can delete the Entity by using `RMB` and choosing `Delete`.<br>

An entity acts as a container, in order to make it do interesting things, we add Components to it.<br>
To do so, `RMB` on your Entity and choose `Add Component`, a list of available Components will fold out for you to choose from.<br>
Which Component types are exposed, depends on which modules are loaded.<br>

# Panels
Napkin mainly consists of several dockable panels that you can arrange to your heart's content.
All panels showing lists of items can be filtered by name, type or value from the textinput at the top.
At the top of these lists you'll also find column headers (such as Name and Type) you can use to sort the list below by clicking on them.

### Resources
All currently present resources and Entities are displayed and can be edited here.
The objects displayed here don't do anything by themselves. Once added to the Scene, they will duly perform their duties. 
 
### Scene
Shows the live objects in your application. The Entities in here represent instances of their original Entity  

### AppRunner
You can quickly start/stop your application here.<br>
Use the file field to browse to your executable. When you run your application through Napkin, its log messages will be forwarded into the log view.<br>
If your application logs object names in the proper format, you can double-click log messages with that link in the Log panel and it will highlight the appropriate object/property in the editor.  
 
### Inspector
This panel shows the **properties** for the currently selected object.<br>
If an object is selected in the `Resources` panel, you can change its default properties here.<br>
If an object is selected in the `Scene` panel, you are editing tweaks or overrides of its properties, the default properties are left alone.<br>
Most properties are changed by typing in a new value in the `value` column. Some values like `Pointers` provide a button on the value field that allows you to pick the target object.<br>
File properties have their own button that shows a file selection dialog in which you can select the file. 

### Log
Any information that's interesting during development is being shown here.<br> 
If you're running your application from Napkin, its output will be shown in this log too.

Each message has a Log Level attached that will indicate its severeness:<br>
`fine` and `debug` messages are usually not that interesting and will not be shown by default, they're mostly for figuring out the more specific details of your application and the editor.<br>
`info` messages communicate a notable state change in the editor or your application.<br>
`warning` indicates you should probably look what is being said.
`fatal` means something is not right and are very useful for discovering why something doesn't work.
Use the filter to find specific messages based on a search string.<br>

The dropdown on the top-right of this panel allows you to show or hide messages based on their level, in order of importance. `warning` is more important than `debug` for example.
   
If a message pops up that has been underlined, you can double `RMB` it to reveal the object or property that message is saying something about.



