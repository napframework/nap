// Local Includes
#include "vao.h"
#include "renderservice.h"

namespace nap
{
	VAOKey::VAOKey(const Material& material, const MeshInstance& meshResource) :
		mMaterial(&material),
		mMeshResource(&meshResource)
	{
	}


	VAOHandle::VAOHandle(RenderService& renderService, const VAOKey& key, opengl::VertexArrayObject* object) :
		mRenderService(&renderService),
		mKey(key),
		mObject(object)
	{
		mRenderService->incrementVAORefCount(mKey);
	}


	VAOHandle::VAOHandle(const VAOHandle& rhs) :
		mRenderService(rhs.mRenderService),
		mKey(rhs.mKey),
		mObject(rhs.mObject)
	{
		mRenderService->incrementVAORefCount(mKey);
	}


	// Note that the move copy ctor does not need to increment the refcount as it 'moves' 
	// the refcount from the other object.
	VAOHandle::VAOHandle(VAOHandle&& rhs) : 
		mRenderService(rhs.mRenderService),
		mKey(rhs.mKey),
		mObject(rhs.mObject)
	{
		rhs.mRenderService = nullptr;
		rhs.mKey = VAOKey();
		rhs.mObject = nullptr;
	}


	VAOHandle& VAOHandle::operator=(const VAOHandle& rhs)
	{
		if (&rhs == this)
			return *this;

		// Make sure to inc refcount before decreasing it, to avoid reaching zero when mObject is the same
		if (rhs.isValid())
			rhs.mRenderService->incrementVAORefCount(rhs.mKey);

		if (isValid())
			mRenderService->decrementVAORefCount(mKey);

		mRenderService	= rhs.mRenderService;
		mKey			= rhs.mKey;
		mObject			= rhs.mObject;

		return *this;
	}


	// Note that the move copy assignment operator does not need to increment the refcount as it 'moves' 
	// the refcount from the other object. We do need to decrement the refcount for whatever is currently
	// in the handle.
	VAOHandle& VAOHandle::operator=(VAOHandle&& rhs)
	{
		if (&rhs == this)
			return *this;

		if (isValid())
			mRenderService->decrementVAORefCount(mKey);

		mRenderService		= rhs.mRenderService;
		mKey				= rhs.mKey;
		mObject				= rhs.mObject;

		rhs.mRenderService	= nullptr;
		rhs.mKey			= VAOKey();
		rhs.mObject			= nullptr;

		return *this;
	}


	VAOHandle::~VAOHandle()
	{
		if (isValid())
			mRenderService->decrementVAORefCount(mKey);
	}
}