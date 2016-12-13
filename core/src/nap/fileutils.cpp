#include "fileutils.h"
#include "stringutils.h"

#include <cstring>

// clang-format off

#ifdef _WIN32
	//#include <fileapi.h>
	#ifdef _MSC_VER
		#include "direntw32.h"
		#include <tchar.h>
		#include <io.h>
	#else
		#include <dirent.h>
		#include <sys/stat.h>
	#endif
#elif __APPLE__
    #include <stdlib.h>
    #include <zconf.h>
    #include <sys/stat.h>
#else
	#include <stdlib.h>
    #include <zconf.h>
    #include <sys/stat.h>
    #include <sys/io.h>
#endif

// clang-format on

#define MAX_PATH_SIZE 260

namespace nap
{
	// List all files in a directory
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
		const char* p = relPath.c_str();
		GetFullPathName(_T(p), MAX_PATH_SIZE, path, filenameComponent);
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

	// Return file extension, empty string if file has no extension
	std::string getFileExtension(const std::string &filename) 
	{
		const size_t idx = filename.find_last_of(".");
		if (idx == std::string::npos)
			return "";
		return filename.substr(idx + 1);
	}


	// Return file name with extension for @file
	std::string getFileName(const std::string& file)
	{
		std::string name = file;
		const size_t last_slash_idx = name.find_last_of("\\/");
		if (last_slash_idx != std::string::npos)
			name = name.erase(0, last_slash_idx + 1);
		return name;
	}


	// Returns name of file without extension
	std::string getFileNameWithoutExtension(const std::string& file)
	{
		std::string rfile = getFileName(file);
		stripFileExtension(rfile);
		return rfile;
	}


	// Get file name without extension
	void stripFileExtension(std::string& file)
	{
		const size_t idx = file.find_last_of(".");
		if (idx != std::string::npos)
		{
			size_t length = file.size() - idx;
			file.erase(idx, length);
		}
	}


	// Strip file extension
	std::string stripFileExtension(const std::string& file)
	{
		std::string stripped = file;
		stripFileExtension(stripped);
		return stripped;
	}


	// Simply appends file extension
	std::string appendFileExtension(const std::string& file, const std::string& ext)
	{
		return file + "." + ext;
	}

	// Check if the file has an extension of type extension
	bool hasExtension(const std::string& file, const std::string& extension)
	{
		return gToLower(getFileExtension(file)) == gToLower(extension);
	}


	// Checks if the file exists on disk
	bool fileExists(const std::string& filename) {
        if (FILE *file = fopen(filename.c_str(), "r")) {
            fclose(file);
            return true;
        }
        return false;
    }

	bool dirExists(const std::string& dirName)
	{
		if (!dirName.empty())
		{
			if (access(dirName.c_str(), 0) == 0)
			{
				struct stat status;
				stat(dirName.c_str(), &status);
				if (status.st_mode & S_IFDIR)
					return true;
			}
		}
		// if any condition fails
		return false;
	}

}
