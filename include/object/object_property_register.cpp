#include "object_property_register.h"
#include "object.h"
#include "../global.h"

namespace ecspp {

ObjectHandle ObjectPropertyRegister::CreateObjectFromType(std::string type, std::string objectName)
{
	auto resolved = entt::resolve(entt::hashed_string(type.c_str()));

	if (resolved) {
		if (auto func = resolved.func(entt::hashed_string("Create")); func) {
			auto result = func.invoke({}, objectName);
			if (result) {
				return ObjectHandle(*((entt::entity*)result.data()));
			}
			DEBUG_LOG("Create function for object with name " + objectName + " was not successful!");
			return {};
		}
		DEBUG_LOG("Couldn't identify create function for object, make sure it is derived from TaggedObject");
		return {};
	}
	DEBUG_LOG("Couldn't identify object type specified, check for spelling errors...");
	return {};
	
}



std::vector<std::string> ObjectPropertyRegister::GetObjectComponents(entt::entity e)
{
	std::vector<std::string> vec;
	for (auto [id, storage] : Registry::Get().storage()) {
		if (id == entt::type_hash<ObjectProperties>().value()) {
			continue;
		}
		if (std::find(m_RegisteredComponentsByType[Object(e).GetTypeID()].begin(), m_RegisteredComponentsByType[Object(e).GetTypeID()].end(), ObjectPropertyRegister::GetComponentNameByID(id)) == m_RegisteredComponentsByType[Object(e).GetTypeID()].end()) {
			continue;
		}
		if (storage.contains(e)) {
			vec.push_back(GetComponentNameByID(id));
		}
	}
	return vec;

}

std::string ObjectPropertyRegister::GetClassNameByID(entt::id_type id)
{
	if (m_RegisteredObjectNames.find(id) != m_RegisteredObjectNames.end()) {
		return m_RegisteredObjectNames[id];
	}
	return "";
}

void ObjectPropertyRegister::Each(std::function<void(Object)> func)
{
	Registry::Get().each([&](entt::entity e) {
		if (ObjectHandle(e)) {
			func(Object(e));
		}
	});
}

bool ObjectPropertyRegister::DeleteObject(ObjectHandle obj)
{
	if (obj) {
		m_ObjectsToDelete.push_back(obj);
		return true;
	}
	DEBUG_LOG("Could not delete object with id " + obj.ToString() + " because it was not valid!");
	return false;
}

void ObjectPropertyRegister::ClearDeletingQueue()
{
	for (auto& objHandle : m_ObjectsToDelete) {
		if (!objHandle) {

			continue;
		}
		std::vector<ObjectHandle> objectAndAllChildren;
		
		GetAllChildren(objHandle, objectAndAllChildren);
		

		for (auto& objectHandle : objectAndAllChildren) {
			if (!objectHandle) {
				DEBUG_LOG("Could not delete object with id " + objectHandle.ToString() + " because it was not valid! During ObjectPropertyRegister::ClearDeletingQueue()");
				continue;
			}
			Object object(objectHandle.ID());
			auto it = object.Properties().m_ComponentClassNames.begin();
			while (it != object.Properties().m_ComponentClassNames.end()) {
				object.EraseComponentByName(*it);
				it = object.Properties().m_ComponentClassNames.begin();
			}

			if (!HelperFunctions::CallMetaFunction(objectHandle.GetAsObject().GetType(), "Destroy", objectHandle.ID())) {
				DEBUG_LOG("Could not call destroy for object with type: " + objectHandle.GetAsObject().GetType() + ", and name: " + objectHandle.GetAsObject().GetName());
			}
			Registry::Get().destroy(object.ID());
		}

	}
	if (m_ObjectsToDelete.size() > 0) {
		ValidateAllGameObjects();
	}
	m_ObjectsToDelete.clear();
}

bool ObjectPropertyRegister::IsClassRegistered(std::string className)
{
	return entt::resolve(entt::hashed_string(className.c_str())).operator bool();
}


void ObjectPropertyRegister::ValidateAllGameObjects()
{
	Registry::Get().each([](entt::entity e) {
		Object obj(e);

		for (auto& compName : ObjectPropertyRegister::GetObjectComponents(e)) {
			auto handle = ObjectPropertyRegister::GetComponentByName<Component>(e,compName);
			if(handle){
				Component& comp = handle.Get();
				comp.SetMaster(e);
			}
		}


		});
}


void ObjectPropertyRegister::GetAllChildren(ObjectHandle current, std::vector<ObjectHandle>& vec)
{
	current.GetAsObject().Properties().ClearParent();

	vec.push_back(current);

	for (auto& handle : current.GetAsObject().Properties().GetChildren()) {
		if (handle) {
			GetAllChildren(handle, vec);
		}
	}
}

};