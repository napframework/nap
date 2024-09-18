/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <vector>
#include <assert.h>

#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

namespace nap
{
	class NAPAPI SamplerDeclaration final
	{
		RTTI_ENABLE()

	public:
		enum class EType : uint8_t
		{
			Type_1D,
			Type_2D,
			Type_3D,
			Type_Cube
		};

		SamplerDeclaration(const std::string& name, int binding, VkShaderStageFlagBits stage, EType type) :
			mName(name), mBinding(binding), mStage(stage), mType(type), mIsArray(false), mNumElements(1) {}

        SamplerDeclaration(const std::string& name, int binding, VkShaderStageFlagBits stage, EType type, bool isArray, int numElements) :
            mName(name), mBinding(binding), mStage(stage), mType(type), mIsArray(isArray), mNumElements(numElements)
        {
            assert(mNumElements > 0);
        }

		std::string				mName;
		int						mBinding = -1;
		VkShaderStageFlagBits	mStage;
		EType					mType = EType::Type_2D;
        bool                    mIsArray = false;
        int                     mNumElements = 1;
	};

	using SamplerDeclarations = std::vector<SamplerDeclaration>;
}
