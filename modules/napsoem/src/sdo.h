#pragma once

#include <nap/numeric.h>
#include <utility/dllexport.h>
#include <rtti/rtti.h>
#include <vector>
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Base class of all SDO (Service Data Object) objects.
	 * Groups together useful SDO information such as the SDO index, subindex and description.
	 * Used to read / write data to a specific register using a nap::EthercatMaster.
	 * This requires the slave to have a mailbox and to support the "CANopen over EtherCAT" (CoE) protocol.
	 * Derive from this class to create your own SDO read & write objects.
	 */
	class NAPAPI SDOArgument : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * @param index object index
		 * @param subIndex object sub index
		 * @param single only read / write a single sub index or complete access
		 */
		SDOArgument(uint16 index, uint16 subIndex, bool single);

		/**
		 * @param index object index
		 * @param subIndex object sub index
		 * @param name name of index to set
		 * @param single only read / write a single sub index or complete access
		 */
		 SDOArgument(uint16 index, uint16 subIndex, const std::string& name, bool single);

		// Default destructor
		virtual ~SDOArgument() = default;

		/**
		 * Converts index and sub-index into numerical representation.
		 * @param errorState contains error message if conversion fails
		 * @return if conversion succeeded
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * Override in derived classes
		 * @return size in bytes of the data block that is set
		 */
		virtual int getSize() = 0;

		/**
		 * Override in derived classes.
		 * Returns the pointer to the block of data.
		 * @return pointer to the block of data
		 */
		virtual void* getData() = 0;

		/**
		 * @return name (optional)
		 */
		const std::string& getDescription() const { return mDescription; }

		/**
		 * @return SDO index
		 */
		uint16 getIndex() const						{ return mIndex; }

		/**
		 * @return SDO sub index
		 */
		uint16 getSubIndex() const					{ return mSubIndex; }

		/**
		 * @return if only a single sub index is written or read at a time.
		 */
		bool singleSubIndex() const					{ return mSingle; }

		bool mSingle = true;			///< Property: 'Single' Only read / write a single subindex. Complete access to index otherwise.
		std::string mDescription;		///< Property: 'Description' optional description
		std::string mIndexStr;			///< Property: 'Index' index as string, for example: '0x8011'. Converted to number on init().
		std::string mSubIndexStr;		///< Property: 'SubIndex' sub index as string, for example: '0x03'. Converted to number on init().
		int mBase = 16;					///< Property: 'Base' the number base used for conversion on init, defaults to 16 (hex).

	private:
		uint16 mIndex = 0x00;			///< Index of the register
		uint16 mSubIndex = 0x00;		///< Sub index of the register
	};


	//////////////////////////////////////////////////////////////////////////
	// SDOValue
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Single SDO (Service Data Object) value.
	 * Groups together useful SDO information such as the SDO index, subindex, description and value to get or set.
	 * Used to read / write a value to a single sub-index of a register using a nap::EthercatMaster.
	 * This requires the slave to have a mailbox and to support the "CANopen over EtherCAT" (CoE) protocol.
	 */
	template<typename T>
	class SDOValue : public SDOArgument
	{
		RTTI_ENABLE(SDOArgument)
	public:

		// Default constructor
		SDOValue() : SDOArgument(0x00, 0x00, true)				{ }

		/**
		 * @param index object index
		 * @param subIndex object sub index
		 * @param value object value
		 */
		SDOValue(uint16 index, uint16 subIndex, T value) : 
			SDOArgument(index, subIndex, true), 
			mValue(value)										{ }

		/**
		 * @param index object index
	 	 * @param subIndex object sub index
		 * @param name name of index to set
		 * @param value object value
		 */
		SDOValue(uint16 index, uint16 subIndex, const std::string& name, T value) :
			SDOArgument(index, subIndex, name, true), 
			mValue(value)										{ }

		/**
		 * Size in bytes of the value
		 * @return size in bytes of the data block that is set
		 */
		virtual int getSize()								{ return sizeof(T); }

		/**
		* Returns pointer to the value.
		* @return pointer to the value
		*/
		virtual void* getData()								{ return &mValue; }

	public:
		T mValue;											///< Property: 'Value' data to set or get
	};


	//////////////////////////////////////////////////////////////////////////
	// SDO value template definitions
	//////////////////////////////////////////////////////////////////////////

	using SDO_int8		= SDOValue<nap::int8>;
	using SDO_uint8		= SDOValue<nap::uint8>;
	using SDO_int16		= SDOValue<nap::int16>;
	using SDO_uint16	= SDOValue<nap::uint16>;
	using SDO_int32		= SDOValue<nap::int32>;
	using SDO_uint32	= SDOValue<nap::uint32>;
	using SDO_float		= SDOValue<float>;
	using SDO_double	= SDOValue<double>;
	using SDO_int64		= SDOValue<nap::int64>;
	using SDO_uint64	= SDOValue<nap::uint64>;


	//////////////////////////////////////////////////////////////////////////
	// SDO Group
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Groups together a set of CANopen service dictionary objects.
	 * These objects together can form, for example, a configuration.
	 */
	class NAPAPI SDOGroup : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::vector<ResourcePtr<SDOArgument>> mObjects;		///< Property: 'Objects' all service dictionary objects to group
	};
}
