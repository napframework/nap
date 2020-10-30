/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	namespace rtti
	{
		enum class EPropertyValidationMode : uint8_t
		{
			DisallowMissingProperties,				///< When a required property is missing from the file, an error will be returned
			AllowMissingProperties					///< When a required property is missing from the file, no errors will be returned
		};

		/**
		 * The deserializer can operate in two modes: using only raw pointers or without any raw pointers at all. The main distinction
		 * between the two modes lie in hotloading: in the normal flow of operation, all objects are managed by ResourceManager and
		 * internally by ObjectPtrManager. These classes enable the hotloading of objects and they do so by patching internal pointers 
		 * in non-raw pointers like ObjectPtr, ResourcePtr, ComponentPtr. In such situations, raw pointers are not allowed. 
		 * However, ObjectPtr and alike are not thread-safe and they should only be used from the main thread. In cases where objects
		 * are deserialized directly without the use of ResourceManager, you can use raw pointers instead of ObjectPtr and alike. 
		 * By enabling OnlyRawPointers or NoRawPointers, the deserializer verifies the correct use of all pointers throughout deserialization.
		 */
		enum class EPointerPropertyMode : uint8_t
		{
			NoRawPointers,							///< Raw pointers cannot be used to point to other object, only ObjectPtr, ResourcePtr or ComponentPtr, otherwise an error will be returned
			OnlyRawPointers,						///< Only raw pointers can be used to point to other objects, an error will be returned in case of other pointer types
			AllPointerTypes							///< All types of pointers are supported; no checking is done on raw or non-raw
		};

	} //< End Namespace rtti
}
