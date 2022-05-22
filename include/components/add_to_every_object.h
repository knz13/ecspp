#pragma once
#include "../object/object_property_register.h"
#include "component_behavior.h"
// dummy class to just specify to add to all objects created from the start

namespace ecspp {

template<typename T>
class AddToEveryObject {
public:
    AddToEveryObject(){
        (void*)m_DummyVar;
    };

private:
    static inline bool m_DummyVar = [](){ObjectPropertyRegister::MakeComponentOmnipresent<T>(); return false;}();
};

};