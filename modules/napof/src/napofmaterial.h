#pragma once

// RTTI Includes
#include <rtti/rtti.h>

// Nap Includes
#include <nap/serviceablecomponent.h>
#include <nap/signalslot.h>
#include <nap/logger.h>
#include <nap/coremodule.h>

#include <napofattributes.h>

// OF Includes
#include <ofShader.h>

namespace nap
{

	/**
	@brief NAP Openframeworks material component
	**/
	class OFMaterial : public ServiceableComponent
	{
		friend class OFService;

		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)

	public:
		using MaterialTextureMap = std::unordered_map<std::string, const ofTexture*>;
		using BindFunction = std::function<void(AttributeBase&, ofShader&, int&)>;

		// register bind function for a specific type using a lambda
		static void registerBindFunction(RTTI::TypeInfo type, BindFunction bindFunction);

		// register bind function for a specific type using a function pointer
		static void registerBindFunction(RTTI::TypeInfo type, void(*bindFunction)(AttributeBase&, ofShader&, int&)) { 
			registerBindFunction(type, [bindFunction](AttributeBase& attr, ofShader& shader, int& tex) { bindFunction(attr, shader, tex); }); 
		}

		//@name Material
		OFMaterial();

		//@name Attributes
		Attribute<std::string> mShader = { this, "Shader" };

		// returns a binding by name
		AttributeBase* getBinding(const std::string& name);

		//@name Utility
		const ofShader& getShader() const													{ return mOFShader; }
		bool isLoaded() const																{ return mOFShader.isLoaded();  }

		//@name Slots
		NSLOT(mShaderChanged, const std::string&, shaderChanged)

		//@Name Binding
		void bind();
		void unbind();
	
	private:
		// static initialization of map with bind functions
		static std::map<RTTI::TypeInfo, BindFunction>& getBindFunctions() {
			static std::map<RTTI::TypeInfo, BindFunction> funcs;
			return funcs;
		}
		
		// bookkeeping of all dynamically created attributes that are used as shader bindings
		std::set<AttributeBase*> mBindings;

		///@name Shader
		ofShader			mOFShader;

		void shaderChanged(const std::string&);

		// called when a new attribute is created to register it as a shader binding
		void onChildAdded(nap::Object& object)
		{
			auto attribute = dynamic_cast<AttributeBase*>(&object);
			assert(attribute);
			mBindings.emplace(attribute);
		}

		// called when an attribute is removed so it will be removed from the shader bindings
		void onChildRemoved(nap::Object& object) {
			auto attribute = dynamic_cast<AttributeBase*>(&object);
			assert(attribute);
			mBindings.erase(attribute);
		}


		///@name Utility
		template <typename T>
		void addBinding(const std::string& inName, T& inValue, std::unordered_map<std::string, T>& inMap);

		template <typename T>
		void addPtrBinding(const std::string& inName, T& inValue, std::unordered_map<std::string, T*>& inMap);

		template <typename T>
		void removeBinding(const std::string& inName, std::unordered_map<std::string, T>& inMap);

		template <typename T>
		void setValue(const std::string& inName, T& inValue, std::unordered_map<std::string, T>& inMap);

		template <typename T>
		void setValue(const std::string& inName, T& inValue, std::unordered_map<std::string, T*>& inMap);
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////


	/**
	@brief Adds a binding to the incoming map with name@ inName
	**/
	template <typename T>
	void OFMaterial::addBinding(const std::string& inName, T& inValue, std::unordered_map<std::string, T>& inMap)
	{
		inMap.emplace(std::make_pair(inName, inValue));
	}


	/**
	@brief Adds a binding to the incoming map with name@ inName
	**/
	template <typename T>
	void OFMaterial::addPtrBinding(const std::string& inName, T& inValue, std::unordered_map<std::string, T*>& inMap)
	{
		inMap.emplace(std::make_pair(inName, &inValue));
	}


	/**
	@brief Removes a binding from the incoming map with name @inName
	**/
	template <typename T>              
	void OFMaterial::removeBinding(const std::string& inName, std::unordered_map<std::string, T>& inMap)
	{
		auto& v = inMap.find(inName);
		if (v == inMap.end())
		{
			Logger::warn("Unable to find element in bindings: " + inName);
			return;
		}
		inMap.erase(v);
	}


	/**
	@brief Sets the binding to the value @T
	**/
	template <typename T>
	void OFMaterial::setValue(const std::string& inName, T& inValue, std::unordered_map<std::string, T>& inMap)
	{
		auto& v = inMap.find(inName);
		if (v == inMap.end())
		{
			Logger::warn("Unable to find element in bindings: " + inName + " ,adding..");
			return addBinding<T>(inName, inValue, inMap);
		}
		v->second = inValue;
	}



	/**
	@brief Sets the binding to the value @T
	**/
	template <typename T>
	void OFMaterial::setValue(const std::string& inName, T& inValue, std::unordered_map<std::string, T*>& inMap)
	{
		auto& v = inMap.find(inName);
		if (v == inMap.end())
		{
			Logger::warn("Unable to find element in bindings: " + inName + " ,adding..");
			return addPtrBinding<T>(inName, inValue, inMap);
			return;
		}
		v->second = &inValue;
	}
}

RTTI_DECLARE(nap::OFMaterial)
