#pragma once

#include <nap/coremodule.h>
#include <nap/operator.h>
#include <rtti/rtti.h>
#include <SFML/Graphics.hpp>

RTTI_DECLARE(sf::RenderWindow)

class CreateRenderWindowOperator : public nap::Operator {
    RTTI_ENABLE_DERIVED_FROM(nap::Operator)
public:
    CreateRenderWindowOperator() : nap::Operator(), mCreateWindowPlug(this, "Create", [&]() { onCall();}) {
    }

    void onCall() {

    }

private:
    nap::InputTriggerPlug mCreateWindowPlug;

    sf::RenderWindow* mRenderWindow = nullptr;
    nap::OutputPullPlug<sf::RenderWindow*> mGetRenderWindow = { this, "RenderWindow", [&](sf::RenderWindow*& win) { win = mRenderWindow; }};

};
RTTI_DECLARE(CreateRenderWindowOperator)