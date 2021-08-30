/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "materialcommon.h"
#include "mesh.h"

namespace nap
{
	// Forward Declares
	class Shader;
	class ComputeShader;

	/**
	 * Key associated with a specific Vulkan pipeline. 
	 * The render engine uses this key to cache Vulkan pipelines so they can be re-used at runtime.
	 * Pipeline creation is considered a heavy Vulkan operation, we therefore cache pipelines to speed up rendering.
	 */
	struct NAPAPI PipelineKey
	{
		/**
		 * Creates the key based on the provided arguments.
		 */
		PipelineKey
		(
			const Shader& shader,
			EDrawMode drawMode,
			EDepthMode depthMode,
			EBlendMode blendMode,
			ECullWindingOrder cullWindingOrder,
			VkFormat colorFormat,
			VkFormat depthFormat,
			VkSampleCountFlagBits sampleCount,
			bool sampleShading,
			ECullMode cullMode,
			EPolygonMode polygonMode
		);

		// TODO: Concatenate all properties into single / multiple 64bit values
		bool operator==(const PipelineKey& rhs) const;

		const Shader*			mShader = nullptr;
		EDrawMode				mDrawMode = EDrawMode::Triangles;
		EDepthMode				mDepthMode = EDepthMode::NotSet;
		EBlendMode				mBlendMode = EBlendMode::NotSet;
		ECullWindingOrder		mCullWindingOrder = ECullWindingOrder::Clockwise;
		VkFormat				mColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat				mDepthFormat = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits	mSampleCount = VK_SAMPLE_COUNT_1_BIT;
		bool					mSampleShading = false;
		ECullMode				mCullMode = ECullMode::Back;
		EPolygonMode			mPolygonMode = EPolygonMode::Fill;
	};

	struct NAPAPI ComputePipelineKey
	{
		/**
		 * Creates the key based on the provided arguments.
		 */
		ComputePipelineKey(const ComputeShader& shader);

		bool operator==(const ComputePipelineKey& rhs) const;

		const ComputeShader* mShader = nullptr;
	};
}

namespace std
{
	/**
	 * Pipeline key has generator.
	 * TODO: Concatenate all properties into single / multiple 64bit values
	 * @param key the key to generate the hash for.
	 * @return pipeline key hash
	 */
	template<>
	struct hash<nap::PipelineKey>
	{
		size_t operator()(const nap::PipelineKey& key) const
		{
			size_t shader_hash			= hash<size_t>{}((size_t)key.mShader);
			size_t draw_mode_hash		= hash<size_t>{}((size_t)key.mDrawMode);
			size_t depth_mode_hash		= hash<size_t>{}((size_t)key.mDepthMode);
			size_t blend_mode_hash		= hash<size_t>{}((size_t)key.mBlendMode);
			size_t cull_winding_hash	= hash<size_t>{}((size_t)key.mCullWindingOrder);
			size_t color_format_hash	= hash<size_t>{}((size_t)key.mColorFormat);
			size_t depth_format_hash	= hash<size_t>{}((size_t)key.mDepthFormat);
			size_t sample_count_hash	= hash<size_t>{}((size_t)key.mSampleCount);
			size_t sample_shading_hash	= hash<size_t>{}((size_t)key.mSampleShading);
			size_t cull_mode_hash		= hash<size_t>{}((size_t)key.mCullMode);
			size_t poly_mode_hash		= hash<size_t>{}((size_t)key.mPolygonMode);
			return shader_hash ^ draw_mode_hash ^ depth_mode_hash ^ blend_mode_hash ^ cull_winding_hash ^ color_format_hash ^ depth_format_hash ^ sample_count_hash ^ sample_shading_hash ^ cull_mode_hash ^ poly_mode_hash;
		}
	};


	template<>
	struct hash<nap::ComputePipelineKey>
	{
		size_t operator()(const nap::ComputePipelineKey& key) const
		{
			size_t shader_hash = hash<size_t>{}((size_t)key.mShader);
			return shader_hash;
		}
	};
}

