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
template <class CustomProductType_t>
class IProductClassRegistrar
{
public:
    /**
     * @brief Create a Product object
     *
     * @return std::unique_ptr<CustomProductType_t>
     */
    virtual std::unique_ptr<CustomProductType_t> createProduct() = 0;

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
    bool registerClassWithId(IProductClassRegistrar<CustomProductType_t>* Registry, std::string ID)
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto iter = m_mapProductClassRegistry.find(ID);
        if (iter != m_mapProductClassRegistry.end())
        {
            spdlog::warn("[{}] Error with Repeatedly insert the class with "
                         "ID[{}] into the factory, pls check it",
                __FUNCTION__, ID.c_str());

            return false;
        }
        m_mapProductClassRegistry[ID] = Registry;
        return true;
    }

    /**
     * @brief Get the Product Class object
     *
     * @param ID
     * @return std::unique_ptr<CustomProductType_t>
     */
    std::unique_ptr<CustomProductType_t> getProductClass(std::string ID)
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_mapProductClassRegistry.find(ID) != m_mapProductClassRegistry.end())
        {
            return m_mapProductClassRegistry[ID]->createProduct();
        }
        spdlog::warn("[{}] No product class found for ID[{}]", __FUNCTION__, ID.c_str());
        return std::unique_ptr<CustomProductType_t>(nullptr);
    }


    /**
     * @brief 删除一个已注册的产品注册生成器
     * TODO:[此接口不应对外，本已实现当产品注册生成器生命周期终止时会自动删除此注册器，不应由使用者管理是否删除，可以通过派生类实现此接口对外基类隐藏]
     */
    void removeProductClassByID(std::string ProduceId)
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto iter = m_mapProductClassRegistry.find(ProduceId);
        if (iter != m_mapProductClassRegistry.end())
        {
            m_mapProductClassRegistry.erase(iter);
            return;
        }
        spdlog::warn("[{}] remove the produce ID[{}] register failed that not "
                     "found, pls check it",
            __FUNCTION__, ProduceId.c_str());
    }
    ProductClassFactory(const ProductClassFactory&) = delete;
    const ProductClassFactory& operator=(const ProductClassFactory&) = delete;

private:
    ProductClassFactory() = default;
    ~ProductClassFactory() = default;

    /**
     * @brief 保存注册过的产品，key:产品名字 , value:产品类型存
     *
     */
    std::map<std::string, IProductClassRegistrar<CustomProductType_t>*> m_mapProductClassRegistry;

    /**
     * @brief 多线程同步锁，成本待考量
     *
     */
    std::mutex m_mutex;
};

/**
 * @brief 产品注册模板类，用于创建具体产品和从工厂里注册产品 模板参数 CustomProductType_t
 * 表示的类是产品抽象类（基类），CustomProductImpl_t 表示的类是具体产品（产品种类的子类）
 *
 * @tparam CustomProductType_t
 * @tparam CustomProductImpl_t
 */
template <class CustomProductType_t, class CustomProductImpl_t>
class ProductClassRegistrar : IProductClassRegistrar<CustomProductType_t>
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
        m_needDelete = ProductClassFactory<CustomProductType_t>::instance().registerClassWithId(this, ID);
    }
    /**
     * @brief 删除析构掉的产品注册器 Destroy the Product Class Registrar object
     * TODO:[不应该在注册器中保存过多的信息，应该提供出基类模板供所有产品继承用于提供获取 produceID 信息]
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
    std::unique_ptr<CustomProductType_t> createProduct()
    {
        return std::unique_ptr<CustomProductType_t>(std::make_unique<CustomProductImpl_t>());
    }

private:
    std::string m_customProductImplId;
    bool m_needDelete;
};
}   // namespace Tooling