/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <QMenu>

namespace napkin
{
	// Forward declares
	template<typename T>
	class MenuOptionController;

	/**
	 * Individual callable menu option for item type of T
	 */
	template<typename T>
	class MenuOption final
	{
		template<typename T> friend class MenuOptionController;
	public:
		// Menu option callback
		using Callback = std::function<void(T&, QMenu&)>;

		/**
		 * Construct option with possible action
		 * @action action callback
		 */
		MenuOption(const Callback& action) : mCallback(action) { }

	private:
		Callback mCallback;
	};


	/**
	 * Collects and assigns menu options for item type of T
	 */
	template<typename T>
	class MenuOptionController final
	{
	public:
		MenuOptionController() = default;
		/**
		 * Assigns the given callback to a new menu option
		 * @param action the callback to assign to the menu option
		 */
		void addOption(const typename MenuOption<T>::Callback& action)	{ mOptions.emplace_back(MenuOption<T>(action)); }

		/**
		 * Populates the menu 
		 */
		void populate(T& item, QMenu& menu);

	private:
		std::vector<MenuOption<T>> mOptions;	///< All available menu action options
	};


	template<typename T>
	void MenuOptionController<T>::populate(T& item, QMenu& menu)
	{
		for (const auto& action : mOptions)
		{
			action.mCallback(item, menu);
		}
	}
}
