#include "../include/ecspp.h"
#include "catch2/catch_test_macros.hpp"


class TestObjectProperties {
public:
    int hello = 0;
};

class TestObject : public ecspp::RegisterObjectType<TestObject> ,
                   public ecspp::RegisterStorage<TestObject,TestObjectProperties> {
public:
    TestObject(entt::entity e) : RegisterObjectType(e),RegisterStorage(e) {

    };

    TestObjectProperties& GetStorage() {
        return Storage();
    }

    
};


template<typename T>
class TestComponent : public ecspp::ComponentSpecifier<T,TestObject> {


};

struct RandomComponent : public TestComponent<RandomComponent> {

    int valueOne = 1;
    int valueTwo = 2;

};


TEST_CASE("Object Testing","[require]") {

    ecspp::DeleteAllObjects();

    TestObject obj = TestObject::CreateNew("I'm an object!");

    REQUIRE(obj.Valid());

    REQUIRE(obj.GetComponentsNames().size() == 0);

    REQUIRE(obj.GetParent() == ecspp::ObjectHandle());

    REQUIRE(obj.GetType() == "TestObject");

    REQUIRE(obj.GetChildren().size() == 0);

    SECTION("Adding and Erasing components") {
        REQUIRE(obj.Empty());
        REQUIRE(RandomComponent::AliveCount() == 0);

        obj.AddComponent<RandomComponent>();

        REQUIRE(!obj.Empty());
        REQUIRE(RandomComponent::AliveCount() == 1);
        REQUIRE(obj.GetComponentsNames()[0] == "RandomComponent");

        obj.EraseComponent<RandomComponent>();

        REQUIRE(RandomComponent::AliveCount() == 0);
        REQUIRE(obj.Empty());

        REQUIRE(ecspp::DeleteObject(obj));

        ecspp::ObjectPropertyRegister::ClearDeletingQueue();

    }

    SECTION("Modifying components") {
        REQUIRE(obj.Empty());
        REQUIRE(RandomComponent::AliveCount() == 0);

        obj.AddComponent<RandomComponent>();

        REQUIRE(obj.GetComponent<RandomComponent>().GetTypeName() == "RandomComponent");
        REQUIRE(obj.GetComponent<RandomComponent>().valueOne == 1);
        REQUIRE(obj.GetComponent<RandomComponent>().valueTwo == 2);

        obj.EraseComponent<RandomComponent>();

        REQUIRE(ecspp::DeleteObject(obj));

        ecspp::ObjectPropertyRegister::ClearDeletingQueue();

    }

    SECTION("Adding and Removing components by name") {


        REQUIRE(RandomComponent::AliveCount() == 0);

        obj.AddComponentByName("RandomComponent");

        REQUIRE(RandomComponent::AliveCount() == 1);
        REQUIRE(obj.GetComponentByName("RandomComponent").operator bool());

        
        RandomComponent& component = obj.GetComponentByName("RandomComponent").GetAs<RandomComponent>();

        component.valueOne = 2;

        REQUIRE(obj.GetComponent<RandomComponent>().valueOne == 2);

        obj.EraseComponentByName("RandomComponent");

        REQUIRE(RandomComponent::AliveCount() == 0);
        REQUIRE(obj.Empty());

        REQUIRE(ecspp::DeleteObject(obj));

        ecspp::ClearDeletingQueue();

    }

    
    REQUIRE(TestObject::GetNumberOfObjects() == 0);

    

}

TEST_CASE("Parenting tests") {

    ecspp::DeleteAllObjects();

    TestObject objectOne = TestObject::CreateNew("Test One");
    TestObject objectTwo = TestObject::CreateNew("Test Two");
    TestObject objectThree = TestObject::CreateNew("Test Three");
    
    REQUIRE(!objectOne.GetParent());
    REQUIRE(!objectTwo.GetParent());
    REQUIRE(!objectThree.GetParent());

    objectOne.SetParent(objectTwo);

    REQUIRE(objectOne.GetParent().ID() == objectTwo.ID());
    REQUIRE(objectTwo.IsInChildren(objectOne));

    objectTwo.SetParent(objectThree);

    REQUIRE(objectTwo.GetParent().ID() == objectThree.ID());
    REQUIRE(objectThree.IsInChildren(objectOne));
    REQUIRE(objectThree.IsInChildren(objectTwo));

    REQUIRE(TestObject::GetNumberOfObjects() == 3);

    REQUIRE(ecspp::DeleteObject(objectOne));
    REQUIRE(ecspp::DeleteObject(objectTwo));
    REQUIRE(ecspp::DeleteObject(objectThree));

    ecspp::ClearDeletingQueue();

    REQUIRE(TestObject::GetNumberOfObjects() == 0);

}


TEST_CASE("Creating object by type name") {

    ecspp::DeleteAllObjects();

    ecspp::ObjectHandle obj = ecspp::CreateNewObject("TestObject", "Hi!");

    REQUIRE(obj.operator bool());

    REQUIRE(obj.GetAsObject().GetType() == "TestObject");

    REQUIRE(obj.GetAsObject().GetName() == "Hi!");

    REQUIRE(ecspp::DeleteObject(obj.GetAsObject()));

    ecspp::ClearDeletingQueue();

    ecspp::ObjectHandle secondObj = ecspp::CreateNewObject("InvalidType", "I'm not valid :/");

    REQUIRE(!secondObj);
    
}
    

TEST_CASE("Copying objects without knowing their type") {

    ecspp::DeleteAllObjects();

    ecspp::ObjectHandle firstHandle = ecspp::CreateNewObject("TestObject", "Hi!");

    REQUIRE(firstHandle.operator bool());
    
    REQUIRE(firstHandle.GetAsObject().AddComponentByName("RandomComponent").operator bool());

    REQUIRE(firstHandle.GetAsObject().GetComponentsNames().size() == 1);

    REQUIRE(firstHandle.GetAsObject().HasComponent<RandomComponent>());

    firstHandle.GetAsObject().GetComponent<RandomComponent>().valueOne = 3;

    ecspp::ObjectHandle secondHandle = ecspp::CopyObject(firstHandle);

    REQUIRE(secondHandle.operator bool());

    REQUIRE(secondHandle.GetAsObject().HasComponent<RandomComponent>());

    REQUIRE(secondHandle.GetAsObject().GetComponent<RandomComponent>().valueOne == 3);

}

TEST_CASE("Testing Storage") {
    ecspp::DeleteAllObjects();

    TestObject obj = TestObject::CreateNew("Hi!");
    
    REQUIRE(obj.GetStorage().hello == 0);

}

TEST_CASE("Many Objects") {

    ecspp::DeleteAllObjects();

    std::vector<TestObject> objects;

    for (int i = 0; i < 100; i++) {
        objects.push_back(TestObject::CreateNew("Object!"));
    }

    REQUIRE(TestObject::GetNumberOfObjects() == 100);

    for (auto& obj : objects) {
        obj.AddComponent<RandomComponent>();
        REQUIRE(obj.GetComponentsNames().size() == 1);
    }

    for (auto& obj : objects) {
        ecspp::DeleteObject(obj);
    }

    ecspp::ClearDeletingQueue();

    REQUIRE(TestObject::GetNumberOfObjects() == 0);


}

template<typename Derived>
struct TestTemplatedDerived : public ecspp::RegisterObjectType<Derived> {
public:
    TestTemplatedDerived(entt::entity e) : ecspp::RegisterObjectType<Derived>(e) {};

    bool TestMethod() { return true; };

    virtual bool TestVirtualMethod(std::string e) { return false; };
};

struct FinalDerived : public TestTemplatedDerived<FinalDerived> {
public:
    FinalDerived(entt::entity e) : TestTemplatedDerived<FinalDerived>(e) {};

    bool TestVirtualMethod(std::string e) override { return true; };
};


TEST_CASE("Getting templated derived object") {
    FinalDerived obj = FinalDerived::CreateNew("Hi!");

    REQUIRE(ecspp::ObjectHandle(obj).GetAs<TestTemplatedDerived>().TestMethod());

    ecspp::DeleteObject(obj);

    ecspp::ClearDeletingQueue();
}

TEST_CASE("Calling virtual function from base") {
    FinalDerived obj = FinalDerived::CreateNew("Hi!");

    REQUIRE(ecspp::ObjectHandle(obj).GetAs<ecspp::Object>().CallVirtualFunction<&FinalDerived::TestVirtualMethod>("e") == true);
    
    
};