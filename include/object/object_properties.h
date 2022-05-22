#pragma once
#include "registry.h"








class ObjectProperties {
public:

	ObjectProperties(std::string name,entt::id_type masterType,entt::entity e);

	std::string GetName() const;
	void SetName(std::string name);

	


	ObjectHandle GetParent();
	void SetParent(Object e);
	void ClearParent();
	bool IsInChildren(Object obj);
	void RemoveChildren(Object e);
	void AddChildren(Object e);
	const std::vector<ObjectHandle>& GetChildren() const;

	const std::vector< std::string>& GetComponentsNames() const;

private:
	void SetComponentsNames(std::vector<std::string> vec);

	


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

