#pragma once

// Local Includes
#include "nshaderutils.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	class Texture2D;

	/**
	 * Base class for all types of uniforms, whether texture or value.
	 */
	class NAPAPI Uniform : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		* @return the type that this uniform can handle. This should map to the shader's type.
		*/
		virtual opengl::EGLSLType getGLSLType() const = 0;

		std::string mName;		///< Name of uniform as in shader
	};

	/**
	 * Represents a 'struct' uniform. A struct can contain other uniforms, but does not directly map onto a uniform in the shader
	 */
	class NAPAPI UniformStruct : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		void addUniform(Uniform& uniform);
		Uniform* findUniform(const std::string& name);

		virtual opengl::EGLSLType getGLSLType() const { return opengl::EGLSLType::Int; }

	public:
		std::vector<rtti::ObjectPtr<Uniform>> mUniforms;
	};

	/**
	 * Represents an 'array of structures' uniform. A UniformStructArray can contain UniformStructs, but does not directly map onto a uniform in the shader
	 */
	class NAPAPI UniformStructArray : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:
		void insertStruct(int index, UniformStruct& uniformStruct);

		virtual opengl::EGLSLType getGLSLType() const { return opengl::EGLSLType::Int; }

	public:
		std::vector<rtti::ObjectPtr<UniformStruct>> mStructs;
	};

	/**
	* Represents a 'value' uniform, or: something that is not a texture.
	* Derived classes should store value data and implement push() to update the
	* value in the shader.
	*/
	class NAPAPI UniformValue : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const = 0;
	};

	/**
	* Represents an array of 'value' uniforms.
	* Derived classes should return the correct amount of elements that are in the array.
	*/
	class NAPAPI UniformValueArray : public UniformValue
	{
		RTTI_ENABLE(UniformValue)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const = 0;
	};

	/**
	* Represents a texture uniform.
	* Derived classes should activate the texture unit, bind the appropriate texture
	* to the unit (whether 1D, 2D or 3D) and update the uniform in the shader.
	*/
	class NAPAPI UniformTexture : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		* @param textureUnit: the texture unit to activate for this texture.
		*
		* @return The number of texture units used by this uniform
		*/
		virtual int push(const opengl::UniformDeclaration& declaration, int textureUnit) const = 0;
	};


	/**
	* Represents an array of 'value' uniforms.
	* Derived classes should return the correct amount of elements that are in the array.
	*/
	class NAPAPI UniformTextureArray : public UniformTexture
	{
		RTTI_ENABLE(UniformTexture)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const = 0;
	};

	/**
	* Stores integer data and is capable of updating the integer uniform in the shader.
	*/
	class NAPAPI UniformInt : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		* @param value integer value to set.
		*/
		void setValue(int value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		* @return integer GLSL type.
		*/
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Int; }

		int mValue = 0;			///< Data storage
	};


	/**
	* Stores integer data and is capable of updating the integer uniform in the shader.
	*/
	class NAPAPI UniformFloat : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		 * @param value integer value to set.
		 */
		void setValue(float value) { mValue = value; }

		/**
		 * Updates the uniform in the shader.
		 * @param declaration: the uniform declaration from the shader that is used to set the value.
		 */
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * @return integer GLSL type.
		 */
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Float; }

		float mValue = 0.0f;			///< Data storage
	};


	/**
	* Stores vec4 data and is capable of updating the vec4 uniform in the shader.
	*/
	class NAPAPI UniformVec3 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		 * @param value vec4 value to set.
		 */
		void setValue(const glm::vec3& value) { mValue = value; }

		/**
		 * Updates the uniform in the shader.
		 * @param declaration: the uniform declaration from the shader that is used to set the value.
		 */
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * @return vec4 GLSL type.
		 */
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Vec3; }

		glm::vec3 mValue;		///< Data storage
	};


	/**
	* Stores vec4 data and is capable of updating the vec4 uniform in the shader.
	*/
	class NAPAPI UniformVec4 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		 * @param value vec4 value to set.
		 */
		void setValue(const glm::vec4& value)	{ mValue = value; }

		/**
		 * Updates the uniform in the shader.
		 * @param declaration: the uniform declaration from the shader that is used to set the value.
		 */
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * @return vec4 GLSL type.
		 */
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Vec4; }

		glm::vec4 mValue;		///< Data storage
	};


	/**
	* Stores mat4 data and is capable of updating the mat4 uniform in the shader.
	*/
	class NAPAPI UniformMat4 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		* @param value mat4 value to set.
		*/
		void setValue(const glm::mat4& value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		* @return mat4 GLSL type.
		*/
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Mat4; }

		glm::mat4 mValue;		///< Data storage
	};


	/**
	* Texture2D type uniform
	*/
	class NAPAPI UniformTexture2D : public UniformTexture
	{
		RTTI_ENABLE(UniformTexture)
	public:

		/**
		* @param texture The texture resource to set for this uniform.
		*/
		void setTexture(Texture2D& texture) { mTexture = &texture; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		* @param textureUnit: the texture unit to activate for this texture.
		*
		* @return The number of texture units used by this uniform
		*/
		virtual int push(const opengl::UniformDeclaration& declaration, int textureUnit) const override;

		/**
		* @return texture GLSL type.
		*/
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Tex2D; }

		rtti::ObjectPtr<Texture2D> mTexture = nullptr;		///< Texture to use for this uniform
	};
	
	//////////////////////////////////////////////////////////////////////////

	/**
	* Stores an array of integer data and is capable of updating the integer uniform in the shader.
	*/
	class NAPAPI UniformIntArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)
	public:

		UniformIntArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformIntArray(int inSize) :
			mValues(inSize)
		{
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mValues.size(); }

		/**
		* @return integer GLSL type.
		*/
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Int; }

		std::vector<int> mValues;			///< Data storage
	};


	/**
	* Stores an array of float data and is capable of updating the integer uniform in the shader.
	*/
	class NAPAPI UniformFloatArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)
	public:

		UniformFloatArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformFloatArray(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Updates the uniform in the shader.
		 * @param declaration: the uniform declaration from the shader that is used to set the value.
		 */
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * @return integer GLSL type.
		 */
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Float; }

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mValues.size(); }

		std::vector<float> mValues;			///< Data storage
	};


	/**
	* Stores an array of vec3 data and is capable of updating the vec3 uniform in the shader.
	*/
	class NAPAPI UniformVec3Array : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)
	public:

		UniformVec3Array() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformVec3Array(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Updates the uniform in the shader.
		 * @param declaration: the uniform declaration from the shader that is used to set the value.
		 */
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * @return vec4 GLSL type.
		 */
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Vec3; }

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mValues.size(); }

		std::vector<glm::vec3> mValues;		///< Data storage
	};


	/**
	* Stores an array of vec4 data and is capable of updating the vec4 uniform in the shader.
	*/
	class NAPAPI UniformVec4Array : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)
	public:

		UniformVec4Array() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformVec4Array(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Updates the uniform in the shader.
		 * @param declaration: the uniform declaration from the shader that is used to set the value.
		 */
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		 * @return vec4 GLSL type.
		 */
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Vec4; }

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mValues.size(); }

		std::vector<glm::vec4> mValues;		///< Data storage
	};


	/**
	* Stores an array of mat4 data and is capable of updating the mat4 uniform in the shader.
	*/
	class NAPAPI UniformMat4Array : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)
	public:

		UniformMat4Array() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformMat4Array(int inSize) :
			mValues(inSize)
		{
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		* @return mat4 GLSL type.
		*/
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Mat4; }

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mValues.size(); }

		std::vector<glm::mat4> mValues;		///< Data storage
	};


	/**
	* Array of Texture2D type uniform. 
	*/
	class NAPAPI UniformTexture2DArray : public UniformTextureArray
	{
		RTTI_ENABLE(UniformTextureArray)
	public:

		UniformTexture2DArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformTexture2DArray(int inSize) :
			mTextures(inSize)
		{
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		* @param textureUnit: the texture unit to activate for this texture.
		*
		* @return The number of texture units used by this uniform
		*/
		virtual int push(const opengl::UniformDeclaration& declaration, int textureUnit) const override;

		/**
		* @return texture GLSL type.
		*/
		virtual opengl::EGLSLType getGLSLType() const override { return opengl::EGLSLType::Tex2D; }

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mTextures.size(); }

		std::vector<rtti::ObjectPtr<Texture2D>> mTextures;		///< Texture to use for this uniform

	private:
		mutable std::vector<int> mTextureUnits;
	};
}
