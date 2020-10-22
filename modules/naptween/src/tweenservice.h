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

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI TweenService : public Service
	{
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

		template<typename T>
		std::unique_ptr<TweenHandle<T>> createTween(T startValue, T endValue, float duration, TweenEasing easeType);

		/**
		 * removes a tween
		 * @param pointer to tween
		 */
		void removeTween(TweenBase* tween);
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
	private:
		std::vector<std::unique_ptr<TweenBase>> mTweens;
		std::vector<TweenBase*> 				mTweensToRemove;
	};

	template<typename T>
	std::unique_ptr<TweenHandle<T>> TweenService::createTween(T startValue, T endValue, float duration, TweenEasing easeType)
	{
		std::unique_ptr<Tween<T>> tween = std::make_unique<Tween<T>>(startValue, endValue, duration);
		tween->setEase(easeType);

		std::unique_ptr<TweenHandle<T>> tween_handle = std::make_unique<TweenHandle<T>>(*this, tween.get());

		mTweens.emplace_back(std::move(tween));

		return std::move(tween_handle);
	}
}
