/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QtGui/QIcon>
#include <rtti/object.h>
#include <QtCore/QMap>

namespace napkin 
{
	class FileType
    {
	public:
		FileType(const nap::rtti::EPropertyFileType filetype, const QString& desc, const QStringList& ext)
				: mFileType(filetype), mDescription(desc), mExtensions(ext) {}
		nap::rtti::EPropertyFileType mFileType;
		QString mDescription;
		QStringList mExtensions;

		bool operator==(const FileType& other) { return mFileType == other.mFileType; }
	};

    /**
     * Conveniently retrieve icons by object type
     */
    class ResourceFactory
    {
    public:
        ResourceFactory();

        const QIcon getIcon(const nap::rtti::Object& object) const;

		/**
		 * Get a Qt-compatible file filter for the given property.
		 * @param prop The property that will point to a filename
		 * @return A string that may be used for file browse dialogs for example.
		 */
		const QString getFileFilter(const nap::rtti::Property& prop) const;

        /**
         * Simply get all file extensions registered by FreeImage
         * @return A list of file extensions (without the dot)
         */
        const QStringList getImageExtensions();

        /**
         * Retrieve all extensions defined by libav.
         * WARNING: This only lists the input formats
         * @return A list of file extensions (without the dot)
         */
        const QStringList getVideoExtensions();

		/**
		 * Retrieve all extensions defined by freetype
		 */
		const QStringList getFontExtensions();

    private:
        /**
         * Given a (string) property, return the type of file it points to.
         * @param prop The property
         * @return
         */
		const FileType& getFiletype(const nap::rtti::Property& prop) const;


        QList<QPair<rttr::type, QString>> mObjectIconMap;
		QList<FileType> mFileTypes;
		FileType mAnyFileType = {nap::rtti::EPropertyFileType::Any, "Any Files", {}};

        QStringList mImageExtensions;
        QStringList mVideoExtensions;
		QStringList mFontExtensions;
    };
}
