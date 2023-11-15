/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <vector>
#include "uniforminstance.h"
#include "bufferbindinginstance.h"
#include "shadervariabledeclarations.h"
#include "gpubuffer.h"

namespace nap
{
	/**
	 * Blend mode for Materials.
	 */
	enum class EBlendMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend mode is used instead
		Opaque,					///< Regular opaque, similar to (One, Zero) blend
		AlphaBlend,				///< Transparent object (SrcAlpha, InvSrcAlpha) blend
		Additive				///< Additive, (One, One) blend
	};


	/**
	 * Denotes the cull winding order of a mesh.
	 */
	enum class ECullWindingOrder : int
	{
		Clockwise = 0,
		CounterClockwise
	};


	/**
	 * Determines how the z-buffer is used for reading and writing.
	 */
	enum class EDepthMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend is used instead
		InheritFromBlendMode,	///< Transparent objects do not write depth, but do read depth. Opaque objects read and write depth.
		ReadWrite,				///< Read and write depth
		ReadOnly,				///< Only read depth
		WriteOnly,				///< Only write depth
		NoReadWrite				///< Neither read or write depth
	};


	/**
	 *	Supported depth compare modes for shadow texture samplers e.g. `sampler2DShadow`.
	 */
	enum class EDepthCompareMode : int
	{
		NotSet = 0,
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};


	/**
	 * Hash value for shader specialization constants
	 */
	using ShaderConstantHash = uint;


	/**
	 * Non-hierarchical structure that holds pointers to all uniform leaf elements. These can point to either Material
	 * or MaterialInstance instance uniforms, depending on whether the resource is overridden by an instance.
	 * Rebuilt each time an override is made or new instance is created at runtime. This is handled in
	 * MaterialInstance::update().
	 */
	class UniformBufferObject
	{
	public:
		using UniformList = std::vector<const UniformLeafInstance*>;

		UniformBufferObject(const BufferObjectDeclaration& declaration) :
			mDeclaration(&declaration)
		{
			assert(declaration.mDescriptorType == EDescriptorType::Uniform);
		}

		const BufferObjectDeclaration*			mDeclaration;
		UniformList								mUniforms;
	};


	/**
	 * Non-hierarchical structure that holds pointers to all uniform leaf elements. These can point to either Material
	 * or MaterialInstance instance buffer bindings, depending on whether the resource is overridden by an instance.
	 * Rebuilt each time an override is made or new instance is created at runtime. This is handled in
	 * MaterialInstance::update().
	 */
	class ShaderStorageBufferObject
	{
	public:
		using BufferBindingList = std::vector<const BufferBindingInstance*>;

		ShaderStorageBufferObject(const BufferObjectDeclaration& declaration) :
			mDeclaration(&declaration)
		{
			assert(declaration.mDescriptorType == EDescriptorType::Storage);
		}

		const BufferObjectDeclaration*			mDeclaration;
		const BufferBindingInstance*			mBufferBinding;
	};
}
