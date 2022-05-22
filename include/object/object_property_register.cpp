#include "object_property_register.h"
#include "../kv.h"


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

void ObjectPropertyRegister::ClearEntities()
{
	Registry::Get().each([](entt::entity e) {
		Object obj(e);

		std::string name = obj.GetName();

		if (!obj.HasComponent<InternalUse>() && obj.IsOfType<GameObject>()) {
			Registry::Get().destroy (obj.ID());
		}

		});

	ValidateAllGameObjects();
}

std::vector<std::string> ObjectPropertyRegister::GetObjectComponents(entt::entity e)
{
	std::vector<std::string> vec;
	for (auto [id, storage] : Registry::Get().storage()) {
		if (id == entt::type_hash<ObjectProperties>().value()) {
			continue;
		}
		if (std::find(m_RegisteredComponentsByType[Object(e).GetTypeOfObject()].begin(), m_RegisteredComponentsByType[Object(e).GetTypeOfObject()].end(), ObjectPropertyRegister::GetComponentNameByID(id)) == m_RegisteredComponentsByType[Object(e).GetTypeOfObject()].end()) {
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

bool ObjectPropertyRegister::SerializeScene(std::string savePath)
{

	if (std::filesystem::is_directory(savePath)) {
		return false;
	}

	

	YAML::Node mainRoot;

	

	GameObject::ForEach([&](GameObject obj) {

		if (!obj.HasComponent<InternalUse>()) {
			YAML::Node node;
			
			HelperFunctions::SerializeVariable("id",obj.ID(),node);
			SerializeObject(ObjectHandle(obj.ID()), node);
			

			if (obj.Properties().m_Parent) {
				node["parent"] = obj.Properties().m_Parent.ID();
			}
			else {
				node["parent"] = -1;
			}


			mainRoot.push_back(node);
		
		
		}
		
	});


	YAML::Emitter em;
	em << mainRoot;

	std::string val = em.c_str();

	

	std::ofstream stream;

	stream.open(savePath);

	
	stream << val;
	

	stream.close();

	GuiLayer::ExplorerView::Reload();

	return true;
}

bool ObjectPropertyRegister::DeserializeScene(std::string path)
{
	if (!std::filesystem::exists(path)) {
		return false;
	}
	
	if (!(std::filesystem::path(path).extension().string() == ".scene")) {
		return false;
	}

	

	YAML::Node root = YAML::LoadFile(path);

	if (!root.IsDefined()) {
		return false;
	}

	static std::unordered_map<entt::entity, entt::entity> oldNewCorrelationalMap;

	oldNewCorrelationalMap.clear();

	for (auto object : root) {
		
		ObjectHandle handle = DeserializeObject("GameObject", object);

		if (!handle) {
			continue;
		}

		entt::entity oldValue;

		HelperFunctions::DeserializeVariable("id", oldValue, object);

		

		oldNewCorrelationalMap[oldValue] = handle.ID();

	}

	for (auto object : root) {

		if (!object["parent"]) {
			continue;
		}
		if (object["parent"].as<int>() == -1) {
			continue;
		}

		entt::entity oldValue;

		HelperFunctions::DeserializeVariable("id", oldValue, object);

		if (!ObjectHandle(oldNewCorrelationalMap[oldValue])) {
			continue;
		}

		entt::entity oldParent;

		HelperFunctions::DeserializeVariable("parent", oldParent, object);

		if (!ObjectHandle(oldNewCorrelationalMap[oldParent])) {
			continue;
		}

		Object(oldNewCorrelationalMap[oldValue]).Properties().SetParent(Object(oldNewCorrelationalMap[oldParent]));


	}

	return true;


}

void ObjectPropertyRegister::ValidateAllGameObjects()
{
	Registry::Get().each([](entt::entity e) {
		Object obj(e);

		for (auto& compName : ObjectPropertyRegister::GetObjectComponents(e)) {
			Component* comp = ObjectPropertyRegister::GetComponentByName<Component>(e,compName);
			comp->SetMaster(e);

		}


		});
}

ObjectHandle ObjectPropertyRegister::DeserializeObject(std::string objectType,YAML::Node& node)
{
	if(!node.IsDefined()){
		return {};
	}
	if (!IsClassRegistered(objectType)) {
		return {};
	}

	if (!node["name"]) {
		return {};
	}
	
	std::string name;
	name = node["name"].as<std::string>();

	ObjectHandle obj = ObjectPropertyRegister::CreateObjectFromType(objectType,name);

	if (!obj) {
		return {};
	}

	if (!obj.GetAsObject().DeserializeBaseObject(node)) {
		DEBUG_LOG("Could not deserialize base object with name " + name +  "!");
		return obj;
	}

	auto deserializationResult = HelperFunctions::CallMetaFunction(objectType,"Deserialize",obj.ID(),&node);

	if (!deserializationResult) {
		DEBUG_LOG("Could not deserialize object with name " + name + " and type " + objectType + "!");
		return obj;
	}

	if (!(node["Components"])) {
		DEBUG_LOG("Could not add components to object with name" + name + " and type " + objectType + "!" + "\nBecause components was not found in the YAML::Node!");
		return obj;
	}
	

	for (auto children : node["Components"]) {
		
		if (!children.IsDefined()) {
			continue;
		}

		if (!children["type"]) {
			continue;
		}

		std::string type;
		
		type = children["type"].as<std::string>();

		Component* comp = ObjectPropertyRegister::GetComponentByName<Component>(obj.ID(), type);

		if (!comp) {
			DEBUG_LOG("Could not create component with type " + type + "!");
			continue;
		}
		YAML::Node& componentNode = children;
		auto result = HelperFunctions::CallMetaFunction(type,"Deserialize",obj.ID(),&componentNode);
		
		if (!result || !(*((bool*)result.data()))) {
			DEBUG_LOG("Could not deserialize component of type " + type + "!");
		};

	}
	return obj;



	

}

bool ObjectPropertyRegister::SerializeObject(ObjectHandle obj, YAML::Node& node)
{
	if (!obj) {
		DEBUG_LOG("Trying to serialize invalid object with id: " + obj.ToString());
		return false;
	}

	YAML::Node& outerRef = node;

	

	if (!obj.GetAsObject().SerializeBaseObject(outerRef)) {
		DEBUG_LOG("Base object serialization for object " + obj.GetAsObject().GetName() + " failed unexpectedly.");
		return false;
	}

	auto serializeResult = HelperFunctions::CallMetaFunction(obj.GetAsObject().GetType(), "Serialize", obj.ID());

	if (!serializeResult) {
		DEBUG_LOG("Meta function for object " + obj.GetAsObject().GetName() + " of type " + obj.GetAsObject().GetType() + " did not succeed!");
		outerRef.reset();
		return false;
	}
	else{
		outerRef["type"][obj.GetAsObject().GetType()] = (*((YAML::Node*)serializeResult.data()));
	}


	

	for (auto& componentName : ObjectPropertyRegister::GetObjectComponents(obj.ID())) {
		
		if (IsClassRegistered(componentName)) {	
			
			auto result = HelperFunctions::CallMetaFunction(componentName, "Serialize", obj.ID());
			if (result) {
				if (!* ((bool*)result.data())) {
					DEBUG_LOG("Could not serialize component " + componentName + " for object " + obj.GetAsObject().GetName());
					
				}
				else {
					YAML::Node& componentNode = *((YAML::Node*)result.data());
					componentNode["type"] = componentName;
					outerRef["Components"].push_back(componentNode);
				}
			}
			else {
				DEBUG_LOG("Could not serialize component " + componentName + " for object " + obj.GetAsObject().GetName() + " because Serialize meta function was not successful!");
				
			}
			
			
		}
	}

	return true;

	





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
