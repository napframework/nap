#include "cvframe.h"

RTTI_BEGIN_CLASS(nap::CVFrame)
RTTI_END_CLASS

namespace nap
{
	CVFrame::CVFrame(CVFrame&& other)
	{
		mMatrices = std::move(other.mMatrices);
	}


	CVFrame::CVFrame(const cv::UMat& matrix)
	{
		add(matrix);
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
		add(std::move(cv::UMat()));
		return mMatrices[mMatrices.size() - 1];
	}


	void CVFrame::deepCopyTo(CVFrame& outFrame) const
	{
		outFrame.mMatrices.clear();
		outFrame.mMatrices.reserve(mMatrices.size());
		for (auto& matrix : mMatrices)
		{
			cv::UMat& copy_matrix = outFrame.addNew();
			matrix.copyTo(copy_matrix);
		}
	}


	CVFrame CVFrame::clone() const
	{
		CVFrame clone;
		this->deepCopyTo(clone);
		return clone;
	}
}
