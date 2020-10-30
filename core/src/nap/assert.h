/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Called by NAP_ASSERT in debug mode, aborts program execution if expression evaluates to false.
	 * Do not call this function directly in your application, use the NAP_ASSERT macro instead.
	 * The expression, file and line number are printed to console.
	 * @param exprs expression as string
	 * @param expr the expression to evaluate
	 * @param file the document the assert originates from
	 * @param line line number in file
	 */
	void NAPAPI _assert(const char* exprs, bool expr, const char* file, int line);

	/**
	 * Called by NAP_ASSERT in debug mode, aborts program execution if expression evaluates to false.
	 * Do not call this function directly in your application, use the NAP_ASSERT_MSG macro instead.
	 * The expression, file, line number and additional message are printed to console.
	 * @param exprs expression as string
	 * @param expr the expression to evaluate
	 * @param file the document the assert originates from
	 * @param line line number in file
	 * @param message the message to print
	 */
	void NAPAPI _assertMsg(const char* exprs, bool expr, const char* file, int line, const char* message);
}

#ifndef NDEBUG
	#define	NAP_ASSERT(Expr) \
	nap::_assert(#Expr, Expr, __FILE__, __LINE__);
	#define NAP_ASSERT_MSG(Expr, Msg) \
	nap::_assertMsg(#Expr, Expr, __FILE__, __LINE__, Msg);
#else
	#define NAP_ASSERT(Expr);
	#define NAP_ASSERT_MSG(Expr, Message);
#endif	// !NDEBUG
