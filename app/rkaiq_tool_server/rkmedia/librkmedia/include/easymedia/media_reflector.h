// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MEDIA_REFLECTOR_H_
#define EASYMEDIA_MEDIA_REFLECTOR_H_

#include <algorithm>

#include "key_string.h"
#include "reflector.h"

#define DEFINE_MEDIA_CHILD_FACTORY(REAL_PRODUCT, IDENTIFIER, FINAL_EXPOSE_PRODUCT, PRODUCT)                            \
    DEFINE_CHILD_FACTORY(REAL_PRODUCT, IDENTIFIER, FINAL_EXPOSE_PRODUCT, PRODUCT,                                      \
                                                                                                                       \
                         public                                                                                        \
                         :                                                                                             \
                                                                                                                       \
                         virtual bool AcceptRules(const std::map<std::string, std::string>& map) const override;       \
                                                                                                                       \
                         /* common type: "video", "audio", "image", "stream", etc.                                     \
                            Even "video && audio".                                                                     \
                            Empty "" means any type while nullptr means nothing.                                       \
                            self-defined type: "video:h264", etc.                                                      \
                            more ref to media_type.h */                                                                \
                         static const char* ExpectedInputDataType(); static const char* OutPutDataType();              \
                                                                                                                       \
    )

#define DEFINE_MEDIA_CHILD_FACTORY_EXTRA(REAL_PRODUCT)                                                                 \
    bool REAL_PRODUCT##Factory::AcceptRules(const std::map<std::string, std::string>& map) const                       \
    {                                                                                                                  \
        static std::list<std::string> expected_data_type_list;                                                         \
        static std::list<std::string> out_data_type_list;                                                              \
        static const char* static_keys[] = {KEY_INPUTDATATYPE, KEY_OUTPUTDATATYPE, NULL};                              \
        static const decltype(ExpectedInputDataType)* static_call[] = {&ExpectedInputDataType, &OutPutDataType, NULL}; \
        static std::list<std::string>* static_list[] = {&expected_data_type_list, &out_data_type_list, NULL};          \
        const char** keys = static_keys;                                                                               \
        const decltype(ExpectedInputDataType)** call = static_call;                                                    \
        std::list<std::string>** list = static_list;                                                                   \
        while (*keys) {                                                                                                \
            auto it = map.find(*keys);                                                                                 \
            if (it == map.end()) {                                                                                     \
                if ((*call)())                                                                                         \
                    return false;                                                                                      \
            } else {                                                                                                   \
                const std::string& value = it->second;                                                                 \
                if (!value.empty() && !has_intersection(value.c_str(), (*call)(), *list))                              \
                    return false;                                                                                      \
            }                                                                                                          \
            ++keys;                                                                                                    \
            ++call;                                                                                                    \
            ++list;                                                                                                    \
        }                                                                                                              \
        return true;                                                                                                   \
    }

#define DEFINE_MEDIA_NEW_PRODUCT_BY(REAL_PRODUCT, PRODUCT, COND)                                                       \
    std::shared_ptr<PRODUCT> FACTORY(REAL_PRODUCT)::NewProduct(const char* param)                                      \
    {                                                                                                                  \
        auto ret = std::make_shared<REAL_PRODUCT>(param);                                                              \
        if (ret && ret->COND)                                                                                          \
            return nullptr;                                                                                            \
        return ret;                                                                                                    \
    }

#endif // #ifndef EASYMEDIA_MEDIA_REFLECTOR_H_
