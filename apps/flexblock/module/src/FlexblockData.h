#pragma once

#include <glm/glm.hpp>
#include <stddef.h>
#include <string>
#include <vector>

namespace nap
{
	struct FlexblockShapeSizeValues
	{
	public:
		glm::vec3 object;
		glm::vec3 frame;
	};

	struct FlexblockShapeSize
	{
	public:
		std::string					name;
		FlexblockShapeSizeValues	values;
	};

	struct FlexblockShapePoints
	{
		std::vector<glm::vec3> object;
		std::vector<glm::vec3> frame;
	};

	struct FlexblockElements
	{
		std::vector<std::vector<int>> object;
		std::vector<std::vector<int>> object2frame;
		std::vector<std::vector<int>> frame;
	};

	struct FlexblockShape
	{
	public:
		std::string						name;
		int								inputs;
		std::vector<FlexblockShapeSize> sizes;
		FlexblockElements				elements;
		FlexblockShapePoints			points;
	};

	//
	struct FlexblockSizeValues
	{
	public:
		glm::vec3 object;
		glm::vec3 frame;
	};

	//
	struct FlexblockSize
	{
	public:
		std::string			name;
		FlexblockSizeValues values;
	};
}
