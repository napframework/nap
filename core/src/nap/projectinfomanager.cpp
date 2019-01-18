#include "projectinfomanager.h"
#include "logger.h"

#include <utility/fileutils.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>
#include <nap/core.h>

namespace nap
{	
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
			if (!errorState.check(json_element.IsString(), "Entries in 'modules' array in project info field must be a strings"))
				return false;
			
			result.mModules.push_back(json_element.GetString());
		}
		
		result.mTitle = document["title"].GetString();
		result.mVersion = document["version"].GetString();
		
		return true;
	}
	
	bool loadProjectInfoFromFile(const Core& core, ProjectInfo& result, utility::ErrorState& errorState) 
	{
		std::string projectFile;
		if (!errorState.check(core.findProjectFilePath(PROJECT_INFO_FILENAME, projectFile), "Couldn't find project.json beside binary or in project folder"))
			return false;

		bool loaded = false;

#ifdef ANDROID		
		// TODO ANDROID Temporary project info loading, likely won't handle files over 1MB, needs error handling, etc
	    AAsset* asset = AAssetManager_open(core.getAndroidAssetManager(), "project.json", AASSET_MODE_UNKNOWN);
	    if (asset != NULL) 
	    {
	        long size = AAsset_getLength(asset);
	        char* buffer = (char*) malloc (sizeof(char)*size);
	        AAsset_read (asset, buffer, size);
	        loaded = deserializeProjectInfoJSON(std::string(buffer, size), result, errorState);
	        AAsset_close(asset);
	    }		
#else
		// Open the file
		std::ifstream in(projectFile, std::ios::in | std::ios::binary);
		loaded = loadProjectInfoFromStream(core, result, errorState, in);
		in.close();
#endif

		return errorState.check(loaded, "Unable to open file %s", utility::getAbsolutePath(projectFile).c_str());
	}

    bool loadProjectInfoFromStream(const Core &core, ProjectInfo &result, utility::ErrorState &errorState,
                         		   std::istream &in) 
    {

        if (!errorState.check(in.good(), "Input in is not good"))
            return false;

        // Create buffer of appropriate size
        in.seekg(0, std::ios::end);
        size_t len = in.tellg();
        std::string buffer;
        buffer.resize(len);

        // Read all data
        in.seekg(0, std::ios::beg);
        in.read(&buffer[0], len);

        return deserializeProjectInfoJSON(buffer, result, errorState);

    }
}
