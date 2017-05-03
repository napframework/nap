#include "RTTITestClasses.h"
#include <rtti/rtti.h>
#include <nap/rttiutilities.h>
#include <nap/JSonReader.h>
#include <nap/logger.h>
#include <nap/core.h>

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <nap/resource.h>
#include <nap/stringutils.h>

#include <nap/rttipath.h>

typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> JSONTextWriter;

class RTTIStreamWriter
{
public:
	
	virtual bool Start() = 0;
	virtual bool Finish() = 0;

	virtual bool StartRootObject(const RTTI::TypeInfo& type) = 0;
	virtual bool EndRootObject() = 0;
	
	virtual bool StartNestedObject(const RTTI::TypeInfo& type) = 0;
	virtual bool EndNestedObject() = 0;
	
	virtual bool StartArray(int length) = 0;
	virtual bool EndArray() = 0;
	
	virtual bool WriteProperty(const std::string& propertyName) = 0;
	virtual bool WritePointer(const std::string& pointeeID) = 0;
	virtual bool WritePrimitive(const RTTI::TypeInfo& type, const RTTI::Variant& value) = 0;
};

class RTTIJSONStreamWriter : public RTTIStreamWriter
{
public:
	RTTIJSONStreamWriter() :
		mWriter(mStringBuffer)
	{
	}

	virtual bool Start() override
	{
		return mWriter.StartObject();
	}

	virtual bool Finish() override
	{
		if (!mWriter.EndObject())
			return false;

		return true;
	}

	std::string GetJSON()
	{
		return mStringBuffer.GetString();
	}

	virtual bool StartRootObject(const RTTI::TypeInfo& type) override
	{
		if (!mWriter.String(type.get_name().data()))
			return false;

		return mWriter.StartObject();
	}

	virtual bool EndRootObject() override
	{
		return mWriter.EndObject();
	}

	virtual bool StartNestedObject(const RTTI::TypeInfo& type) override
	{
		return mWriter.StartObject();
	}

	virtual bool EndNestedObject() override
	{
		return mWriter.EndObject();
	}

	virtual bool StartArray(int length) override
	{
		return mWriter.StartArray();
	}

	virtual bool EndArray() override
	{
		return mWriter.EndArray();
	}

	virtual bool WriteProperty(const std::string& propertyName) override
	{
		return mWriter.String(propertyName);
	}

	virtual bool WritePointer(const std::string& pointeeID) override
	{
		//std::string pointee_id = "<" + pointeeID + ">";
		return mWriter.String(pointeeID);
	}

	virtual bool WritePrimitive(const RTTI::TypeInfo& type, const RTTI::Variant& value) override
	{
		if (type.is_arithmetic())
		{
			if (type == RTTI::TypeInfo::get<bool>())
				return mWriter.Bool(value.to_bool());
			else if (type == RTTI::TypeInfo::get<char>())
				return mWriter.Bool(value.to_bool());
			else if (type == RTTI::TypeInfo::get<int8_t>())
				return mWriter.Int(value.to_int8());
			else if (type == RTTI::TypeInfo::get<int16_t>())
				return mWriter.Int(value.to_int16());
			else if (type == RTTI::TypeInfo::get<int32_t>())
				return mWriter.Int(value.to_int32());
			else if (type == RTTI::TypeInfo::get<int64_t>())
				return mWriter.Int64(value.to_int64());
			else if (type == RTTI::TypeInfo::get<uint8_t>())
				return mWriter.Uint(value.to_uint8());
			else if (type == RTTI::TypeInfo::get<uint16_t>())
				return mWriter.Uint(value.to_uint16());
			else if (type == RTTI::TypeInfo::get<uint32_t>())
				return mWriter.Uint(value.to_uint32());
			else if (type == RTTI::TypeInfo::get<uint64_t>())
				return mWriter.Uint64(value.to_uint64());
			else if (type == RTTI::TypeInfo::get<float>())
				return mWriter.Double(value.to_double());
			else if (type == RTTI::TypeInfo::get<double>())
				return mWriter.Double(value.to_double());

			return false;
		}
		else if (type.is_enumeration())
		{
			bool conversion_succeeded = false;
			auto result = value.to_string(&conversion_succeeded);
			if (conversion_succeeded)
			{
				return mWriter.String(value.to_string());
			}
			else
			{
				conversion_succeeded = false;
				auto value_int = value.to_uint64(&conversion_succeeded);
				if (conversion_succeeded)
					return mWriter.Uint64(value_int);
			}

			return false;
		}
		else if (type == RTTI::TypeInfo::get<std::string>())
		{
			return mWriter.String(value.to_string());
		}

		return false;
	}

private:
	rapidjson::StringBuffer mStringBuffer;
	JSONTextWriter			mWriter;
};

using namespace nap;

class RTTISerializer
{
public:
	typedef std::vector<Object*> ObjectList;

	RTTISerializer(RTTIStreamWriter& writer) :
		mStream(writer)
	{
	}

	bool Write(const ObjectList& rootObjects)
	{
		// Copy the list of objects to write to an internal list that we can modify
		mObjectsToWrite = rootObjects;

		if (!mStream.Start())
			return false;

		for (int index = 0; index < mObjectsToWrite.size(); ++index)
		{
			Object* object = mObjectsToWrite[index];

			const RTTI::TypeInfo& type = object->get_type();
			if (!mStream.StartRootObject(type))
				return false;

			if (!WriteObjectRecursive(object))
				return false;

			if (!mStream.EndRootObject())
				return false;
		}

		if (!mStream.Finish())
			return false;

		return true;
	}

private:
	bool WriteObjectRecursive(const RTTI::Instance object)
	{
		RTTI::Instance actual_object = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;

		for (const RTTI::Property& prop : actual_object.get_derived_type().get_properties())
		{
			RTTI::Variant prop_value = prop.get_value(actual_object);
			if (!prop_value)
				continue;

			if (!WriteProperty(prop, prop_value))
				return false;
		}

		return true;
	}

	bool WriteProperty(const RTTI::Property& property, const RTTI::Variant& value)
	{
		auto value_type = value.get_type();
		auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
		bool is_wrapper = wrapped_type != value_type;

		mStream.WriteProperty(property.get_name().data());
		if (value_type.is_array())
		{
			if (!WriteArray(value.create_array_view()))
				return false;

			return true;
		}
		else if (value_type.is_pointer())
		{
			if (!wrapped_type.is_derived_from<nap::Object>())
				return false;

			Object* pointee = value.convert<Object*>();
			if (pointee != nullptr)
			{
				AddObjectToWrite(pointee);

				const std::string& pointee_id = pointee->mID;
				if (pointee_id.empty())
					return false;

				return mStream.WritePointer(pointee_id);
			}
			else
			{
				return mStream.WritePointer(std::string());
			}
		}
		else
		{
			bool is_primitive = wrapped_type.is_arithmetic() || wrapped_type.is_enumeration() || wrapped_type == RTTI::TypeInfo::get<std::string>();
			if (is_primitive)
			{
				return mStream.WritePrimitive(wrapped_type, is_wrapper ? value.extract_wrapped_value() : value);
			}
			else
			{
				auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
				if (!child_props.empty())
				{
					if (!mStream.StartNestedObject(wrapped_type))
						return false;

					if (!WriteObjectRecursive(value))
						return false;

					if (!mStream.EndNestedObject())
						return false;

					return true;
				}
			}
		}

		return false;
	}

	bool WriteArray(const RTTI::VariantArray& array)
	{
		if (!mStream.StartArray(array.get_size()))
			return false;

		for (int i = 0; i < array.get_size(); ++i)
		{
			RTTI::Variant var = array.get_value_as_ref(i);
			if (var.is_array())
			{
				if (!WriteArray(var.create_array_view()))
					return false;
			}
			else
			{
				RTTI::Variant wrapped_var = var.extract_wrapped_value();
				RTTI::TypeInfo value_type = wrapped_var.get_type();

				bool is_primitive = value_type.is_arithmetic() || value_type.is_enumeration() || value_type == RTTI::TypeInfo::get<std::string>();
				if (value_type.is_pointer())
				{
					if (!value_type.is_derived_from<nap::Object>())
						return false;

					Object* pointee = wrapped_var.convert<Object*>();
					if (pointee != nullptr)
					{
						AddObjectToWrite(pointee);

						const std::string& pointee_id = pointee->mID;
						if (pointee_id.empty())
							return false;

						if (!mStream.WritePointer(pointee_id))
							return false;
					}
					else
					{
						if (!mStream.WritePointer(std::string()))
							return false;
					}
				}
				else if (is_primitive)
				{
					if (!mStream.WritePrimitive(value_type, wrapped_var))
						return false;
				}
				else // object
				{
					if (!mStream.StartNestedObject(value_type))
						return false;

					if (!WriteObjectRecursive(wrapped_var))
						return false;

					if (!mStream.EndNestedObject())
						return false;
				}
			}
		}

		return mStream.EndArray();
	}

	void AddObjectToWrite(Object* object)
	{
		if (std::find(mObjectsToWrite.begin(), mObjectsToWrite.end(), object) != mObjectsToWrite.end())
			return;

		mObjectsToWrite.push_back(object);
	}

private:
	RTTIStreamWriter& mStream;
	ObjectList mObjectsToWrite;
};

BaseClass* createTestHierarchy()
{
	DerivedClass* pointee = new DerivedClass();
	pointee->mID								= "Pointee";
	pointee->mIntProperty						= 42;
	pointee->mStringProperty					= "Pointee String";
	pointee->mNestedCompound.mFloatProperty		= 16.0f;
	pointee->mNestedCompound.mPointerProperty	= pointee;
	pointee->mArrayOfInts.push_back(1);
	pointee->mArrayOfInts.push_back(2);
	pointee->mArrayOfInts.push_back(3);
	pointee->mArrayOfCompounds.push_back(DataStruct(4.0f, pointee));
	pointee->mArrayOfCompounds.push_back(DataStruct(5.0f));
	pointee->mArrayOfCompounds.push_back(DataStruct(6.0f));
	pointee->mArrayOfPointers.push_back(pointee);

	BaseClass* root = new BaseClass();
	root->mID									= "Root";
	root->mIntProperty							= 42 / 2;
	root->mStringProperty						= "Root String";
	root->mPointerProperty						= pointee;

	return root;
}

bool ResolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers)
{
	std::map<std::string, Object*> objects_by_id;
	for (auto& object : objects)
		objects_by_id.insert({ object->mID, object.get() });
	
	for (const UnresolvedPointer& unresolvedPointer : unresolvedPointers)
	{
		RTTI::ResolvedRTTIPath resolved_path = unresolvedPointer.mRTTIPath.Resolve(unresolvedPointer.mObject);
		if (!resolved_path.IsValid())
			return false;

		std::map<std::string, Object*>::iterator pos = objects_by_id.find(unresolvedPointer.mTargetID);
		if (pos == objects_by_id.end())
			return false;

		if (!resolved_path.SetValue(pos->second))
			return false;
	}

	return true;
}


int main(int argc, char* argv[])
{
	Logger::setLevel(Logger::debugLevel());

	Core core;
	core.initialize();

	// Create test hierarchy
	BaseClass* root = createTestHierarchy();
	 
	// Create path to float property in array of nested compounds
 	RTTI::RTTIPath float_property_path;
 	float_property_path.PushAttribute("ArrayOfCompounds");
 	float_property_path.PushArrayElement(0);
 	float_property_path.PushAttribute("FloatProperty");
 
	// Convert path to string
 	std::string path_str = float_property_path.ToString();

	// Convert back and verify the path is the same
 	RTTI::RTTIPath path_copy = RTTI::RTTIPath::FromString(path_str);
	if (path_copy != float_property_path)
		return -1;
 
	// Resolve the path and verify it succeeded
 	RTTI::ResolvedRTTIPath resolved_path = float_property_path.Resolve(root->mPointerProperty);
	if (!resolved_path.IsValid())
		return -1;

	// Verify setting the value works
	float old_value = resolved_path.GetCurrentValue().convert<float>();
	if (!resolved_path.SetValue(8.0f))
		return -1; 

	// Restore value so we can compare later
	resolved_path.SetValue(old_value);

	// Write to json
	RTTIJSONStreamWriter stream;
	RTTISerializer writer(stream);
	if (!writer.Write({ root }))
		return -1;

	// Print json
	std::string json = stream.GetJSON();
	std::cout << json << std::endl;

	// Read json and verify it succeeds
	ReadJSONFileResult read_result;
	InitResult init_result;
	if (!readJSON(json, read_result, init_result))
		return -1;

	// Resolve links
	if (!ResolveLinks(read_result.mReadObjects, read_result.mUnresolvedPointers))
		return -1;

	// Sort read objects into id mapping
	std::map<std::string, Object*> objects_by_id;
	for (auto& object : read_result.mReadObjects)
		objects_by_id.insert({ object->mID, object.get() });

	// Compare root objects
	if (!RTTI::areObjectsEqual(*objects_by_id["Root"], *root, RTTI::EPointerComparisonMode::BY_ID))
		return -1;

	// Compare pointee-objects
	if (!RTTI::areObjectsEqual(*objects_by_id["Pointee"], *root->mPointerProperty, RTTI::EPointerComparisonMode::BY_ID))
		return -1;

	return 0;
}