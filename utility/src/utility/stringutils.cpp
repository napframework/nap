// Core Includes
#include "utility/stringutils.h"

// Std Includes
#include <regex>

namespace nap
{
	namespace utility
	{

		// Splits a string based on @inDelim
		void splitString(const std::string& inString, const char inDelim, std::vector<std::string>& ioParts)
		{
			// Clear
			ioParts.clear();

			// Create stream
			std::stringstream ss(inString);

			// Extract item and add
			std::string item;
			while (std::getline(ss, item, inDelim))
				ioParts.push_back(item);
		}

		std::vector<std::string> splitString(const std::string& string, const char delim)
		{
			std::vector<std::string> ret;
			splitString(string, delim, ret);
			return ret;
		}


		// Write a string to ostream (binary)
		void writeString(std::ostream& stream, const std::string& text)
		{
			uint32_t size = text.size();

			stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
			stream.write(text.data(), size);
		}


		// Read a string from istream (binary)
		std::string readString(std::istream& stream)
		{
			uint32_t size;
			stream.read(reinterpret_cast<char*>(&size), sizeof(size));

			std::string text(size, 0);
			stream.read(&text[0], size);

			return text;
		}


		// Converts a string to lower case characters
		void toLower(std::string& ioString)
		{
			std::transform(ioString.begin(), ioString.end(), ioString.begin(), ::tolower);
		}


		// Converts @ioString to lower case characters, returns a copy
		std::string toLower(const std::string& ioString)
		{
			std::string out_string = ioString;
			std::transform(out_string.begin(), out_string.end(), out_string.begin(), ::tolower);
			return out_string;
		}

		void tokenize(const std::string& str, std::list<std::string>& tokens, const std::string& delims, bool omitTokens)
		{
			if (omitTokens) {
				std::regex re("([^" + delims + "]+)");
				std::regex_iterator<std::string::const_iterator> rit(str.begin(), str.end(), re);
				std::regex_iterator<std::string::const_iterator> rend;

				while (rit != rend) {
					tokens.push_back(rit->str());
					rit++;
				}
			}
			else {
				std::regex re("([" + delims + "]|[^" + delims + "]+)");
				std::regex_iterator<std::string::const_iterator> rit(str.begin(), str.end(), re);
				std::regex_iterator<std::string::const_iterator> rend;

				while (rit != rend) {
					tokens.push_back(rit->str());
					rit++;
				}
			}
		}


		std::string stripNamespace(const std::string& str) {
			std::regex re("([^:]+)$");
			std::regex_iterator<std::string::const_iterator> rit(str.begin(), str.end(), re);
			std::regex_iterator<std::string::const_iterator> rend;
			if (rit != rend)
				return rit->str();
			return str;
		}


		bool startsWith(const std::string& inString, const std::string& inSubString, bool caseSensitive)
		{
			int(*compare_func)(const char*, const char*, size_t);
#ifdef _WIN32
			compare_func = caseSensitive ? &strncmp : &strnicmp;
#else
			compare_func = caseSensitive ? &strncmp : &strncasecmp;
#endif
			return compare_func(inString.c_str(), inSubString.c_str(), strlen(inSubString.c_str())) == 0;
		}


		bool endsWith(const std::string& string, const std::string& subString, bool caseSensitive /*= true*/)
		{
			auto str_len = string.size();
			auto sub_len = subString.size();
			if (sub_len > str_len)
				return 0;

			int(*compare_func)(const char*, const char*, size_t);
#ifdef _WIN32
			compare_func = caseSensitive ? &strncmp : &strnicmp;
#else
			compare_func = caseSensitive ? &strncmp : &strncasecmp;
#endif
			return compare_func(string.c_str() + str_len - sub_len, subString.c_str(), sub_len) == 0;
		}


		bool contains(const std::string& inString, const std::string& inSubString, bool caseSensitive)
		{
			// case sensitive
			if (caseSensitive)
				return inString.find(inSubString) != std::string::npos;

			// case in-sensitive
			std::locale loc;
			auto it = std::search(
				inString.begin(), inString.end(),
				inSubString.begin(), inSubString.end(),
				[&loc](char ch1, char ch2) { return std::tolower(ch1, loc) == std::tolower(ch2, loc); }
			);
			return (it != inString.end());
		}

		std::string trim(const std::string& s)
		{
			if (s.empty())
				return s;

			size_t first = s.find_first_not_of(' ');
			size_t last = s.find_last_not_of(' ');
			return s.substr(first, (last - first + 1));
		}

		std::string namedFormat(const std::string& subject, const std::unordered_map<std::string, std::string>& rep)
		{
			std::string result = subject;
			for (const auto& e : rep)
				replaceAllInstances(result, '{' + e.first + '}', e.second);
			return result;
		}

		std::string replaceTemplateType(const std::string& typeName, const std::string& templateTypeName) {
			size_t bracketIndex = typeName.find('<');
			return typeName.substr(0, bracketIndex) + "<" + templateTypeName + ">";
		}
		
		// Replace all instances of search string with replacement
		void replaceAllInstances(std::string& inString, const std::string& find, const std::string& replace)
		{
			if (inString.empty())
				return;
			
			for(std::string::size_type i = 0; (i = inString.find(find, i)) != std::string::npos;)
			{
				inString.replace(i, find.length(), replace);
				i += replace.length();
			}
		}
		
		// Replace all instances of search string with replacement, in a copy
		std::string replaceAllInstances(const std::string& inString, const std::string& find, const std::string& replace)
		{
			if (inString.empty())
				return "";
			
			std::string outString = inString;
			
			for(std::string::size_type i = 0; (i = outString.find(find, i)) != std::string::npos;)
			{
				outString.replace(i, find.length(), replace);
				i += replace.length();
			}
			return outString;
		}

		int getLine(const std::string& buffer, size_t offset) {
			int line = 1;
			size_t line_offset = 0;
			while (true)
			{
				line_offset = buffer.find('\n', line_offset);
				if (line_offset == std::string::npos || line_offset > offset)
					break;
				++line;
				line_offset += 1;
			}
			return line;
		}

	}

}
