#pragma once

// std includes
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <locale>
#include <memory>

namespace nap
{
	/**
	String utilities not offered by std::string
	**/

	namespace utility
	{
		/*
		 * Splits a string based on @inDelim, populates @ioParts
		 * @param string the sequence to split
		 * @param delim the delimiter used to split the string
		 * @param ioParts list of individual split parts
		 */
		void splitString(const std::string string, char delim, std::vector<std::string>& ioParts);

		/**
		 * Writes a string to an output stream
		 * @param stream the output stream to write to
		 * @param text the string to write
		 */
		void writeString(std::ostream& stream, const std::string& text);

		/**
		 * Reads a string form an input stream
		 * @param stream the stream to read the string from
		 * @return the string read from the input stream
		 */
		std::string readString(std::istream& stream);

		/**
		 * Converts all upper case characters in @ioString to lower case characters
		 * @param ioString the input string that is converted to a lower case string
		 */
		void toLower(std::string& ioString);

		/**
		 * Converts all upper case characters in @ioString to lower case characters
		 * @param string the input string that is converted
		 * @return the lower case version of @string
		 */
		std::string toLower(const std::string& string);

		/**
		 * Strips all name space related identifiers from @str
		 * @param str the string to remove the namespace from
		 * @return the stripped string
		 */
		std::string stripNamespace(const std::string& str);

		/**
		 * Tokenize @str in to @tokens
		 * @param str the string to tokenize
		 * @param delims the delimiters used for the the tokenization process
		 * @param omitTokens if the tokens are discarded from the result
		 */
		void tokenize(const std::string& str, std::list<std::string>& tokens, const std::string& delims, bool omitTokens = false);

		/**
		 * Checks if @string starts with @subString
		 * @param string the string to check
		 * @param subString the part of the string to check for
		 * @param caseSensitive if the check should take in to account
		 */
		bool startsWith(const std::string& string, const std::string& subString, bool caseSensitive = true);

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

		/**
		 * Replace all instances of search string with replacement
		 * @param inString The input string to search in
		 * @param find The search string
		 * @param replace The replacement string
		 */
		void replaceAllInstances(std::string& inString, const std::string& find, const std::string& replace);
		
		/**
		 * Replace all instances of search string with replacement string, in a copy
		 * @param inString The input string to search in
		 * @param find The search string
		 * @param replace The replacement string
		 * @return A copy of the input string with all instances of the search term replaced
		 */
		std::string replaceAllInstances(const std::string& inString, const std::string& find, const std::string& replace);
	}
}
