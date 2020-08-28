#include <nap/core.h>
#include <utility/errorstate.h>
#include <nap/logger.h>

#include "fbxconverter.h"
#include "commandline.h"

using namespace nap;

/**
 * Converts all .fbx files in the command line argument to individual .mesh files 
 * Results are stored in the specified output directory
 * Wildcards are allowed, ie: c:\mydir\*.fbx. In that case all .fbx files in 'mydir' are converted
 * Example: fbxconverter.exe -o c:\outdir c:\mydir\*.fbx c:\otherdir\cube.fbx
 */
int main(int argc, char* argv[])
{
	// Parse commandline
	CommandLine commandLine;
	if (!CommandLine::parse(argc, argv, commandLine))
		return -1;
	Logger::setLevel(Logger::debugLevel());

	// Validate all files are fbx files and convert wildcard argument
	std::vector<std::string> files_to_convert;
	for (const std::string& file : commandLine.mFilesToConvert)
	{
		// We should only have fbx files at this point
		if (!utility::endsWith(file, ".fbx"))
		{
			Logger::fatal("Input file %s is not a FBX file", file.c_str());
			return -1;
		}

		// Check if the file to convert contains a wildcard, if so expand
		if (utility::endsWith(file, "*.fbx", false))
		{
			std::string fbx_dir = utility::getFileDir(file);
			std::vector<std::string> outFiles;
			utility::listDir(fbx_dir.c_str(), outFiles, true);
			for (const auto& ffile : outFiles)
			{
				if (utility::endsWith(ffile, ".fbx", false))
					files_to_convert.emplace_back(ffile);
			}
		}
		else
		{
			files_to_convert.emplace_back(file);
		}
	}

	// Determine convert options
	EFBXConversionOptions convert_options = commandLine.mForceConvert ? EFBXConversionOptions::CONVERT_ALWAYS : EFBXConversionOptions::CONVERT_IF_NEWER;

	// Convert files
	for (const std::string& file : files_to_convert)
	{
		Logger::info("Converting %s to %s", file.c_str(), commandLine.mOutputDirectory.c_str());

		std::vector<std::string> converted_files;
		utility::ErrorState convert_result;
		if (!convertFBX(file, commandLine.mOutputDirectory, convert_options, converted_files, convert_result))
		{
			Logger::fatal("\tFailed to convert: %s", convert_result.toString().c_str());
			return -1;
		}
		else
		{
			if (converted_files.empty())
			{
				Logger::info("\t-> All files up to date");
			}
			else
			{
				for (const std::string& converted_file : converted_files)
					Logger::info("\t-> %s", converted_file.c_str());
			}
		}
	}

	return 0;
} 