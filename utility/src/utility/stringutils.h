#pragma once

// std includes
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <locale>
#include <memory>
#include <unordered_map>

namespace nap
{
	/**
	String utilities not offered by std::string
	**/

	namespace utility
	{
		/*
		 * Splits a string based on the given delimiter. Results are stored in ioParts.
		 * @param string the sequence to split
		 * @param delim the character used to split the string
		 * @param ioParts list of individual split parts
		 */
		void splitString(const std::string& string, const char delim, std::vector<std::string>& ioParts);

		/*
		 * Splits a string based on the given delimiter.
		 * @param string the sequence to split.
		 * @param delim the delimiter used to split the string.
		 * @return vector of split parts.
		 */
		std::vector<std::string> splitString(const std::string& string, const char delim);

		/**
		 * Joins a list of strings together.
		 * For example: joinString({"one", "two", "three"}, ", ") -> becomes: "one, two, three"
		 * @param list The list of strings to join
		 * @param delim The delimiter to inject between the elements
		 * @return The joined string.
		 */
		template<typename T>
		std::string joinString(const T& list, const std::string& delim);

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
		 * Converts all upper case characters in ioString to lower case characters
		 * @param ioString the input string that is converted to a lower case string
		 */
		void toLower(std::string& ioString);

		/**
		 * Converts all upper case characters in ioString to lower case characters
		 * @param string the input string that is converted
		 * @return the lower case version of the given string
		 */
		std::string toLower(const std::string& string);

		/**
		 * Strips all name space related identifiers from the given string.
		 * @param str the string to remove the namespace from
		 * @return the stripped string
		 */
		std::string stripNamespace(const std::string& str);

		/**
		 * Tokenize str into tokens.
		 * @param str the string to tokenize
		 * @param tokens the tokens used to process the string
		 * @param delims the delimiters used for the the tokenization process
		 * @param omitTokens if the tokens are discarded from the result
		 */
		void tokenize(const std::string& str, std::list<std::string>& tokens, const std::string& delims, bool omitTokens = false);

		/**
		 * Checks if string starts with subString
		 * @param string the string to check
		 * @param subString the part of the string to check for
		 * @param caseSensitive if the lookup is case sensitive or not
		 * @return if the given string starts with subString
		 */
		bool startsWith(const std::string& string, const std::string& subString, bool caseSensitive = true);

		/**
		 * Checks if string ends with subString
		 * @param string the string to check
		 * @param subString the part of the string to check for
		 * @param caseSensitive if the lookup is case sensitive or now
		 * @return if string ends with substring
		 */
		bool endsWith(const std::string& string, const std::string& subString, bool caseSensitive = true);

		/**
		 * Checks if subString is present in string
		 * @param string the full string that could contain substring
		 * @param subString part of the string that could be present in string
		 * @param caseSensitive if the lookup is case sensitive or not
		 * @return if string contains subString
		 */
		bool contains(const std::string& string, const std::string& subString, bool caseSensitive = true);

		/**
		 * Strips white space characters from a string
		 * @param string the string to remove white space characters from
		 * @return the string without white space characters
		 */
		std::string trim(const std::string& string);

		/**
		 * Strips white space characters from the start(left) of a string
		 * @param string the string to remove white space characters from
		 * @return the string without white space characters
		 */
		std::string lTrim(const std::string& string);

		/**
		 * Strips white space characters from the end(right) of a string string
		 * @param string the string to remove white space characters from
		 * @return the string without white space characters
		 */
		std::string rTrim(const std::string& string);

		/**
		 * Converts T in to a string using an std stringstream.
		 * @param thing the object to convert into a string
		 * @return the object as a string
		 */
		template <typename T>
		std::string addresStr(T thing);

		/**
		 * Formats a string based on the incoming arguments
		 * example: "%s contains %d number of items", object.name().c_str(), i
		 * @param format the string to format
		 * @param Args... the arguments to replace in @format
		 * @return the formatted string
		 */
		template <typename... Args>
		static std::string stringFormat(const std::string& format, Args... args);

		std::string namedFormat(const std::string& subject, const std::unordered_map<std::string, std::string>& rep);

		std::vector<std::string> namedFormat(const std::vector<std::string>& subjects, const std::unordered_map<std::string, std::string>& rep);
		/**
		 * Given a templated type name, replace its template parameter with the provided template type.
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
		 * Replace all instances of a string with a replacement string. 
		 * @param inString The input string to search in
		 * @param find The string to replace
		 * @param replace The replacement string
		 * @return A copy of the input string with all instances of the search term replaced
		 */
		std::string replaceAllInstances(const std::string& inString, const std::string& find, const std::string& replace);

		/**
		 * Based on a string and a character offset into this string, return the line number
		 * @param buffer The string to search
		 * @param offset Character offset into the provided buffer
		 * @return The line number at which the character at offset appears
		 */
		int getLine(const std::string& buffer, size_t offset);


		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		std::string joinString(const T& list, const std::string& delim)
		{
			std::stringstream ss;
			for (size_t i = 0, len = list.size(); i < len; i++)
			{
				if (i > 0)
					ss << delim;
				ss << list.at(i);
			}
			return ss.str();
		}


		template <typename... Args>
		std::string stringFormat(const std::string& format, Args... args)
		{
			size_t size = (size_t)(snprintf(nullptr, 0, format.c_str(), args...) + 1); // Extra space for '\0'
			std::unique_ptr<char[]> buf(new char[size]);
			snprintf(buf.get(), size, format.c_str(), args...);
			return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
		}

		template <typename T>
		std::string addresStr(T thing)
		{
			const void* addr = static_cast<const void*>(thing);
			std::stringstream ss;
			ss << addr;
			return ss.str();
		}
	}
}
