#pragma once

#include "../../vendor/entt/single_include/entt/entt.hpp"
#include <iostream>

// dummy class to make the functions available to the register;

namespace escpp {

class ObjectBase {

protected:
	virtual void Init() {};
	virtual void Destroy() {};

	friend class ObjectPropertyRegister;


};
};