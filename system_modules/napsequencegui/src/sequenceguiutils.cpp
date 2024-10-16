/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <sequenceguiutils.h>

namespace nap
{
    std::vector<int> convertTimeToMMSSMSArray(double time)
    {
        return
            {
                (int) (time) / 60,
                (int) (time) % 60,
                (int) (time * 100.0) % 100
            };
    }


    double convertMMSSMSArrayToTime(const std::vector<int>& timeArray)
    {
        assert(timeArray.size() == 3);
        return (((double) timeArray[2]) / 100.0f) + (double) timeArray[1] + ((double) timeArray[0] * 60.0);
    }
}

