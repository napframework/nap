/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rtti.h"
#include "path.h"
#include "deserializeresult.h"
#include "epropertyvalidationmode.h"

// External Includes
#include <utility/dllexport.h>
#include <rapidjson/pointer.h>
#include <string>

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}

	namespace rtti
	{
		class Factory;

		/**
		 * RTTI conversion state, populated during json document traversal.
		 */
		struct NAPAPI ReadState
		{
			ReadState(EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result) :
				mPropertyValidationMode(propertyValidationMode),
				mPointerPropertyMode(pointerPropertyMode),
				mFactory(factory),
				mResult(result)
			{ }

			EPropertyValidationMode			mPropertyValidationMode;
			EPointerPropertyMode			mPointerPropertyMode;
			Path							mCurrentRTTIPath;
			Factory&						mFactory;
			DeserializeResult&				mResult;
			std::unordered_set<std::string>	mObjectIDs;
		};


		/**
		 * Deserialize a set of objects and their data from the specified JSON string
		 *
		 * @param json The JSON string to deserialize
		 * @param propertyValidationMode whether missing required properties should be treated as errors
		 * @param pointerPropertyMode controls ownership of the created objects. Use 'NoRawPointers' in process and 'OnlyRawPointers' out of process.
		 * @param factory RTTI object factory. 
		 * @param result the result of the deserialization operation
		 * @param errorState contains the error when de-serialization fails.
		 *
		 * @return true if deserialization succeeded, false if not. In case of failure the errorState contains detailed error info.
		 */
		bool NAPAPI deserializeJSON(const std::string& json, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Read and deserialize a set of objects and their data from the specified JSON file
		 *
		 * @param path The JSON file to deserialize
		 * @param propertyValidationMode Whether missing required properties should be treated as errors
		 * @param pointerPropertyMode controls ownership of the created objects. Use 'NoRawPointers' in process and 'OnlyRawPointers' out of process.
		 * @param factory RTTI object factory.
		 * @param result The result of the deserialization operation
		 * @param errorState contains the error when deserialization fails.
		 *
		 * @return true if deserialization succeeded, false if not. In case of failure the errorState contains detailed error info.
		 */
		bool NAPAPI deserializeJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Deserialize a set of objects and their data from the specified JSON string
		 *
		 * @param jsonArray The JSON value to deserialize
		 * @param propertyValidationMode whether missing required properties should be treated as errors
		 * @param pointerPropertyMode controls ownership of the created objects. Use 'NoRawPointers' in process and 'OnlyRawPointers' out of process.
		 * @param factory RTTI object factory.
		 * @param result the result of the deserialization operation
		 * @param errorState contains the error when de-serialization fails.
		 *
		 * @return true if deserialization succeeded, false if not. In case of failure the errorState contains detailed error info.
		 */
		bool NAPAPI deserializeObjects(const rapidjson::Value& jsonArray, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Read and deserialize the first rtti::object from a JSON file. Ownership is transferred.
		 * @param path location of the JSON file on disk.
		 * @param propertyValidationMode property validation mode
		 * @param factory RTTI object creation factory.
		 * @param errorState contains the error if the operation fails.
		 * @return de-serialized rtti::object
		 */
		std::unique_ptr<nap::rtti::Object> NAPAPI getObjectFromJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, Factory& factory, utility::ErrorState& errorState);

		/**
		 * Read and deserialize the first rtti::object of type T from a JSON file. Ownership is transferred.
		 * @param path location of the JSON file on disk.
		 * @param propertyValidationMode property validation mode
		 * @param factory RTTI object creation factory.
		 * @param errorState contains the error if the operation fails.
		 * @return de-serialized rtti::object
		 */
		template<typename T>
		std::unique_ptr<T> getObjectFromJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, Factory& factory, utility::ErrorState& errorState);

		/**
		 * Parse JSON text from a read-only string.
		 * @param json string to parse
		 * @param document output document
		 * @param errorState contains the error if string can't be parsed
		 */
		bool NAPAPI JSONDocumentFromString(const std::string& json, rapidjson::Document& document, nap::utility::ErrorState& errorState);


		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		std::unique_ptr<T>
			getObjectFromJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, Factory& factory, utility::ErrorState& errorState)
		{
			auto obj = getObjectFromJSONFile(path, propertyValidationMode, factory, errorState);
			if (obj == nullptr)
			{
				errorState.fail("Failed to extract object of type: %s", 
					RTTI_OF(T).get_name().to_string().c_str());
				return{};
			}
			auto t = rtti_cast<T>(obj);
			if (t == nullptr)
				errorState.fail("Expected %s, got %s in file %s",
					RTTI_OF(T).get_name().data(),
					obj->get_type().get_name().data(),
					path.c_str());
			return t;
		}

	} //< End Namespace nap
}
