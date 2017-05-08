#pragma once

#include <string>
#include <memory>

namespace opengl
{
	class Mesh;
}

namespace nap
{
	struct InitResult;
	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, InitResult& initResult);	 
	std::unique_ptr<opengl::Mesh> loadMesh(const std::string& meshPath, InitResult& initResult);
}
