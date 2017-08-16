#pragma once

#include <nap/component.h>
#include <etherdreaminterface.h>
#include <vector>

namespace nap
{
	class LaserShapeComponentInstance;

	/**
	 *	All properties shared by every laser shape
	 */
	struct LaseShapeProperties
	{
		int		mNumberOfPoints = 500;				//< Number of points in the laser shape
		float	mSpeed = 1.0f;						//< Transformation speed
	};


	/**
	 * Base class for simple laser shapes to draw
	 */
	class LaserShapeComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		// Type to instantiate
		virtual const rtti::TypeInfo getInstanceType() const override				
		{ 
			return RTTI_OF(LaserShapeComponentInstance); 
		}

	public:
		// Properties associated with the laser shape
		LaseShapeProperties mShapeProperties;
	};


	/**
	 *	Base instance class of laser shapes to draw
	 */
	class LaserShapeComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LaserShapeComponentInstance(EntityInstance& entity) : ComponentInstance(entity)		{ }

		// Initialize the component
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Copied shape properties
		LaseShapeProperties mShapeProperties;

		// Update global time
		virtual void update(double deltaTime) override;

		// Returns the current pointer to points
		EtherDreamPoint* getPoints()							{ return &(mPoints.front()); }

		uint getCount() const									{ return static_cast<uint>(mPoints.size()); }

	protected:
		// All the points associates with this shape, created on initialization
		std::vector<EtherDreamPoint> mPoints;
		
		// Current time, scaled by speed
		float mCurrentTime = 0.0f;
	};
}
