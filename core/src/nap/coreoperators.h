#pragma once

#include "operator.h"
#include "coreattributes.h"


namespace nap {

    // TODO: OWNERSHIP BROKEN
    template<typename T>
    class MultiInputPlug {
    public:
        MultiInputPlug(nap::Operator &parent, const std::string &name) : mParent(parent), mBaseName(name) {
            // TODO -> FIX OWNERSHIP
            addPlug();
        }

        const std::vector<nap::InputPullPlug<T> *> &plugs() const { return mPlugs; }

    private:
        void addPlug() {
            auto plug = new nap::InputPullPlug<float>(&mParent, mBaseName);
            plug->connected.connect([&](Plug&) { addPlug(); });
            plug->disconnected.connect([&](Plug&) { condensePlugs(); });
            mPlugs.push_back(plug);
        }

        void condensePlugs() {
            assert(false); // Not implemented
        }

        nap::Operator &mParent;
        const std::string mBaseName;
        std::vector<nap::InputPullPlug<T> *> mPlugs;
    };


    class AttributeObjectOperator : public Operator {
    public:
        AttributeObjectOperator() : Operator() {}


    };
    
    
    class FloatOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        Attribute<float> mValue = { this, "valueAttr", 0. };
        nap::OutputPullPlug<float> output = { this, &FloatOperator::pullValue, "value" };
        nap::InputPullPlug<float> input = { this, "input" };
        
    private:
        void pullValue(float& outValue)
        {
            if (input.isConnected())
            {
                float result;
                input.pull(result);
                mValue.setValue(result);
            }
            outValue = mValue.getValue();
        }
        
    };


    class AddFloatOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        AddFloatOperator() : nap::Operator() {
            // Add initial term
        }

        InputPullPlug<float> mTermA = {this, "termA"};
        InputPullPlug<float> mTermB = {this, "termb"};

//        MultiInputPlug<float> mTerms = {*this, "term"};
        nap::OutputPullPlug<float> sum = {this, "Sum", [&](float &sum) { calcSum(sum); }};

    private:
        // Calculate sum of all the inlets
        void calcSum(float &outSum) {
            float a;
            float b;
            mTermA.pull(a);
            mTermB.pull(b);

            outSum = a + b;
        }

    };


    class MultFloatOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        MultFloatOperator() : nap::Operator() { }

//        MultiInputPlug<float> mFactors = {*this, "factor"};
        nap::InputPullPlug<float> mFactorA = {this, "factorA"};
        nap::InputPullPlug<float> mFactorB = {this, "factorB"};
        nap::OutputPullPlug<float> product = {this, "product", [&](float &product) { calcProduct(product); }};
    private:
        void calcProduct(float &outProduct) {
            float a;
            mFactorA.pull(a);
            float b;
            mFactorB.pull(b);
            outProduct = a * b;
        }

    };


    class SimpleTriggerOperator : nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        SimpleTriggerOperator() : nap::Operator() { }

        nap::OutputTriggerPlug mOutTrigger = {this, "OutTrigger"};
        nap::InputTriggerPlug mInTrigger = {this, "InTrigger", [&]() {
            mOutTrigger.trigger();
        }};
    };

}


RTTI_DECLARE(nap::AddFloatOperator)
RTTI_DECLARE(nap::SimpleTriggerOperator)
RTTI_DECLARE(nap::MultFloatOperator)
RTTI_DECLARE(nap::FloatOperator)

