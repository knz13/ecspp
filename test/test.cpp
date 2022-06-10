#include "../include/ecspp.h"
#include "catch2/catch_test_macros.hpp"


class TestComponent : public ecspp::Component {
public:
    virtual int TestVirtualFunc() { return 1; };

private:
    int variable = 2;
    int variableTwo = 4;
};
class TestObjectProperties {
public:
    int hello = 0;
};

class TestObject : public ecspp::RegisterObjectType<TestObject> ,
                   public ecspp::RegisterStorage<TestObject,TestObjectProperties>,
                   public ecspp::RegisterComponent<TestObject,TestComponent>
{
public:
    TestObject(entt::entity e) : RegisterObjectType(e),RegisterStorage(e) {

    };

    TestObjectProperties& GetStorage() {
        return Storage();
    }

    
};

class OtherOtherClass {
public:
    void PublicMethod() {};
    void OtherPublicMethod() {};
};

class OtherClass {
private:
    bool trueVal = true;
    bool falseVal = false;
};



struct RandomComponent : public ecspp::DefineComponent<RandomComponent,TestComponent>,public OtherClass,public OtherOtherClass  {
public:
    int TestVirtualFunc() override { return 2; };

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

        REQUIRE(obj.EraseComponent<RandomComponent>());

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

        
        RandomComponent& component = *obj.GetComponentByName("RandomComponent").GetAs<RandomComponent>();

        component.valueOne = 2;

        REQUIRE(obj.GetComponent<RandomComponent>().valueOne == 2);

        obj.EraseComponentByName("RandomComponent");

        REQUIRE(RandomComponent::AliveCount() == 0);
        REQUIRE(obj.Empty());

        REQUIRE(ecspp::DeleteObject(obj));

        ecspp::ClearDeletingQueue();

    }

    
    

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

    virtual float TestVirtualMethod(std::string e) { return 2; };
    virtual void TestVoidVirtualMethod(int* ptr) {
        if (ptr) {
            *ptr = 2;
        }
    };
};

struct FinalDerived : public TestTemplatedDerived<FinalDerived> {
public:
    FinalDerived(entt::entity e) : TestTemplatedDerived<FinalDerived>(e) {};

    float TestVirtualMethod(std::string e) override { return 10; };
    void TestVoidVirtualMethod(int* ptr) {
        if (ptr) {
            *ptr = 3;
        }
    };
};


struct OtherTestComponent : public ecspp::DefineComponent<OtherTestComponent,TestComponent> {

};

TEST_CASE("Getting templated derived object") {
    FinalDerived obj = FinalDerived::CreateNew("Hi!");

    REQUIRE(ecspp::ObjectHandle(obj).GetAs<TestTemplatedDerived>().TestMethod());

    ecspp::DeleteObject(obj);

    ecspp::ClearDeletingQueue();
}

TEST_CASE("Calling virtual function from base") {
    FinalDerived obj = FinalDerived::CreateNew("Hi!");

    ecspp::Object middleObj(obj);

    REQUIRE(middleObj.CallVirtualFunction<&TestTemplatedDerived<ecspp::Object>::TestVirtualMethod>("e") == 10);

    int val = 1;
    middleObj.CallVirtualFunction<&FinalDerived::TestVoidVirtualMethod>(&val);

    REQUIRE(val == 3);
    
    
};

TEST_CASE("Getting components names and calling base function") {

    for (int i = 0; i < 100; i++) {
        FinalDerived obj = FinalDerived::CreateNew("Hi");

        obj.AddComponent<RandomComponent>();
        if (i % 2 == 0) {
            obj.AddComponent<OtherTestComponent>();
        }
    }
    FinalDerived::ForEach([](FinalDerived obj) {
        for (auto& compName : obj.GetComponentsNames()) {
            auto comp = obj.GetComponentByName(compName);
            REQUIRE(comp);
            if (comp) {
                if (compName == "OtherTestComponent") {
                    REQUIRE(comp.GetAs<TestComponent>()->TestVirtualFunc() == 1);
                }
                else {
                    REQUIRE(comp.GetAs<TestComponent>()->TestVirtualFunc() == 2);
                }
            }
        }
        });


}