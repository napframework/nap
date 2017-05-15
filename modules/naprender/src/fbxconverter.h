#pragma once

#include <string>
#include <memory>
#include <vector>

namespace opengl
{
	class Mesh;
}

namespace nap
{
	class ErrorState;

	/**
	 * Options to specify when converting FBX
	 */
	enum class EFBXConversionOptions
	{
		CONVERT_ALWAYS,		// Always convert FBX
		CONVERT_IF_NEWER	// Only convert FBX if the destination does not exist or is older
	};

	/**
	 * Convert (split) a FBX into multiple parts. Currently only converts the meshes.
	 *
	 * @param fbxPath The FBX file to convert
	 * @param outputDirectory Absolute or relative directory that the converted files should be placed in
	 * @param convertOptions Options for the convert
	 * @param convertedFiles List of files that were converted from the FBX
	 * @param errorState The error state
	 * @return Whether the conversion succeeded or not
	 */
	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, EFBXConversionOptions convertOptions, std::vector<std::string>& convertedFiles, ErrorState& errorState);	 

	/**
	 * Load a mesh from the specified mesh. The mesh is expected to be our own mesh format as converted by convertFBX
	 *
	 * @param meshPath Path to the .mesh file to load
	 * @param errorState The error state if the function fails
	 * @return The loaded mesh if successful, nullptr on failure
	 */
	std::unique_ptr<opengl::Mesh> loadMesh(const std::string& meshPath, ErrorState& errorState);
}
