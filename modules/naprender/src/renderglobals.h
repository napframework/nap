/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <string>
#include <glm/glm.hpp>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * These uniforms are set automatically by all nap::RenderableComponent(s) when present in the vertex shader.
	 * ~~~~~
	 *	...
	 *	uniform nap
	 *	{
	 *		uniform mat4 projectionMatrix;
	 *		uniform mat4 viewMatrix;
	 *		uniform mat4 modelMatrix;
	 *	} mvp;
	 *
	 *	...
	 *	void main(void)
	 *	{
     *		gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix;	
	 *	}
	 * ~~~~~
	 */
	namespace uniform
	{
		inline constexpr const char* mvpStruct = "nap";						///< default model view projection struct name
		inline constexpr const char* modelMatrix = "modelMatrix";				///< uniform model matrix name
		inline constexpr const char* viewMatrix = "viewMatrix";				///< uniform view matrix name
		inline constexpr const char* projectionMatrix = "projectionMatrix";	///< uniform projection matrix name
	}


	/**
	 * System defaults for the most common used mesh vertex attributes.
	 * If no bindings are specified in the material, the system will automatically try to 
	 * bind the vertex data of a mesh to the inputs of a shader using these names.
	 *
	 * When CUSTOM vertex buffers are used, you have to manually bind all the mesh vertex attributes,
	 * including the default ones, to the shader inputs. Both nap::Material and nap::MaterialInstance 
	 * allow you to explicitly bind vertex data to shader inputs.
	 */
	namespace vertexid
	{
		inline constexpr const char* position = "Position";					///< Default mesh position vertex attribute name
		inline constexpr const char* normal	  = "Normal";	///< Default mesh normal vertex attribute name		
		inline constexpr const char* tangent  = "Tangent";	///< Default mesh tangent vertex attribute name		
		inline constexpr const char* bitangent = "Bitangent"; ///< Default mesh bi-tangent vertex attribute name
		inline constexpr const char* uv		   = "UV0";		  ///< Default mesh uv vertex attribute name, 1 channel
		inline constexpr const char* color	   = "Color0";	  ///< Default mesh color attribute name, 1 channel

		/**
		 * Returns the name of the vertex uv attribute based on the queried uv channel, ie: UV0, UV1 etc.
		 * @param uvChannel: the uv channel index
		 * @return the name of the vertex attribute
		 */
		const NAPAPI std::string getUVName(int uvChannel);

		/**
		* Returns the name of the vertex color attribute based on the queried color channel, ie: "Color0", "Color1" etc.
		* @param colorChannel: the color channel index
		* @return the name of the color vertex attribute
		*/
		const NAPAPI std::string getColorName(int colorChannel);

		/**
		 * Default shader vertex input names. 
		 * TODO: Give vertex attribute and shader input the same names,
		 * removing the need declare separate shader input names -> update demos
		 */
		namespace shader
		{
			inline constexpr const char* position = "in_Position"; ///< Default shader position vertex input name
			inline constexpr const char* normal	  = "in_Normals";  ///< Default shader normal vertex input name
			inline constexpr const char* tangent  = "in_Tangent";  ///< Default shader tangent vertex input name
			inline constexpr const char* bitangent = "in_Bitangent"; ///< Default shader bi-tangent vertex input name
			inline constexpr const char* color	   = "in_Color0"; ///< Default shader color vertex input name, 1 channel
			inline constexpr const char* uv		   = "in_UV0";	  ///< Default shader uv vertex input name, 1 channel

			/**
			 * @param channel index to generate name for
			 * @return Default shader uv vertex input name: 'in_UV#'
			 */
			const NAPAPI std::string getUVInputName(int channel);

			/**
			 * @param channel index to generate name for
			 * @return Default shader color vertex input name: 'in_Color#'
			 */
			const NAPAPI std::string getColorInputName(int channel);
		}
	}
}
