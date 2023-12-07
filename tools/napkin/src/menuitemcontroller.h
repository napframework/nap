/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <QMenu>

// Local Includes
#include "rttiitem.h"

namespace napkin
{
	class MenuItemController;

	/**
	 * Individual assignable menu action
	 */
	class MenuAction final
	{
		friend class MenuItemController;
	public:
		using Callback = std::function<void(napkin::RTTIItem&, QMenu&)>;

		/**
		 * Construct action with populator callback
		 * @action menu population callback
		 */
		MenuAction(const Callback& action) : mCallback(action) { }

	private:
		Callback mCallback;
	};


	//////////////////////////////////////////////////////////////////////////
	// MenuItemController
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Groups menu items and controls population of menu using managed actions
	 */
	class MenuItemController final
	{
	public:
		MenuItemController() = default;
		/**
		 * Assigns the given callback to a new menu action
		 * @param action the callback to assign to the action
		 */
		void addOption(const MenuAction::Callback& action) { mOptions.emplace_back(MenuAction(action)); }

		/**
		 * Populates the menu 
		 */
		void populate(RTTIItem& item, QMenu& menu);

	private:
		std::vector<MenuAction> mOptions;	///< All available menu action options
	};
}
