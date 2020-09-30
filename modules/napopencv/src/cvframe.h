#pragma once

#include <rtti/rtti.h>
#include <vector>
#include <opencv2/core/mat.hpp>

namespace nap
{
	// Forward Declares
	class CVAdapter;

	/**
	 * Single OpenCV frame that contains one or multiple OpenCV matrices and a source. 
	 * Together the matrices define the content of a frame. 
	 * Often a frame has only one matrix (Color) but certain adapters might return a frame that contain more, 
	 * for example: color, depth and point cloud information. A frame can be copied and moved.
	 * A frame can have a source. Often this is a nap::CVAdapter. 
	 *
	 * Note that a frame copy (or assignment) operation does not copy the actual content of the matrices.
	 * Frames are only a pointer into a block of shared data. The data itself keeps track of the frames that reference it.
	 * When the reference count of a data container drops to 0 the data is deleted. 
	 * The default copy or assignment operation is therefore very fast.
	 *
	 * To create an actual deep copy of a frame, including all the matrices and source, call 
	 * CVFrame::deepCopyTo() or CVFrame::clone().
	 */
	class NAPAPI CVFrame final
	{
		RTTI_ENABLE()
	public:
		// Default Constructor
		CVFrame() = default;
		
		// Default Destructor
		~CVFrame() = default;
		
		// Default copy constructor
		CVFrame(const CVFrame& other) = default;
		
		// Default copy assignment operator
		CVFrame& operator=(const CVFrame& other) = default;
		
		// Move constructor
		CVFrame(CVFrame&& other);
		
		// Move assignment operator
		CVFrame& operator=(CVFrame&& other);

		/**
		 * Constructs a new frame with the given number of matrices.
		 * @param count the number of matrices to create.
		 */
		CVFrame(int count);

		/**
		 * Constructs a new frame with the given number of matrices 
		 * together with the device that created this frame.
		 * @param count the number of matrices to create.
		 * @param source the device that created this frame.
		 */
		CVFrame(int count, CVAdapter* source);

		/**
		 * Array subscript overload. Does not perform a bounds check!
		 * @return the matrix at the given index
		 */
		cv::UMat& operator[](std::size_t index)					{ return mMatrices[index]; }

		/**
		 * Array subscript overload. Does not perform a bounds check!
		 * @return the matrix at the given index
		 */
		const cv::UMat& operator[](std::size_t index) const		{ return mMatrices[index]; }

		/**
		 * Performs a deep copy of this frame.
		 * The default copy operation does not copy the actual content of the matrices, only increases the ref count.
		 * The content of the given frame is cleared.
		 * @param outFrame the frame to copy the data of this frame to.
		 */
		void copyTo(CVFrame& outFrame) const;

		/**
		 * Clones this frame including all of it's content and source.
		 * The default copy operation does not copy the actual content of the matrices, only increases the ref count.
		 * @return a clone of this frame.
		 */
		CVFrame clone() const;

		/**
		 * @return the number of matrices this frame contains.
		 */
		int getCount() const									{ return static_cast<int>(mMatrices.size()); }

		/**
		 * Adds a new empty matrix to this frame.
		 * @return the newly created matrix.
		 */
		cv::UMat& addNew();

		/**
		 * Adds the given matrix to this frame.
		 * @param matrix new OpenCV matrix to add to this frame.
		 */
		void add(const cv::UMat& matrix);

		/**
		 * Gives a matrix to this frame. Ownership is transferred.
		 * @param matrix new OpenCV matrix to add to this frame.
		 */
		void add(cv::UMat&& matrix);

		/**
		 * Clears all frame data.
		 */
		void clear();

		/**
		 * @return if this frame does not contain any data.
		 */
		bool empty() const										{ return mMatrices.empty(); }
		
		/**
		 * @return if this frame has a source.
		 */
		bool hasSource() const									{ return mSource != nullptr; }

		/**
		 * @return the source that created this frame, nullptr when there is no source.
		 */
		const CVAdapter* getSource() const						{ return mSource; }

		/**
		 * @return the source that created this frame, nullptr when there is no source.
		 */
		CVAdapter* getSource()									{ return mSource; }

	private:
		std::vector<cv::UMat> mMatrices;	///< All OpenCV matrices associated with the frame
		CVAdapter* mSource = nullptr;		///< The source that created this frame.
	};
}