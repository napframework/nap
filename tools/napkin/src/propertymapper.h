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
		 * Select and create a specific shader binding based on the selected property
		 * @param parent the widget to parent the selection dialog to
		 */
		void map(QWidget* parent);

	private:
		const nap::ShaderVariableDeclaration* selectVariableDeclaration(const nap::BufferObjectDeclarationList& list, QWidget* parent);
		void addVariableBinding(const nap::ShaderVariableDeclaration& declaration, const PropertyPath& propPath);

		const nap::SamplerDeclaration* selectSamplerDeclaration(QWidget* parent);
		void addSamplerBinding(const nap::SamplerDeclaration& declaration);

		PropertyPath mPath;
		nap::BaseMaterial* mMaterial = nullptr;
		nap::BaseShader* mShader = nullptr;
	};
}
