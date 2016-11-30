#pragma once


#include <string>
#include <vector>

namespace nap
{
    /**
     * List all files in a directory
     * @param directory The directory to search in
     * @param outFilenames A vector of files to populate with absolute filenames
     * @return False on faillure
     */
	bool listDir(const char* directory, std::vector<std::string>& outFilenames);

    /**
     * Given a relative path, return an absolute path
     * @param relPath The path to convert
     * @return An absolute file path
     */
	std::string getAbsolutePath(const std::string& relPath);

    /**
     * Return the extension of the given filename. Eg. "my.directory/myFile.tar.gz" -> "gz"
     * @param filename The filename to get the extension from
     * @return The file extension without the dot
     */
	std::string getFileExtension(const std::string& filename);

    /**
     * Check whether a file exists or not.
     * @param filename The absolute or relative file path to check for
     * @return true if the file exists, false otherwise
     */
    bool fileExists(const std::string& filename);
}