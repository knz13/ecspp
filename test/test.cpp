#include "ecspp.h"
#include "gtest/gtest.h"

class TestObject;

template<typename T>
class TestComponent : public ecspp::ComponentSpecifier<T,TestObject> {


};

class TestObjectProperties {
    int hello = 0;
};

class TestObject : public ecspp::TaggedObject<TestObject,TestComponent<ecspp::ComponentHelpers::Null>,TestObjectProperties> {



};



TEST(ObjectTests,ObjectCreationTest) {
    TestObject obj = TestObject::CreateNew("I'm an object!");


    TestObject::DeleteObject(obj);

}

