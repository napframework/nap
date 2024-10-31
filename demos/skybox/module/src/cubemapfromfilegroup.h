/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/group.h>
#include <cubemapfromfile.h>

namespace nap
{
	// Group that only holds cube map from file resources
	using CubeMapFromFileGroup = Group<CubeMapFromFile>;
}
