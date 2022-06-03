#pragma once

#include "object/object.h"
#include "object/tagged_object.h"
#include "components/component_specifier.h"
#include "components/component.h"
#include "components/add_only_to.h"
#include "components/add_to_every_object.h"

namespace ecspp {
	inline void ClearDeletingQueue() {
		ObjectPropertyRegister::ClearDeletingQueue();
	};

	inline ObjectHandle CreateNewObject(std::string name) {
		return ObjectPropertyRegister::CreateNew<Object>(name);
	}


	inline ObjectHandle CreateNewObject(std::string type, std::string name) {
		return ObjectPropertyRegister::CreateObjectFromType(type, name);
	}

	inline void ForEachObject(std::function<void(ObjectHandle)> func) {
		ObjectPropertyRegister::Each(func);
	};

	inline std::vector<std::string> GetObjectComponents(Object obj) {
		return ObjectPropertyRegister::GetObjectComponents(obj.ID());
	}

	template<typename T>
	inline T CopyObject(T other) {
		return ObjectPropertyRegister::CopyObject(other);
	};

	inline ObjectHandle CopyObject(ObjectHandle other) {
		if (!other) {
			return {};
		}
		return { ObjectPropertyRegister::CopyObject<Object>(other.GetAsObject()) };
	}

	inline ObjectHandle FindObjectByName(std::string name) {
		return ObjectPropertyRegister::FindObjectByName(name);
	}

	inline bool DeleteObject(Object obj) {
		return ObjectPropertyRegister::DeleteObject(obj);
	};

	inline bool DeleteObject(ObjectHandle handle) {
		if (handle) {
			return ObjectPropertyRegister::DeleteObject(handle.GetAsObject());
		}
		return false;
	}

	inline bool DeleteObject(entt::entity e) {
		return DeleteObject(ObjectHandle(e));
	}

	inline std::string GetClassNameByID(entt::id_type id) {
		return ObjectPropertyRegister::GetClassNameByID(id);
	};

	inline bool IsClassRegistered(std::string className) {
		return ObjectPropertyRegister::IsClassRegistered(className);
	}

	template<typename... Args>
	inline entt::meta_any CallMetaFunction(std::string className, std::string funcName, Args&&... args) {
		return HelperFunctions::CallMetaFunction(className, funcName, std::forward(args)...);
	}

};