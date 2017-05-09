#pragma once

#include <string>
#include <memory>

namespace opengl
{
	class Mesh;
}

namespace nap
{
	class ErrorState;
	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, ErrorState& errorState);	 
	std::unique_ptr<opengl::Mesh> loadMesh(const std::string& meshPath, ErrorState& errorState);
}
