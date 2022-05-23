#include "object_properties.h"
#include "object.h"
#include "../global.h"

namespace ecspp {
ObjectHandle::ObjectHandle(entt::entity ent)
{
	m_Handle = ent;
}

ObjectHandle::ObjectHandle(Object obj) : m_Handle(obj.ID())
{
}

ObjectHandle::ObjectHandle()
{
	isNull = true;
}

Object ObjectHandle::GetAsObject() const
{
	return Object(m_Handle);
}



entt::entity ObjectHandle::ID() const
{
	return m_Handle;
}

std::string ObjectHandle::ToString() const
{
	return std::to_string((uint32_t)m_Handle);
}

bool ObjectHandle::IsType(entt::id_type type) const
{
	return GetAsObject().IsOfType(type);
}

bool ObjectHandle::operator==(const ObjectHandle& other) const
{
	return this->m_Handle == other.m_Handle;
}




ObjectProperties::ObjectProperties(std::string name, entt::id_type masterType, entt::entity e) : m_Name(name), m_Master(e),m_MasterType(masterType)
{
}

std::string ObjectProperties::GetName() const
{
	return m_Name;
}

void ObjectProperties::SetName(std::string name)
{
	m_Name = name;
}





ObjectHandle ObjectProperties::GetParent()
{
	return m_Parent;
}

void ObjectProperties::SetParent(Object e)
{
	if (e.HasSameObjectTypeAs(m_Master.GetAsObject())) {
		this->m_Parent = ObjectHandle(e.ID());
		e.AddChildren(m_Master.GetAsObject());
	}
}

void ObjectProperties::ClearParent()
{
	if (m_Parent) {
		m_Parent.GetAsObject().RemoveChildren(m_Master.GetAsObject());
	}
	this->m_Parent = ObjectHandle();
}

bool ObjectProperties::IsInChildren(Object obj)
{
	ObjectHandle handle(obj.ID());
	if (m_Children.size() == 0) {
		return false;
	}
	auto it = std::find(m_Children.begin(), m_Children.end(), handle);
	if (it != m_Children.end()) {
		return true;
	}
	for (auto& id : m_Children) {
		if (id.GetAsObject().IsInChildren(obj)) {
			return true;
		}
	}
	return false;
}

void ObjectProperties::RemoveChildren(Object e)
{
	ObjectHandle handle(e.ID());
	auto it = std::find(m_Children.begin(), m_Children.end(), handle);
	if (e.Valid() && it != m_Children.end()) {
		m_Children.erase(it);
	}
}

void ObjectProperties::AddChildren(Object e)
{
	ObjectHandle handle(e.ID());
	if (e.Valid() && std::find(m_Children.begin(), m_Children.end(), handle) == m_Children.end() && e.HasSameObjectTypeAs(m_Master.GetAsObject())) {
		m_Children.push_back(e.ID());
	}
}

const std::vector<ObjectHandle>& ObjectProperties::GetChildren() const
{
	return m_Children;
}

const std::vector<std::string>& ObjectProperties::GetComponentsNames() const
{
	return m_ComponentClassNames;
}

void ObjectProperties::SetComponentsNames(std::vector<std::string> vec)
{
	m_ComponentClassNames = vec;
}


};

