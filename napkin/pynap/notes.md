
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

Author runtime or file?
-----------------------
If the user creates an Entity in the editor, does she create an Entity or EntityInstance?