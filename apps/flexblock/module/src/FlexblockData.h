#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>
#include <stddef.h>
#include <string>
#include <vector>
#include <memory>

namespace nap
{
	class NAPAPI FlexBlockShapeSizeValues : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapeSizeValues();

		virtual bool init(utility::ErrorState& errorState) override;

		glm::vec3 mObject;
		glm::vec3 mFrame;
	};

	class NAPAPI FlexBlockShapeSize : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapeSize();

		virtual bool init(utility::ErrorState& errorState) override;

		std::string								mName;
		ResourcePtr<FlexBlockShapeSizeValues>	mValues;
	};

	class NAPAPI FlexBlockShapePoints : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapePoints();

		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<glm::vec3> mObject;
		std::vector<glm::vec3> mFrame;
	};

	class NAPAPI FlexBlockElements : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockElements();

		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<std::vector<int>> mObject;
		std::vector<std::vector<int>> mObject2Frame;
		std::vector<std::vector<int>> mFrame;
	};

	class NAPAPI FlexBlockShape : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShape();

		virtual bool init(utility::ErrorState& errorState) override;

		std::string										mName;
		int												mInputs;
		std::vector<ResourcePtr<FlexBlockShapeSize>>	mSizes;
		ResourcePtr<FlexBlockElements>					mElements;
		ResourcePtr<FlexBlockShapePoints>				mPoints;
	};
}
