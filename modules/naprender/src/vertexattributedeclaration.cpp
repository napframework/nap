/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "vertexattributedeclaration.h"

namespace nap
{
	// Constructor
	VertexAttributeDeclaration::VertexAttributeDeclaration(const std::string& name, int location, VkFormat format) :
		mName(name),
		mLocation(location),
		mFormat(format)
	{
	}
}
