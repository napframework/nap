#include "testlink.h"

#include <nap.h>
#include "../testmanager.h"

using namespace nap;
using namespace std;

bool testLink() {

    // Setup harness
    Core core;

    int valueA = 123;
    int valueB = 456;
    int valueC = 789;

    auto& trunkA = core.addEntity("TrunkA");
    auto& branchA = trunkA.addEntity("BranchA");
    auto& leafA = branchA.addEntity("LeafA");
    auto& patchA = leafA.addComponent<PatchComponent>();
    auto attrA = patchA.getOrCreateAttribute<int>("AttrA");
    attrA->setValue(valueA);

    auto& trunkB = core.addEntity("TrunkB");
    auto& branchB = trunkB.addEntity("BranchB");
    auto& patchB = branchB.addComponent<PatchComponent>();
    auto attrB = patchB.getOrCreateAttribute<int>("AttrB");

    attrB->setValue(valueB);

    // Create link
    attrB->link(*attrA);

    int valA = attrA->getValue();
    int valB = attrB->getValue();
    TEST_ASSERT(valA == valB, "Expected linked value");

    // Break link, but keep link target path, fall back to attribute's own value
    patchA.removeAttribute(*attrA); // attrA is dead!
    TEST_ASSERT(attrB->getValue() == valueB, "Expected unlinked value");

    // Recreate attribute and relink
    auto attrC = patchA.getOrCreateAttribute<int>("AttrA"); // Mimic old AttrA
    attrC->setValue(valueC);
    TEST_ASSERT(attrB->getValue() == attrC->getValue(), "Expected re-linked value");

    // Unlink
    attrB->unLink();
    TEST_ASSERT(attrB->getValue() != attrC->getValue(), "Expected non-linked value");

    return true;
}

