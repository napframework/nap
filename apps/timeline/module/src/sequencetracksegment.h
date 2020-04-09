#pragma once

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <fcurve.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI SequenceTrackSegment : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState) override;

		template<typename T>
		T& getDerived();

		template<typename T>
		const T& getDerivedConst() const;
	public:
		double										mStartTime = 0.0;
		double										mDuration = 1.0;
	};

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T& nap::SequenceTrackSegment::getDerived()
	{
		assert(this->get_type().is_derived_from(RTTI_OF(T)));
		return static_cast<T&>(*this);
	}

	template<typename T>
	const T& nap::SequenceTrackSegment::getDerivedConst() const
	{
		assert(this->get_type().is_derived_from(RTTI_OF(T)));
		return static_cast<const T&>(*this);
	}
}
