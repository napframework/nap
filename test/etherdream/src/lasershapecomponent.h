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
		float	mSize = 1.0f;						//< Size of the shape
		float	mBrightness = 1.0f;					//< Brightness of the shape
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
		LaserShapeComponentInstance(EntityInstance& entity, Component& resource) : 
			ComponentInstance(entity, resource)
		{ 
		}

		// Initialize the component
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Copied shape properties
		LaseShapeProperties mShapeProperties;

		// Update global time
		virtual void update(double deltaTime) override;

		// Returns the current pointer to points
		std::vector<EtherDreamPoint>& getPoints()				{ return mPoints; }

		// Return a copy of the current points
		std::vector<EtherDreamPoint> getPointCopy() const		{ return mPoints; }

		uint getCount() const									{ return static_cast<uint>(mPoints.size()); }

	protected:
		// All the points associates with this shape, created on initialization
		std::vector<EtherDreamPoint> mPoints;
		
		// Current time, scaled by speed
		float mCurrentTime = 0.0f;
	};
}
