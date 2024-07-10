/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

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
			CmdLine							command					("Keygen");
			ValueArg<std::string>			output_directory		("o", "outdir", "Output directory (absolute)", true, "", "path_to_output_directory");
			ValueArg<std::string>			seed					("s", "seed", "Random seed", true, "", "random_seed");
			ValueArg<std::string>			name					("n", "name", "Key name", false, "key", "key_name");
            ValueArg<int32_t>			    bits				    ("b", "bits", "Number of bits", false, 4096, "number_of_bits");

			command.add(output_directory);
			command.add(name);
			command.parse(argc, argv);

			commandLine.mOutputDirectory = output_directory.getValue();
			commandLine.mKeyName = name.getValue();
			commandLine.mSeed = seed.getValue();
            commandLine.mBits = bits.getValue();
		}
		catch (ArgException& e)
		{
			std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
			return false;
		}
		return true;
	}

	std::string					mOutputDirectory;
	std::string					mKeyName;
	std::string					mSeed;
    std::int32_t                mBits;
};
