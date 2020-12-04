/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/stringutils.h>
#include <vector>
#include <string>

namespace nap
{
	namespace utility
	{
		/**
		 * ErrorState is a class that can be used to maintain an error state over (nested) function calls. It's useful to be able to return detailed error information
		 * from somewhere deep down the stack. It's kind of a poor-man's exception handling in that regard.
		 */
		class ErrorState
		{
		public:
			/**
			 * Check whether the specified condition evaluates to true and adds an error message to the state if not.
			 * Often checks are performed on initialization to ensure the resource is valid, for example:
			 *
			 * ~~~~~{.cpp}
			 *	if (!errorState.check(scene != nullptr, "Unable to load scene"))
			 *		return false;
			 * ~~~~~
			 *
			 * @param successCondition The condition to check (i.e. true/false)
			 * @param errorMessage The error message that belongs to the 'fail' state
			 * @return Whether the condition evaluated to true (i.e. success) or not
			 */
			template<typename T>
			bool check(bool successCondition, T&& errorMessage)
			{
				if (!successCondition)
					mErrorList.emplace_back(std::forward<T>(errorMessage));
				return successCondition;
			}

			/**
			 * Same as non-templated check(), but allows for easy formatting of error messages.
			 *
			 * ~~~~~{.cpp}
			 *	if (!errorState.check(file != nullptr, "Unable to load file: %s", file.c_str()))
			 *		return false;
			 * ~~~~~
			 * @param successCondition The condition to check (i.e. true/false)
			 * @param format The error message that that is formatted
			 * @param args the formatting arguments
			 * @return Whether the condition evaluated to true (i.e. success) or not
			 */
			template <typename... Args>
			bool check(bool successCondition, const char* format, Args&&... args)
			{
				return check(successCondition, stringFormat(format, std::forward<Args>(args)...));
			}

			/**
			 * Add a failure message to the stack. 
			 * Useful in situations where you already know that you've failed, in which case the check() function is redundant.
			 *
			 * ~~~~~{.cpp}
			 * error.fail("missing data");
			 * return false;
			 * ~~~~~
			 * 
			 * @param errorMessage The error message, can be a string literal, std::string etc.
			 */
			template<typename T>
			void fail(T&& errorMessage)
			{
				mErrorList.emplace_back(std::forward<T>(errorMessage));
			}

			/**
			 * Same as non-templated fail(), but allows for easy formatting of error messages
			 *
			 * ~~~~~{.cpp}
			 * error.fail("unable to read file: %s", file.c_str());
			 * return false;
			 * ~~~~~
			 *
			 * @param format the error message that is formatted
			 * @param args the format arguments
			 */
			template <typename... Args>
			void fail(const char* format, Args&&... args)
			{
				fail(stringFormat(format, std::forward<Args>(args)...));
			}

			/**
			 * Format the error state of this object into a human-readable message
			 * @return The full error message
			 */
			const std::string toString() const;

			/**
			 * @return True if there are any errors, false when everything is okay.
			 */
			bool hasErrors() const { return !mErrorList.empty(); }

		private:
			std::vector<std::string> mErrorList;
		};

	}
}
