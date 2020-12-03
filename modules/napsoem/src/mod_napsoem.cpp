/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <utility/module.h>

// IMPORTANT: winpcap is required on windows.Download it here : https://www.winpcap.org/
// The SOEM module builds against the winpcap library but does not ship with the dll or driver.
// Installing winpcap should be enough. Administator priviliges are not required.
NAP_SERVICE_MODULE("mod_napsoem", "0.2.0", "nap::SOEMService")
