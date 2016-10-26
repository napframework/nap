#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/operator.h>
#include <nap/attribute.h>
#include <nap/coreattributes.h>
#include <nap/coretypeconverters.h>

namespace nap
{
    //! Operator that stores a float value.
    class FloatPushOperator : public Operator
    {
        RTTI_ENABLE_DERIVED_FROM(Operator)
    public:
        //! Constructor
        FloatPushOperator() = default;
        
        //! Attribute to store the float value
        Attribute<float> attribute = { this, "value", 0. };
        
        //! An input plug bound to the attribute: input through this plug will be stored in the attribute
        InputPushPlug<float> input = { this, "input", attribute };
        
        //! An output plug bound to the attribute: value changes of the attribute will be output through this plug
        OutputPushPlug<float> output = { this, "output", attribute };
    };
    
    
    //! Operator that performs addition (+) arithmetic with a left and a right operand
    class PlusPushOperator : public Operator
    {
        RTTI_ENABLE_DERIVED_FROM(Operator)
    public:
        //! Constructor
        PlusPushOperator() = default;
        
        //! Input plug for left operand. setLeft() will be called on input through this plug.
        InputPushPlug<float> leftInput = { this, &PlusPushOperator::setLeft, "left" };
        
        //! Input plug for right operand. setRight() will be called on input through this plug.
        InputPushPlug<float> rightInput = { this, &PlusPushOperator::setRight, "right" };
        
        //! The output plug. The result of the addition is pushed through this plug by setLeft() and setRight()
        OutputPushPlug<float> output = { this, "output" };
        
    private:
        void setLeft(float input)
        {
            left = input;
            output.push(left + right);
        }
        
        void setRight(float input)
        {
            right = input;
            output.push(left + right);
        }
        
        float left = 0;
        float right = 0;        
    };
    
}

RTTI_DECLARE(nap::FloatPushOperator)
RTTI_DECLARE(nap::PlusPushOperator)
