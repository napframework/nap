/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/rtti.h>
#include <rtti/object.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>

namespace nap
{
	namespace SequenceGUIClipboards
	{
		class Clipboard
		{
			RTTI_ENABLE()
		public:
			Clipboard() = default;
			virtual ~Clipboard() = default;

			bool serialize(const rtti::Object* object);

			template<typename T>
			T* deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects);

			template<typename T>
			bool isClipboard()
			{
				return this->get_type() == RTTI_OF(T);
			}

			template<typename T>
			T* getDerived()
			{
				assert(isClipboard<T>());
				return static_cast<T*>(this);
			}

		protected:
			std::string mSerializedObject;
		};

		using SequenceClipboardPtr = std::unique_ptr<Clipboard>;

		// use this method to create an action
		template<typename T, typename... Args>
		static SequenceClipboardPtr createClipboard(Args&&... args)
		{
			return std::make_unique<T>(args...);
		}

		class Empty : public Clipboard { RTTI_ENABLE() };

		template<typename T>
		T* Clipboard::deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects)
		{
			//
			rtti::DeserializeResult result;
			utility::ErrorState errorState;

			//
			rtti::Factory factory;
			if (!rtti::deserializeJSON(
				mSerializedObject,
				rtti::EPropertyValidationMode::DisallowMissingProperties,
				rtti::EPointerPropertyMode::NoRawPointers,
				factory,
				result,
				errorState))
			{
				nap::Logger::error("Error deserializing, error : %s " , errorState.toString().c_str());
				return nullptr;
			}

			// Resolve links
			if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
			{
				nap::Logger::error("Error resolving links : %s " , errorState.toString().c_str());

				return nullptr;
			}

			//
			T* return_ptr = nullptr;

			// Move ownership of read objects
			createdObjects.clear();
			for (auto& read_object : result.mReadObjects)
			{
				//
				if (read_object->get_type().is_derived_from<T>())
				{
					return_ptr = dynamic_cast<T*>(read_object.get());
				}

				createdObjects.emplace_back(std::move(read_object));
			}

			// init objects
			for (auto& object_ptr : createdObjects)
			{
				if (!object_ptr->init(errorState))
					return nullptr;
			}

			if( return_ptr == nullptr )
			{
				nap::Logger::error("return object is null");
			}

			return return_ptr;
		}
	}
}
