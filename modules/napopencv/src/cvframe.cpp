/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "cvframe.h"

RTTI_BEGIN_CLASS(nap::CVFrame)
RTTI_END_CLASS

namespace nap
{
	CVFrame::CVFrame(CVFrame&& other)
	{
		mMatrices = std::move(other.mMatrices);
		mSource = std::move(other.mSource);
	}


	CVFrame::CVFrame(int count)
	{
		mMatrices.resize(count);
	}


	CVFrame::CVFrame(int count, CVAdapter* source) : CVFrame(count)
	{
		mSource = source;
	}


	CVFrame& CVFrame::operator=(CVFrame&& other)
	{
		mMatrices = std::move(other.mMatrices);
		mSource = std::move(other.mSource);
		return *this;
	}


	void CVFrame::add(const cv::UMat& matrix)
	{
		mMatrices.emplace_back(matrix);
	}


	void CVFrame::add(cv::UMat&& matrix)
	{
		mMatrices.emplace_back(std::move(matrix));
	}


	cv::UMat& CVFrame::addNew()
	{
		add(cv::UMat());
		return mMatrices.back();
	}


	void CVFrame::copyTo(CVFrame& outFrame) const
	{
		outFrame.mMatrices.resize(mMatrices.size());
		for (auto i = 0; i < mMatrices.size(); i++)
		{
			mMatrices[i].copyTo(outFrame.mMatrices[i]);
		}
		outFrame.mSource = mSource;
	}


	CVFrame CVFrame::clone() const
	{
		CVFrame clone;
		clone.mMatrices.reserve(mMatrices.size());
		for (auto& matrix : mMatrices)
		{
			clone.mMatrices.emplace_back(matrix.clone());
		}
		clone.mSource = mSource;
		return clone;
	}


	void CVFrame::clear()
	{
		mMatrices.clear();
	}
}
