/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nap/core.h>

namespace nap
{
    /**
     * Converts a double time value (in seconds) into an array containing 3 integers representing minutes, seconds and
     * milliseconds in that specific order.
     * @param time the time value in seconds
     * @return array containing converted time values
     */
    std::vector<int> NAPAPI convertTimeToMMSSMSArray(double time);

    /**
     * Converts an array containing 3 integers representing minutes, seconds and milliseconds in that specific order
     * into a time double representing that time in seconds.
     * Asserts when time array size is different then 3
     * @param timeArray reference to the time array
     * @return the time value in seconds
     */
    double NAPAPI convertMMSSMSArrayToTime(const std::vector<int>& timeArray);
}