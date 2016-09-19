#pragma once

#include <rtti/rtti.h>
#include <nap/attribute.h>
#include <string>
#include <Awesomium/JSObject.h>
#include <nap/event.h>

namespace nap
{
	class BrowserComponent;

	/**
	@brief JScriptObject
	A javascript object associated with a callback
	Used by the browser component to forward javascript messages to c++
	**/
	class JavaScriptCallable
	{
		friend class BrowserComponent;
		RTTI_ENABLE()
	public:
		// Construction
		JavaScriptCallable(const std::string& inName, const std::string& inCallback, bool inReturnValue = false) :
			object(inName),
			callback(inCallback),
			returnvalue(inReturnValue)		{ }
		JavaScriptCallable()				{ }

		// If the returned object is valid
		bool isValid() const				{ return !object.empty(); }

		// Properties
		std::string object = "";			//< Object to bind to in javascript
		std::string callback = "";			//< Callback method on object
		bool returnvalue = false;

	protected:
		mutable Awesomium::JSObject mObject;
	};
}


/**
@brief type-converters for serialization
**/
namespace nap
{
	bool convert_jscriptcallable_to_string(const JavaScriptCallable& inValue, std::string& outValue);
	bool convert_string_to_jscriptcallable(const std::string& inValue, JavaScriptCallable& outValue);
}

RTTI_DECLARE_DATA(nap::JavaScriptCallable)


