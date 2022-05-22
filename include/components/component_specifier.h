#pragma once
#include "component.h"

namespace ecspp {

template<typename ComponentName,typename ObjectType>
class ComponentSpecifier : public Component {
public:
	std::string GetName() {
		return HelperFunctions::GetClassName<ComponentName>();
	}


protected:

	TemplatedObjectHandle<ObjectType> GetMasterObject() const {
		return TemplatedObjectHandle<ObjectType>(Component::m_MasterHandle);
	};

private:
	static inline bool dummyVar = []() {
		if (HelperFunctions::HashClassName<ComponentName>() != HelperFunctions::HashClassName<ComponentHelpers::Null>()) {
			ObjectPropertyRegister::RegisterClassAsComponentOfType<ObjectType, ComponentName>();
		}
		return false;
	}();


};

};