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
#include <rtti/objectptr.h>
#include <nap/logger.h>

namespace nap
{
	namespace SequenceGUIClipboards
	{
		/**
		 * Clipboard is a class that can contain a serialized  object related to the Sequencer
		 * The state of the gui can contain a clipboard, that can be de-serialized at a certain point
		 * Typically, the clipboard contains a certain curve segment or event segment, and that segment can be pasted
		 * at a certain location in a certain Track
		 */
		class NAPAPI Clipboard
		{
			RTTI_ENABLE()
		public:
			/**
			 * Default Constructor
			 */
			Clipboard() = default;

			/**
			 * Default decontructor
			 */
			virtual ~Clipboard() = default;

			/**
			 * Serialize an object
			 * @param object pointer object to serialize
			 * @param errorState holds information about any errors
			 */
			void serialize(const rtti::Object* object, utility::ErrorState& errorState);

			/**
			 * Deserialize clipboard content to object of type T
			 * @tparam T the object type to deserialze
			 * @param createdObjects vector containing created objects
			 * @param errorState holds information about any errors
			 * @return pointer to root object, can be null and must be of type T
			 */
			template<typename T>
			T* deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, utility::ErrorState& errorState);

			/**
			 * returns true when clipboard is clipboard of derived type T
			 * @tparam T the derived type
			 * @return true when T is of derived type
			 */
			template<typename T>
			bool isClipboard()
			{
				return this->get_type() == RTTI_OF(T);
			}

			/**
			 * returns raw pointer to derived class T of this clipboard
			 * performs static cast, exception on fail, always use isClipboard<T> to check for type
			 * @tparam T the derived type
			 * @return pointer to T
			 */
			template<typename T>
			T* getDerived()
			{
				assert(isClipboard<T>());
				return static_cast<T*>(this);
			}

		protected:
			// the serialized object
			std::string mSerializedObject;
		};

		// shortcut
		using SequenceClipboardPtr = std::unique_ptr<Clipboard>;

		// use this method to create a clipboard
		template<typename T, typename... Args>
		static SequenceClipboardPtr createClipboard(Args&&... args)
		{
			return std::make_unique<T>(std::forward<Args>(args)...);
		}

		/**
		 * Empty clipboard
		 */
		class NAPAPI Empty : public Clipboard { RTTI_ENABLE() };


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		T* Clipboard::deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, utility::ErrorState& errorState)
		{
			// De-serialize
			rtti::DeserializeResult result;
			rtti::Factory factory;
			if (!rtti::deserializeJSON(mSerializedObject, rtti::EPropertyValidationMode::DisallowMissingProperties,
				rtti::EPointerPropertyMode::NoRawPointers, factory, result, errorState))
				return nullptr;

			// Resolve links
			if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
				return nullptr;

			// Make sure we deserialized something
			T* root_object = nullptr;
			if(!errorState.check(result.mReadObjects.size() > 0, "No objects deserialized"))
				 return nullptr;

			auto* first_object = result.mReadObjects[0].get();
			if( !errorState.check(first_object->get_type() == RTTI_OF(T), "Root object not of correct type"))
				return nullptr;

			// Move ownership of read objects
			root_object = static_cast<T*>(first_object);
			createdObjects.clear();
			for (auto& read_object : result.mReadObjects)
			{
				if (read_object->get_type().is_derived_from<T>())
					root_object = dynamic_cast<T*>(read_object.get());
				createdObjects.emplace_back(std::move(read_object));
			}

			// init objects
			for (auto& object_ptr : createdObjects)
			{
				if (!errorState.check(object_ptr->init(errorState), "Error initializing object : %s " , errorState.toString().c_str()))
					return nullptr;
			}

			if( !errorState.check(root_object != nullptr, "return object is null"))
				return nullptr;

			return root_object;
		}
	}
}
