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
			CmdLine							command					("LicenseGenerator");
			ValueArg<std::string>			output_directory		("o",  "outdir", "Output directory (absolute)", true, "", "path_to_output_directory");
			ValueArg<std::string>			private_key				("k",  "key", "Private key", true, "", "path_to_private_key_file");
			ValueArg<std::string>			signature				("s",  "signature", "Application signature", true, "", "application_signature");
			ValueArg<std::string>			first_name				("f",  "first_name", "First name", true, "", "first_name");
			ValueArg<std::string>			last_name				("l",  "last_name", "Last name", true, "", "last_name");
			ValueArg<std::string>			mail					("m",  "mail", "Client mail", false, "", "client_mail");
			ValueArg<std::string>			date					("d",  "date", "Date", false, "", "license_expiry_date, for_example: '30/12/2021'");

			command.add(output_directory);
			command.add(private_key);
			command.add(signature);
			command.add(first_name);
			command.add(last_name);
			command.add(mail);
			command.add(date);
			command.parse(argc, argv);

			commandLine.mOutputDirectory = output_directory.getValue();
			commandLine.mKey  = private_key.getValue();
			commandLine.mSignature = signature.getValue();
			commandLine.mFistName = first_name.getValue();
			commandLine.mLastName = last_name.getValue();
			commandLine.mMail = mail.getValue();
			commandLine.mDate = date.getValue();
		}
		catch (ArgException& e)
		{
			std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
			return false;
		}
		return true;
	}

	std::string					mOutputDirectory;
	std::string					mKey;
	std::string					mMail;
	std::string					mFistName;
	std::string					mLastName;
	std::string					mDate;
	std::string					mSignature;
};
