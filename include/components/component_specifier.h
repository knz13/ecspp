#pragma once
#include "component.h"

namespace ecspp {

template<typename ComponentName,typename ComponentType>
class DefineComponent : public ComponentType {
public:
	DefineComponent() {
		(void)dummyVar;
		Component::SetType(HelperFunctions::HashClassName<ComponentName>());
	};
	
	std::string GetTypeName() override {
		return HelperFunctions::GetClassName<ComponentName>();
	}

	static size_t AliveCount() {
		return Registry().storage<ComponentName>().size();
	}

	static void ForEach(std::function<void(ComponentName&)> func) {
		auto view = Registry().view<ComponentName>();
		for (auto entity : view) {
			func(view.get<ComponentName>(entity));
		}
	}

	ObjectHandle GetMasterObject() const {
		return { this->GetMasterHandle()};
	};

private:
	using Component::SetType;
	static inline bool dummyVar = []() {
		ObjectPropertyRegister::RegisterClassAsComponentOfType<ComponentName, ComponentType>();
		return true;
	}();


};

template<typename ObjectType,typename ComponentType>
class RegisterComponent {
public:
	RegisterComponent() {
		(void)dummyVar;
	};

private:
	static inline bool dummyVar = []() {
		if constexpr (!std::is_same<ComponentType, HelperClasses::Null>::value) {
			ObjectPropertyRegister::RegisterComponentBaseAsDerivingFromObject<ObjectType, ComponentType>();
		}
		return false;
	}();
};

};