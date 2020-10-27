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
			bool deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, rtti::ObjectPtr<T>& rootObject);

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
		bool Clipboard::deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, rtti::ObjectPtr<T>& rootObject)
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
				return false;
			}

			// Resolve links
			if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
			{
				nap::Logger::error("Error resolving links : %s " , errorState.toString().c_str());

				return false;
			}

			//
			T* root_object = nullptr;

			if(result.mReadObjects.size() > 0 )
			{
				auto* first_object = result.mReadObjects[0].get();
				if( first_object->get_type() != RTTI_OF(T) )
				{
					nap::Logger::error("Root object not of correct type");
					return false;
				}else
				{
					root_object = static_cast<T*>(first_object);
				}
			}else
			{
				nap::Logger::error("No objects deserialized");
				return false;
			}

			// Move ownership of read objects
			createdObjects.clear();
			for (auto& read_object : result.mReadObjects)
			{
				//
				if (read_object->get_type().is_derived_from<T>())
				{
					root_object = dynamic_cast<T*>(read_object.get());
				}

				createdObjects.emplace_back(std::move(read_object));
			}

			// init objects
			for (auto& object_ptr : createdObjects)
			{
				if (!object_ptr->init(errorState))
				{
					nap::Logger::error("Error initializing object : %s " , errorState.toString().c_str());
					return false;
				}
			}

			if( root_object == nullptr )
			{
				nap::Logger::error("return object is null");
				return false;
			}

			rootObject = rtti::ObjectPtr<T>(root_object);

			return true;
		}
	}
}
