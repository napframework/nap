/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "bitmap.h"

namespace nap
{
	/**
	 * Bitmap extension class that encodes/decodes image data for writing to/reading from disk.
	 */
	class NAPAPI BitmapFileBuffer final
	{
	public:

		/**
		* All supported output extensions
		*/
		enum class EImageFileFormat : int
		{
			PNG,		///< Portable Network Graphics (*.PNG)
			JPEG,		///< Independent JPEG Group (*.JPG, *.JIF, *.JPEG, *.JPE)
			TIFF,		///< Tagged Image File Format (*.TIF, *.TIFF)
			BMP			///< Windows or OS/2 Bitmap File (*.BMP)
		};

		/**
		* Creates an empty bitmap file buffer 
		*/
		BitmapFileBuffer();

		/**
		* Creates a bitmap file buffer from a bitmap
		* @param bitmap the source bitmap
		* @param copyData if true, allocates an additional file buffer to copy the bitmap data to, else wraps the image data
		*/
		BitmapFileBuffer(const Bitmap& bitmap, bool copyData);

		/**
		* Creates a bitmap file buffer from a bitmap
		* @param surfaceDescriptor the surface descriptor used to allocate the file buffer
		* @param data a pointer to the raw bitmap data
		* @param copyData if true, allocates an additional file buffer to copy the bitmap data to, else wraps the image data
		*/
		BitmapFileBuffer(const SurfaceDescriptor& surfaceDescriptor, const void* data, bool copyData);

		/**
		* Creates a bitmap file buffer from a surface descriptor
		* @param surfaceDescriptor the surface descriptor used to allocate the file buffer
		*/
		BitmapFileBuffer(const SurfaceDescriptor& surfaceDescriptor);

		/**
		* Unloads the internal bitmap handle
		*/
		~BitmapFileBuffer();

		/**
		 * BitmapFileBuffer cannot be copied
		 */
		BitmapFileBuffer(const BitmapFileBuffer& rhs) = delete;
		BitmapFileBuffer& operator=(const BitmapFileBuffer& rhs) = delete;

		/**
		 * BitmapFileBuffer cannot be moved
		 */
		BitmapFileBuffer(BitmapFileBuffer&& rhs) = delete;
		BitmapFileBuffer& operator=(BitmapFileBuffer&& rhs) = delete;

		/**
		* Loads an image from disk and creates the bitmap file buffer
		* @param path the path to the image on disk to load
		* @param outSurfaceDescriptor the surface descriptor containing information about the image
		* @param errorState contains the error if the image could not be loaded from disk
		*/
		bool load(const std::string& path, SurfaceDescriptor& outSurfaceDescriptor, utility::ErrorState& errorState);

		/**
		 * Writes this bitmap to the given location on disk with the specified image format
		 * @param path the path including filename and image extension of the output file e.g. "targetFolder/MyOutputFile.png"
		 * @param errorState contains the error if the image could not be saved to disk
		 */
		bool save(const std::string& path, utility::ErrorState& errorState);


		/**
		* Returns a pointer to the internal pixel data
		* @return a pointer to the internal pixel data
		*/
		void* getData();
		
		/**
		 * Returns a handle to the FreeImage bitmap. This comprises the FreeImage info header and pixel data
		 * Cast to a FIBITMAP pointer as follows:
		 *
		 *~~~~~{.cpp}
		 * FIBITMAP* fi_bitmap = reinterpret_cast<FIBITMAP*>(bitmap_file_buffer.getHandle());
		 *~~~~~
		 *
		 * @return a handle to the FreeImage bitmap
		 */
		void* getHandle();

	private:
		/**
		 * Allocates this bitmap
		 * @param surfaceDescriptor the surface descriptor used to allocate the file buffer
		 * @param errorState contains the error if the file buffer could not be created
		 * @return whether allocation succeeded
		 */
		bool allocate(const SurfaceDescriptor& surfaceDescriptor, utility::ErrorState& errorState);

		/**
		 * Allocates this bitmap and sets the data
		 * @param surfaceDescriptor the surface descriptor used to allocate the file buffer
		 * @param data a pointer to the raw bitmap data
		 * @param copyData if true, allocates an additional file buffer to copy the bitmap data to, else wraps the image data
		 * @param errorState contains the error if the file buffer could not be created
		 * @return whether allocation and data copy/wrap succeeded
		 */
		bool setData(const SurfaceDescriptor& surfaceDescriptor, const void* data, bool copyData, utility::ErrorState& errorState);

		/**
		 * Releases the internal file buffer
		 */
		void release();

		/**
		 * Raw handle to internal bitmap file buffer
		 */
		void* mBitmapHandle = nullptr;
	};

}
