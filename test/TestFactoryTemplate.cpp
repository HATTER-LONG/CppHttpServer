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

using g_BaseClassFactory = ToolKit::ProductClassFactory<class BaseClass>;

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
            ToolKit::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId());
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
        ToolKit::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId());
        WHEN("Register another product to the factory")
        {
            ToolKit::ProductClassRegistrar<class BaseClass, class DerivedAnotherClass> registanotherProduct(
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
            ToolKit::ProductClassRegistrar<class BaseClass, class DerivedAnotherClass> registanotherProduct(
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

class BaseClassWithArgs
{
public:
    BaseClassWithArgs() = delete;
    BaseClassWithArgs(bool Arg) { m_hasArgs = Arg; }
    virtual ~BaseClassWithArgs() = default;

    /**
     * @brief Get the Class Product ID object
     *
     * @return std::string
     */
    virtual std::string getClassProductId() = 0;

    bool m_hasArgs { false };
};

class DeviedClassWithArgs : public BaseClassWithArgs
{
public:
    DeviedClassWithArgs(bool Arg)
            : BaseClassWithArgs(Arg)
    {
    }

    DeviedClassWithArgs(bool Arg, int Arg2)
            : BaseClassWithArgs(Arg)
    {
        m_ownArgs = Arg2;
    }
    ~DeviedClassWithArgs() override = default;

    std::string getClassProductId() override { return productId(); }
    static std::string productId() { return "DeviedClassWithArgsProduct"; }

    int m_ownArgs { -1 };
};
using g_BaseClassWithArgsFactory = ToolKit::ProductClassFactory<class BaseClassWithArgs>;

TEST_CASE("Test the factory template function of one argument", "[factory template test]")
{
    GIVEN("Factory with a product has one arg")
    {
        ToolKit::ProductClassRegistrar<class BaseClassWithArgs, class DeviedClassWithArgs, bool> resgistProduct(
            DeviedClassWithArgs::productId());

        WHEN("Get a product with one args")
        {
            bool result = true;
            auto oneProduct4DerivedClass =
                g_BaseClassWithArgsFactory::instance().getProductClass(DeviedClassWithArgs::productId(), result);
            THEN("Check target argument") { REQUIRE(oneProduct4DerivedClass->m_hasArgs == result); }
        }
    }
}

TEST_CASE("Test the factory template function of mult arguments", "[factory template test]")
{
    GIVEN("Factory with a product has mult arg")
    {
        ToolKit::ProductClassRegistrar<class BaseClassWithArgs, class DeviedClassWithArgs, bool, int> resgistProduct {
            DeviedClassWithArgs::productId()
        };
        WHEN("Get a product with mult args")
        {
            bool result = true;
            int resultInt = 10;
            auto oneProduct4DerivedClass =
                g_BaseClassWithArgsFactory::instance().getProductClass(DeviedClassWithArgs::productId(), result, resultInt);
            THEN("Check target argument")
            {
                REQUIRE(oneProduct4DerivedClass->m_hasArgs);
                auto* ptr = dynamic_cast<DeviedClassWithArgs*>(oneProduct4DerivedClass.get());
                REQUIRE(ptr->m_ownArgs == resultInt);
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
    { ToolKit::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId()); };

    GIVEN("Factory with a product")
    {
        ToolKit::ProductClassRegistrar<class BaseClass, class DerivedClass> registProduct(DerivedClass::productId());
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

class InstanceDeviedClassWithArgs : public BaseClassWithArgs
{
public:
    static InstanceDeviedClassWithArgs* instance(bool Arg, int Arg2)
    {
        static InstanceDeviedClassWithArgs* o = new InstanceDeviedClassWithArgs(Arg, Arg2);
        return o;
    }

private:
    InstanceDeviedClassWithArgs(bool Arg)
            : BaseClassWithArgs(Arg)
    {
    }

    InstanceDeviedClassWithArgs(bool Arg, int Arg2)
            : BaseClassWithArgs(Arg)
    {
        m_ownArgs = Arg2;
    }


    ~InstanceDeviedClassWithArgs() override = default;

public:
    std::string getClassProductId() override { return productId(); }
    static std::string productId() { return "InstanceDeviedClassWithArgsProduct"; }

    int m_ownArgs { -1 };
};

TEST_CASE("Test the factory template get instance product", "[factory template test]")
{
    GIVEN("Factory without products")
    {
        bool arg1 = true;
        int arg2 = 20;
        WHEN("Regist a product with instance factory template")
        {
            ToolKit::InstanceProductClassRegistrar<class BaseClassWithArgs, class InstanceDeviedClassWithArgs, bool, int>
                resgistProduct(InstanceDeviedClassWithArgs::productId());
            THEN("Try get a instance product")
            {
                auto oneProduct4DerivedClass =
                    ToolKit::ProductClassFactory<BaseClassWithArgs>::instance().getInstanceProductClass(
                        InstanceDeviedClassWithArgs::productId(), arg1, arg2);
                REQUIRE(oneProduct4DerivedClass.get() == InstanceDeviedClassWithArgs::instance(true, 2));
                REQUIRE(oneProduct4DerivedClass->m_hasArgs == arg1);
                auto* ptr = dynamic_cast<InstanceDeviedClassWithArgs*>(oneProduct4DerivedClass.get());
                REQUIRE(ptr->m_ownArgs == arg2);
            }
        }
        WHEN("Second Regist the same product")
        {
            ToolKit::InstanceProductClassRegistrar<class BaseClassWithArgs, class InstanceDeviedClassWithArgs, bool, int>
                resgistProduct(InstanceDeviedClassWithArgs::productId());
            THEN("Try get the product")
            {
                auto oneProduct4DerivedClass =
                    ToolKit::ProductClassFactory<BaseClassWithArgs>::instance().getInstanceProductClass(
                        InstanceDeviedClassWithArgs::productId(), arg1, arg2);
                REQUIRE(oneProduct4DerivedClass.get() == InstanceDeviedClassWithArgs::instance(true, 2));
            }
            THEN("Try get the product whit wrong method")
            {
                auto oneProduct4DerivedClass = ToolKit::ProductClassFactory<BaseClassWithArgs>::instance().getProductClass(
                    InstanceDeviedClassWithArgs::productId(), arg1, arg2);
                REQUIRE(oneProduct4DerivedClass.get() == nullptr);
            }
        }
    }
}