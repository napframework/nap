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
			ValueArg<std::string>			private_key				("k",  "key", "Path to private key file", true, "", "path_to_private_key_file");
			ValueArg<std::string>			application				("a",  "application", "Application name", true, "", "application_name");
			ValueArg<std::string>			first_name				("f",  "first_name", "First name", true, "", "first_name");
			ValueArg<std::string>			last_name				("l",  "last_name", "Last name", true, "", "last_name");
			ValueArg<std::string>			mail					("m",  "mail", "Client mail address", false, "", "client_mail");
			ValueArg<std::string>			date					("d",  "date", "Expiry date, format: day/month/year -> 30/12/2025", false, "", "expiry_date");
			ValueArg<std::string>			tag						("t",  "tag", "Additional information", false, "", "tag");
			ValueArg<std::string>			uuid					("u",  "uuid", "Generate unique user id", false, "", "uuid");

			command.add(output_directory);
			command.add(private_key);
			command.add(application);
			command.add(first_name);
			command.add(last_name);
			command.add(mail);
			command.add(date);
			command.add(tag);
			command.add(uuid);
			command.parse(argc, argv);

			commandLine.mOutputDirectory = output_directory.getValue();
			commandLine.mKey  = private_key.getValue();
			commandLine.mApplication = application.getValue();
			commandLine.mFistName = first_name.getValue();
			commandLine.mLastName = last_name.getValue();
			commandLine.mMail = mail.getValue();
			commandLine.mDate = date.getValue();
			commandLine.mTag = tag.getValue();
			commandLine.mUuid = uuid.getValue();
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
	std::string					mApplication;
	std::string					mTag;
	std::string					mUuid;
};
