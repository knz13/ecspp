#pragma once
#include "../object/object_property_register.h"

namespace ecspp {

template<typename Master,typename... Dependents>
class AddOnlyTo {
public:
	AddOnlyTo() {
		m_DummyVar = true;
	};

private:
	static inline bool m_DummyVar = []() {
		ObjectPropertyRegister::MakeComponentPresentIn<Master,Dependents...>(); 
		return false; 
	}();

};

};