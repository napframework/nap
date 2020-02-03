// Local Includes
#include "nvertexattributedeclaration.h"

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
