/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <unordered_map>

namespace nap
{
	/**
	 * All available OpenCV capture related errors.
	 */
	enum class CVCaptureError : int
	{
		OpenError		= 0,	///< Capture device could not be opened
		PropertyError	= 1,	///< Capture Property could not be set
		GrabError		= 2,	///< Next frame could not be grabbed, device disconnected or end of stream
		DecodeError		= 3		///< Grabbed frame could not be decoded
	};

	// Map used for storing cv capture related errors
	using CVCaptureErrorMap = std::unordered_map<CVCaptureError, std::string>;
}
