#pragma once
#include "object.h"
#include "../components/component_specifier.h"
#include <filesystem>


namespace ecspp {

template<typename DerivedObjectClass>
class ObjectTag {

private:
	int dummy = 0;

};


template<typename Derived,typename DerivedComponent,typename DerivedStorage>
class TaggedObject : public Object {
public:
	TaggedObject(entt::entity e) : Object(e) {
		(void)dummyVariable;
	};

	static void ForEach(std::function<void(Derived)> function) {
		auto resolved = entt::resolve(HelperFunctions::HashClassName<Derived>());

		if (resolved) {
			if (auto func = resolved.func(entt::hashed_string("ForEach")); func) {
				func.invoke({}, function);
			}
		}
	
	}

	template<typename... Args>
	static Derived CreateNew(std::string name,Args&&... args) {
		return ObjectPropertyRegister::CreateNew<Derived>(name,std::forward<Args>(args)...);
	}

	void ForSelfAndEachChild(std::function<void(Derived)> func) {
		func(*((Derived*)this));
		if (GetChildren().size() == 0) {
			return;
		}
		for (auto id : GetChildren()) {
			if(id){
				id.GetAs<Derived>().ForSelfAndEachChild(func);
			}
		}
	};

	void ForEachChild(std::function<void(Derived)> func){
		if (GetChildren().size() == 0) {
			return;
		}
		for (auto id : GetChildren()) {
			if(id){
				func(id.GetAs<Derived>());
			}
		}

	}


	static const std::vector<std::string>& GetRegisteredComponentsForType() {
		return ObjectPropertyRegister::m_RegisteredComponentsByType[HelperFunctions::HashClassName<Derived>()];
	};

	NamedComponentHandle<DerivedComponent> AddComponentByName(std::string stringToHash) {
		return ObjectPropertyRegister::AddComponentByName<DerivedComponent>(this->ID(), stringToHash);
	};

	NamedComponentHandle<DerivedComponent> GetComponentByName(std::string stringToHash) {
		return ObjectPropertyRegister::GetComponentByName<DerivedComponent>(this->ID(), stringToHash);
	};
	
protected:
	
	DerivedStorage& Storage() {
		return Registry::Get().get_or_emplace<DerivedStorage>(ID());
	}

private:
	
	
	static inline int dummyVariable = []() {
		ObjectPropertyRegister::RegisterClassAsObjectTag<ObjectTag<Derived>, Derived>();
		ObjectPropertyRegister::RegisterClassAsPropertyStorage<DerivedStorage, Derived>();
		
		return 0;
	}();


};


};