#pragma once


#include <string>
#include <vector>

namespace nap
{
	bool listDir(const char* directory, std::vector<std::string>& outFilenames);
	std::string getAbsolutePath(const std::string& relPath);
	std::string getFileExtension(const std::string& filename);
}