#pragma once
#include "object.h"
#include "object_handle.h"
#include "../components/component_specifier.h"
#include <filesystem>


namespace ecspp {

template<typename DerivedObjectClass>
class ObjectTag {

private:
	int dummy = 0;

};

template<class ObjectType,class StorageType>
class RegisterStorage {
public:
	RegisterStorage() {

	};
	
protected:
	StorageType& Storage() {
		return Registry::Get().get_or_emplace<StorageType>(m_Handle);
	}

private:
	void SetMaster(entt::entity e) {
		m_Handle = e;
	};
	static inline bool dummyVar = []() {
		ObjectPropertyRegister::RegisterClassAsPropertyStorage<StorageType,ObjectType>();
		return false;
	}();
	entt::entity m_Handle = entt::null;

	friend class ObjectPropertyRegister;
};




template<typename Derived>
class RegisterObjectType : public Object {
public:
	RegisterObjectType(entt::entity e) : Object(e) {
		(void)dummyVariable;
	};

	static size_t GetNumberOfObjects() {
		size_t count = 0;
		ForEach([&](Derived derived){
			count++;
		});

		return count;
	}

	static void ForEach(std::function<void(Derived)> function) {
		auto resolved = entt::resolve(HelperFunctions::HashClassName<Derived>());

		if (resolved) {
			if (auto func = resolved.func(entt::hashed_string("ForEach")); func) {
				func.invoke({}, function);
			}
		}
	
	}

	static bool DeleteObject(Derived derived) {
		return ObjectPropertyRegister::DeleteObject({derived});
	};

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
				id.template GetAs<Derived>().ForSelfAndEachChild(func);
			}
		}
	};

	void ForEachChild(std::function<void(Derived)> func){
		if (GetChildren().size() == 0) {
			return;
		}
		for (auto id : GetChildren()) {
			if(id){
				func(id.template GetAs<Derived>());
			}
		}

	}


	static const std::vector<std::string>& GetRegisteredComponentsForType() {
		return ObjectPropertyRegister::m_RegisteredComponentsByType[HelperFunctions::HashClassName<Derived>()];
	};
	
protected:
	
	

private:
	
	
	static inline int dummyVariable = []() {
		ObjectPropertyRegister::RegisterClassAsObjectTag<ObjectTag<Derived>, Derived>();
		return 0;
	}();


};

};

