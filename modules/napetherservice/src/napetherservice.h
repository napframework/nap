#pragma once

// Nap Includes
#include <nap/service.h>
#include <nap/configure.h>
#include <napofsplinecomponent.h>
#include <napoftracecomponent.h>
#include <napoftransform.h>

// Include rtti
#include <rtti/rtti.h>

// Local inlcudes
#include <napethercamera.h>

// Naivi OF Includes
#include <ofetherdream.h>

// Std Includes
#include <vector>

namespace nap
{
	/**
	@brief nap etherdream service tied to openframeworks
	**/
	class EtherDreamService : public Service
	{
		// Enable RTTI
		RTTI_ENABLE_DERIVED_FROM(Service);

		// Declare itself as service
		NAP_DECLARE_SERVICE()

	public:
		EtherDreamService();

		// Draws current scene to laser
		void draw();

		// Controllable laser attribtues
		Attribute<bool> mSend =					{ this, "SendToLaser", true };
		Attribute<bool> mFlipX =				{ this, "FlipX", true  };
		Attribute<bool> mFlipY =				{ this, "FlipY", false };
		NumericAttribute<int> mCloseCount =		{ this, "CloseCount", 25, 10,100 };
		NumericAttribute<int> mMinPointCount =  { this, "MinPoints", 100, 100, 1000 };
		NumericAttribute<int> mMaxPointCount =  { this, "MaxPoints", 1000,100, 1000 };

		void Stop()								{ mEtherdream.Kill(); }

	private:
		//Laser interface
		nofNEtherDream			mEtherdream;				//< Ether-dream threaded service
		LaserPoints				mLaserPoints = nullptr;		//< Points that will be send to the laser

		// Returns a set of valid drawable laser components, @return total amount of points to draw
		nap::OFSplineComponent* getRenderableSplineComponent();

		// Returns a valid trace component
		nap::OFTraceComponent*  getRenderableTraceComponent();

		void closeSpline(uint originalCount, std::vector<EAD_Pnt_s>& laserPoints);

		// Populate Laser Buffer
		void populateLaserBuffer(const nap::OFTransform& inLaserTransform, const nap::EtherDreamCamera& inCamera, const std::vector<ofVec3f>& inVerts, const std::vector<ofFloatColor>& inColors, const nap::OFTransform& inObjectTransform, bool isClosed);
	};
}

RTTI_DECLARE(nap::EtherDreamService)
