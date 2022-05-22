#pragma once

#include <iostream>
#include <unordered_map>
#include <concepts>
#include "../components/component.h"
#include "registry.h"
#include "../helpers/helpers.h"
#include "../../vendor/entt/single_include/entt/entt.hpp"
#include "object_properties.h"
#include "object_base.h"




template<typename T>
struct NamedComponentHandle {
public:

	NamedComponentHandle(T* comp)
	{
		m_Component = comp;
	};

	
	T* GetComponentPointer() {
		return m_Component;
	}

	operator bool() {
		return m_Component != nullptr;
	};

private:
	T* m_Component = nullptr;


};






class Object;
class ObjectPropertyRegister {
public:

	template<typename Component,typename... Args>
	static void MakeComponentPresentIn() {
		(ObjectPropertyRegister::MakeComponentPresentBackground<Component, Args>(), ...);
	}

	template<typename T>
	static void MakeComponentOmnipresent() {
		m_ComponentsToMakeOmnipresent.push_back(HelperFunctions::GetClassName<T>());
	};

	template<typename Storage,typename MainClass>
	static void RegisterClassAsPropertyStorage() {
		m_PropertyStorageContainer[HelperFunctions::HashClassName<MainClass>()] = [](entt::entity e) {
			Registry::Get().emplace<Storage>(e);
		};
	};

	

	template<typename Tag,typename Attached>
	static void RegisterClassAsObjectTag() {
		entt::id_type hash = HelperFunctions::HashClassName<Attached>();
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::ForEachByTag<Tag,Attached>>(entt::hashed_string("ForEach"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CreateObjectAndReturnHandle<Attached>>(entt::hashed_string("Create"));
		entt::meta<Attached>().type(hash).template func<&HelperFunctions::GetClassDisplayName<Attached>>(entt::hashed_string("Get Display Name"));
		entt::meta<Attached>().type(hash).template func<&ObjectBase::CallShowPropertiesForObject<Attached>>(entt::hashed_string("Show Properties"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CallSerializeForClass<Attached>>(entt::hashed_string("Serialize"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CallDeserializeForClass<Attached>>(entt::hashed_string("Deserialize"));
		entt::meta<Attached>().type(hash).template func<&ObjectPropertyRegister::CallDestroyForObject<Attached>>(entt::hashed_string("Destroy"));
		m_RegisteredObjectTagsStartingFuncs[hash] = [](entt::entity e) {
			Registry::Get().emplace<Tag>(e);
		};
		m_RegisteredTagsByType[hash] = entt::type_hash<Tag>().value();
		m_RegisteredObjectNames[hash] = HelperFunctions::GetClassName<Attached>();
		
		
	}

	static ObjectHandle CreateObjectFromType(std::string type,std::string objectName);

	template<typename T,typename... Args>
	static void InitializeObject(entt::entity ent,Args&&... args) {

		entt::id_type hash = HelperFunctions::HashClassName<T>();
		
		
		
		if (m_RegisteredObjectTagsStartingFuncs.find(hash) != m_RegisteredObjectTagsStartingFuncs.end()) {
			m_RegisteredObjectTagsStartingFuncs[hash](ent);
		}

		if (m_PropertyStorageContainer.find(hash) != m_PropertyStorageContainer.end()) {
			m_PropertyStorageContainer[hash](ent);
		}

		T obj(ent,args...);

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

	
	static void ClearEntities();

	template<typename T,typename... Args>
	static T CreateNew(std::string name,Args&&... args) {
		static_assert(std::is_base_of<Object, T>::value);

		entt::entity ent = Registry::Get().create();

		int index = 1;
		if (Registry::FindObjectByName(name)) {
			if (name.find_last_of(")") == std::string::npos || (name.find_last_of(")") != name.size() - 1)) {
				name += "(" + std::to_string(index) + ")";
			}
		}

		while (Registry::FindObjectByName(name)) {
			index++;
			name.replace(name.find_last_of("(") + 1, std::to_string(index - 1).size(), std::to_string(index));
		}

		Registry::Get().emplace<ObjectProperties>(ent, name, HelperFunctions::HashClassName<T>(), ent);

		ObjectPropertyRegister::InitializeObject<T,Args...>(ent,args...);

		return T(ent);
	}


	

	template<typename ComponentType,typename Component>
	static void RegisterClassAsComponentOfType() {
		m_RegisteredComponentsByType[HelperFunctions::HashClassName<ComponentType>()].push_back(HelperFunctions::GetClassName<Component>());
		RegisterClassAsComponent<Component>();
		m_RegisteredComponentsNames[entt::type_hash<Component>().value()] = HelperFunctions::GetClassName<Component>();
	};
	
	static std::vector<std::string> GetObjectComponents(entt::entity e);

	static std::string GetClassNameByID(entt::id_type id);
	
	template<typename T>
	static entt::entity CreateObjectAndReturnHandle(std::string name) {
		T obj = CreateNew<T>(name);

		return obj.ID();
	};

	static void Each(std::function<void(Object)> func);

	static bool DeleteObject(ObjectHandle obj);

	static void ClearDeletingQueue();

	static bool IsClassRegistered(std::string className);

	template<typename T>
	static bool IsTypeOfObject() {
		return m_RegisteredTagsByType.find(HelperFunctions::HashClassName<T>()) != m_RegisteredTagsByType.end();
	}

	static bool SerializeScene(std::string savePath);

	static bool DeserializeScene(std::string path);

	
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

	template<typename T,typename... Args>
	static bool AddComponent(entt::entity e,Args&&... args) {
		bool value = false;
		try {
			value = HasComponent<T>(e);
		}
		catch (std::runtime_error& err) {
			throw err;
		}


		if (!value) {
			Component* comp = (Component*) & Registry::Get().emplace<T>(e, std::forward<Args>(args)...);
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
	
	template<typename T,typename... Args>
	static NamedComponentHandle<T> GetComponentWithArgs(entt::entity e,Args&&... args) {
		bool couldAdd = false;

		try {
			AddComponent<T,Args...>(e,std::forward<Args>(args)...);
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
	static ReturnType* AddComponentByName(entt::entity e,std::string stringToHash) {
		if (!Registry::Get().valid(e)) {
			return nullptr;
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
		return returnData;
		
	};

	template<typename ReturnType>
	static ReturnType* GetComponentByName(entt::entity e,std::string name) {
		ReturnType* comp = nullptr;
		
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
	static bool CopyComponent(entt::entity first,entt::entity second) {
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


	template<typename,typename,typename>
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
	
	static void ValidateAllGameObjects();
	

	static ObjectHandle DeserializeObject(std::string objectType,YAML::Node& node);
	static bool SerializeObject(ObjectHandle obj, YAML::Node& node);

	template<typename T>
	static bool CallDeserializeForComponent(entt::entity e,YAML::Node* node) {
		if (!ObjectHandle(e)) {
			return false;
		}
		if (!node) {
			return false;
		}

		T* comp = ObjectPropertyRegister::GetComponentByName<T>(e, HelperFunctions::GetClassName<T>());

		if (!comp) {
			return false;
		}

		return ((Component*)(comp))->Deserialize(*node);

	};

	template<typename T>
	static YAML::Node CallSerializeForComponent(entt::entity e) {
		if (!ObjectHandle(e)) {
			return {};
		}
		
		T* comp = ObjectPropertyRegister::GetComponentByName<T>(e, HelperFunctions::GetClassName<T>());

		if (!comp) {
			return {};
		}

		return ((Component*)(comp))->Serialize();


		

	};


	template<typename T>
	static bool CallDeserializeForClass(entt::entity e,YAML::Node* node) {
		T obj(e);

		if (!node) {
			return false;
		}

		return ((ObjectBase*)(&obj))->Deserialize(*node);
	}

	template<typename T>
	static YAML::Node CallSerializeForClass(entt::entity e) {
		T obj(e);

		return ((ObjectBase*)(&obj))->Serialize();
		

	}
	
	static void GetAllChildren(ObjectHandle current, std::vector<ObjectHandle>& vec);
	

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
		entt::meta<T>().type(hash).template func<&HelperFunctions::GetClassDisplayName<T>>(entt::hashed_string("Get Display Name"));
		entt::meta<T>().type(hash).template func<&ObjectPropertyRegister::CallSerializeForComponent<T>>(entt::hashed_string("Serialize"));
		entt::meta<T>().type(hash).template func<&ObjectPropertyRegister::CallDeserializeForComponent<T>>(entt::hashed_string("Deserialize"));
		

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