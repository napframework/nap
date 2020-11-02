/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <rtti/factory.h>

// local includes
#include "tweeneasing.h"
#include "tween.h"
#include "tweenhandle.h"
#include "tweenmode.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * The TweenService is responsible for creating, updating and retaining all Tweens
	 * Once you call createTween<T> on the TweenService. It will construct a new Tween and keep the unique_ptr to the Tween stored internally.
	 * Then, the TweenService will make a handle to that tween and return a unique_ptr to the TweenHandle which needs to be managed outside the TweenService ( typically, by the class from where you call the createTween<T> method )
	 * The function of the Handle is to provide the user access to the Tween without having to worry about memory managament. Once the unique_ptr of the handle is out of scope, the created handle will be deconstructed and notify the TweenService to mark the Tween for deletion
	 * This is to prevent memory access violations that can occur when you dispose a Tween during a call to its Update, Killed or Complete signal
	 */
	class NAPAPI TweenService : public Service
	{
		friend class TweenHandleBase;

		RTTI_ENABLE(Service)
	public:

		/**
		 * Constructor
		 */
		TweenService(ServiceConfiguration* configuration);

		/**
		 * Deconstructor
		 */
		virtual ~TweenService();

		/**
		 * registers object creator method that can be passed on to the rtti factory
		 * @param objectCreator unique pointer to method
		 */
		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(TweenService*));

		/**
		 * creates a Tween, service retains unique_ptr to tween and returns a TweenHandle that enables the user to have access to the Tweens functionality outside the TweenService
		 * Once the handle is deconstructed, it will notify the service that the Tween can be deleted. The tween will then be deleted during the update loop but outside the scope of any Update, Complete or Killed signals
		 * @tparam T the value type to tween
		 */
		template<typename T>
		std::unique_ptr<TweenHandle<T>> createTween(T startValue, T endValue, float duration, ETweenEasing easeType = ETweenEasing::LINEAR, ETweenMode mode = ETweenMode::NORMAL);
	protected:

		/**
		 * registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * updates any tweens
		 * @param deltaTime deltaTime
		 */
		virtual void update(double deltaTime) override;

		/**
		 * called when service is shutdown, deletes all tweens
		 */
		virtual void shutdown() override;
	private:
		/**
		 * removes a tween, called by tween handle
		 * @param pointer to tween
		 */
		void removeTween(const TweenBase* tween);

		// vector holding the tweens
		std::vector<std::unique_ptr<TweenBase>> mTweens;

		// vector holding tweens that need to be removed
		std::vector<const TweenBase*> 			mTweensToRemove;
	};

	template<typename T>
	std::unique_ptr<TweenHandle<T>> TweenService::createTween(T startValue, T endValue, float duration, ETweenEasing easeType, ETweenMode mode)
	{
		// construct tween
		std::unique_ptr<Tween<T>> tween = std::make_unique<Tween<T>>(startValue, endValue, duration);
		tween->setEase(easeType);
		tween->setMode(mode);

		// construct handle
		std::unique_ptr<TweenHandle<T>> tween_handle = std::make_unique<TweenHandle<T>>(*this, tween.get());

		// move ownership of tween
		mTweens.emplace_back(std::move(tween));

		// return unique_ptr to handle
		return std::move(tween_handle);
	}
}
