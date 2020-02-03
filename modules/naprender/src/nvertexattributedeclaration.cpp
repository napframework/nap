// Local Includes
#include "nvertexattributedeclaration.h"

namespace opengl
{
	// Constructor
	VertexAttributeDeclaration::VertexAttributeDeclaration(const std::string& name, int location, VkFormat format) :
		mName(name),
		mLocation(location),
		mFormat(format)
	{
	}
}
