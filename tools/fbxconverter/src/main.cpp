#include <nap/core.h>
#include <utility/errorstate.h>
#include <nap/logger.h>

#include "fbxconverter.h"
#include "commandline.h"

using namespace nap;

// Note: we don't need to do any wildcard expansion, since the shell does that for us due to a linker option
// See https://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
int main(int argc, char* argv[])
{
	// Parse commandline
	CommandLine commandLine;
	if (!CommandLine::parse(argc, argv, commandLine))
		return -1;

	Logger::setLevel(Logger::debugLevel());

	// Validate all files are fbx files
	std::vector<std::string> files_to_convert;
	for (const std::string& file : commandLine.mFilesToConvert)
	{
		if (utility::getFileExtension(file) != "fbx")
		{
			Logger::fatal("Input files %s is not a FBX file", file.c_str());
			return -1;
		}
		
		if (utility::getFileName(file) != "*.fbx")
			files_to_convert.push_back(file);
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