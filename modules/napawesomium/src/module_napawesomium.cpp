#include <nap/module.h>
#include <napbrowsercomponent.h>
#include <nap/coremodule.h>
#include <napjscriptcallable.h>

NAP_MODULE_BEGIN(NAP_AWESOMIUM)
{
	// Data types 
	NAP_REGISTER_DATATYPE(nap::JavaScriptCallable)

	// Components
	NAP_REGISTER_COMPONENT(nap::BrowserComponent)

	// Type Converters
	NAP_REGISTER_TYPECONVERTER(nap::convert_jscriptcallable_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_jscriptcallable)
}
NAP_MODULE_END(NAP_AWESOMIUM)