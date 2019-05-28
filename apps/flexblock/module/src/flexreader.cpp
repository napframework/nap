#include "flexreader.h"

// json
#include "rtti/jsonreader.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>
#include <utility/fileutils.h>

// glm
#include <glm/geometric.hpp>

namespace nap
{
	std::vector<std::shared_ptr<FlexblockSize>> flexreader::readSizes(std::string filename, utility::ErrorState& errorState)
	{
		std::vector<std::shared_ptr<FlexblockSize>> sizes;

		std::string path = filename;
		std::string buffer;
		utility::readFileToString(path, buffer, errorState);

		rapidjson::Document document;
		rapidjson::ParseResult parseResult = document.Parse(buffer.c_str());
		if (!parseResult)
		{
			// TODO: handle parse result
			printf("Error reading json\n");
		}

		assert(document.IsArray());
		for (rapidjson::SizeType i = 0; i < document.Size(); i++)
		{
			std::shared_ptr<FlexblockSize> size = std::make_shared<FlexblockSize>();

			assert(document[i].HasMember("name") && document[i]["name"].IsString());
			size->name = document[i]["name"].GetString();

			assert(document[i].HasMember("values"));
			assert(document[i]["values"].HasMember("object"));

			size->values.object = glm::vec3(
				document[i]["values"]["object"]["x"].GetFloat(),
				document[i]["values"]["object"]["y"].GetFloat(),
				document[i]["values"]["object"]["z"].GetFloat()
			);

			assert(document[i]["values"].HasMember("frame"));
			size->values.frame = glm::vec3(
				document[i]["values"]["frame"]["x"].GetFloat(),
				document[i]["values"]["frame"]["y"].GetFloat(),
				document[i]["values"]["frame"]["z"].GetFloat()
			);

			sizes.push_back(size);
		}

		return sizes;
	}

	std::vector<std::shared_ptr<FlexblockShape>> flexreader::readShapes(std::string filename, utility::ErrorState& errorState)
	{
		std::vector<std::shared_ptr<FlexblockShape>> shapes;

		std::string path = filename;
		std::string buffer;
		utility::readFileToString(path, buffer, errorState);

		rapidjson::Document document;
		rapidjson::ParseResult parseResult = document.Parse(buffer.c_str());
		if (!parseResult)
		{
			// TODO: handle parse result
			printf("Error reading json\n");
		}

		if (document.IsArray())
		{
			for (rapidjson::SizeType i = 0; i < document.Size(); i++)
			{
				std::shared_ptr<FlexblockShape> shape = std::make_shared<FlexblockShape>();

				//
				assert(document[i].HasMember("name"));
				shape->name = document[i]["name"].GetString();

				//
				assert(document[i].HasMember("inputs"));
				shape->inputs = document[i]["inputs"].GetInt();

				// sizes
				assert(document[i].HasMember("sizes"));

				//
				auto& sizes = document[i]["sizes"];
				for (rapidjson::SizeType j = 0; j < sizes.Size(); j++)
				{
					FlexblockShapeSize size;

					assert(sizes[j].HasMember("name"));
					size.name = sizes[j]["name"].GetString();

					assert(sizes[j].HasMember("values"));
					assert(sizes[j]["values"].HasMember("object"));
					assert(sizes[j]["values"].HasMember("frame"));

					size.values.object = glm::vec3(
						sizes[j]["values"]["object"]["x"].GetFloat(),
						sizes[j]["values"]["object"]["y"].GetFloat(),
						sizes[j]["values"]["object"]["z"].GetFloat());

					size.values.frame = glm::vec3(
						sizes[j]["values"]["frame"]["x"].GetFloat(),
						sizes[j]["values"]["frame"]["y"].GetFloat(),
						sizes[j]["values"]["frame"]["z"].GetFloat());

					shape->sizes.push_back(size);
				}

				// elements
				assert(document[i].HasMember("elements"));
				auto& docElements = document[i]["elements"];

				assert(docElements.HasMember("object"));
				assert(docElements["object"].IsArray());

				for (rapidjson::SizeType k = 0; k < docElements["object"].Size(); k++)
				{
					std::vector<int> p = std::vector<int>();
					p.push_back(docElements["object"][k][0].GetInt());
					p.push_back(docElements["object"][k][1].GetInt());
					shape->elements.object.push_back(p);
				}

				assert(docElements.HasMember("object2frame"));
				assert(docElements["object2frame"].IsArray());

				for (rapidjson::SizeType k = 0; k < docElements["object2frame"].Size(); k++)
				{
					std::vector<int> p = std::vector<int>();
					p.push_back(docElements["object2frame"][k][0].GetInt());
					p.push_back(docElements["object2frame"][k][1].GetInt());
					shape->elements.object2frame.push_back(p);
				}

				assert(docElements.HasMember("frame"));
				assert(docElements["frame"].IsArray());

				for (rapidjson::SizeType k = 0; k < docElements["frame"].Size(); k++)
				{
					std::vector<int> p = std::vector<int>();
					p.push_back(docElements["frame"][k][0].GetInt());
					p.push_back(docElements["frame"][k][1].GetInt());
					shape->elements.frame.push_back(p);
				}

				// points
				assert(document[i].HasMember("points"));

				// points
				auto& docPoints = document[i]["points"];
				assert(docPoints.HasMember("object"));
				assert(docPoints["object"].IsArray());
				for (rapidjson::SizeType j = 0; j < docPoints["object"].Size(); j++)
				{
					shape->points.object.push_back(
						glm::vec3(
							docPoints["object"][j][0].GetFloat(),
							docPoints["object"][j][1].GetFloat(),
							docPoints["object"][j][2].GetFloat()));
				}

				assert(docPoints["frame"].IsArray());
				for (rapidjson::SizeType j = 0; j < docPoints["frame"].Size(); j++)
				{
					shape->points.frame.push_back(
						glm::vec3(
							docPoints["frame"][j][0].GetFloat(),
							docPoints["frame"][j][1].GetFloat(),
							docPoints["frame"][j][2].GetFloat()));
				}

				shapes.push_back(shape);
			}
		}

		return shapes;
	}
}