#pragma once

// std includes
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <locale>
#include <memory>

/**
String utilities not offered by std::string
**/

namespace utility
{
	/**
	@brief Splits a string based on @inDelim, populates @ioParts
	**/
	void gSplitString(const std::string inString, char inDelim, std::vector<std::string>& ioParts);

	void gWriteString(std::ostream& stream, const std::string& text);

	std::string gReadString(std::istream& stream);

	void gToLower(std::string& ioString);

	std::string gToLower(const std::string& inString);

	std::string gStripNamespace(const std::string& str);

	void gTokenize(const std::string& str, std::list<std::string>& tokens, const std::string& delims,
				   bool omitTokens = false);

	bool gStartsWith(const std::string& inString, const std::string& inSubString, bool caseSensitive = true);

	bool gContains(const std::string& inString, const std::string& inSubString, bool caseSensitive = true);

	std::string trim(const std::string& s);

	template <typename T>
	inline std::string addresStr(T thing)
	{
		const void* addr = static_cast<const void*>(thing);
		std::stringstream ss;
		ss << addr;
		return ss.str();
	}

	template <typename... Args>
	static std::string stringFormat(const std::string& format, Args... args)
	{
		size_t size = (size_t)(snprintf(nullptr, 0, format.c_str(), args...) + 1); // Extra space for '\0'
		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, format.c_str(), args...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

    /**
     * Given a templated type name, replace its template parameter with the provided template type
     * @param typeName The original templated type name, eg. "nap::MyType<SomeClass<float>>"
     * @param templateTypeName A replacement type name, eg. "float"
     * @return A modified type name such as "nap::MyType<float>"
     */
    std::string replaceTemplateType(const std::string& typeName, const std::string& templateTypeName);
}
