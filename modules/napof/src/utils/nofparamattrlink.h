#pragma once

// OF Includes
#include <ofParameter.h>

// Nap Includes
#include <nap/attribute.h>
#include <nap/signalslot.h>

/**
@brief Abstract class that links an of parameter to an attribute
**/
class OFAbstractParamAttrLink
{
public:
	// Constructor
	OFAbstractParamAttrLink(ofAbstractParameter& param, nap::AttributeBase& attrib);
	
	// Destructor
	virtual ~OFAbstractParamAttrLink();

	// Getters
	std::string					getName() const;
	ofAbstractParameter*		getParameter() const	{ return mParameter.get(); }
	nap::AttributeBase*			getAttribute() const	{ return mAttribute; }
	bool						isLinked() const		{ return mAttribute != nullptr; }

	// Sets a new attribute to connect to a parameter
	void						setAttribute(nap::AttributeBase& attribute);

protected:
	// Members
	shared_ptr<ofAbstractParameter>	mParameter = nullptr;
	nap::AttributeBase*				mAttribute = nullptr;

	// Overrides for derived members
	virtual void				stopListening() = 0;
	virtual void				attributeChanged(nap::AttributeBase& new_attr) = 0;

	// Slots
	void attributeRemoved(nap::Object& object);
	NSLOT(mAttributeRemoved, nap::Object&, attributeRemoved)
};


/**
@brief Specialization of an attribute link to an of parameter

Handles values changes from a to b and b to a
**/
template<typename T>
class OFParamAttrLink : public OFAbstractParamAttrLink
{
public:
	OFParamAttrLink(ofParameter<T>& param, nap::Attribute<T>& attribute);
	~OFParamAttrLink();

protected:
	virtual void		stopListening() override;
	virtual void		attributeChanged(nap::AttributeBase& new_attr) override;

private:
	// Callbacks
	void				parameterValueChanged(T& value);
	void				attributeValueChanged(const T& value);
    void                attributeRangeChanged(const nap::NumericAttribute<T>&);

	// Utility
	ofParameter<T>*		getParameter();
	nap::Attribute<T>*	getAttribute();

	// SLOTS
	NSLOT(mAttributeValueChanged, const T&, attributeValueChanged)
    NSLOT(mAttributeRangeChanged, const nap::NumericAttribute<T>&, attributeRangeChanged)
    
};


/**
@brief Specialization of a SignalAttribute

Creates a bool parameter that triggers the SignalAttribute when on
Automatically turns the parameter back off
**/
class OFParamSignalLink : public OFAbstractParamAttrLink
{
public:
	OFParamSignalLink(ofParameter<bool>& param, nap::SignalAttribute& attribute);

protected:
	virtual void			stopListening() override;
	virtual void			attributeChanged(nap::AttributeBase& new_attr) override;

private:
	void					parameterValueChanged(bool& value);
	nap::SignalAttribute*	getSignalAttribute();
	ofParameter<bool>*		getParameter();
};



//////////////////////////////////////////////////////////////////////////
// Template Definitions
//////////////////////////////////////////////////////////////////////////

/**
@brief Constructor

Registers itself as a listener
**/
template<typename T>
OFParamAttrLink<T>::OFParamAttrLink(ofParameter<T>& param, nap::Attribute<T>& attribute) : 
	OFAbstractParamAttrLink(param, attribute)
{
	param.addListener(this, &OFParamAttrLink<T>::parameterValueChanged);
	attribute.valueChangedSignal.connect(mAttributeValueChanged);
    
    // type check has to be done with dynamic cast because RTTI on NumericAttribute<bool> fails compile time
    auto numericAttribute = dynamic_cast<nap::NumericAttribute<T>*>(&attribute);
    if (numericAttribute)
        numericAttribute->rangeChanged.connect(mAttributeRangeChanged);
}


template<typename T>
OFParamAttrLink<T>::~OFParamAttrLink()
{
	stopListening();
}


/**
@brief When the attribute changes, update the parameter
**/
template<typename T>
void OFParamAttrLink<T>::attributeValueChanged(const T& value)
{
	if (mParameter == nullptr)
	{
		assert(false);
		return;
	}
	getParameter()->set(value);
}


/**
 @brief When the attribute changes, update the parameter
 **/
template<typename T>
void OFParamAttrLink<T>::attributeRangeChanged(const nap::NumericAttribute<T>& attribute)
{
    if (mParameter == nullptr)
    {
        assert(false);
        return;
    }
    getParameter()->setMin(attribute.getMin());
    getParameter()->setMax(attribute.getMax());
}


/**
@brief When the parameter changes, update the attribute
**/
template<typename T>
void OFParamAttrLink<T>::parameterValueChanged(T& value)
{
	if (mAttribute == nullptr)
	{
		assert(false);
		return;
	}
	getAttribute()->setValue(value);
}


/**
@brief When an attribute disconnects (ie is removed), stop listening
**/
template<typename T>
void OFParamAttrLink<T>::stopListening()
{
	getParameter()->removeListener(this, &OFParamAttrLink<T>::parameterValueChanged);
}


/**
@brief When an attribute is swapped, stop listening to state changes
**/
template<typename T>
void OFParamAttrLink<T>::attributeChanged(nap::AttributeBase& new_attr)
{
	getAttribute()->valueChangedSignal.disconnect(mAttributeValueChanged);
    
    auto numericAttribute = dynamic_cast<nap::NumericAttribute<T>*>(getAttribute());
    if (numericAttribute)
        numericAttribute->rangeChanged.disconnect(mAttributeRangeChanged);
}



/**
@brief Get specialized parameter
**/
template<typename T>
ofParameter<T>* OFParamAttrLink<T>::getParameter()
{
	return mParameter == nullptr ? nullptr : static_cast<ofParameter<T>*>(mParameter.get());
}


/**
@brief Returns the specialized attribute
**/
template<typename T>
nap::Attribute<T>* OFParamAttrLink<T>::getAttribute()
{
	return mAttribute == nullptr ? nullptr : static_cast<nap::Attribute<T>*>(mAttribute);
}
