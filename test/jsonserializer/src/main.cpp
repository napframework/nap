#include <nap.h>
#include <unistd.h>
#include <thread>
#include <jsonserializer.h>

#include "diff_match_patch.h"

using namespace nap;


class DummyComponent : public Component
{
RTTI_ENABLE_DERIVED_FROM(Component)
public:
    virtual ~DummyComponent() = default;

    Attribute<float> floatAttribute = {this, "predefined_floatAttr", 0.};
    Attribute<int> intAttribute = {this, "predefined_intAttr", 0};
    Attribute<std::vector<float>> vecFloatAttribute{this, "predefined_vecFloatAttr"};
    Attribute<std::vector<int>> vecIntAttribute{this, "predefined_vecIntAttr"};
};
RTTI_DECLARE(DummyComponent)
RTTI_DEFINE(DummyComponent)

std::shared_ptr<Core> createObjectTree()
{
    auto core = std::make_shared<Core>();
    auto& root = core->getRoot();
    auto& patch = root.addComponent<PatchComponent>("patch");
    patch.setName("stijn");
    auto& rootAttr = patch.addAttribute("attrRootOfTwo", RTTI_OF(Attribute<float>));
    ((Attribute<float>&)rootAttr).setValue(1.41421356237f);

    auto& dummy = root.addComponent<DummyComponent>("dummy");
    dummy.floatAttribute.setValue(1);
    dummy.intAttribute.setValue(1);
    dummy.vecFloatAttribute.setValue({1., 2., 3., 4.});
    dummy.vecIntAttribute.setValue({1, 2, 3, 4});

    CompoundAttribute& compAttr = dummy.addCompoundAttribute("myCompound");
    compAttr.addAttribute<float>("oneHundred", 100.0f);
    compAttr.addAttribute<int>("twoHundred", 200);

    auto& a = root.addEntity("A");
    auto& aComp = a.addComponent<PatchComponent>();
    auto& aAttr = aComp.addAttribute("attrPI", RTTI_OF(Attribute<float>));
    ((Attribute<float>&)aAttr).setValue(3.14159265359f);
    auto& b = a.addEntity("B");
    auto& comp = b.addComponent<PatchComponent>();
    auto& myAt = comp.addAttribute("attrGoldenRatio", RTTI_OF(Attribute<float>));
    ((Attribute<float>&)myAt).setValue(1.61803398875f);

    return core;
}

int main(int argc, char* argv[]) {

    std::shared_ptr<Core> core = createObjectTree();

    JSONSerializer ser;
    std::string string1 = ser.toString(core->getRoot());
    std::cout << string1 << std::endl;

    std::cout << "=====================================" << std::endl;

    JSONDeserializer deser;
    Core newCore;
    deser.fromString(string1, newCore);

    std::string string2 = ser.toString(newCore.getRoot());
    std::cout << string2 << std::endl;

    using DMP = diff_match_patch<std::string>;
    if (string1 != string2) {
        DMP dmp;
        DMP::Diffs diffs = dmp.diff_main(string1, string2);
        for (DMP::Diff diff : diffs) {
            std::cout << diff.text << std::endl;
        }
    }

    return 0;
}