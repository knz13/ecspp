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

template<typename T>
struct NamedComponentHandle {
public:

	NamedComponentHandle() {

	};

	NamedComponentHandle(T* comp)
	{
		m_Component = comp;
	};


	
	T* GetComponentPointer() {
		return m_Component;
	}

	Component& Get() {
		return *m_Component;
	};

	template<typename Type>
	Type& GetAs() {
		return *((Type*)m_Component);
	};

	operator bool() {
		return m_Component != nullptr;
	};

private:
	T* m_Component = nullptr;


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
			Registry::Get().emplace<Storage>(e);
		};
	};



	template<typename Tag, typename Attached>
	static void RegisterClassAsObjectTag() {
		entt::id_type hash = HelperFunctions::HashClassName<Attached>();
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::ForEachByTag<Tag, Attached>>(entt::hashed_string("ForEach"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CreateObjectAndReturnHandle<Attached>>(entt::hashed_string("Create"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CallDestroyForObject<Attached>>(entt::hashed_string("Destroy"));
		m_RegisteredObjectTagsStartingFuncs[hash] = [](entt::entity e) {
			Registry::Get().emplace<Tag>(e);
		};
		m_RegisteredTypesByTag[entt::type_hash<Tag>().value()] = hash;
		m_RegisteredTagsByType[hash] = entt::type_hash<Tag>().value();
		m_RegisteredObjectNames[hash] = HelperFunctions::GetClassName<Attached>();


	}

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
			ECSPP_DEBUG_LOG("Couldn't identify create function for object, make sure it is derived from TaggedObject");
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
				obj.AddComponentByName(componentName);
			}
		}

		for (auto& componentName : m_ComponentsToMakeOmnipresent) {
			obj.AddComponentByName(componentName);
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
				newOne.Properties().SetParent(T(lastObject));
			}
			lastObject = newOne.ID();
			});
		return T(firstObject);
	};

	static ObjectHandle FindObjectByName(std::string name) {
		for (auto [handle, comp] : Registry::Get().storage<ObjectProperties>().each()) {
			if (comp.GetName() == name) {
				return ObjectHandle(handle);
			}
		}
		return ObjectHandle();

	};

	template<typename T, typename... Args>
	static T CreateNew(std::string name, Args&&... args) {
		static_assert(std::is_base_of<Object, T>::value);

		entt::entity ent = Registry::Get().create();

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

		Registry::Get().emplace<ObjectProperties>(ent, name, HelperFunctions::HashClassName<T>(), ent);

		ObjectPropertyRegister::InitializeObject<T, Args...>(ent, args...);

		return T(ent);
	}




	template<typename ComponentType, typename Component>
	static void RegisterClassAsComponentOfType() {
		m_RegisteredComponentsByType[HelperFunctions::HashClassName<ComponentType>()].push_back(HelperFunctions::GetClassName<Component>());
		RegisterClassAsComponent<Component>();
		m_RegisteredComponentsNames[entt::type_hash<Component>().value()] = HelperFunctions::GetClassName<Component>();
	};

	static std::vector<std::string> GetObjectComponents(entt::entity e) {
		std::vector<std::string> vec;
		for (auto [id, storage] : Registry::Get().storage()) {
			if (id == entt::type_hash<ObjectProperties>().value()) {
				continue;
			}

			std::string objectType = GetObjectType(e);

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
		Registry::Get().each([&](entt::entity e) {
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
					ECSPP_DEBUG_LOG("Could not call destroy for object with type: " + objectType );
				}
				Registry::Get().destroy(objectHandle.ID());
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
		if (Registry::Get().valid(e)) {
			return true;
		}
		throw std::runtime_error("Using Invalid entity!");

	}

	template<typename T>
	static bool HasComponent(entt::entity e) {
		try {
			IsHandleValid(e);
		}
		catch (std::runtime_error& err) {
			throw err;
		}

		return Registry::Get().all_of<T>(e);
	};

	template<typename T, typename... Args>
	static bool AddComponent(entt::entity e, Args&&... args) {
		bool value = false;
		try {
			value = HasComponent<T>(e);
		}
		catch (std::runtime_error& err) {
			throw err;
		}


		if (!value) {
			Component* comp = (Component*)&Registry::Get().emplace<T>(e, std::forward<Args>(args)...);
			comp->SetMaster(e);
			comp->Init();

			return true;
		}
		else {
			return false;
		}
	}

	template<typename T>
	static NamedComponentHandle<T> GetComponent(entt::entity e) {
		bool couldAdd = false;

		try {
			AddComponent<T>(e);
		}
		catch (std::runtime_error& err) {
			throw err;
		}

		if (couldAdd) {
			return NamedComponentHandle<T>(&Registry::Get().get<T>(e));
		}
		else {
			if (HasComponent<T>(e)) {
				return NamedComponentHandle<T>(&Registry::Get().get<T>(e));
			}
			else {

				return NamedComponentHandle<T>(nullptr);
			}
		}
	};

	template<typename T, typename... Args>
	static NamedComponentHandle<T> GetComponentWithArgs(entt::entity e, Args&&... args) {
		bool couldAdd = false;

		try {
			AddComponent<T, Args...>(e, std::forward<Args>(args)...);
		}
		catch (std::runtime_error& err) {
			throw err;
		}

		if (couldAdd) {
			return NamedComponentHandle<T>(&Registry::Get().get<T>(e));
		}
		else {
			if (HasComponent<T>(e)) {
				return NamedComponentHandle<T>(&Registry::Get().get<T>(e));
			}
			else {

				return NamedComponentHandle<T>(nullptr);
			}
		}
	};





	template<typename ReturnType>
	static NamedComponentHandle<ReturnType> AddComponentByName(entt::entity e, std::string stringToHash) {
		if (!Registry::Get().valid(e)) {
			return {};
		}

		auto resolved = entt::resolve(entt::hashed_string(stringToHash.c_str()));
		ReturnType* returnData = nullptr;
		if (resolved) {
			entt::meta_any owner = resolved.construct(e);

			returnData = (ReturnType*)owner.data();

			if (returnData == nullptr) {
				throw std::runtime_error("Couldn't add component of type: " + stringToHash);
			}

		}
		return { returnData };

	};

	template<typename ReturnType>
	static NamedComponentHandle<ReturnType> GetComponentByName(entt::entity e, std::string name) {
		NamedComponentHandle<ReturnType> comp;

		try {
			comp = AddComponentByName<ReturnType>(e, name);
		}
		catch (std::runtime_error& err) {
			throw err;
		}

		return comp;
	};

	template<typename T>
	static bool EraseComponent(entt::entity e) {

		if (HasComponent<T>(e)) {
			T* comp = GetComponent<T>(e).GetComponentPointer();
			((Component*)comp)->Destroy();



			Registry::Get().storage<T>().erase(e);
			return true;
		}
		return false;
	}

	template<typename T>
	static bool CopyComponent(entt::entity first, entt::entity second) {
		if (HasComponent<T>(first) && HasComponent<T>(second)) {
			T& firstComp = *GetComponent<T>(first).GetComponentPointer();
			T& secondComp = *GetComponent<T>(second).GetComponentPointer();
			secondComp = firstComp;
			return true;
		}
		else {
			return false;
		}
	}


	template<typename, typename, typename>
	friend class TaggedObject;
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
		Registry::Get().each([](entt::entity e) {
			for (auto& compName : ObjectPropertyRegister::GetObjectComponents(e)) {
				auto handle = ObjectPropertyRegister::GetComponentByName<Component>(e, compName);
				if (handle) {
					Component& comp = handle.Get();
					comp.SetMaster(e);
				}
			}
			});
	}


	static std::string GetObjectType(entt::entity e) {
		auto wholeStorage = Registry::Get().storage();

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

		for (auto& handle : Registry::Get().get<ObjectProperties>(current.ID()).GetChildren()) {
			if (handle) {
				GetAllChildren(handle, vec);
			}
		}
	}

	
	

	template<typename T>
	static T& CreateComponent(entt::entity e) {
		NamedComponentHandle<T> handle = GetComponent<T>(e);
		return *handle.GetComponentPointer();
	};

	template<typename T>
	static entt::id_type RegisterClassAsComponent() {
		entt::id_type hash = HelperFunctions::HashClassName<T>();
		entt::meta<T>().type(hash).template ctor<&CreateComponent<T>,entt::as_ref_t>();
		entt::meta<T>().type(hash).template func<&CopyComponent<T>>(entt::hashed_string("Copy Component"));
		entt::meta<T>().type(hash).template func<&EraseComponent<T>>(entt::hashed_string("Erase Component"));
		entt::meta<T>().type(hash).template func<&HasComponent<T>>(entt::hashed_string("Has Component"));
		
		

		return hash;
	}

	template<typename T>
	static T DuplicateObject(T other) {
		T obj = ObjectPropertyRegister::CreateNew<T>(other.Properties().GetName());

		for (auto [id, storage] : Registry::Get().storage()) {
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

	template<typename Tag,typename Attached>
	static void ForEachByTag(std::function<void(Attached)> func) {
		auto view = Registry::Get().view<Tag>();
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
	inline static std::unordered_map<entt::id_type, entt::id_type> m_RegisteredTagsByType;
	inline static std::unordered_map<entt::id_type, entt::id_type> m_RegisteredTypesByTag;
	inline static std::unordered_map<std::string, entt::id_type> m_RegisteredTagsByName;
	inline static std::unordered_map<entt::id_type, std::string> m_RegisteredComponentsNames;
	inline static std::unordered_map<entt::id_type, std::string> m_RegisteredObjectNames;
	
	

};


template<typename T>
class TemplatedObjectHandle {
public:
	TemplatedObjectHandle(entt::entity ent) {
		m_Handle = ent;
		isNull = false;
	}
	TemplatedObjectHandle() {
		isNull = true;
	}

	T GetAsObject() {
		return T(m_Handle);
	}

	operator bool() {
		if (isNull) {
			return false;
		}
		return Registry::Get().valid(m_Handle) && ObjectPropertyRegister::IsTypeOfObject<T>();
	};

private:
	entt::entity m_Handle = entt::null;
	bool isNull = false;
};

};