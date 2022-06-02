#pragma once
#include "component.h"

namespace ecspp {

template<typename ComponentName,typename ObjectType>
class ComponentSpecifier : public Component {
public:
	std::string GetTypeName() {
		return HelperFunctions::GetClassName<ComponentName>();
	}

	static size_t AliveCount() {
		return Registry::Get().storage<ComponentName>().size();
	}

protected:

	TemplatedObjectHandle<ObjectType> GetMasterObject() const {
		return TemplatedObjectHandle<ObjectType>(Component::m_MasterHandle);
	};

private:
	static inline bool dummyVar = []() {
		if (HelperFunctions::HashClassName<ComponentName>() != HelperFunctions::HashClassName<HelperClasses::Null>()) {
			ObjectPropertyRegister::RegisterClassAsComponentOfType<ObjectType, ComponentName>();
		}
		return false;
	}();


};

};