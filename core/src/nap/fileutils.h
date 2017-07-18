#pragma once

// External Includes
// External Includes
#include <string>
#include <vector>

// Local Includes
#include "dllexport.h"

namespace nap
{
    /**
     * List all files in a directory
     * @param directory The directory to search in
     * @param outFilenames A vector of files to populate with absolute filenames
     * @return False on faillure
     */
	NAPAPI bool listDir(const char* directory, std::vector<std::string>& outFilenames);

    /**
     * Given a relative path, return an absolute path
     * @param relPath The path to convert
     * @return An absolute file path
     */
	NAPAPI std::string getAbsolutePath(const std::string& relPath);

    /**
     * Return the extension of the given filename. Eg. "my.directory/myFile.tar.gz" -> "gz"
     * @param filename The filename to get the extension from
     * @return The file extension without the dot
     */
	NAPAPI std::string getFileExtension(const std::string& filename);

	/**
	 * @Return the name of the given file with extension, empty string if file has no extension
	 * @param file the file to extract the name frame
	 */
	NAPAPI std::string getFileName(const std::string& file);

    /**
     * @Return the directory of the given file
     * @param file the file to extract the name frame
     */
    NAPAPI std::string getFileDir(const std::string& file);
    
	/**
	* @return file name without extension
	* @param file path that is stripped
	*/
	NAPAPI std::string getFileNameWithoutExtension(const std::string& file);

	/**
	 * @param file the file to strip the extension from
	 */
	NAPAPI void stripFileExtension(std::string& file);

	/**
	 * @return file without extension
	 * @param file the file to string the extension from
	 */
	NAPAPI std::string stripFileExtension(const std::string& file);

	/**
	 * @return file with appended extension
	 * @param file the file to add extension to
	 * @param ext extension without preceding '.'
	 */
	NAPAPI std::string appendFileExtension(const std::string& file, const std::string& ext);

	/**
	 * @return if the file has the associated file extension
	 * @param file the file to check extension for
	 * @param extension the file extension without preceding '.'
	 */
	NAPAPI bool hasExtension(const std::string& file, const std::string& extension);

    /**
     * Check whether a file exists or not.
     * @param filename The absolute or relative file path to check for
     * @return true if the file exists, false otherwise
     */
    NAPAPI bool fileExists(const std::string& filename);

	/**
	 * Check wheter a folder exists or not
	 */
	NAPAPI bool dirExists(const std::string& dirName);

    /**
     * Dump a string to a file.
     * @param filename The name of the file to write to.
     * @param contents The string to write
     */
    NAPAPI void writeStringToFile(const std::string& filename, const std::string& contents);

	/**
	 * TODO: This may well be a platform independent 'toCanonicalFilename' or 'URI'
	 *      as described here: https://en.wikipedia.org/wiki/File_URI_scheme
	 *
	 * @return returns a string that can be used to compare against other filenames. Is also suitable for use as key in map or set.
	 * @param filename: the source filename.
	 */
	NAPAPI const std::string toComparableFilename(const std::string& filename);

	/**
	* @return return true when file are logically equal (uses toComparableFilename).
	* @param fileNameA: filename to compare against filenameB.
	* @param filenameB: filename to comapre against filenameA.
	*/
	NAPAPI bool isFilenameEqual(const std::string& filenameA, const std::string& filenameB);

	/**
	* Get the modification time of the specified path
	*
	* @param path The path to the file
	* @param modTime The modification time of the file
	* @return Whether file modification time was successfully retrieved
	*/
	NAPAPI bool getFileModificationTime(const std::string& path, uint64_t& modTime);	
}
