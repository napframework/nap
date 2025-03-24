/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility/dllexport.h>

namespace nap
{
	class RenderService;
	class MeshInstance;
	
	namespace utility
	{
		class ErrorState;
	}
	
	/**
	 * Options to specify when converting FBX
	 */
	enum class EFBXConversionOptions
	{
		CONVERT_ALWAYS,		///< Always convert FBX
		CONVERT_IF_NEWER	///< Only convert FBX if the destination does not exist or is older
	};

	/**
	 * Converts (splits) an .fbx file into multiple .mesh parts. Only the geometry is extracted.
	 * Vertex, Normal, Color, UV and Tangent information is extracted. Up to 8 Color and UV channels are supported. 
	 * Tangents and bi-tangents are automatically computed when normals are present.
	 * @param fbxPath The FBX file to convert
	 * @param outputDirectory Absolute or relative directory that the converted files should be placed in
	 * @param convertOptions Options for the convert
	 * @param convertCompute Convert vec3 attributes to vec4 enabling compute-friendly 16-byte data alignment
	 * @param noTangents Do not generate tangents and bitangents
	 * @param convertedFiles List of files that were converted from the FBX
	 * @param errorState The error state
	 * @return Whether the conversion succeeded or not
	 */
	NAPAPI bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, EFBXConversionOptions convertOptions, bool convertCompute, bool noTangents, std::vector<std::string>& convertedFiles, utility::ErrorState& errorState);

	/**
	 * Load a mesh from the specified mesh. The mesh is expected to be our own mesh format as converted by convertFBX
	 * The mesh is not yet initialized. Call init() to upload all the mesh data to the GPU. This gives the user
	 * the option to add additional vertex attributes
	 * @param renderService the render engine
	 * @param meshPath Path to the .mesh file to load
	 * @param errorState The error state if the function fails
	 * @return The loaded mesh if successful, nullptr on failure
	 */
	NAPAPI std::unique_ptr<MeshInstance> loadMesh(RenderService& renderService, const std::string& meshPath, utility::ErrorState& errorState);
}
