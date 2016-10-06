// Local includes
#include <napetherservice.h>

// Core includes
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/entity.h>

// Nap OF includes
#include <napofservice.h>
#include <assert.h>
#include <nap/coremodule.h>

//////////////////////////////////////////////////////////////////////////

static int16_t sEtherInterpolate(float inValue, float inMin, float inMax, bool flip)
{
	return (int16_t)gFit(inValue, inMin, inMax, flip ? gEtherMaxValueFloat : 
		gEtherMinValueFloat, flip ? gEtherMinValueFloat : gEtherMaxValueFloat);
}

static int16_t sEtherInterpolateColor(float inValue)
{
	return (int16_t)ofLerp(0.0f, gEtherMaxValueFloat, inValue);
}

static const ofFloatColor sBlackLaserPoint(0.0f, 0.0f, 0.0f, 1.0f);

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	/**
	@brief Register all ether dream related types
	**/
	void EtherDreamService::sRegisterTypes(nap::Core& inCore, const nap::Service& inService)
	{
		inCore.registerType(inService, RTTI_OF(nap::EtherDreamCamera));
	}

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief Tries to load the dll on construction and runs the service
	**/
	EtherDreamService::EtherDreamService()
	{
		// Setup ether dream and start running
		mEtherdream.Init();
		if (mEtherdream.GetState() != nofNEtherDream::State::EtherDreamFound)
		{
			Logger::warn("Unable to load and start ether dream dac");
			return;
		}
		mEtherdream.Start();
	}



	/**
	@brief Draws the current scene to the laser
	**/
	void EtherDreamService::draw()
	{
		if (mEtherdream.GetState() != nofNEtherDream::State::EtherDreamFound)
			return;

		// We first need to find our laser camera
		std::vector<nap::EtherDreamCamera*> cams;
		getObjects<nap::EtherDreamCamera>(cams);

		// Make sure there's a camera
		if (cams.size() == 0)
		{
			Logger::warn("No laser camera found, skipping laser render step");
			return;
		}

		// Only supports once cam!
		nap::EtherDreamCamera* laser_cam = cams[0];
		if (cams.size() > 1)
		{
			Logger::warn("Multiple laser cams found, picking first one");
		}

		// If we don't have an entity to render, do nothing
		if (!laser_cam->mRenderEntity.isValid())
			return;

		// Make sure it's an entity
		Entity* spline_entity = laser_cam->mRenderEntity.getTarget<Entity>();

		// Component to draw
		nap::OFRenderableComponent* component_to_draw(nullptr);
		const	SplineVertexData* verts(nullptr);
		const	SplineColorData* colors(nullptr);
		bool	isClosed(true);

		if (laser_cam->mTraceMode.getValue())
		{
			nap::OFTraceComponent* trace_component = spline_entity->getComponent<nap::OFTraceComponent>();
			if (trace_component == nullptr)
				return;

			component_to_draw = trace_component;
			verts  = &(trace_component->getVerts());
			colors = &(trace_component->getColors());
			isClosed = false;
		}
		else
		{
			nap::OFSplineComponent* spline_component = spline_entity->getComponent<nap::OFSplineComponent>();
			if (spline_component == nullptr)
				return;

			component_to_draw = spline_component;
			verts  = &(spline_component->mSpline.getValueRef().GetVertexDataRef());
			colors = &(spline_component->mSpline.getValueRef().GetColorDataRef());
			isClosed = spline_component->isClosed();
		}

		// Make sure the size is in range for the laser
		if (verts->size() < mMinPointCount.getValue() || verts->size() > mMaxPointCount.getValue())
		{
			nap::Logger::warn(*this, "invalid point count, exceeds laser range");
			return;
		}

		// Get object transform
		nap::OFTransform* obj_transform = spline_entity->getComponent<nap::OFTransform>();
		if (obj_transform == nullptr)
		{
			Logger::warn("Unable to find spline transform, can't draw to laser");
			return;
		}

		//////////////////////////////////////////////////////////////////////////
		// Cam Transform
		//////////////////////////////////////////////////////////////////////////

		Entity* laser_entity = laser_cam->getParent();
		nap::OFTransform* laser_transform = laser_entity->getComponent<nap::OFTransform>();
		if (laser_transform == nullptr)
		{
			Logger::warn("No transform component found on laser camera");
			return;
		}

		// Populate laser buffer
		populateLaserBuffer(*laser_transform, *laser_cam, *verts, *colors, *obj_transform, isClosed);

		// Send point
		mEtherdream.SendData(std::move(mLaserPoints));
	}



	/**
	@brief Returns first available drawable spline component
	**/
	nap::OFSplineComponent* EtherDreamService::getRenderableSplineComponent()
	{
		// Get the openframeworks service
		nap::OFService* of_service = getCore().getService<nap::OFService>();

		// Get components we can draw, return if empty
		std::vector<nap::OFSplineComponent*> spline_components;
		of_service->getObjects<nap::OFSplineComponent>(spline_components);

		if (spline_components.size() == 0)
			return nullptr;

		// Extract valid spline
		for (auto& spline : spline_components)
		{
			if(!spline->mEnableDrawing.getValue())
				continue;

			if (spline->GetPointCount() > mMaxPointCount.getValue() || spline->GetPointCount() < mMinPointCount.getValue())
			{
				nap::Logger::warn(*this, "spline: %s has an invalid point count", spline->getName().c_str());
				continue;
			}

			return spline;
		}

		return nullptr;
	}



	/**
	@brief Returns a first available drawable trace component
	**/
	nap::OFTraceComponent* EtherDreamService::getRenderableTraceComponent()
	{
		// Get the openframeworks service
		nap::OFService* of_service = getCore().getService<nap::OFService>();

		// Get components we can draw, return if empty
		std::vector<nap::OFTraceComponent*> trace_components;
		of_service->getObjects<nap::OFTraceComponent>(trace_components);

		if (trace_components.size() == 0)
			return nullptr;

		for (auto& tracer : trace_components)
		{
			if (!tracer->mEnableDrawing.getValue())
				continue;

			if (tracer->mCount.getValue() > mMaxPointCount.getValue() || tracer->mCount.getValue() < mMinPointCount.getValue())
			{
				nap::Logger::warn(*this, "tracer: %s has an invalid point count", tracer->getName().c_str());
				continue;
			}

			return tracer;
		}

		return nullptr;
	}


	/**
	@brief Populates the laser buffer (mLaserPoints) that can be send to the laser
	**/
	void EtherDreamService::populateLaserBuffer(const nap::OFTransform& inLaserTransform, const nap::EtherDreamCamera& inCamera, const std::vector<ofVec3f>& inVerts, const std::vector<ofFloatColor>& inColors, const nap::OFTransform& inObjectTransform, bool isClosed)
	{
		assert(inVerts.size() == inColors.size());

		// Create new list
		mLaserPoints = std::make_unique<std::vector<EAD_Pnt_s>>();

		// Get frustrum dimensions and transform
		ofVec2f frustrum(0.0f, 0.0f);

		const ofMatrix4x4& cam_global_xform = inLaserTransform.getGlobalTransform();
		frustrum.x = cam_global_xform.getTranslation().x;
		frustrum.y = cam_global_xform.getTranslation().y;

		// Get frustrum dimensions
		float fr_width = inCamera.mFrustrumWidth.getValue();
		float fr_height = inCamera.mFrustrumHeight.getValue();

		// Get frustrum bounds
		float div_h = fr_height / 2.0f;
		float div_w = fr_width  / 2.0f;

		ofVec2f min_bounds(frustrum.x - (fr_width / 2.0f), frustrum.y - (fr_height / 2.0f));
		ofVec2f max_bounds(frustrum.x + (fr_width / 2.0f), frustrum.y + (fr_height / 2.0f));

		// Reserve all the points
		isClosed ? mLaserPoints->resize(inVerts.size()) : mLaserPoints->resize(inVerts.size() + mCloseCount.getValue());

		// Sample xform for object movement
		const ofMatrix4x4& global_xform = inObjectTransform.getGlobalTransform();

		// Get reference to points
		std::vector<EAD_Pnt_s>& points = *mLaserPoints;

		// Go over the curves and set data
		for (uint32 i = 0; i < inVerts.size(); i++)
		{
			// Get vertex
			ofVec3f cv = inVerts[i];

			// Transform vertex
			cv = cv * global_xform;

			// Get color
			const ofFloatColor& cc = inColors[i];

			// Sets
			points[i].X = sEtherInterpolate(cv.x, min_bounds.x, max_bounds.x, mFlipX.getValue());
			points[i].Y = sEtherInterpolate(cv.y, min_bounds.y, max_bounds.y, mFlipY.getValue());
			points[i].R = sEtherInterpolateColor(cc.r);
			points[i].G = sEtherInterpolateColor(cc.g);
			points[i].B = sEtherInterpolateColor(cc.b);
			points[i].I = sEtherInterpolateColor(cc.a);
		}

		if (!isClosed)
		{
			closeSpline(inVerts.size(), points);
		}
	}


	/**
	@brief Closes a spline by adding additional laser points
	**/
	void EtherDreamService::closeSpline(uint originalCount, std::vector<EAD_Pnt_s>& laserPoints)
	{
		// Get first and last point
		EAD_Pnt_s& last_point  = laserPoints[originalCount-1];
		EAD_Pnt_s& first_point = laserPoints[0];

		// Get values to interpolate
		ofVec2f start_point(last_point.X, last_point.Y);
		ofVec2f end_point(first_point.X, first_point.Y);

		// Calculate lookup incremental value
		float diff = 1.0f / float(gMax<uint>(mCloseCount - 1, 1));
		
		// laser point sample index starts at end of previous buffer
		uint current_idx = originalCount;

		for (uint i = 0; i < mCloseCount; i++)
		{
			float lookup = float(i) * diff;
			ofVec2f out_vec;
			gMixVector2f(start_point, end_point, lookup, out_vec);

			laserPoints[current_idx].X = (int16_t)out_vec.x;
			laserPoints[current_idx].Y = (int16_t)out_vec.y;
			laserPoints[current_idx].R = gEtherMinValue;
			laserPoints[current_idx].G = gEtherMinValue;
			laserPoints[current_idx].B = gEtherMinValue;
			laserPoints[current_idx].I = gEtherMaxValue;

			current_idx++;
		}
	}
}

RTTI_DEFINE(nap::EtherDreamService)