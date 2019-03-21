Napkin {#napkin}
=======================
*	[What Is Napkin](@ref what_is_napkin)
*	[Launching Napkin](@ref launching_napkin)
*	[General Structure](@ref napkin_structure)
	* 	[Document Management](@ref napkin_doc_management)
	*	[Resource Management](@ref napkin_res_management)
*	[Panels](@ref napkin_panels)
	* [Resources](@ref napkin_resources)
	* [Scene](@ref napkin_scene)
	* [Apprunner](@ref napkin_apprunner)
	* [Inspector](@ref napkin_inspector)
	* [Log](@ref napkin_log)

What is Napkin? {#what_is_napkin}
=======================

NAP's main storage format is JSON, which we consider to be quite a readable format. But as projects get larger, it helps to have a more ergonomical view on the data you are managing. Napkin is our JSON file editor that allows you to change your JSON files in a more visual manner. Fire up Napkin, launch your NAP application (from within or outside of the editor) and edit your data to see your changes reflected in the running application upon every save. This way you can get near-realtime feedback from even the most invasive structural changes in your app.

Take note that the data files work cross platform and can be shared between Windows, macOS and Linux.

### Terminology
The following jargon will be used in this document:
	- `File > New` A menu item called *New* in the menu *File* in the menu bar at the top of the window or your screen
	- `LMB` Click left mouse button
	- `RMB` Click right mouse button
	- `CTRL + X` Press the *X* key while holding *CTRL* (on Windows) or *CMD* (the curly symbol on Mac)

![](@ref content/napkin.png)

Launching Napkin {#launching_napkin}
=======================

When you build a project Napkin is deployed alongside the project binary within the project's `bin` folder.  For example if you have a project titled NewProject and do a debug build of the project, Napkin will be deployed into the folder `projects/newproject/bin/Clang-x86_64-Debug` (naturally `Clang-x86_64-Debug` will vary depending on your platform).

Napkin is also included with packaged projects and in that case will sit in the root of the created release. See [Project Management](@ref project_management) for further details, including how to disable including Napkin.


General Structure {#napkin_structure}
=======================

NAP Projects consist mainly of an executable, one or more JSON files and likely a collection of assets like images, sound, 3D models, etc. Napkin runs mostly on the same code as your application, this ensures a solid consistency between what is shown in the editor and your application. 

Document Management {#napkin_doc_management}
-----------------------

To make a new file, hit `File > New` or `CTRL + N` to create an empty document.<br> 
Alternatively, you can use `File > Open` or `CTRL+O` to open an existing file, you know the drill.
Napkin can edit one file at a time, but you're free to open as many editors as you want.

Resource Management {#napkin_res_management}
-----------------------

A document contains so-called resources and Entities that in themselves don't to much, but can be placed into a Scene later.
 
To create an Entity in the resources:
In the `Resources` panel, `RMB` on `Entities` and choose `Entity`.<br>
Double `LMB` on its name *Entity* and type to change the name to something that will describe its purpose.<br>
You can delete the Entity by using `RMB` and choosing `Delete`.<br>

An entity acts as a container, in order to make it do interesting things, we add Components to it.<br>
To do so, `RMB` on your Entity and choose `Add Component`, a list of available Components will fold out for you to choose from.<br>
Which Component types are exposed, depends on which modules are loaded.<br>

Panels {#napkin_panels}
=======================

Napkin mainly consists of several dockable panels that you can arrange to your heart's content.
All panels showing lists of items can be filtered by name, type or value from the textinput at the top.
At the top of these lists you'll also find column headers (such as Name and Type) you can use to sort the list below by clicking on them.

Resources {#napkin_resources}
-----------------------

![](@ref content/napkin-panel-resources.png)
All currently present resources and Entities are displayed and can be edited here.
The objects displayed here don't do anything by themselves. Once added to the Scene, they will duly perform their duties. 
 
Scene {#napkin_scene}
-----------------------

![](@ref content/napkin-panel-scene.png)
Shows the live objects in your application. The Entities in here represent instances of their original Entity  

AppRunner {#napkin_apprunner}
-----------------------

![](@ref content/napkin-panel-apprunner.png)
You can quickly start/stop your application here.<br>
Use the file field to browse to your executable. When you run your application through Napkin, its log messages will be forwarded into the log view.<br>
If your application logs object names in the proper format, you can double-click log messages with that link in the Log panel and it will highlight the appropriate object/property in the editor.  
 
Curve {#napkin_curveeditor}
-----------------------
![](@ref content/napkin-panel-curve.png)
When a FCurve resource is selected in the Resources panel, the Curve panel allows for visual editing of that function curve.

Controls:
- `LMB` on curve points or tangent handles to select them
- `SHIFT + LMB` on curve points to add them to the selection
- `CTRL + LMB` add a point on the curve at the clicked location
- `LMB + drag` in the background to rubberband select point handles or tangent handles
- `ALT + MMB` pan the view
- `ALT + RMB` zoom the view horizontally
- `SHIFT + RMB` zoom the view vertically
- `SHIFT + ALT + RMB` zoom the view both vertically and horizontally  
- `F` to frame the selected handles inside the view (if no handles are selected, frame all)
- `A` to frame the entire curve inside the view

Use the buttons in the toolbar or `RMB` in the curve view to change the interpolation of segments.

Tangent handles can be "broken" for discontinuous curves or "aligned" for c2 continuous curves.

Because of the one-dimensional evaluation nature of function curves, the editor will keep you from creating curves with "overhang" (ie. curves that have multiple solutions). 
In order to do so, the effective distance of the tangent handles will be limited.   

 
Inspector {#napkin_inspector}
-----------------------

![](@ref content/napkin-panel-inspector.png)
This panel shows the **properties** for the currently selected object.<br>
If an object is selected in the `Resources` panel, you can change its default properties here.<br>
If an object is selected in the `Scene` panel, you are editing tweaks or overrides of its properties, the default properties are left alone.<br>
Most properties are changed by typing in a new value in the `value` column. Some values like `Pointers` provide a button on the value field that allows you to pick the target object.<br>
File properties have their own button that shows a file selection dialog in which you can select the file. 

Log {#napkin_log}
-----------------------

![](@ref content/napkin-panel-log.png)
Any information that's interesting during development is being shown here.<br>
If you're running your application from Napkin, its output will be shown in this log too.

Each message has a Log Level attached that will indicate its severity:<br>
	- `fine` and `debug` messages are usually not that interesting and will not be shown by default, they're mostly for figuring out the more specific details of your application and the editor.<br>
	- `info` messages communicate a notable state change in the editor or your application.<br>
	- `warning` indicates you should probably look what is being said.
	- `fatal` means something is not right and are very useful for discovering why something doesn't work.
Use the filter to find specific messages based on a search string.<br>

The dropdown on the top-right of this panel allows you to show or hide messages based on their level, in order of importance. `warning` is more important than `debug` for example.
   
If a message pops up that has been underlined, you can double `RMB` it to reveal the object or property that message is saying something about.
