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
		return registry.storage<ComponentName>().size();
	}

	void ForEach(std::function<void(ComponentName&)> func) {
		auto view = registry.view<ComponentName>();
		for (auto entity : view) {
			func(view.get<ComponentName>(entity));
		}
	}

	NamedObjectHandle<ObjectType> GetMasterObject() const {
		return NamedObjectHandle<ObjectType>(Component::m_MasterHandle);
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