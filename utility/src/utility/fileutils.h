/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "dllexport.h"
#include "errorstate.h"

// External Includes
#include <string>
#include <vector>

namespace nap
{
	namespace utility
	{
		namespace path
		{
#ifdef _WIN32
			inline constexpr const char* separator = "\\";
#else
			inline constexpr const char* separator = "/";
#endif
		}

		/**
		* List all files in a directory
		* @param directory The directory to search in
		* @param outFilenames A vector of files to populate with filenames
		* @param absolute Whether to populate absolute filenames
		* @return False on failure
		*/
		bool listDir(const char* directory, std::vector<std::string>& outFilenames, bool absolute=true);

		/**
		 * Check if the given path is absolute
		 * @param path The path to check
		 * @return True if the path is absolute, false otherwise
		 */
		bool isAbsolutePath(const std::string& path);

		/**
		 * Given a relative path, return an absolute path.
		 * @param relPath The path to convert
		 * @return Absolute file path, empty string if path can't be resolved.
		 */
		std::string getAbsolutePath(const std::string& relPath);

		/**
		 * Return the extension of the given filename. Eg. "my.directory/myFile.tar.gz" -> "gz"
		 * @param filename The filename to get the extension from
		 * @return The file extension without the dot
		 */
		std::string getFileExtension(const std::string& filename);

		/**
		 * @param file the file to extract the name frame
		 * @return the name of the given file with extension, empty string if file has no extension
		 */
		std::string getFileName(const std::string& file);

		/**
		 * @param file the filename
		 * @return the directory of the given file
		 */
		std::string getFileDir(const std::string& file);

		/**
		 * @return file name without extension
		 * @param file path that is stripped
		 */
		std::string getFileNameWithoutExtension(const std::string& file);

		/**
		 * @param file the file to strip the extension from
		 */
		void stripFileExtension(std::string& file);

		/**
		 * @return file without extension
		 * @param file the file to string the extension from
		 */
		std::string stripFileExtension(const std::string& file);

		/**
		 * @return file with appended extension
		 * @param file the file to add extension to
		 * @param ext extension without preceding '.'
		 */
		std::string appendFileExtension(const std::string& file, const std::string& ext);

		/**
		 * @return if the file has the associated file extension
		 * @param file the file to check extension for
		 * @param extension the file extension without preceding '.'
		 */
		bool hasExtension(const std::string& file, const std::string& extension);

		/**
		 * Check whether a file exists or not.
		 * @param filename The absolute or relative file path to check for
		 * @return true if the file exists, false otherwise
		 */
		bool fileExists(const std::string& filename);

		/**
		 * Check if a directory exists or not
		 * @param dirName name of the directory to check for
		 * @return if the directory exists
		 */
		bool dirExists(const std::string& dirName);

		/**
		 * Makes sure the given directory exists,
		 * attempts to create the directory if it does not exist, recursively
		 * @param dirName name of the directory to check for
		 * @return if directory exists
		 */
		bool ensureDirExists(const std::string& dirName);

		/**
		 * Attempts to create a directory with the given name, recursively.
		 * Creation will fail if the path is invalid or the directory already exists.
		 * @param directory path to the directory to create, absolute or relative.
		 * @return If the directory has been created.
		 */
		bool makeDirs(const std::string& directory);

		/**
		 * Delete the file at the specified path.
		 * @param path The path to the file to delete
		 * @return true on success, false if it failed
		 */
		bool deleteFile(const std::string& path);

		/**
		 * Dump a string to a file.
		 * @param filename The name of the file to write to.
		 * @param contents The string to write
		 */
		void writeStringToFile(const std::string& filename, const std::string& contents);

		/**
		 * TODO: This may well be a platform independent 'toCanonicalFilename' or 'URI'
		 *      as described here: https://en.wikipedia.org/wiki/File_URI_scheme
		 *
		 * @return returns a string that can be used to compare against other filenames. Is also suitable for use as key in map or set.
		 * @param filename: the source filename.
		 */
		std::string toComparableFilename(const std::string& filename);

		/**
		 * @param filenameA filename to compare against filenameB.
		 * @param filenameB filename to comapre against filenameA.
		 * @return return true when file are logically equal (uses toComparableFilename).
		 */
		bool isFilenameEqual(const std::string& filenameA, const std::string& filenameB);

		/**
		 * Get the modification time of the specified path
		 *
		 * @param path The path to the file
		 * @param modTime The modification time of the file
		 * @return Whether file modification time was successfully retrieved
		 */
		bool getFileModificationTime(const std::string& path, uint64_t& modTime);

		/**
		 * @return the full path to the executable including the application name
		 */
		std::string getExecutablePath();

		/**
		 * @return the full path to the executable directory.
		 */
		std::string getExecutableDir();

		/**
		 * Change current working directory
		 *
		 * @param newDir The working directory to change to
		 * @return if working directory changed
		 */
		bool changeDir(const std::string& newDir);

        /**
         * Returns the current working directory
         * @return the current working directory
         */
         std::string getCWD();

		/**
		 * Read the contents of a file into a string.
		 * @param filename Name of the file to read from
		 * @param outBuffer The string in which the contents will be stored
		 * @param err Will contain the error details if there were any problems.
		 * @return True if the read succeeded, false otherwise
		 */
		bool readFileToString(const std::string& filename, std::string& outBuffer, utility::ErrorState& err);

		/**
		 * Find a file in one of the given directories.
		 * @param filename The file to look for, including extension
		 * @param dirs The directories to search in
		 * @return The absolute path to the found file or an empty string if none was found
		 */
		std::string findFileInDirectories(const std::string& filename, const std::vector<std::string>& dirs);

		/**
		 * Join parts path parts using the correct path separator for the current platform
		 */
		std::string joinPath(const std::vector<std::string>& parts);

		/**
		 * Returns a file path with the correct path separator for the current platform.
		 * @param path path name to correct
		 * @return A file path with the correct path separator for the current platform
		 */
		std::string forceSeparator(const std::string& path);
	}
}
