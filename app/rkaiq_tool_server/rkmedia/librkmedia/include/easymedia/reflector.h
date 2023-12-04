// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_REFLECTOR_H_
#define EASYMEDIA_REFLECTOR_H_

#include <map>
#include <memory>
#include <string>

#include "utils.h"

// all come the external interface
#define REFLECTOR(PRODUCT) PRODUCT##Reflector

// use internally
#define FACTORY(PRODUCT) PRODUCT##Factory

// macro of declare reflector
#define DECLARE_REFLECTOR(PRODUCT)                                                                                     \
    class PRODUCT;                                                                                                     \
    class PRODUCT##Factory;                                                                                            \
    class _API PRODUCT##Reflector                                                                                      \
    {                                                                                                                  \
      public:                                                                                                          \
        static const char* FindFirstMatchIdentifier(const char* rules);                                                \
        static bool IsMatch(const char* identifier, const char* rules);                                                \
        template <class T> static std::shared_ptr<T> Create(const char* request, const char* param = nullptr)          \
        {                                                                                                              \
            if (!IsDerived<T, PRODUCT>::Result) {                                                                      \
                LOG("The template class type is not derived of required type\n");                                      \
                return nullptr;                                                                                        \
            }                                                                                                          \
                                                                                                                       \
            const char* identifier = PRODUCT##Factory::Parse(request);                                                 \
            if (!identifier)                                                                                           \
                return nullptr;                                                                                        \
                                                                                                                       \
            auto it = factories.find(identifier);                                                                      \
            if (it != factories.end()) {                                                                               \
                const PRODUCT##Factory* f = it->second;                                                                \
                if (!T::Compatible(f)) {                                                                               \
                    LOG("%s is not compatible with the template\n", request);                                          \
                    return nullptr;                                                                                    \
                }                                                                                                      \
                return std::static_pointer_cast<T>(const_cast<PRODUCT##Factory*>(f)->NewProduct(param));               \
            }                                                                                                          \
            LOG("%s is not Integrated\n", request);                                                                    \
            return nullptr;                                                                                            \
        }                                                                                                              \
        static void RegisterFactory(std::string identifier, const PRODUCT##Factory* factory);                          \
        static void DumpFactories();                                                                                   \
                                                                                                                       \
      private:                                                                                                         \
        PRODUCT##Reflector() = default;                                                                                \
        ~PRODUCT##Reflector() = default;                                                                               \
        PRODUCT##Reflector(const PRODUCT##Reflector&) = delete;                                                        \
        PRODUCT##Reflector& operator=(const PRODUCT##Reflector&) = delete;                                             \
                                                                                                                       \
        static std::map<std::string, const PRODUCT##Factory*> factories;                                               \
    };

// macro of define reflector
#define DEFINE_REFLECTOR(PRODUCT)                                                                                      \
    std::map<std::string, const PRODUCT##Factory*> PRODUCT##Reflector::factories;                                      \
    const char* PRODUCT##Reflector::FindFirstMatchIdentifier(const char* rules)                                        \
    {                                                                                                                  \
        for (auto& it : factories) {                                                                                   \
            const PRODUCT##Factory* f = it.second;                                                                     \
            if (f->AcceptRules(rules))                                                                                 \
                return it.first.c_str();                                                                               \
        }                                                                                                              \
        return nullptr;                                                                                                \
    }                                                                                                                  \
    bool PRODUCT##Reflector::IsMatch(const char* identifier, const char* rules)                                        \
    {                                                                                                                  \
        auto it = factories.find(identifier);                                                                          \
        if (it == factories.end()) {                                                                                   \
            LOG("%s is not Integrated\n", identifier);                                                                 \
            return false;                                                                                              \
        }                                                                                                              \
        return it->second->AcceptRules(rules);                                                                         \
    }                                                                                                                  \
    void PRODUCT##Reflector::RegisterFactory(std::string identifier, const PRODUCT##Factory* factory)                  \
    {                                                                                                                  \
        auto it = factories.find(identifier);                                                                          \
        if (it == factories.end())                                                                                     \
            factories[identifier] = factory;                                                                           \
        else                                                                                                           \
            printf("repeated identifier : %s\n", identifier.c_str());                                                  \
    }                                                                                                                  \
    void PRODUCT##Reflector::DumpFactories()                                                                           \
    {                                                                                                                  \
        printf("\n%s:\n", #PRODUCT);                                                                                   \
        for (auto& it : factories) {                                                                                   \
            printf(" %s", it.first.c_str());                                                                           \
        }                                                                                                              \
        printf("\n\n");                                                                                                \
    }

// macro of declare base factory
#define DECLARE_FACTORY(PRODUCT)                                                                                       \
    class PRODUCT;                                                                                                     \
    class PRODUCT##Factory                                                                                             \
    {                                                                                                                  \
      public:                                                                                                          \
        virtual const char* Identifier() const = 0;                                                                    \
        static _API const char* Parse(const char* request);                                                            \
        virtual std::shared_ptr<PRODUCT> NewProduct(const char* param) = 0;                                            \
        bool AcceptRules(const char* rules) const                                                                      \
        {                                                                                                              \
            std::map<std::string, std::string> map;                                                                    \
            if (!parse_media_param_map(rules, map))                                                                    \
                return false;                                                                                          \
            return AcceptRules(map);                                                                                   \
        }                                                                                                              \
        virtual bool AcceptRules(const std::map<std::string, std::string>& map) const = 0;                             \
                                                                                                                       \
      protected:                                                                                                       \
        PRODUCT##Factory() = default;                                                                                  \
        virtual ~PRODUCT##Factory() = default;                                                                         \
                                                                                                                       \
      private:                                                                                                         \
        PRODUCT##Factory(const PRODUCT##Factory&) = delete;                                                            \
        PRODUCT##Factory& operator=(const PRODUCT##Factory&) = delete;                                                 \
    };

#define DEFINE_FACTORY_COMMON_PARSE(PRODUCT)                                                                           \
    const char* PRODUCT##Factory::Parse(const char* request)                                                           \
    {                                                                                                                  \
        return request;                                                                                                \
    }

#define FACTORY_IDENTIFIER_DEFINITION(IDENTIFIER)                                                                      \
    const char* Identifier() const override                                                                            \
    {                                                                                                                  \
        return IDENTIFIER;                                                                                             \
    }

#define FACTORY_INSTANCE_DEFINITION(FACTORY)                                                                           \
    static const FACTORY& Instance()                                                                                   \
    {                                                                                                                  \
        static const FACTORY object;                                                                                   \
        return object;                                                                                                 \
    }

#define FACTORY_REGISTER(FACTORY, REFLECTOR, FINAL_EXPOSE_PRODUCT)                                                     \
    class Register_##FACTORY                                                                                           \
    {                                                                                                                  \
      public:                                                                                                          \
        Register_##FACTORY()                                                                                           \
        {                                                                                                              \
            const FACTORY& obj = FACTORY::Instance();                                                                  \
            REFLECTOR::RegisterFactory(obj.Identifier(), &obj);                                                        \
            FINAL_EXPOSE_PRODUCT::RegisterFactory(&obj);                                                               \
        }                                                                                                              \
    };                                                                                                                 \
    Register_##FACTORY reg_##FACTORY;

#define DEFINE_ERR_GETSET()                                                                                            \
  protected:                                                                                                           \
    class ErrGetSet                                                                                                    \
    {                                                                                                                  \
      public:                                                                                                          \
        ErrGetSet() : err_val(0)                                                                                       \
        {                                                                                                              \
        }                                                                                                              \
        void Set(int val)                                                                                              \
        {                                                                                                              \
            err_val = val;                                                                                             \
        }                                                                                                              \
        int Get()                                                                                                      \
        {                                                                                                              \
            return err_val;                                                                                            \
        }                                                                                                              \
                                                                                                                       \
      private:                                                                                                         \
        int err_val;                                                                                                   \
    };                                                                                                                 \
    ErrGetSet IErr;                                                                                                    \
                                                                                                                       \
  public:                                                                                                              \
    void SetError(int val)                                                                                             \
    {                                                                                                                  \
        IErr.Set(val);                                                                                                 \
    }                                                                                                                  \
    int GetError()                                                                                                     \
    {                                                                                                                  \
        return IErr.Get();                                                                                             \
    }

#define DECLARE_PART_FINAL_EXPOSE_PRODUCT(PRODUCT)                                                                     \
  public:                                                                                                              \
    static bool Compatible(const PRODUCT##Factory* factory);                                                           \
    static void RegisterFactory(const PRODUCT##Factory* factory);                                                      \
                                                                                                                       \
  private:                                                                                                             \
    static std::list<const PRODUCT##Factory*> compatiable_factories;

#define DEFINE_PART_FINAL_EXPOSE_PRODUCT(FINAL_EXPOSE_PRODUCT, PRODUCT)                                                \
    std::list<const PRODUCT##Factory*> FINAL_EXPOSE_PRODUCT::compatiable_factories;                                    \
    bool FINAL_EXPOSE_PRODUCT::Compatible(const PRODUCT##Factory* factory)                                             \
    {                                                                                                                  \
        auto it = std::find(compatiable_factories.begin(), compatiable_factories.end(), factory);                      \
        if (it != compatiable_factories.end())                                                                         \
            return true;                                                                                               \
        return false;                                                                                                  \
    }                                                                                                                  \
    void FINAL_EXPOSE_PRODUCT::RegisterFactory(const PRODUCT##Factory* factory)                                        \
    {                                                                                                                  \
        auto it = std::find(compatiable_factories.begin(), compatiable_factories.end(), factory);                      \
        if (it == compatiable_factories.end())                                                                         \
            compatiable_factories.push_back(factory);                                                                  \
    }

// macro of define child factory
// EXTRA_CODE: extra class declaration
#define DEFINE_CHILD_FACTORY(REAL_PRODUCT, IDENTIFIER, FINAL_EXPOSE_PRODUCT, PRODUCT, EXTRA_CODE)                      \
    class REAL_PRODUCT##Factory : public PRODUCT##Factory                                                              \
    {                                                                                                                  \
      public:                                                                                                          \
        FACTORY_IDENTIFIER_DEFINITION(IDENTIFIER)                                                                      \
        std::shared_ptr<PRODUCT> NewProduct(const char* param) override;                                               \
        FACTORY_INSTANCE_DEFINITION(REAL_PRODUCT##Factory)                                                             \
                                                                                                                       \
      private:                                                                                                         \
        REAL_PRODUCT##Factory() = default;                                                                             \
        ~REAL_PRODUCT##Factory() = default;                                                                            \
        EXTRA_CODE                                                                                                     \
    };                                                                                                                 \
                                                                                                                       \
    FACTORY_REGISTER(REAL_PRODUCT##Factory, PRODUCT##Reflector, FINAL_EXPOSE_PRODUCT)

#endif // EASYMEDIA_REFLECTOR_H_
