#pragma once

#include <glm/glm.hpp>
#include <stddef.h>
#include <string>
#include <vector>
#include <memory>

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

	class FlexblockShape;
	typedef std::shared_ptr<FlexblockShape> FlexblockShapePtr;

	class FlexblockShape
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

	class FlexblockSize;
	typedef std::shared_ptr<FlexblockSize> FlexblockSizePtr;

	//
	class FlexblockSize
	{
	public:
		std::string			name;
		FlexblockSizeValues values;
	};
}
