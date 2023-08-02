/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "propertypath.h"

// External Includes
#include <material.h>

namespace napkin
{
	/**
	 * Allows the user to select and create a property based on available shader bindings
	 */
	class MaterialPropertyMapper final
	{
	public:
		// Constructor
		MaterialPropertyMapper(const PropertyPath& propertyPath, nap::BaseMaterial& material);

		/**
		 * Copy is not allowed
		 */
		MaterialPropertyMapper(MaterialPropertyMapper&) = delete;

		/**
		 * Copy assignment is not allowed
		 */
		MaterialPropertyMapper& operator=(const MaterialPropertyMapper&) = delete;

		/**
		 * Move is not allowed
		 */
		MaterialPropertyMapper(MaterialPropertyMapper&&) = delete;

		/**
		 * Move assignment is not allowed
		 */
		MaterialPropertyMapper& operator=(MaterialPropertyMapper&&) = delete;

		/**
		 * If the item can be mapped or not. 
		 * @return if the item can be mapped or not.
		 */
		bool mappable() const;

		/**
		 * Allows the user to select a shader binding
		 * @param parent the widget to parent the selection dialog to
		 */
		void map(QWidget* parent);

	private:
		void handleUniformBinding();
		void handleSamplerBinding();
		void handleBufferBinding();

		nap::BaseMaterial& mMaterial;
		const PropertyPath& mPath;
		nap::BaseShader* mShader = nullptr;
	};
}
