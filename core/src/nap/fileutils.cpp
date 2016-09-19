#include "fileutils.h"

#include <cstring>

// clang-format off
#ifdef _MSC_VER
	#include "direntw32.h"
#else
	#include <dirent.h>
#endif

#ifdef _WIN32
	#include <fileapi.h>
#else
	#include <stdlib.h>
#endif
// clang-format on

#define MAX_PATH_SIZE 260

namespace nap
{

	bool listDir(const char* directory, std::vector<std::string>& outFilenames)
	{
		DIR* dir;
		struct dirent* ent;
		if ((dir = opendir(directory)) == nullptr) return false;

		while ((ent = readdir(dir)) != nullptr) {
			if (!strcmp(ent->d_name, ".")) continue;
			if (!strcmp(ent->d_name, "..")) continue;

			char buffer[255];
			sprintf(buffer, "%s/%s", directory, ent->d_name);
			outFilenames.push_back(buffer);
		}
		closedir(dir);
		return true;
	}

	std::string getAbsolutePath(const std::string& relPath)
	{
#ifdef _WIN32
		TCHAR path[MAX_PATH_SIZE];
		TCHAR** filenameComponent = nullptr;
#ifdef _MSC_VER
		GetFullPathName((LPCWSTR) relPath.c_str(), MAX_PATH_SIZE, path, filenameComponent);
#else
		GetFullPathName((LPCSTR) relPath.c_str(), MAX_PATH_SIZE, path, filenameComponent);
#endif
		return std::string((char*)path);
#else
		char resolved[MAX_PATH_SIZE];
		realpath(relPath.c_str(), resolved);
		return std::string(resolved);
#endif
	}

	// Lelijk
	std::string getFileExtension(const std::string &filename) {
		return filename.substr(filename.find_last_of(".") + 1);
	}


}
