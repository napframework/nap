#pragma once

#include "operator.h"

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
            plug->connectedSignal.connect([&](const nap::OutputPlugBase &plug) { addPlug(); });
            plug->disconnectedSignal.connect([&](const nap::OutputPlugBase &plug) { condensePlugs(); });
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


    class AddFloatOperator : nap::Operator {
    RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        AddFloatOperator() : nap::Operator() {
            // Add initial term
        }

    private:
        // Calculate sum of all the inlets
        void calcSum(float &outSum) {
            float n;
            outSum = 0;
            for (const auto &term : mTerms.plugs()) {
                term->pull(n);
                outSum += n;
            }
        }

        MultiInputPlug<float> mTerms = {*this, "term"};
        nap::OutputPullPlug<float> sum = {this, "Sum", [&](float &sum) { calcSum(sum); }};
    };


    class MultFloatOperator : nap::Operator {
    RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        MultFloatOperator() : nap::Operator() { }

    private:
        void calcProduct(float &outProduct) {
            float n;
            outProduct = 0;
            for (const auto &factor : mFactors.plugs()) {
                factor->pull(n);
                outProduct *= n;
            }
        }

        MultiInputPlug<float> mFactors = {*this, "factor"};
        nap::OutputPullPlug<float> product = {this, "product", [&](float &product) { calcProduct(product); }};
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
