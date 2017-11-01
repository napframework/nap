#include "fileutils.h"
#include "utility/stringutils.h"

#include <cstring>

// clang-format off

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
	#ifdef _MSC_VER
		#include "direntw32.h"
		#include <tchar.h>
		#include <io.h>
		#include <fstream>
		#include <windows.h>
	#else
		#include <fileapi.h>
		#include <dirent.h>
		#include <sys/stat.h>
		#include <fstream>
	#endif
#elif __APPLE__
    #include <stdlib.h>
    #include <zconf.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <fstream>
	#include <unistd.h>
	#include <mach-o/dyld.h>
#else
    #include <zconf.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <fstream>
	#include <unistd.h>
	#include <sstream>
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
    
    
    std::string getFileDir(const std::string& file)
    {
        std::string name = file;

		// Replace any references to self in the path, ie. "/./" on Unix.  This fixes an issue when trying
		// to get the current dir running from command line on OSX.
		utility::replaceAllInstances(name, "/./", "/");
		
        const size_t last_slash_idx = name.find_last_of("/");
        if (last_slash_idx != std::string::npos)
            name = name.erase(last_slash_idx, name.size() - last_slash_idx);
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
		return utility::gToLower(getFileExtension(file)) == utility::gToLower(extension);
	}


	// Checks if the file exists on disk
	bool fileExists(const std::string& filename) 
	{
		struct stat result;
		if (stat(filename.c_str(), &result) != 0)
			return false;

		return (result.st_mode & S_IFMT) != 0;
    }

    
	bool dirExists(const std::string& dirName)
	{
		if (!dirName.empty())
		{
			if (access(dirName.c_str(), 0) == 0)
			{
				struct stat status;
				if (stat(dirName.c_str(), &status) != 0)
					return false;

				if (status.st_mode & S_IFDIR)
					return true;
			}
		}
		// if any condition fails
		return false;
	}

    
    void writeStringToFile(const std::string& filename, const std::string& contents) {
        std::ofstream out(filename);
        out << contents;
        out.close();
    }


	const std::string toComparableFilename(const std::string& filename)
	{
		std::string comparable = filename;
		std::transform(comparable.begin(), comparable.end(), comparable.begin(), ::tolower);
		std::replace(comparable.begin(), comparable.end(), '\\', '/');
		return comparable;
	}


	bool isFilenameEqual(const std::string& filenameA, const std::string& filenameB)
	{
		return toComparableFilename(filenameA) == toComparableFilename(filenameB);
	}


	bool getFileModificationTime(const std::string& path, uint64_t& modTime)
	{
		struct stat result;
		if (stat(path.c_str(), &result) != 0)
			return false;
		
		// Fail if not a file
		if ((result.st_mode & S_IFMT) == 0)
			return false;

		modTime = result.st_mtime;
		return true;
	}


	std::string getExecutablePath()
	{
		std::string out_path;
		unsigned int bufferSize = 512;
		std::vector<char> buffer(bufferSize + 1);

#if defined(_WIN32)
		::GetModuleFileName(NULL, &buffer[0], bufferSize);

#elif defined(__linux__)
		// Construct a path to the symbolic link pointing to the process executable.
		// This is at /proc/<pid>/exe on Linux systems (we hope).
		int pid = getpid();
		std::ostringstream oss;
		oss << "/proc/" << pid << "/exe";
		std::string link = oss.str();

		// Read the contents of the link.
		int count = readlink(link.c_str(), &buffer[0], bufferSize);
		if (count == -1) throw std::runtime_error("Could not read symbolic link");
		buffer[count] = '\0';

#elif defined(__APPLE__)
		if (_NSGetExecutablePath(&buffer[0], &bufferSize))
		{
			buffer.resize(bufferSize);
			_NSGetExecutablePath(&buffer[0], &bufferSize);
		}
#else
	#error Cannot yet find the executable on this platform
#endif
		out_path = &buffer[0];
		
#if defined(_WIN32)
		// Replace any Windows-style backslashes with forward slashes
		utility::replaceAllInstances(out_path, "\\", "/");
#endif
		
		return out_path;
	}


	std::string getExecutableDir()
	{
		return getFileDir(getExecutablePath());
	}
}
