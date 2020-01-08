#include "cvframe.h"

RTTI_BEGIN_CLASS(nap::CVFrame)
RTTI_END_CLASS

namespace nap
{
	CVFrame::CVFrame(CVFrame&& other)
	{
		mMatrices = std::move(other.mMatrices);
	}


	CVFrame::CVFrame(int count)
	{
		mMatrices.resize(count);
	}


	CVFrame& CVFrame::operator=(CVFrame&& other)
	{
		mMatrices = std::move(other.mMatrices);
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
	}


	CVFrame CVFrame::clone() const
	{
		CVFrame clone;
		this->copyTo(clone);
		return clone;
	}


	void CVFrame::clear()
	{
		mMatrices.clear();
	}
}
