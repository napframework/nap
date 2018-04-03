#include "projectinfomanager.h"
#include "logger.h"

#include <utility/fileutils.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>


namespace nap
{	
	const std::string possibleProjectParents[] =
	{
		"projects", // User projects against packaged NAP
		"examples", // Example projects
		"demos", // Demo projects
		"apps", // Applications in NAP source
		"test" // Old test projects in NAP source
	};

	int getLine(const std::string& json, size_t offset)
	{
		int line = 1;
		int line_offset = 0;
		while (true)
		{
			line_offset = json.find('\n', line_offset);
			if (line_offset == std::string::npos || line_offset > offset)
				break;
			++line;
			line_offset += 1;
		}
		return line;
	}
	
	bool deserializeProjectInfoJSON(const std::string& json, ProjectInfo& result, utility::ErrorState& errorState)
	{
		// Try to parse the json file
		rapidjson::Document document;
		rapidjson::ParseResult parse_result = document.Parse(json.c_str());
		if (!parse_result)
		{
			errorState.fail("Error parsing json: %s (line: %d)", rapidjson::GetParseError_En(parse_result.Code()), getLine(json, parse_result.Offset()));
			return false;
		}
		
		// Basic validation
		rapidjson::Value::ConstMemberIterator modules = document.FindMember("modules");
		if (!errorState.check(modules != document.MemberEnd(), "Unable to find required 'modules' project info field"))
			return false;
		
		if (!errorState.check(modules->value.IsArray(), "'modules' project info field must be an array"))
			return false;

		if (!errorState.check(document.HasMember("title"), "Unable to find required 'title' project info field"))
			return false;
		
		if (!errorState.check(document["title"].IsString(), "'version' project info field must be a string"))
			return false;

		if (!errorState.check(document.HasMember("version"), "Unable to find required 'version' project info field"))
			return false;

		if (!errorState.check(document["version"].IsString(), "'version' project info field must be a string"))
			return false;
	
		// Populate into info struct and return
		if (!result.mModules.empty())
			result.mModules.clear();
		
		for (std::size_t index = 0; index < modules->value.Size(); ++index)
		{
			const rapidjson::Value& json_element = modules->value[index];
			if (!errorState.check(json_element.IsString(), "Entries in 'modules' array in project info field must be an strings"))
				return false;
			
			result.mModules.push_back(json_element.GetString());
		}
		
		result.mTitle = document["title"].GetString();
		result.mVersion = document["version"].GetString();
		
		return true;
	}
	
	bool loadProjectInfoFromJSON(ProjectInfo& result, utility::ErrorState& errorState)
	{
		// Check for our project.json in its normal location, beside the binary
		std::string projectInfoToRead;
		if (utility::fileExists(PROJECT_INFO_FILENAME)) {
			projectInfoToRead = PROJECT_INFO_FILENAME;
		}
		else {
#ifndef NAP_PACKAGED_BUILD
			// When working against NAP source find our project.json in the tree structure in the project source.
			// This is effectively a workaround for wanting to keep all binaries in the same root folder on Windows
			// so that we avoid module DLL copying hell.
			const bool projectInfoFound = findProjectInfoForNonPackagedFramework(projectInfoToRead, errorState);
			if (!projectInfoFound)
				return false;
#else // NAP_PACKAGED_BUILD
			// Fail if we're not running against NAP source and we don't have a project.json
			errorState.fail("project.json not found alongside binary" );
			return false;
#endif // NAP_PACKAGED_BUILD
		}
		
		// Open the file
		std::ifstream in(projectInfoToRead, std::ios::in | std::ios::binary);
		if (!errorState.check(in.good(), "Unable to open file %s", PROJECT_INFO_FILENAME))
			return false;
		
		// Create buffer of appropriate size
		in.seekg(0, std::ios::end);
		size_t len = in.tellg();
		std::string buffer;
		buffer.resize(len);
		
		// Read all data
		in.seekg(0, std::ios::beg);
		in.read(&buffer[0], len);
		in.close();
		
		if (!deserializeProjectInfoJSON(buffer, result, errorState))
			return false;
		
		return true;
	}
	
	bool findProjectInfoForNonPackagedFramework(std::string& projectInfoToRead, utility::ErrorState& errorState)
	{
		const std::string exeDir = utility::getExecutableDir();
		const std::string napRoot = utility::getAbsolutePath(exeDir + "/../../");
		const std::string projectName = utility::getFileNameWithoutExtension(utility::getExecutablePath());
		
		projectInfoToRead = "";
		
		// Iterate possible project locations
		for (auto& parentPath : possibleProjectParents)
		{
			std::string testDataPath = napRoot + "/" + parentPath + "/" + projectName;
			if (utility::dirExists(testDataPath))
			{
				// We found our project folder, now let's verify we have a project.json in there
				testDataPath += "/";
				testDataPath += PROJECT_INFO_FILENAME;
				if (utility::fileExists(testDataPath))
				{
					projectInfoToRead = testDataPath;
					break;
				}
				else 
				{
					errorState.fail("Couldn't find project.json beside binary or in project folder %s", testDataPath.c_str());
					return false;
				}
			}
		}
		
		// Error if we didn't find a source folder for our project
		if (projectInfoToRead == "")
		{
			errorState.fail("Couldn't find source folder for project %s when searching for project.json", projectName.c_str());
			return false;
		}
		
		return true;
	}
}
