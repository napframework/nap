#include "FlexBlockComponent.h"
#include "rtti/jsonreader.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>
#include <math.h>

// json
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>
#include <utility/fileutils.h>

#include <glm/geometric.hpp>

// nap::FlexBlockComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockComponent)
	RTTI_PROPERTY("ControlPointsMesh", &nap::FlexBlockComponent::mControlPointsMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameMesh", &nap::FlexBlockComponent::mFrameMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlexBlockMesh", &nap::FlexBlockComponent::mFlexBlockMesh, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::FlexBlockComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexBlockComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void FlexBlockComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool FlexBlockComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexBlockComponent* resource = getComponent<FlexBlockComponent>();

		// assign meshes
		mControlPointsMesh = resource->mControlPointsMesh;
		mFlexBlockMesh = resource->mFlexBlockMesh;
		mFrameMesh = resource->mFrameMesh;


		// Read & parse json files
		// TODO : handle errors...
		readShapes();
		readSizes();

		// TODO : define this somewhere else
		frequency = 200;
		objshape = shapes[0];
		objsize = sizes[0];
		countInputs = objshape.inputs;

		//
		force_object = 10;
		force_object_spring = 0.02;
		force_object2frame = 2;
		change_speed = 1;

		maxacc = 2.0 / frequency;
		maxspeed = 5.0 / frequency;

		lengtherror = 0;

		// points
		points_object = objshape.points.object;
		points_frame = objshape.points.frame;

		// elements
		elements_object = objshape.elements.object;
		elements_object2frame = objshape.elements.object2frame;
		elements_frame = objshape.elements.frame;

		// convert zero indexed elements
		for (int i = 0; i < elements_object2frame.size(); i++)
		{
			elements_object2frame[i][1] += points_object.size();
		}
		for (int i = 0; i < elements_frame.size(); i++)
		{
			elements_frame[i][0] += points_object.size();
			elements_frame[i][1] += points_object.size();
		}

		// convert unit points to real size
		for (int i = 0; i < points_object.size(); i++)
		{
			points_object[i].x *= objsize.values.object.x * 0.5f;
			points_object[i].y *= objsize.values.object.y * 0.5f;
			points_object[i].z *= objsize.values.object.z * 0.5f;
		}
		for (int i = 0; i < points_frame.size(); i++)
		{
			points_frame[i].x *= objsize.values.frame.x * 0.5f;
			points_frame[i].y *= objsize.values.frame.y * 0.5f;
			points_frame[i].z *= objsize.values.frame.z * 0.5f;
		}

		// init
		points = std::vector<glm::vec3>(points_object.size() + points_frame.size());
		elements = std::vector<std::vector<int>>(elements_object.size() + elements_object2frame.size());
		for (int i = 0; i < elements.size(); i++)
		{
			elements[i] = std::vector<int>(2);
		}

		elements_all = std::vector<std::vector<int>>(elements_object.size() + elements_object2frame.size() + elements_frame.size());
		elements_vector = std::vector<glm::vec3>(elements.size());
		elements_length = std::vector<float>(elements.size());
		elements_length_ref = std::vector<float>(elements.size());
		elements_object_length = std::vector<float>(elements.size());
		elements_input = std::vector<float>(countInputs);
		point_change = std::vector<glm::vec3>(points_object.size());
		point_change_corr = std::vector<glm::vec3>(points_object.size());
		element_indices = std::vector<int>(2);
		element_indices[0] = elements_object.size();
		element_indices[1] = elements_object.size() + elements_object2frame.size();
		override = std::vector<float>(4);

		// concat points
		concatPoints();

		// concat elements
		concatElements();

		// calc elements
		calcElements();

		elements_length_ref = std::vector<float>(elements_length.size());
		for (int i = 0 ; i < elements_length.size(); i++)
		{
			elements_length_ref[i] = elements_length[i];
		}

		// calc input
		calcInput();

		// init control points
		auto box = mFlexBlockMesh->getBox();
		auto min = box.getMin();
		auto max = box.getMax();

		points_object = std::vector<glm::vec3>(8, glm::vec3(0, 0, 0));

		points_object[0] = { min.x, min.y, max.z };	//< Front Lower left
		points_object[1] = { max.x, min.y, max.z };	//< Front Lower right
		points_object[2] = { min.x, max.y, max.z };	//< Front Top left
		points_object[3] = { max.x, max.y, max.z };	//< Front Top right

		points_object[4] = { max.x, min.y, min.z };	//< Back Lower left
		points_object[5] = { min.x, min.y, min.z };	//< Back lower right
		points_object[6] = { max.x, max.y, min.z };	//< Back Top left
		points_object[7] = { min.x, max.y, min.z };	//< Back Top right

		// init frame points
		auto frame = mFrameMesh->getBox();
		min = frame.getMin();
		max = frame.getMax();

		points_frame = std::vector<glm::vec3>(8, glm::vec3(0, 0, 0));

		points_frame[0] = { min.x, min.y, max.z };	//< Front Lower left
		points_frame[1] = { max.x, min.y, max.z };	//< Front Lower right
		points_frame[2] = { min.x, max.y, max.z };	//< Front Top left
		points_frame[3] = { max.x, max.y, max.z };	//< Front Top right

		points_frame[4] = { max.x, min.y, min.z };	//< Back Lower left
		points_frame[5] = { min.x, min.y, min.z };	//< Back lower right
		points_frame[6] = { max.x, max.y, min.z };	//< Back Top left
		points_frame[7] = { min.x, max.y, min.z };	//< Back Top right

		return true;
	}

	void FlexBlockComponentInstance::readSizes()
	{
		utility::ErrorState errorState;
		std::string path = "sizes.json";
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
			FlexblockSize size;

			assert(document[i].HasMember("name") && document[i]["name"].IsString());
			size.name = document[i]["name"].GetString();

			assert(document[i].HasMember("values"));
			assert(document[i]["values"].HasMember("object"));

			size.values.object = glm::vec3(
				document[i]["values"]["object"]["x"].GetFloat(),
				document[i]["values"]["object"]["y"].GetFloat(),
				document[i]["values"]["object"]["z"].GetFloat()
			);

			assert(document[i]["values"].HasMember("frame"));
			size.values.frame = glm::vec3(
				document[i]["values"]["frame"]["x"].GetFloat(),
				document[i]["values"]["frame"]["y"].GetFloat(),
				document[i]["values"]["frame"]["z"].GetFloat()
			);

			sizes.push_back(size);
		}
	}

	void FlexBlockComponentInstance::readShapes()
	{
		utility::ErrorState errorState;
		std::string path = "shapes.json";
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
				FlexblockShape shape;

				//
				assert(document[i].HasMember("name"));
				shape.name = document[i]["name"].GetString();

				//
				assert(document[i].HasMember("inputs"));
				shape.inputs = document[i]["inputs"].GetInt();

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
						sizes[j]["values"]["object"]["z"].GetFloat() );

					size.values.frame = glm::vec3(
						sizes[j]["values"]["frame"]["x"].GetFloat(),
						sizes[j]["values"]["frame"]["y"].GetFloat(),
						sizes[j]["values"]["frame"]["z"].GetFloat());

					shape.sizes.push_back(size);
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
					shape.elements.object.push_back(p);
				}

				assert(docElements.HasMember("object2frame"));
				assert(docElements["object2frame"].IsArray());

				for (rapidjson::SizeType k = 0; k < docElements["object2frame"].Size(); k++)
				{
					std::vector<int> p = std::vector<int>();
					p.push_back(docElements["object2frame"][k][0].GetInt());
					p.push_back(docElements["object2frame"][k][1].GetInt());
					shape.elements.object2frame.push_back(p);
				}

				assert(docElements.HasMember("frame"));
				assert(docElements["frame"].IsArray());

				for (rapidjson::SizeType k = 0; k < docElements["frame"].Size(); k++)
				{
					std::vector<int> p = std::vector<int>();
					p.push_back(docElements["frame"][k][0].GetInt());
					p.push_back(docElements["frame"][k][1].GetInt());
					shape.elements.frame.push_back(p);
				}

				// points
				assert( document[i].HasMember("points") );
				
				// points
				auto& docPoints = document[i]["points"];
				assert(docPoints.HasMember("object"));
				assert(docPoints["object"].IsArray());
				for (rapidjson::SizeType j = 0; j < docPoints["object"].Size(); j++)
				{
					shape.points.object.push_back(
						glm::vec3(
							docPoints["object"][j][0].GetFloat(), 
							docPoints["object"][j][1].GetFloat(),
							docPoints["object"][j][2].GetFloat()));
				}

				assert(docPoints["frame"].IsArray());
				for (rapidjson::SizeType j = 0; j < docPoints["frame"].Size(); j++)
				{
					shape.points.frame.push_back(
						glm::vec3(
							docPoints["frame"][j][0].GetFloat(),
							docPoints["frame"][j][1].GetFloat(),
							docPoints["frame"][j][2].GetFloat()));
				}

				shapes.push_back(shape);
			}
		}
	}

	void FlexBlockComponentInstance::SetControlPoint(int index, glm::vec3 position)
	{
		//
		points_object[index] = position;

		// update the control points mesh
		mControlPointsMesh->setControlPoints(points_object);

		// update ropes of frame
		mFrameMesh->setControlPoints(points_object);

		// update the box
		mFlexBlockMesh->setControlPoints(points_object);
	}

	glm::vec3 FlexBlockComponentInstance::GetControlPoint(int index)
	{
		return points_object[index];
	}

	void FlexBlockComponentInstance::update(double deltaTime)
	{
		// calculate points
		// this is where the magic happens

		starttime += deltaTime * 1000.0;

		calcInput();

		for (int i = 0; i < points_object.size(); i++)
		{
			// init
			point_force = glm::vec3(0, 0, 0);
			point_force_corr = glm::vec3(0, 0, 0);

			// external forces
			std::vector<int> suspension_element_ids = get_ids_of_suspension_elements_on_point(i);
			
			for (int j = 0; j < suspension_element_ids.size(); j++)
			{
				point_force += get_suspension_force_on_point_of_element(suspension_element_ids[i], i);
			}
		
			// Internal forces + suspension forces on the other side of connected elements
			// Get the connected elements(both directions) :

			std::vector<int> object_element_ids_0;
			for (int j = 0; j < elements_object.size(); j++)
			{
				if (elements_object[j][0] == i)
				{
					object_element_ids_0.push_back(j);
				}
			}

			std::vector<int> object_element_ids_1;
			for (int j = 0; j < elements_object.size(); j++)
			{
				if (elements_object[j][1] == i)
				{
					object_element_ids_1.push_back(j);
				}
			}

			// Forces due to external force on other points
			for (int j = 0; j < object_element_ids_0.size(); j++)
			{
				point_force += get_projected_suspension_forces_on_opposite_point_of_element(object_element_ids_0[j], 1);
			}

			// stayed at run function line 176
		}
	}

	glm::vec3 FlexBlockComponentInstance::get_projected_suspension_forces_on_opposite_point_of_element(int object_element_id, int opposite_column)
	{
		// Predicted force due to force on other point
		int opposite_point = elements_object[object_element_id][opposite_column];
		auto suspension_element_ids = get_ids_of_suspension_elements_on_point(opposite_point);

		int suspension_element_id = suspension_element_ids[0];

		// stopped here...

		//get_projected_suspension_force_on_opposite_point_of_element(object_element_id, suspension_element_id, opposite_point)
		return{ 0.0f, 0.0f, 0.0f };
	}

	//glm::vec3 FlexBlockComponentInstance::get_projected_suspension_force_on_opposite_point_of_element

	glm::vec3 FlexBlockComponentInstance::get_suspension_force_on_point_of_element(int elidx, int point)
	{
		return elements_length[elidx] * elements_vector[elidx] * elements_input[point];
	}

	std::vector<int> FlexBlockComponentInstance::get_ids_of_suspension_elements_on_point(int point_id)
	{
		std::vector<int> returnVector;
		for (int i = 0; i < elements_object2frame.size(); i++)
		{
			for (int j = 0; j < elements_object2frame[i].size(); j++)
			{
				if (elements_object2frame[i][j] == point_id)
				{
					returnVector.push_back(j + elements_object.size());
				}
			}
		}
		
		return returnVector;
	}

	void FlexBlockComponentInstance::calcInput()
	{
		elements_length_delta = std::vector<float>(elements_length.size());
		for (int i = 0; i < elements_length_delta.size(); i++)
		{
			elements_length_delta[i] = elements_length[i] - elements_length_ref[i];
		}
	}

	void FlexBlockComponentInstance::calcElements()
	{
		for (int i = 0; i < elements.size(); i++)
		{
			glm::vec3 p = points[elements[i][1]] - points[elements[i][0]];
			elements_vector[i] = p;
		}

		float sum = 0.0f;
		std::vector<float> n_elements_length(elements_length.size());
		for (int i = 0; i < elements_vector.size(); i++)
		{
			n_elements_length[i] = glm::length(elements_vector[i]);
		}

		float n_motorspd = 0.0f;
		float a = 0.0f;
		for (int i = 12; i < 19; i++)
		{
			float n_a = math::abs(elements_length[i] - n_elements_length[i]);
			if (n_a > a)
				a = n_a;
		}
		n_motorspd = a * frequency;

		motoracc = (motorspd - n_motorspd) * frequency;

		elements_length = n_elements_length;
		for (int i = 0; i < elements_vector.size(); i++)
		{
			elements_vector[i] /= elements_length[i];
		}
	}

	void FlexBlockComponentInstance::concatElements()
	{
		// elements
		std::vector<std::vector<int>> newElements;
		for (int i = 0; i < elements_object.size(); i++)
		{
			newElements.push_back(elements_object[i]);
		}

		for (int i = 0; i < elements_object2frame.size(); i++)
		{
			newElements.push_back(elements_object2frame[i]);
		}
		elements = newElements;

		// elements_all
		std::vector<std::vector<int>> newElementsAll;
		for (int i = 0; i < elements_object.size(); i++)
		{
			newElementsAll.push_back(elements_object[i]);
		}

		for (int i = 0; i < elements_object2frame.size(); i++)
		{
			newElementsAll.push_back(elements_object2frame[i]);
		}

		for (int i = 0; i < elements_frame.size(); i++)
		{
			newElementsAll.push_back(elements_frame[i]);
		}
		elements_all = newElementsAll;
	}

	void FlexBlockComponentInstance::concatPoints()
	{
		std::vector<glm::vec3> newPoints;
		for (int i = 0; i < points_object.size(); i++)
		{
			newPoints.push_back(points_object[i]);
		}

		for (int i = 0; i < points_frame.size(); i++)
		{
			newPoints.push_back(points_frame[i]);
		}

		points = newPoints;
	}
}