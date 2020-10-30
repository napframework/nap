/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/factory.h>
#include <utility/dllexport.h>

namespace nap
{
	class Core;

	/**
	 * The CoreFactory is a derived class from Factory that is used to automatically create objects that take Core as argument
	 * Any class that doesn't have a constructor with Core as argument will be created normally through the base.
	 */
	class NAPAPI CoreFactory : public rtti::Factory
	{
	public:
		CoreFactory(Core& core);

	protected:
		virtual rtti::Object* createDefaultObject(rtti::TypeInfo typeInfo) override;

	private:
		Core* mCore = nullptr;
	};
}