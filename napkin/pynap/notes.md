
NAP Notes
=========

* ResourceManagerService cannot save files
* Who owns entities? i.e. Where to create an Entity on the fly?

Naming and Intuition
--------------------
* **Entity / EntityInstance** should be **EntityResource / Entity** 
The user will most likely deal with the shorter name much more. (Resources may even be invisible)



Python Notes
============

Differentiate Resources/Instances
---------------------------------
The editor should allow for creation of resources and instances.
There should be a clear separation between the two.<br>
(For example, it does not make sense to create a resource without an instance)

User-constructable types can be discovered through RTTI and as a result need no further editor-side maintenance once 
such a type will be declared in the future. 

Currently there is no way to distinguish resources from instances as all types inherit from `RTTIObject` directly.

Possible solutions so far:
- Introduce a `Resource` and an `Instance` type
- Expose a flag `IsResource` on each object type, allowing for filtering later down the pipeline
- Do magic and expose `getResources` on `ResourceManager` 


Discern instantiable types
--------------------------
No way to know if a type can be instantiated through Python object inspection

Solution:
- Expose utility function `bool isTypeInstantiable(const std::string& typename)`
- Allow all subclasses of certain parent classes to be instantiated such as `nap::Component`
 

Needs to raise exceptions
-------------------------
Errors are logged, but do not raise an exception, there is no way for the python user to handle this situation other
than to capture the stdout pipe and try to search for issues:

`nap.core.getOrCreateService('I_do_not_exist')`

prints 

`LOG[fatal] Invalid type: RenderService`

Additionally, when an assertion fails, the application exits with an error code, leaving the user unable to handle the 
problem. It prints:

`python3.5: /home/bmod/Documents/nap/modules/naprender/src/renderservice.cpp:240: void nap::RenderService::shutdown():
 Assertion 'mRenderer != nullptr' failed.`
   
And exits:
`Process finished with exit code 134 (interrupted by signal 6: SIGABRT)`

Python / C++ Typename Inconsistencies
-------------------------------------
C++ `nap::RenderService` vs Python `RenderService`<br/>
The old system replaced `::` with underscores: `nap__RenderService` 

For example, inspecting a type's name:<br>
`nap.RenderService.__name__` yields `"RenderService"`

The following does not find the C++ type using the RTTI system:
`nap.core.getOrCreateService('RenderService') # LOG[fatal] Invalid type: RenderService`

However, including the C++ namespace does correctly find the type:
`nap.core.getOrCreateService('nap::RenderService') # Success!`

Solutions:

- Use hardcoded type names (nope) 
- Global function to retrieve the _actual_ type name
- Replace `::` with one or preferrably two underscores: `nap__RenderService`
- Stick the namespace `nap::` in front of the type name before passing into RTTI functions
- Translate C++ namespaces (`nap::rtti::RTTIObject`) into python namespaces (`nap.rtti.RTTIObject`) 

Python / C++ Application State Synchronization
----------------------------------------------
The object structure within NAP may change due to internal adjustments.
In order for naps internal state to be reflected in the editor, 
on the Python we must either:
- Poll; keep an internal structure, periodically query the internal (C++) data structure and diff against it.
- Or be notified of relevant changes

The previous NAP version [optionally] had signal support on every level of the object structure, allowing for complete 
synchronization of state between the editor and the core.

This problem is not limited to the use of Python. Were we to build a C++ editor, the GUI code would have to be 
similarly kept up-to-date. Preferrably, this would not be done using polling or directly calling the GUI code from Core, 
as this leads to a famous Italian dish.      

Possible solutions:
- Observer (too broad, needs filtering mechanism)
- EventDispatcher (invasive:inheritance based)
- Signal/Slot (invasive:composition based)
- IPC Messaging (flexible, more complicated to maintain)
- Polling (performance...)

Suggested is Signal/Slot combined with a thin messaging layer in order to tackle threading hell.

Author runtime or file?
-----------------------
If the user creates an Entity in the editor, does she create an Entity or EntityInstance?