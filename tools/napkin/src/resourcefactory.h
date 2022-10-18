/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QtGui/QIcon>
#include <rtti/object.h>
#include <QtCore/QMap>

namespace napkin 
{
	/**
	 * File type helper struct
	 */
	struct FileType
    {
		FileType(const nap::rtti::EPropertyFileType filetype, const QString& desc, const QStringList& ext);
		nap::rtti::EPropertyFileType mType;
		QString mDescription;
		QStringList mExtensions;
		bool operator==(const FileType& other) { return mType == other.mType; }
	};

	/**
	 * Icon helper class.
	 */
	class Icon
	{
	public:
		Icon(const QString& path);
		QIcon get() const				{ return mIcon;  }
		QIcon inverted() const;
		bool valid() const				{ return !mIcon.isNull(); }
	private:
		QString mPath;
		QIcon mIcon;
		mutable QIcon mIconInverted;
	};

    /**
     * Conveniently retrieve icons by object type
     */
    class ResourceFactory
    {
    public:
		ResourceFactory() = default;

		/**
		 * @param object the rtti object to get the icon for
		 * @return an icon for the given rtti object
		 */
        const QIcon getIcon(const nap::rtti::TypeInfo& type) const;

		/**
		 * @param object the rtti object to get the icon for
		 * @return an icon for the given rtti object
		 */
        const QIcon getIcon(const nap::rtti::Object& object) const;

		/**
		 * @param path the path to load the icon from 
		 * @return an icon loaded from the given path
		 */
		const QIcon getIcon(const QString& path) const;

		/**
		 * Get a Qt-compatible file filter for the given property.
		 * @param prop The property that will point to a filename
		 * @return A string that may be used for file browse dialogs for example.
		 */
		const QString getFileFilter(const nap::rtti::Property& prop) const;

    private:
        /**
         * Given a (string) property, return the type of file it points to.
         * @param prop The property
         * @return
         */
		const FileType& getFiletype(const nap::rtti::Property& prop) const;

		// Map that holds all the loaded icons
		mutable std::unordered_map<std::string, Icon> mIcons;
    };
}
