#include "ecspp.h"
#include "catch2/catch_test_macros.hpp"

using namespace ecspp;

class TestObject;

template<typename T>
class TestComponent : public ecspp::ComponentSpecifier<T,TestObject> {


};

class TestObjectProperties {
    int hello = 0;
};

class TestObject : public ecspp::TaggedObject<TestObject,TestComponent<ecspp::ComponentHelpers::Null>,TestObjectProperties> {
public:
    TestObject(entt::entity e) : TaggedObject(e) {

    };


};

struct RandomComponent : public TestComponent<RandomComponent> {

    int valueOne = 1;
    int valueTwo = 2;

};


TEST_CASE("Object Testing","[require]") {
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



    }

    SECTION("Modifying components") {
        REQUIRE(obj.Empty());
        REQUIRE(RandomComponent::AliveCount() == 0);

        obj.AddComponent<RandomComponent>();

        REQUIRE(obj.GetComponent<RandomComponent>().GetTypeName() == "RandomComponent");
        REQUIRE(obj.GetComponent<RandomComponent>().valueOne == 1);
        REQUIRE(obj.GetComponent<RandomComponent>().valueTwo == 2);

        obj.EraseComponent<RandomComponent>();

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

    }

    REQUIRE(TestObject::DeleteObject(obj));

    ecspp::ObjectPropertyRegister::ClearDeletingQueue();

    


}

