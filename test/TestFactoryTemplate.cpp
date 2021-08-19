#include "Infra/FactoryTemplate.h"
#include "ThreadCompetition.hpp"
#include "catch2/catch.hpp"
#include "spdlog/spdlog.h"

#include <memory>
#include <string>
#include <thread>

class BaseClass
{
public:
    BaseClass() = default;
    virtual ~BaseClass() = default;

    /**
     * @brief Get the Class Product ID object
     *
     * @return std::string
     */
    virtual std::string getClassProductId() = 0;
};

class DerivedClass : public BaseClass
{
public:
    DerivedClass() = default;
    ~DerivedClass() override = default;

    std::string getClassProductId() override { return productId(); };
    static std::string productId() { return "DerivedClassProduct"; }
};

class DerivedAnotherClass : public BaseClass
{
public:
    DerivedAnotherClass() = default;
    ~DerivedAnotherClass() override = default;

    std::string getClassProductId() override { return productId(); };
    static std::string productId() { return "DerivedAnotherClassProduct"; }
};

using g_BaseClassFactory = Tooling::ProductClassFactory<class BaseClass>;

TEST_CASE("Test an empty factory template function", "[factory template test]")
{
    GIVEN("Factory without products")
    {
        WHEN("No product registration, but try to obtain the product")
        {
            auto oneProduct4DerivedClass = g_BaseClassFactory::instance().getProductClass(DerivedClass::productId());
            THEN("Check the nullptr returned") { REQUIRE(oneProduct4DerivedClass == nullptr); }
        }
        WHEN("regist DerivedClass Product to Factory")
        {
            Tooling::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId());
            THEN("Try get a DerivedClass Product and check return value")
            {
                auto oneProduct4DerivedClass = g_BaseClassFactory::instance().getProductClass(DerivedClass::productId());
                REQUIRE(oneProduct4DerivedClass != nullptr);
                REQUIRE(oneProduct4DerivedClass->getClassProductId() == DerivedClass::productId());
            }
        }
        WHEN("after local register DerivedClass leave the lifecycle try get the Product again")
        {
            auto oneProduct4DerivedClass = g_BaseClassFactory::instance().getProductClass(DerivedClass::productId());
            THEN("Check the return value") { REQUIRE(oneProduct4DerivedClass == nullptr); }
        }
    }
}

TEST_CASE("Test the factory template function of a product", "[factory template test]")
{
    GIVEN("Factory with a product")
    {
        Tooling::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId());
        WHEN("Register another product to the factory")
        {
            Tooling::ProductClassRegistrar<class BaseClass, class DerivedAnotherClass> registanotherProduct(
                DerivedAnotherClass::productId());
            THEN("Gets the product with new Product ID and verifies return values")
            {
                auto oneProduct4AnotherDerivedClass =
                    g_BaseClassFactory::instance().getProductClass(DerivedAnotherClass::productId());
                REQUIRE(oneProduct4AnotherDerivedClass != nullptr);
                REQUIRE(oneProduct4AnotherDerivedClass->getClassProductId() == DerivedAnotherClass::productId());
            }
            THEN("try to Gets old Product")
            {
                auto oneProduct4DerivedClass = g_BaseClassFactory::instance().getProductClass(DerivedClass::productId());
                REQUIRE(oneProduct4DerivedClass != nullptr);
                REQUIRE(oneProduct4DerivedClass->getClassProductId() == DerivedClass::productId());
            }
        }
        WHEN("Register a product with a duplicate name to the factory")
        {
            Tooling::ProductClassRegistrar<class BaseClass, class DerivedAnotherClass> registanotherProduct(
                DerivedClass::productId());
            THEN("Gets the product with the duplicate name and verifies that it was not inserted successfully")
            {
                auto oneProduct4DerivedClass = g_BaseClassFactory::instance().getProductClass(DerivedClass::productId());
                REQUIRE(oneProduct4DerivedClass != nullptr);
                REQUIRE(oneProduct4DerivedClass->getClassProductId() == DerivedClass::productId());
            }
        }
    }
}



TEST_CASE("Test the factory template function mulithread env get product", "[factory template test]")
{
    auto getProductF = []
    {
        auto ret = g_BaseClassFactory::instance().getProductClass(DerivedClass::productId());
        REQUIRE(ret != nullptr);
    };
    auto registProductF = []
    { Tooling::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId()); };

    GIVEN("Factory with a product")
    {
        Tooling::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId());
        WHEN("Registration and acquisition occur at the same time ")
        {
            const auto threadsnum = 5;

            ThreadCompetition workers(threadsnum);

            for (int i = 0; i < threadsnum; i++)
            {
                if (i == 1)
                    workers.enqueue(registProductF, i);
                else
                    workers.enqueue(getProductF, i);
            }
            workers.notifyAllThreads();
        }
    }
}