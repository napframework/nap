Napkin Architecture Overview
============================
A brief overview of the intended architecture and most important components of the NAP editor, Napkin.
There are two main dependency structures:
- The top-down tree from MainWindow, construction/management of widgets and selection relay
- AppContext/Document-based reactive connections. 
All GUI elements reflect the document (and it's children) state as directly as possible.

All document changes should be pushed into the Document using QUndoCommand instances.

Overall Structure
-----------------
```plantuml
digraph arch {
A -> B
}
```

Important Classes
-----------------
**`napkin::MainWindow`**

The coarsest logic happens here. Notable responsibilities are:
- Panel registration
- Basic main window setup/management
- Selection routing
- Top-level menus and their actions/shortcuts

**`napkin::AppContext`**

We really don't like god-classes, but if there's any spaghetti, you'll find it here.
It is mostly accessed through a singleton-like interface: `AppContext::get()` and it has access to pretty much the whole application. 

And if the time comes we need to refactor spaghetti into smaller components, it should be easy to find it here.

TL;DR: If it looks like spaghetti, it should probably be in AppContext until it matures. 

Responsibilities:
- Manage one or more documents
- Access to other global factories/managers such as ModuleManager, ThemeManager, settings, etc

_Note: Direct access to AppContext should be avoided or reduced_

**`napkin::Document`**

Maintains one `nap::Core` and an accompanying object structure. 
Reflecting the run-time nap application as closely as possible.
The document and its contents will emit signals on any change. 

Generally, all widgets may access Document or PropertyPath (or nap::rtti::Object wrapper) directly.

For example, when an object has been added, the document itself notifies any listeners: "This object has been added".

Note: Currently, the Document is responsible for emitting signals when properties have changed,
this will be moved into `PropertyPath`

**`napkin::PropertyPath`**

Note: PropertyPath will be refactored/renamed to something more appropriate such as **ObjectHandle**

Where the document handles top-level objects, PropertyPath is the more fine-grained object providing access to
any node in the data structure. It may refer to any `nap::rtti::Object` or any `rttr::property`.

A widget representing an object or property will hold a PropertyPath to access the internal data of the document.

Notes
-----
- Subclassing is discouraged, prefer composition. 
Napkin uses inheritance extensively, but tries to not use it where ever it can.
- Every panel is a basic QWidget, the `nap::qt::BaseWindow` will wrap it into a dockwidget.
We're not extending QDockWidget in order for future refactors to be easier.