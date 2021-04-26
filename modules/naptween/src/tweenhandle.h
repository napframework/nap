/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "tween.h"

// external includes
#include <mathutils.h>
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class TweenService;

	/**
	 * TweenHandle base class
	 * Upon deconstruction, lets the service know the corresponding Tween can be deleted
	 */
	class NAPAPI TweenHandleBase
	{
	public:
		/**
		 * Deconstructor
		 * Notifies TweenService to mark mTweenBase for deletion
		 */
		virtual ~TweenHandleBase();
	protected:
		/**
		 * Constructor, needs reference to the TweenService
		 * @param tweenService the TweenService
		 */
		TweenHandleBase(TweenService& tweenService);

		// the TweenService
		TweenService& mService;

		// pointer to base class of tween
		TweenBase* mTweenBase;
	};

	/**
	 * A Handle to provide user access to create Tween functionality
	 * @tparam T the value type to tween
	 */
	template<typename T>
	class TweenHandle : public TweenHandleBase
	{
	public:
		/**
		 * Constructor, needs reference to TweenService and pointer to corresponding tween
		 * @param tweenService reference to the TweenService
		 * @param tween pointer to Tween<T>
		 */
		TweenHandle(TweenService& tweenService, Tween<T>* tween);
	public:
		/**
		 * returns reference to corresponding Tween<T>
		 * @return reference to corresponding Tween<T>
		 */
		Tween<T>& getTween(){ return *mTween; }
	private:
		// pointer to the tween
		Tween<T>* mTween;
	};

	//////////////////////////////////////////////////////////////////////////
	// shortcuts
	//////////////////////////////////////////////////////////////////////////
	using TweenHandleFloat 	= TweenHandle<float>;
	using TweenHandleDouble = TweenHandle<double>;
	using TweenHandleVec2 	= TweenHandle<glm::vec2>;
	using TweenHandleVec3 	= TweenHandle<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// explicit MSVC template specialization exports
	//////////////////////////////////////////////////////////////////////////
	template class NAPAPI TweenHandle<float>;
	template class NAPAPI TweenHandle<double>;
	template class NAPAPI TweenHandle<glm::vec2>;
	template class NAPAPI TweenHandle<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// template definition
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	TweenHandle<T>::TweenHandle(TweenService& tweenService, Tween<T>* tween)
		: TweenHandleBase(tweenService), mTween(tween)
	{
		mTweenBase = tween;
	}
}

