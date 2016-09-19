#include <napjscriptcallable.h>
#include <iostream>
#include <nap/stringutils.h>


bool nap::convert_jscriptcallable_to_string(const nap::JavaScriptCallable& inValue, std::string& outValue)
{
	std::ostringstream ss;
	ss << inValue.object;
	ss << ",";
	ss << inValue.callback;
	ss << ",";
	ss << inValue.returnvalue ? "true" : "false";

	outValue = ss.str();
	return true;
}


bool nap::convert_string_to_jscriptcallable(const std::string& inValue, nap::JavaScriptCallable& outValue)
{
	std::vector<std::string> out_parts;
	gSplitString(inValue, ',', out_parts);
	if (out_parts.size() != 3)
		return false;
	outValue.object      = out_parts[0];
	outValue.callback    = out_parts[1];
	outValue.returnvalue = out_parts[2] == "true";
	return true;
}

RTTI_DEFINE_DATA(nap::JavaScriptCallable)
