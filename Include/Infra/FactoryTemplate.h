#pragma once

#include "spdlog/spdlog.h"

#include <map>
#include <mutex>
#include <string>

namespace Tooling
{
/**
 * @brief 工厂产品注册基类
 *
 * @tparam CustomProductType_t 产品基类类型
 */
template <class CustomProductType_t, typename... TArgs>
class IProductClassRegistrar
{
public:
    /**
     * @brief Create a Product object
     *
     * @return std::unique_ptr<CustomProductType_t>
     */
    virtual std::unique_ptr<CustomProductType_t> createProduct(TArgs...) = 0;

    IProductClassRegistrar(const IProductClassRegistrar&) = delete;
    const IProductClassRegistrar& operator=(const IProductClassRegistrar&) = delete;

protected:
    IProductClassRegistrar() = default;
    virtual ~IProductClassRegistrar() = default;
};

/**
 * @brief 模板工厂类
 *
 * @tparam CustomProductType_t
 */
template <class CustomProductType_t>
class ProductClassFactory
{
public:
    /**
     * @brief 对应具体实例模板工厂单例获取
     *
     * @return ProductClassFactory<CustomProductType_t>&
     */
    static ProductClassFactory<CustomProductType_t>& instance()
    {
        static ProductClassFactory<CustomProductType_t> instance;
        return instance;
    }

    /**
     * @brief 通过产品 ID 注册到对应具体实例的模板工厂中
     *
     * @param registry
     * @param ID
     *
     * @return bool 是否插入成功
     */
    bool registerClassWithId(void* Registry, const std::string& ID)
    {
        std::lock_guard<std::mutex> lk(m_productMutex);
        return registProductor(m_mProductClassRegistry, Registry, ID);
    }
    bool registerInstanceWithId(void* Registry, const std::string& ID)
    {
        std::lock_guard<std::mutex> lk(m_instanceMutex);
        return registProductor(m_mInstanceProductRegistry, Registry, ID);
    }

    /**
     * @brief Get the Product Class object
     *
     * @param ID
     * @return std::unique_ptr<CustomProductType_t>
     */
    template <typename... TArgs>
    std::unique_ptr<CustomProductType_t> getProductClass(std::string ID, TArgs&&... Args)
    {
        std::lock_guard<std::mutex> lk(m_productMutex);
        if (m_mProductClassRegistry.find(ID) != m_mProductClassRegistry.end())
        {
            return static_cast<IProductClassRegistrar<CustomProductType_t, TArgs&&...>*>(m_mProductClassRegistry[ID])
                ->createProduct(Args...);
        }
        spdlog::warn("[{}] No product class found for ID[{}]", __FUNCTION__, ID.c_str());
        return std::unique_ptr<CustomProductType_t>(nullptr);
    }
    /**
     * @brief Get the Instace Product Class object
     *
     * @param ID
     * @return std::shared_ptr<CustomProductType_t>
     */
    template <typename... TArgs>
    std::shared_ptr<CustomProductType_t> getInstanceProductClass(std::string ID, TArgs&&... Args)
    {
        std::lock_guard<std::mutex> lk(m_instanceMutex);
        if (m_mInstanceProductRegistry.find(ID) != m_mInstanceProductRegistry.end())
        {
            auto ptr = static_cast<IProductClassRegistrar<CustomProductType_t, TArgs&&...>*>(m_mInstanceProductRegistry[ID])
                           ->createProduct(Args...);
            return std::shared_ptr<CustomProductType_t>(ptr.release(),
                [](CustomProductType_t*)
                {
                    // avoid delete instance ptr
                });
        }
        spdlog::warn("[{}] No product class found for ID[{}]", __FUNCTION__, ID.c_str());
        return std::shared_ptr<CustomProductType_t>(nullptr);
    }

    /**
     * @brief 删除一个已注册的产品注册生成器
     * TODO:[此接口不应对外，本已实现当产品注册生成器生命周期终止时会自动删除此注册器，不应由使用者管理是否删除，可以通过派生类实现此接口对外基类隐藏]
     */
    void removeProductClassByID(const std::string& ProduceId)
    {
        std::lock_guard<std::mutex> lk(m_productMutex);
        removeProductor(m_mProductClassRegistry, ProduceId);
    }
    void removeInstanceProductByID(const std::string& ProduceId)
    {
        std::lock_guard<std::mutex> lk(m_instanceMutex);
        removeProductor(m_mInstanceProductRegistry, ProduceId);
    }

    ProductClassFactory(const ProductClassFactory&) = delete;
    const ProductClassFactory& operator=(const ProductClassFactory&) = delete;

private:
    ProductClassFactory() = default;
    ~ProductClassFactory() = default;
    using ProductMap = std::map<std::string, void*>;
    bool registProductor(ProductMap& Map, void* Registry, const std::string& ID)
    {
        auto iter = Map.find(ID);
        if (iter != Map.end())
        {
            spdlog::warn("[{}] Error with Repeatedly insert the class with "
                         "ID[{}] into the factory, pls check it",
                __FUNCTION__, ID.c_str());

            return false;
        }
        Map[ID] = static_cast<void*>(Registry);
        return true;
    }

    void removeProductor(ProductMap& Map, const std::string& ProduceId)
    {
        auto iter = Map.find(ProduceId);
        if (iter != Map.end())
        {
            Map.erase(iter);
            return;
        }
        spdlog::warn("[{}] remove the produce ID[{}] register failed that not "
                     "found, pls check it",
            __FUNCTION__, ProduceId.c_str());
    }
    /**
     * @brief 保存注册过的产品，key:产品名字 , value:产品类型存
     *
     */
    ProductMap m_mProductClassRegistry;
    ProductMap m_mInstanceProductRegistry;
    /**
     * @brief 多线程同步锁，成本待考量
     *
     */
    std::mutex m_productMutex;
    std::mutex m_instanceMutex;
};

/**
 * @brief 产品注册模板类，用于创建具体产品和从工厂里注册产品
 *
 * @tparam CustomProductType_t 类是产品抽象类（基类）
 * @tparam CustomProductImpl_t 类是具体产品（产品种类的子类）
 */
template <class CustomProductType_t, class CustomProductImpl_t, typename... TArgs>
class ProductClassRegistrar : IProductClassRegistrar<CustomProductType_t, TArgs&&...>
{
public:
    /**
     * @brief Construct a new Product Class Registrar object
     *
     * @param ID
     */
    explicit ProductClassRegistrar(std::string ID)
            : m_customProductImplId(ID)
    {
        m_needDelete = ProductClassFactory<CustomProductType_t>::instance().registerClassWithId(static_cast<void*>(this), ID);
    }
    /**
     * @brief 删除析构掉的产品注册器 Destroy the Product Class Registrar object
     * TODO:[不应该在注册器中保存过多的信息，应该提供出基类模板供所有产品继承用于提供获取
     * produceID 信息]
     */
    ~ProductClassRegistrar()
    {
        if (m_needDelete)
            ProductClassFactory<CustomProductType_t>::instance().removeProductClassByID(m_customProductImplId);
    }
    /**
     * @brief Create a Product object
     *
     * @return std::unique_ptr<CustomProductType_t>
     */
    std::unique_ptr<CustomProductType_t> createProduct(TArgs&&... Args) override
    {
        return std::unique_ptr<CustomProductType_t>(std::make_unique<CustomProductImpl_t>(std::forward<TArgs>(Args)...));
    }

private:
    std::string m_customProductImplId;
    bool m_needDelete;
};

/**
 * @brief 单例产品注册模板类，用于创建具体产品单例和从工厂里注册产品
 * 需要具体产品实现 静态 instance 方法 //TODO: 侵入式，待优化
 * ex:
 *     class A: public Base
 *     {
 *          static A* instance(Arg...)
 *          {
 *              static A* o = new A(Arg...);
 *              return o;
 *          }
 *          ........
 *     }
 *
 * @tparam CustomProductType_t 类是产品抽象类（基类）
 * @tparam CustomProductImpl_t 类是具体产品（产品种类的子类）
 */
template <class CustomProductType_t, class CustomProductImpl_t, typename... TArgs>
class InstanceProductClassRegistrar : IProductClassRegistrar<CustomProductType_t, TArgs&&...>
{
public:
    /**
     * @brief Construct a new Product Class Registrar object
     *
     * @param ID
     */
    explicit InstanceProductClassRegistrar(std::string ID)
            : m_customProductImplId(ID)
    {
        m_needDelete = ProductClassFactory<CustomProductType_t>::instance().registerInstanceWithId(static_cast<void*>(this), ID);
    }

    /**
     * @brief 删除析构掉的产品注册器 Destroy the Product Class Registrar object
     * TODO:[不应该在注册器中保存过多的信息，应该提供出基类模板供所有产品继承用于提供获取
     * produceID 信息]
     */
    ~InstanceProductClassRegistrar()
    {
        if (m_needDelete)
            ProductClassFactory<CustomProductType_t>::instance().removeInstanceProductByID(m_customProductImplId);
    }

    /**
     * @brief Get Instance Product object
     *
     * @return std::unique_ptr<CustomProductType_t>
     */
    std::unique_ptr<CustomProductType_t> createProduct(TArgs&&... Args) override
    {
        return std::unique_ptr<CustomProductType_t>(CustomProductImpl_t::instance(std::forward<TArgs>(Args)...));
    }

private:
    std::string m_customProductImplId;
    bool m_needDelete;
};
}   // namespace Tooling