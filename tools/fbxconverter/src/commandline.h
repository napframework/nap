/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/fileutils.h>
#undef HAVE_LONG_LONG
#undef HAVE_CONFIG_H
#include <tclap/CmdLine.h>

/**
 * Class to parse the commandline and store the parsed output
 */
class CommandLine
{
public:
	/**
	 * Parse the commandline and output a CommandLine object
	 *
	 * @param argc Number of arguments on the commandline
	 * @param argv Array of arguments on the commandline
	 * @param commandLine The resulting commandline
	 *
	 * @return Whether parsing succeeded or not
	 */
	static bool parse(int argc, char** argv, CommandLine& commandLine)
	{
		using namespace TCLAP;
		try
		{
			CmdLine							command					("FBXConverter");
			ValueArg<std::string>			output_directory		("o", "outdir", "Output directory to convert to (absolute or relative path)", true, "", "path_to_output_directory");
			SwitchArg						force_convert			("f", "force", "Force the files to be converter, even if nothing has changed");
			UnlabeledMultiArg<std::string>	files					("files", "List of .fbx files to convert", true, "list_of_fbx_files");

			command.add(output_directory);
			command.add(force_convert);
			command.add(files);

			command.parse(argc, argv);

			commandLine.mOutputDirectory = nap::utility::getAbsolutePath(output_directory.getValue());
			commandLine.mFilesToConvert = files.getValue();
			commandLine.mForceConvert = force_convert.getValue();
		}
		catch (ArgException& e)
		{
			std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
			return false;
		}

		return true;
	}

	std::string					mOutputDirectory;
	std::vector<std::string>	mFilesToConvert;
	bool						mForceConvert;
};
