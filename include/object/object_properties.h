#pragma once
#include "registry.h"
#include "object_handle.h"





namespace ecspp {

class Object;
class ObjectProperties {
public:

	ObjectProperties(std::string name,entt::id_type masterType,entt::entity e) : m_Name(name), m_Master(e), m_MasterType(masterType)
	{
	}

	std::string GetName() const {
		return m_Name;
	}

	void SetName(std::string name) {
		m_Name = name;
	}

	


	ObjectHandle GetParent() {
		return m_Parent;
	}

	void SetParent(ObjectProperties& e) {
		if (e.m_MasterType == this->m_MasterType) {
			this->m_Parent = ObjectHandle(e.m_Master.ID());
			e.AddChildren(*this);
		}
	}

	void RemoveChildren(ObjectProperties& e) {
		auto it = std::find(m_Children.begin(), m_Children.end(), e.m_Master);
		if (it != m_Children.end()) {
			m_Children.erase(it);
		}
	}
	void AddChildren(ObjectProperties& e) {
		if (std::find(m_Children.begin(), m_Children.end(), e.m_Master) == m_Children.end() && e.m_MasterType == m_MasterType) {
			m_Children.push_back(e.m_Master);
		}
	}
	const std::vector<ObjectHandle>& GetChildren() const {
		return m_Children;
	}


	const std::vector< std::string>& GetComponentsNames() const {
		return m_ComponentClassNames;
	}


private:
	void SetComponentsNames(std::vector<std::string> vec) {
		m_ComponentClassNames = vec;
	}

	


	std::vector<std::string> m_ComponentClassNames;

	entt::id_type m_MasterType;
	std::vector<ObjectHandle> m_Children;
	ObjectHandle m_Parent = ObjectHandle();
	std::string m_Name;
	ObjectHandle m_Master;

	friend class ObjectPropertyRegister;
	friend class Object;
	friend class Registry;
};

};