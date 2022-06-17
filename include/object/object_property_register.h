#pragma once

#include <iostream>
#include <unordered_map>
#include "../components/component.h"
#include "registry.h"
#include "../helpers/helpers.h"
#include "../../vendor/entt/single_include/entt/entt.hpp"
#include "object_properties.h"
#include "object_base.h"
#include "../global.h"


namespace ecspp {

	

struct ComponentHandle {
public:

	ComponentHandle() {

	};

	ComponentHandle(entt::entity e,entt::id_type type)
	{
		m_MasterID = e;
		m_ComponentType = type;

	};

	Component* Get() {
		auto any = HelperFunctions::CallMetaFunction(m_ComponentType,"Cast To Base",m_MasterID);

		if (!any) {
			ECSPP_DEBUG_LOG("Calling Get() without valid master id or component id");
			return nullptr;
		}

		return *(Component**)any.data();
	};

	template<typename Type>
	Type* GetAs();

	bool IsType(entt::id_type id){
		return id == m_ComponentType;
	}

	template<typename T>
	bool IsType() {
		return m_ComponentType == HelperFunctions::HashClassName<T>();
	};

	operator bool() {
		return Registry().valid(m_MasterID) && (Registry().storage(HelperFunctions::GetClassHash(m_ComponentType)) != Registry().storage().end());
	};

private:
	entt::entity m_MasterID = entt::null;
	entt::id_type m_ComponentType = entt::null;

};





class Object;
class ObjectPropertyRegister {
public:

	template<typename Component, typename... Args>
	static void MakeComponentPresentIn() {
		(ObjectPropertyRegister::MakeComponentPresentBackground<Component, Args>(), ...);
	}

	template<typename T>
	static void MakeComponentOmnipresent() {
		m_ComponentsToMakeOmnipresent.push_back(HelperFunctions::GetClassName<T>());
	};

	template<typename Storage, typename MainClass>
	static void RegisterClassAsPropertyStorage() {
		m_PropertyStorageContainer[HelperFunctions::HashClassName<MainClass>()] = [](entt::entity e) {
			Registry().emplace<Storage>(e);
		};
	};

	template<typename ObjectType,typename ComponentType>
	static void RegisterComponentBaseAsDerivingFromObject() {
		m_RegisteredComponentByObjectType[HelperFunctions::HashClassName<ComponentType>()] = HelperFunctions::HashClassName<ObjectType>();
	};


	template<typename Tag, typename Attached>
	static void RegisterClassAsObjectTag() {
		entt::id_type hash = HelperFunctions::HashClassName<Attached>();
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::ForEachByTag<Tag, Attached>>(entt::hashed_string("ForEach"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CreateObjectAndReturnHandle<Attached>>(entt::hashed_string("Create"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CallDestroyForObject<Attached>>(entt::hashed_string("Destroy"));
		entt::meta<Attached>().type(hash).template func<& ObjectPropertyRegister::CallVirtualFunc<Attached>>(entt::hashed_string("CallVirtualFunc"));
		m_RegisteredObjectTagsStartingFuncs[hash] = [](entt::entity e) {
			Registry().emplace<Tag>(e);
		};
		m_RegisteredTypesByTag[entt::type_hash<Tag>().value()] = hash;
		m_RegisteredTagsByType[hash] = entt::type_hash<Tag>().value();
		m_RegisteredObjectNames[hash] = HelperFunctions::GetClassName<Attached>();


	}

	template<typename T>
	static auto CallVirtualFunc(entt::entity e, std::function<entt::meta_any(Object*)> func);

	static ObjectHandle CreateObjectFromType(std::string type, std::string objectName) {
		auto resolved = entt::resolve(entt::hashed_string(type.c_str()));

		if (resolved) {
			if (auto func = resolved.func(entt::hashed_string("Create")); func) {
				auto result = func.invoke({}, objectName);
				if (result) {
					return ObjectHandle(*((entt::entity*)result.data()));
				}
				ECSPP_DEBUG_LOG("Create function for object with name " + objectName + " was not successful!");
				return {};
			}
			ECSPP_DEBUG_LOG("Couldn't identify create function for object, make sure it is derived from RegisterObjectType or RegisterComponentlessObjectType");
			return {};
		}
		ECSPP_DEBUG_LOG("Couldn't identify object type specified, check for spelling errors...");
		return {};

	}

	template<typename T, typename... Args>
	static void InitializeObject(entt::entity ent, Args&&... args) {

		entt::id_type hash = HelperFunctions::HashClassName<T>();



		if (m_RegisteredObjectTagsStartingFuncs.find(hash) != m_RegisteredObjectTagsStartingFuncs.end()) {
			m_RegisteredObjectTagsStartingFuncs[hash](ent);
		}

		if (m_PropertyStorageContainer.find(hash) != m_PropertyStorageContainer.end()) {
			m_PropertyStorageContainer[hash](ent);
		}

		T obj(ent, args...);

		((ObjectBase*)(&obj))->Init();



		if (m_ComponentsToMakeAvailableAtStartByType.find(hash) != m_ComponentsToMakeAvailableAtStartByType.end()) {
			for (auto& componentName : m_ComponentsToMakeAvailableAtStartByType[hash]) {
				AddComponentByName(obj.ID(), componentName);
			}
		}

		for (auto& componentName : m_ComponentsToMakeOmnipresent) {
			AddComponentByName(obj.ID(), componentName);
		}



	}

	template<typename T>
	static T CopyObject(T other) {
		entt::entity firstObject = entt::null;
		entt::entity lastObject = entt::null;

		other.ForSelfAndEachChild([&](T current) {
			T newOne = DuplicateObject<T>(current);
			if (firstObject == entt::null) {
				firstObject = newOne.ID();
			}
			if (lastObject != entt::null) {
				newOne.SetParent(T(lastObject));
			}
			lastObject = newOne.ID();
			});
		return T(firstObject);
	};

	static ObjectHandle FindObjectByName(std::string name) {
		for (auto [handle, comp] : Registry().storage<ObjectProperties>().each()) {
			if (comp.GetName() == name) {
				return ObjectHandle(handle);
			}
		}
		return ObjectHandle();

	};

	template<typename T, typename... Args>
	static T CreateNew(std::string name, Args&&... args) {
		static_assert(std::is_base_of<Object, T>::value);

		entt::entity ent = Registry().create();

		int index = 1;
		if (FindObjectByName(name)) {
			if (name.find_last_of(")") == std::string::npos || (name.find_last_of(")") != name.size() - 1)) {
				name += "(" + std::to_string(index) + ")";
			}
		}

		while (FindObjectByName(name)) {
			index++;
			name.replace(name.find_last_of("(") + 1, std::to_string(index - 1).size(), std::to_string(index));
		}

		Registry().emplace<ObjectProperties>(ent, name, HelperFunctions::HashClassName<T>(), ent);

		ObjectPropertyRegister::InitializeObject<T, Args...>(ent, args...);

		return T(ent);
	}




	template<typename Component, typename ComponentType>
	static void RegisterClassAsComponentOfType() {
		m_RegisteredComponentsByType[m_RegisteredComponentByObjectType[HelperFunctions::HashClassName<ComponentType>()]].push_back(HelperFunctions::GetClassName<Component>());
		RegisterClassAsComponent<Component>();
		entt::meta<Component>().type(HelperFunctions::HashClassName<Component>()).template func<&CastComponentTo<Component,ComponentType>>(entt::hashed_string(("Cast To "+ HelperFunctions::GetClassName<ComponentType>()).c_str()));
		m_RegisteredComponentsNames[entt::type_hash<Component>().value()] = HelperFunctions::GetClassName<Component>();
	};

	static std::vector<std::string> GetObjectComponents(entt::entity e) {
		std::vector<std::string> vec;
		for (auto [id, storage] : Registry().storage()) {
			if (id == entt::type_hash<ObjectProperties>().value()) {
				continue;
			}

			std::string objectType = GetObjectType(e);

			entt::id_type objectHash = entt::hashed_string(objectType.c_str());
			if (std::find(m_RegisteredComponentsByType[entt::hashed_string(objectType.c_str())].begin(), m_RegisteredComponentsByType[entt::hashed_string(objectType.c_str())].end(), ObjectPropertyRegister::GetComponentNameByID(id)) == m_RegisteredComponentsByType[entt::hashed_string(objectType.c_str())].end()) {
				continue;
			}
			if (storage.contains(e)) {
				vec.push_back(GetComponentNameByID(id));
			}
		}
		return vec;

	}

	static std::string GetClassNameByID(entt::id_type id) {
		if (m_RegisteredObjectNames.find(id) != m_RegisteredObjectNames.end()) {
			return m_RegisteredObjectNames[id];
		}
		return "";
	}

	template<typename T>
	static entt::entity CreateObjectAndReturnHandle(std::string name) {
		T obj = CreateNew<T>(name);

		return obj.ID();
	};

	static void Each(std::function<void(ObjectHandle)> func) {
		Registry().each([&](entt::entity e) {
			if (ObjectHandle(e)) {
				func(ObjectHandle(e));
			}
			});
	}

	static bool DeleteObject(ObjectHandle obj) {
		if (obj) {
			m_ObjectsToDelete.push_back(obj);
			return true;
		}
		ECSPP_DEBUG_LOG("Could not delete object with id " + obj.ToString() + " because it was not valid!");
		return false;
	}

	static void ClearDeletingQueue() {
		for (auto& objHandle : m_ObjectsToDelete) {
			if (!objHandle) {

				continue;
			}
			std::vector<ObjectHandle> objectAndAllChildren;

			GetAllChildren(objHandle, objectAndAllChildren);

			for (auto& objectHandle : objectAndAllChildren) {
				if (!objectHandle) {
					ECSPP_DEBUG_LOG("Could not delete object with id " + objectHandle.ToString() + " because it was not valid! During ObjectPropertyRegister::ClearDeletingQueue()");
					continue;
				}
				std::string objectType = GetObjectType(objectHandle.ID());

				if (objectType == "") {
					continue;
				}

				std::vector<std::string> componentNames = GetObjectComponents(objectHandle.ID());
				auto it = componentNames.begin();
				while (it != componentNames.end()) {
					HelperFunctions::CallMetaFunction(*it, "Erase Component", objectHandle.ID());
					it++;
				}

				if (!HelperFunctions::CallMetaFunction(objectType, "Destroy", objectHandle.ID())) {
					ECSPP_DEBUG_LOG("Could not call destroy for object with type: " + objectType);
				}
				Registry().destroy(objectHandle.ID());
			}

		}
		if (m_ObjectsToDelete.size() > 0) {
			ValidateAllGameObjects();
		}
		m_ObjectsToDelete.clear();
	}

	static bool IsClassRegistered(std::string className) {
		return entt::resolve(entt::hashed_string(className.c_str())).operator bool();
	}


	template<typename T>
	static bool IsTypeOfObject() {
		return m_RegisteredTagsByType.find(HelperFunctions::HashClassName<T>()) != m_RegisteredTagsByType.end();
	}




protected:
	static std::string GetComponentNameByID(entt::id_type id) {
		return m_RegisteredComponentsNames[id];
	}


	static bool IsHandleValid(entt::entity e) {
		if (Registry().valid(e)) {
			return true;
		}
		throw std::runtime_error("Using Invalid entity!");

	}

	template<typename T>
	static bool HasComponent(entt::entity e) {
		
		if (!IsHandleValid(e)) {
			return false;
		}
		
		return Registry().all_of<T>(e);
	};

	static void RegisterComponentsNames(entt::entity e);

	template<typename T, typename... Args>
	static T* AddComponent(entt::entity e, Args&&... args) {

		if (!IsHandleValid(e)) {
			ECSPP_DEBUG_LOG("Handle was not valid during add component!");
			return nullptr;
		}

		if (HasComponent<T>(e)) {
			return &Registry().get<T>(e);
		}

		Component* comp = (Component*)&Registry().emplace<T>(e, std::forward<Args>(args)...);
		comp->SetMaster(e);
		comp->Init();

		RegisterComponentsNames(e);

		return &Registry().get<T>(e);;
		
	}

	template<typename T>
	static T* GetComponent(entt::entity e) {
		if (!IsHandleValid(e)) {
			return nullptr;
		}

		if (AddComponent<T>(e)) {
			return &Registry().get<T>(e);
		}
		else {
			if (HasComponent<T>(e)) {
				return &Registry().get<T>(e);;
			}
			else {
				return nullptr;
			}
		}
	};

	template<typename T, typename... Args>
	static T* GetComponentWithArgs(entt::entity e, Args&&... args) {

		if (AddComponent<T, Args...>(e, std::forward<Args>(args)...)) {
			return &Registry().get<T>(e);
		}
		else {
			if (HasComponent<T>(e)) {
				return &Registry().get<T>(e);;
			}
			else {

				return {};
			}
		}
	};
	

	static ComponentHandle AddComponentByName(entt::entity e, std::string stringToHash) {
		if (!Registry().valid(e)) {
			return {};
		}

		auto resolved = entt::resolve(entt::hashed_string(stringToHash.c_str()));
		void* returnData = nullptr;
		if (resolved) {
			if (auto func = resolved.func(entt::hashed_string("Create Component")); func) {

				entt::meta_any owner = func.invoke({}, e);
				returnData = owner.data();

				if (owner.data() == nullptr) {
					ECSPP_DEBUG_LOG("Could not construct component of type " + stringToHash);
				}
			}
		}

		return ComponentHandle(e, entt::hashed_string(stringToHash.c_str()));

	};

	
	static ComponentHandle GetComponentByName(entt::entity e, std::string name) {
		return AddComponentByName(e, name);
	};

	template<typename T>
	static bool EraseComponent(entt::entity e) {
		if (HasComponent<T>(e)) {
			dynamic_cast<Component*>(GetComponent<T>(e))->Destroy();

			Registry().storage<T>().erase(e);
			return true;
		}
		return false;
	}

	template<typename T>
	static bool CopyComponent(entt::entity first, entt::entity second) {
		if (HasComponent<T>(first) && HasComponent<T>(second)) {
			T& firstComp = *GetComponent<T>(first);
			T& secondComp = *GetComponent<T>(second);
			secondComp = firstComp;
			return true;
		}
		else {
			return false;
		}
	}


	template<typename>
	friend class RegisterObjectType;
	friend class Object;

private:

	template<typename T>
	static bool CallDestroyForObject(entt::entity e) {
		if (!ObjectHandle(e)) {
			return false;
		}

		T obj(e);

		((ObjectBase*)(&obj))->Destroy();

		return true;

	}

	static void ValidateAllGameObjects() {
		Registry().each([](entt::entity e) {
			for (auto& compName : ObjectPropertyRegister::GetObjectComponents(e)) {
				auto handle = ObjectPropertyRegister::GetComponentByName(e, compName);
				if (handle) {
					Component& comp = *handle.Get();
					comp.SetMaster(e);
				}
			}
			});
	}


	static std::string GetObjectType(entt::entity e) {
		auto wholeStorage = Registry().storage();

		for (auto [id, storage] : wholeStorage) {
			if (m_RegisteredTypesByTag.find(id) != m_RegisteredTypesByTag.end()) {
				return m_RegisteredObjectNames[m_RegisteredTypesByTag[id]];
			}
		}


		return "";

	};

	


	static void GetAllChildren(ObjectHandle current, std::vector<ObjectHandle>& vec) {
		if (!current) {
			return;
		}
		vec.push_back(current);

		for (auto& handle : Registry().get<ObjectProperties>(current.ID()).GetChildren()) {
			if (handle) {
				GetAllChildren(handle, vec);
			}
		}
	}




	template<typename T>
	static T* CreateComponent(entt::entity e) {
		return GetComponent<T>(e);
	};

	template<typename T>
	static void UpdateComponent(entt::entity e, float deltaTime);

	template<typename T>
	static entt::id_type RegisterClassAsComponent() {
		entt::id_type hash = HelperFunctions::HashClassName<T>();
		entt::meta<T>().type(hash).template func<&CreateComponent<T>>(entt::hashed_string("Create Component"));
		entt::meta<T>().type(hash).template func<&CastComponentToCommonBase<T>>(entt::hashed_string("Cast To Base"));
		entt::meta<T>().type(hash).template func<&UpdateComponent<T>>(entt::hashed_string("Update Component"));
		entt::meta<T>().type(hash).template func<&CopyComponent<T>>(entt::hashed_string("Copy Component"));
		entt::meta<T>().type(hash).template func<&EraseComponent<T>>(entt::hashed_string("Erase Component"));
		entt::meta<T>().type(hash).template func<&HasComponent<T>>(entt::hashed_string("Has Component"));



		return hash;
	}

	


	template<typename T>
	static T DuplicateObject(T other) {
		T obj = ObjectPropertyRegister::CreateNew<T>(other.Properties().GetName());

		for (auto [id, storage] : Registry().storage()) {
			if (id == entt::type_hash<ObjectProperties>().value() || id == ObjectPropertyRegister::m_RegisteredTagsByType[HelperFunctions::HashClassName<T>()]) {
				continue;
			}
			if (storage.contains(other.ID()) && !storage.contains(obj.ID())) {
				obj.AddComponentByName(GetComponentNameByID(id));
				obj.CopyComponentByName(GetComponentNameByID(id), other);
			}
			else if (storage.contains(obj.ID())) {

				obj.CopyComponentByName(GetComponentNameByID(id), other);
			}
		}

		return obj;
	};

	template<typename MainComponentType, typename FinalType>
	static FinalType* CastComponentTo(entt::entity e);

	template<typename T>
	static Component* CastComponentToCommonBase(entt::entity e);

	template<typename Tag,typename Attached>
	static void ForEachByTag(std::function<void(Attached)> func) {
		auto view = Registry().view<Tag>();
		for (auto entity : view) {
			func(Attached(entity));
		}
	}

	template<typename Component,typename T>
	static void MakeComponentPresentBackground() {
		m_ComponentsToMakeAvailableAtStartByType[HelperFunctions::HashClassName<T>()].push_back(HelperFunctions::GetClassName<Component>());
	};

	inline static std::vector<ObjectHandle> m_ObjectsToDelete;
	inline static std::vector <std::string> m_ComponentsToMakeOmnipresent;
	inline static std::unordered_map < entt::id_type, std::function<void(entt::entity)>> m_PropertyStorageContainer;
	inline static std::unordered_map<entt::id_type, std::vector<std::string>> m_ComponentsToMakeAvailableAtStartByType;
	inline static std::unordered_map<entt::id_type, std::function<void(entt::entity)>> m_RegisteredObjectTagsStartingFuncs;
	inline static std::unordered_map<entt::id_type, std::vector<std::string>> m_RegisteredComponentsByType;
	inline static std::unordered_map<entt::id_type, entt::id_type> m_RegisteredComponentByObjectType;
	inline static std::unordered_map<entt::id_type, entt::id_type> m_RegisteredTagsByType;
	inline static std::unordered_map<entt::id_type, entt::id_type> m_RegisteredTypesByTag;
	inline static std::unordered_map<std::string, entt::id_type> m_RegisteredTagsByName;
	inline static std::unordered_map<entt::id_type, std::string> m_RegisteredComponentsNames;
	inline static std::unordered_map<entt::id_type, std::string> m_RegisteredObjectNames;
	
	

};


template<typename T>
class NamedObjectHandle {
public:
	NamedObjectHandle(entt::entity ent) {
		m_Handle = ent;
		isNull = false;
	}
	NamedObjectHandle() {
		isNull = true;
	}

	T GetAsObject() {
		return T(m_Handle);
	}

	operator bool() {
		if (isNull) {
			return false;
		}
		return Registry().valid(m_Handle) && ObjectPropertyRegister::IsTypeOfObject<T>();
	};

private:
	entt::entity m_Handle = entt::null;
	bool isNull = false;
};



};