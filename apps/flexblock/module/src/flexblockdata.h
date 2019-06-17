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
	/**
	* FlexBlockShapeSizeValues
	* Contains information about the size of this shape and its frame
	*/
	class NAPAPI FlexBlockShapeSizeValues : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapeSizeValues();

		virtual bool init(utility::ErrorState& errorState) override;

		glm::vec3 mObject;
		glm::vec3 mFrame;
	};

	/**
	* FlexBlockShapePoints
	* A description of the size of the shape ( will be multiplied with FlexBlockShapePoints in flex algorithm )
	*/
	class NAPAPI FlexBlockShapeSize : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapeSize();

		virtual bool init(utility::ErrorState& errorState) override;

		std::string								mName;
		ResourcePtr<FlexBlockShapeSizeValues>	mValues;
	};

	/**
	* FlexBlockShapePoints
	* A description of how the shape is constructed
	*/
	class NAPAPI FlexBlockShapePoints : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapePoints();

		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<glm::vec3> mObject;
		std::vector<glm::vec3> mFrame;
	};

	/**
	* FlexBlockElements
	* Contains lists of indices, indicating how corners of the frame, object are connected with themselves and eachother
	*/
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

	/**
	 * FlexBlockShape
	 * Contains the information needed to create and render a flexblock shape and execute the flex algorithm
	 */
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
